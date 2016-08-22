// Fill out your copyright notice in the Description page of Project Settings.

#include "VRGesturePluginPrivatePCH.h"
#include "VRGestureRecognizer.h"
#include <algorithm>
#include "RandomNumbers.h"

RandomNumbers RN; 

//--------------------------------------------------------------
UVRGestureRecognizer::UVRGestureRecognizer(const FObjectInitializer& X)
	:Super(X)
{

	GestureManager = X.CreateDefaultSubobject<UVRGestureTemplateManager>(this, "GestureManager");

	CurrentGesture = X.CreateDefaultSubobject<UVRGestureTemplate>(this, "DefaultGesture");

	RecognizerConfig.Dimensions = 3;
	RecognizerConfig.bTranslate = true;
	RecognizerConfig.bSegmentation = false;

	// default numberParticles is 1000, note that the computational cost directly depends on the number of particles
	EngineParameters.numberParticles = 1000;
	// tolerance depends on the range of the data typially tolerance = (data range)/3.0;
	EngineParameters.tolerance = 10.f / 3.f;
	// re sampling threshold is the minimum number of active particles before re sampling all the particles by the estimated posterior distribution. in other words, it re-targets particles around the best current estimates
	EngineParameters.resamplingThreshold = 250;

	EngineParameters.distribution = 0.0f;
	EngineParameters.alignmentVariance = sqrt(0.000001f);

	/* dynamicsVariance
	Change variance of alignment adaptation
	if alignment variance is high the method will adapt faster to high variations Change variance of adaptation in dynamics
	if dynamics adaptation variance is high the method will adapt faster to fast changes in dynamics. Dynamics is 2-dimensional: the first dimension is the speed The second dimension is the acceleration.
	Typically the variance is the average amount the speed or acceleration can change from one sample to another. As an example, if the relative estimated speed can change from 1.1 to 1.2 from one sample to another, the variance should allow a change of 0.1 in speed. So the variance should be set to 0.1*0.1 = 0.01
	*/
	EngineParameters.dynamicsVariance = FVector(sqrt(0.01f));
	EngineParameters.scalingsVariance = FVector(sqrt(0.00001f));
	EngineParameters.rotationsVariance = FVector(sqrt(0.0f));
	// it is possible to leave UVRGestureRecognizer to perform few steps of prediction ahead which can be useful to estimate faster the variations. Default value is 1 which means no prediction ahead
	EngineParameters.predictionSteps = 1;
	EngineParameters.dimWeights = FVector(sqrt(1.0f));
	EngineParameters.alignmentSpreadingCenter = 0.0;
	EngineParameters.alignmentSpreadingRange = 0.2;
	EngineParameters.dynamicsSpreadingCenter = 1.0;
	EngineParameters.dynamicsSpreadingRange = 0.3;
	EngineParameters.scalingsSpreadingCenter = 1.0;
	EngineParameters.scalingsSpreadingRange = 0.3;
	EngineParameters.rotationsSpreadingCenter = 0.0;
	EngineParameters.rotationsSpreadingRange = 0.0;

	tolerancesetmanually = false;

	RN = RandomNumbers(); 
}

//--------------------------------------------------------------


void UVRGestureRecognizer::train() {

	if (GestureManager->GestureTemplates.Num() > 0)
	{
		dynamicsDim = RecognizerConfig.Dimensions;   // hard coded: just speed now
		scalingsDim = RecognizerConfig.Dimensions;

		// manage orientation
		if (RecognizerConfig.Dimensions == 2) rotationsDim = 1;
		else if (RecognizerConfig.Dimensions == 3) rotationsDim = 3;
		else rotationsDim = 0;

		GestureParticles.SetNumUninitialized(EngineParameters.numberParticles);

		initPrior();            // prior on init state values
		initNoiseParameters();  // init noise parameters (transition and likelihood)
	}
}

void UVRGestureRecognizer::StartRecordingNewGesture(int32 GestureID)
{
	// Necessary to manually go to idle before starting to record
	if (state != EVRGestureRecognizerState::Idle)
	{
		UE_LOG(VRGesturePluginLog, Warning, TEXT("[%s::StartRecordingNewGesture] Cannot start, current state is different from idle. State=%s"), *GetName(), *GetEnumValueToString("EVRGestureRecognizerState", state));
		return;
	}

	// Check if the new gesture ID is valid
	if (!GestureManager->IsValidGestureID(GestureID))
	{
		UE_LOG(VRGesturePluginLog, Warning, TEXT("[%s::StartRecordingNewGesture] Cannot start, ID already in use or not valid. GestureID=%d"), *GetName(), GestureID);
		return;
	}

	// Create new gesture template and set as current
	CurrentGesture = NewObject<UVRGestureTemplate>();
	CurrentGesture->GestureID = GestureID;


	state = EVRGestureRecognizerState::Recording;
}

void UVRGestureRecognizer::StopRecordingGesture()
{
	// Necessary to manually go to idle before starting to listen
	if (state != EVRGestureRecognizerState::Recording)
	{
		UE_LOG(VRGesturePluginLog, Warning, TEXT("[%s::StopRecordingGesture] Cannot stop, current state is different from recording. State=%s"), *GetName(), *GetEnumValueToString("EVRGestureRecognizerState", state));
		return;
	}

	if (!GestureManager->AddNewGesture(CurrentGesture))
	{
		UE_LOG(VRGesturePluginLog, Warning, TEXT("[%s::StopRecordingGesture] Failed to add new gesture to gesture manager."), *GetName());
	}
	else
	{
		minRange = FVector(INFINITY, INFINITY, INFINITY);
		maxRange = FVector(-INFINITY, -INFINITY, -INFINITY);

		// compute min/max from the data
		for (auto& Elem : GestureManager->GestureTemplates)
		{
			FVector& tMinRange = Elem.Value->getMinRange();
			FVector& tMaxRange = Elem.Value->getMaxRange();

			if (tMinRange.X < minRange.X) minRange.X = tMinRange.X;
			if (tMinRange.Y < minRange.Y) minRange.Y = tMinRange.Y;
			if (tMinRange.Z < minRange.Z) minRange.Z = tMinRange.Z;

			if (tMaxRange.X > maxRange.X) maxRange.X = tMaxRange.X;
			if (tMaxRange.Y > maxRange.Y) maxRange.Y = tMaxRange.Y;
			if (tMaxRange.Z > maxRange.Z) maxRange.Z = tMaxRange.Z;
		}

		for (auto& Elem : GestureManager->GestureTemplates)
		{
			Elem.Value->setMinRange(minRange);
			Elem.Value->setMaxRange(maxRange);
		}

		train();

		CurrentGesture = nullptr; 
	}

	state = EVRGestureRecognizerState::Idle;
}

void UVRGestureRecognizer::ClearAllGestures()
{
	GestureManager->clear(); 
}

void UVRGestureRecognizer::StartListening(TArray<int32> GestureIDs /*= TArray<int32>()*/)
{
	// Necessary to manually go to idle before starting to listen
	if (state != EVRGestureRecognizerState::Idle)
	{
		UE_LOG(VRGesturePluginLog, Warning, TEXT("[%s::StartListening] Cannot start, current state is different from idle. State=%s"), *GetName(), *GetEnumValueToString("EVRGestureRecognizerState", state));
		return;
	}

	// Check if we have gestures stored
	if (GestureManager->GestureTemplates.Num() <= 0)
	{
		UE_LOG(VRGesturePluginLog, Warning, TEXT("[%s::StartListening] Cannot start, no gesture stored."), *GetName());
		return;
	}

	// if no gestures specified, use all gestures stored
	if (GestureIDs.Num() == 0)
	{
		// Use all gesture stored
		GestureIDs = GestureManager->GetAllGestureIDs();
	}

	// Create dummy template to listen 
	if (CurrentGesture == NULL)
	{
		CurrentGesture = NewObject<UVRGestureTemplate>();
	}

	train();
	state = EVRGestureRecognizerState::Listening;
	CurrentGesture->Reset();
}

void UVRGestureRecognizer::StopListening()
{
	// Necessary to be in listening state to stop listening
	if (state != EVRGestureRecognizerState::Listening)
	{
		UE_LOG(VRGesturePluginLog, Warning, TEXT("[%s::StopListening] Cannot stop, current state is different from listening. State=%s"), *GetName(), *GetEnumValueToString("EVRGestureRecognizerState", state));
		return;
	}

	state = EVRGestureRecognizerState::Idle;
	CurrentGesture->Reset();
}

//--------------------------------------------------------------
void UVRGestureRecognizer::Tick(FVector& InputPoint)
{
	switch (state)
	{
	case EVRGestureRecognizerState::Listening:
		CurrentGesture->addObservation(InputPoint);
		// Update the estimation
		TickListening();
		break;

	case EVRGestureRecognizerState::Recording:
		CurrentGesture->addObservation(InputPoint);
		UE_LOG(VRGesturePluginLog, Log, TEXT("[%s::Tick] Recording - Added point: %s"), *GetName(), *InputPoint.ToString());
		break;

	case EVRGestureRecognizerState::Idle:
	default:
		break;
	}
}

void UVRGestureRecognizer::TickListening()
{
	FVector obs = CurrentGesture->getLastObservation();

	// for each particle: perform updates of state space / likelihood / prior (weights)
	float sumw = 0.0;
	for (int ParticleIndex = 0; ParticleIndex < EngineParameters.numberParticles; ParticleIndex++)
	{
		if (!GestureParticles.IsValidIndex(ParticleIndex))
		{
			// TODO ERROR
			continue;
		}
		FGestureParticle* Particle = &GestureParticles[ParticleIndex];


		for (int m = 0; m < EngineParameters.predictionSteps; m++)
		{
			updatePrior(Particle);
			updateLikelihood(obs, Particle, ParticleIndex);
			updatePosterior(Particle);
		}

		sumw += Particle->Posterior;   // sum posterior to normalise the distribution afterwards
	}

	// normalize the weights and compute the re sampling criterion
	float dotProdw = 0.0;
	for (int ParticleIndex = 0; ParticleIndex < EngineParameters.numberParticles; ParticleIndex++) {

		if (!GestureParticles.IsValidIndex(ParticleIndex))
		{
			// TODO ERROR
			continue;
		}
		FGestureParticle* Particle = &GestureParticles[ParticleIndex];

		Particle->Posterior /= sumw;
		dotProdw += Particle->Posterior * Particle->Posterior;
	}
	// avoid degeneracy (no particles active, i.e. weight = 0) by re sampling
	if ((1. / dotProdw) < EngineParameters.resamplingThreshold)
		resampleAccordingToWeights(obs);

	// estimate outcomes
	// results are in every gesture templates objects 
	estimates();

	// Output estimates
	//UE_LOG(VRGesturePluginLog, Log, TEXT("[%s::TickListening] Estimations"), *GetName());
	for (auto& Elem : GestureManager->GestureTemplates)
	{
		//UE_LOG(VRGesturePluginLog, Log, TEXT("[%s::TickListening] #%d Likelihood:%f Progression:%f Scale: %s Rotation %s Dynamics %s"), *GetName(), Elem.Value->GestureID, Elem.Value->estimatedProbabilities, Elem.Value->estimatedAlignment, *Elem.Value->estimatedScalings.ToString(), *Elem.Value->estimatedRotations.ToString(), *Elem.Value->estimatedDynamics.ToString());
	}
}








//--------------------------------------------------------------
void UVRGestureRecognizer::initPrior()
{
	for (int ParticleIndex = 0; ParticleIndex < EngineParameters.numberParticles; ParticleIndex++)
	{
		if (!GestureParticles.IsValidIndex(ParticleIndex))
		{
			// TODO ERROR
			continue;
		}
		FGestureParticle* Particle = &GestureParticles[ParticleIndex];

		Particle->Progression = (RN.GetRandomUniform() - 0.5) * EngineParameters.alignmentSpreadingRange + EngineParameters.alignmentSpreadingCenter;    // spread phase

																																					   // dynamics
		Particle->Dynamic.X = (RN.GetRandomUniform() - 0.5) * EngineParameters.dynamicsSpreadingRange + EngineParameters.dynamicsSpreadingCenter; // spread speed
		Particle->Dynamic.Y = (RN.GetRandomUniform() - 0.5) * EngineParameters.dynamicsSpreadingRange; // spread acceleration

																									 // scalings
		Particle->Scale.X = (RN.GetRandomUniform() - 0.5) * EngineParameters.scalingsSpreadingRange + EngineParameters.scalingsSpreadingCenter; // spread scalings
		Particle->Scale.Y = (RN.GetRandomUniform() - 0.5) * EngineParameters.scalingsSpreadingRange + EngineParameters.scalingsSpreadingCenter; // spread scalings
		Particle->Scale.Z = (RN.GetRandomUniform() - 0.5) * EngineParameters.scalingsSpreadingRange + EngineParameters.scalingsSpreadingCenter; // spread scalings

																																			  // rotations
		if (rotationsDim != 0)
		{
			Particle->Rotation.X = (RN.GetRandomUniform() - 0.5) * EngineParameters.rotationsSpreadingRange + EngineParameters.rotationsSpreadingCenter;    // spread rotations
			Particle->Rotation.Y = (RN.GetRandomUniform() - 0.5) * EngineParameters.rotationsSpreadingRange + EngineParameters.rotationsSpreadingCenter;    // spread rotations
			Particle->Rotation.Z = (RN.GetRandomUniform() - 0.5) * EngineParameters.rotationsSpreadingRange + EngineParameters.rotationsSpreadingCenter;    // spread rotations
		}

		if (RecognizerConfig.bTranslate)
		{
			Particle->Offset.X = 0.0;
			Particle->Offset.Y = 0.0;
			Particle->Offset.Z = 0.0;
		}

		Particle->Prior = 1.0 / (float)EngineParameters.numberParticles;

		// set the posterior to the prior at the initialization
		Particle->Posterior = Particle->Prior;

		// auto select a gesture id based on the one available 
		Particle->GestureID = GestureManager->GetGestureIDFromParticleIndex(ParticleIndex);
	}

}
//--------------------------------------------------------------
void UVRGestureRecognizer::initNoiseParameters() {

	// ADAPTATION OF THE TOLERANCE IF DEFAULT PARAMETERS
	// ---------------------------
	if (!tolerancesetmanually) {
		float obsMeanRange = 0.0f;

		for (auto& Elem : GestureManager->GestureTemplates)
		{
			for (int d = 0; d < RecognizerConfig.Dimensions; d++)
			{
				obsMeanRange += (Elem.Value->getMaxRange()[d] - Elem.Value->getMinRange()[d])
					/ RecognizerConfig.Dimensions;
			}
		}
		for (auto& Elem : GestureManager->GestureTemplates){
			for (int d = 0; d<RecognizerConfig.Dimensions; d++)
				obsMeanRange += (Elem.Value->getMaxRange()[d] - Elem.Value->getMinRange()[d])
				/ RecognizerConfig.Dimensions;
		}
		obsMeanRange /= GestureManager->GestureTemplates.Num();
		EngineParameters.tolerance = obsMeanRange / 4.0f;  // dividing by an heuristic factor [to be learned?]
	}
}

//--------------------------------------------------------------
void UVRGestureRecognizer::updatePrior(FGestureParticle* Particle) {

	if (Particle == NULL)
	{
		UE_LOG(VRGesturePluginLog, Error, TEXT("[%s::updatePrior] Particle == NULL"), *GetName());
		return;
	}

	// Update alignment / dynamics / scalings
	float L = GestureManager->GetTemplateLength(Particle->GestureID);
	if (L == 0)
	{
		UE_LOG(VRGesturePluginLog, Error, TEXT("[%s::updatePrior] Template path is equal to zero. GestureID:%d"), *GetName(), Particle->GestureID);
		return;
	}

	Particle->Progression += RN.GetRandomNormal() * EngineParameters.alignmentVariance + Particle->Dynamic.X / L; // +Particle->Dynamic.Y / (L*L);

	Particle->Dynamic.X += RN.GetRandomNormal() * EngineParameters.dynamicsVariance.X + Particle->Dynamic.Y / L;
	Particle->Dynamic.Y += RN.GetRandomNormal() * EngineParameters.dynamicsVariance.X;

	Particle->Scale.X += RN.GetRandomNormal() * EngineParameters.scalingsVariance.X;
	Particle->Scale.Y += RN.GetRandomNormal() * EngineParameters.scalingsVariance.Y;
	Particle->Scale.Z += RN.GetRandomNormal() * EngineParameters.scalingsVariance.Z;

	if (rotationsDim != 0)
	{
		Particle->Rotation.X += RN.GetRandomNormal() * EngineParameters.rotationsVariance.X;
		Particle->Rotation.Y += RN.GetRandomNormal() * EngineParameters.rotationsVariance.Y;
		Particle->Rotation.Z += RN.GetRandomNormal() * EngineParameters.rotationsVariance.Z;
	}

	// update prior (Bayesian incremental inference)
	Particle->Prior = Particle->Posterior;
}

//--------------------------------------------------------------
void UVRGestureRecognizer::updateLikelihood(FVector obs, FGestureParticle* Particle, int32 ParticleIndex)
{
	if (Particle == NULL)
	{
		UE_LOG(VRGesturePluginLog, Error, TEXT("[%s::updateLikelihood] Particle == NULL"), *GetName());
		return;
	}

	FVector vobs = obs;

	if (RecognizerConfig.bTranslate)
	{
		vobs = vobs - Particle->Offset;
	}

	if (Particle->Progression < 0.0)
	{
		Particle->Progression = fabs(Particle->Progression);  // re-spread at the beginning
		if (RecognizerConfig.bSegmentation)
			Particle->GestureID = GestureManager->GetGestureIDFromParticleIndex(ParticleIndex);  // Select new gesture id (In case new ones or deleted ones)
	}
	else if (Particle->Progression > 1.0)
	{
		if (RecognizerConfig.bSegmentation)
		{
			Particle->Progression = fabs(1.0 - Particle->Progression); // re-spread at the beginning
			Particle->GestureID = GestureManager->GetGestureIDFromParticleIndex(ParticleIndex); // Select new gesture id (In case new ones or deleted ones)
		}
		else {
			Particle->Progression = fabs(2.0 - Particle->Progression); // re-spread at the end
		}
	}

	// take vref from template at the given alignment	
	UVRGestureTemplate* GestureTemplate = *GestureManager->GestureTemplates.Find(Particle->GestureID);
	if (!GestureTemplate)
	{
		UE_LOG(VRGesturePluginLog, Log, TEXT("[%s::updateLikelihood] Failed to retrieve gesture with ID %d"), *GetName(), Particle->GestureID);
		return;
	}

	float cursor = Particle->Progression;
	int frameindex = std::min((GestureTemplate->getTemplateLength() - 1), (int)(floor(cursor * GestureTemplate->getTemplateLength())));

	FVector vref = GestureTemplate->templateRaw[frameindex];

	// Apply scaling coefficients
	vref *= Particle->Scale;

	// Apply rotation coefficients

	// Rotate template sample according to the estimated angles of rotations (3d)
	vector<vector< float> > RotMatrix = getRotationMatrix3d(Particle->Rotation.X, Particle->Rotation.Y, Particle->Rotation.Z);
	vector<float> vrefVec;
	vrefVec.push_back(vref.X);
	vrefVec.push_back(vref.Y);
	vrefVec.push_back(vref.Z);

	vrefVec = multiplyMat(RotMatrix, vrefVec);

	vector<float> vobsVec;
	vobsVec.push_back(vobs.X);
	vobsVec.push_back(vobs.Y);
	vobsVec.push_back(vobs.Z);

	vector<float> dimWeights;
	dimWeights.push_back(EngineParameters.dimWeights.X);
	dimWeights.push_back(EngineParameters.dimWeights.Y);
	dimWeights.push_back(EngineParameters.dimWeights.Z);

	// weighted euclidean distance
	float dist = distance_weightedEuclidean(vrefVec, vobsVec, dimWeights);

	if (EngineParameters.distribution == 0.0f) {    // Gaussian distribution
		Particle->Likelihood = exp(-dist * 1 / (EngineParameters.tolerance * EngineParameters.tolerance));
	}
	else {            // Student's distribution
		Particle->Likelihood = pow(dist / EngineParameters.distribution + 1, -EngineParameters.distribution / 2 - 1);    // dimension is 2 .. pay attention if editing]
	}
}

//--------------------------------------------------------------
void UVRGestureRecognizer::updatePosterior(FGestureParticle* Particle) {

	if (Particle == NULL)
	{
		// TODO ERROR
		return;
	}
	Particle->Posterior = Particle->Prior * Particle->Likelihood;
}

//--------------------------------------------------------------
void UVRGestureRecognizer::resampleAccordingToWeights(FVector obs)
{

	// cumulative dist
	int NumberOfParticles = EngineParameters.numberParticles;

	TArray<float>    Dist;
	TArray<int>	OldGestureID;
	TArray<float> OldProgression;
	TArray<FVector>	OldDynamic;
	TArray<FVector>	OldScale;
	TArray<FVector>	OldRotation;

	// Allocate space 
	Dist.SetNumUninitialized(EngineParameters.numberParticles);
	OldGestureID.SetNumUninitialized(NumberOfParticles);
	OldProgression.SetNumUninitialized(NumberOfParticles);
	OldDynamic.SetNumUninitialized(NumberOfParticles);
	OldScale.SetNumUninitialized(NumberOfParticles);
	OldRotation.SetNumUninitialized(NumberOfParticles);

	// Save old data
	for (int ParticleIndex = 0; ParticleIndex < NumberOfParticles; ParticleIndex++)
	{
		if (!GestureParticles.IsValidIndex(ParticleIndex))
		{
			// TODO ERROR
			continue;
		}
		FGestureParticle* Particle = &GestureParticles[ParticleIndex];

		OldGestureID[ParticleIndex] = Particle->GestureID;
		OldProgression[ParticleIndex] = Particle->Progression;
		OldDynamic[ParticleIndex] = Particle->Dynamic;
		OldScale[ParticleIndex] = Particle->Scale;
		OldRotation[ParticleIndex] = Particle->Rotation;
	}


	// Calculate distance 
	Dist[0] = 0;
	for (int ParticleIndex = 1; ParticleIndex < NumberOfParticles; ParticleIndex++)
	{
		if (!GestureParticles.IsValidIndex(ParticleIndex))
		{
			// TODO ERROR
			continue;
		}
		FGestureParticle* Particle = &GestureParticles[ParticleIndex];

		Dist[ParticleIndex] = Dist[ParticleIndex - 1] + Particle->Posterior;
	}
	float u0 = (RN.GetRandomUniform() - 0.5) / NumberOfParticles;

	int i = 0;
	for (int ParticleIndex = 0; ParticleIndex < NumberOfParticles; ParticleIndex++)
	{
		if (!GestureParticles.IsValidIndex(ParticleIndex))
		{
			// TODO ERROR
			continue;
		}
		FGestureParticle* Particle = &GestureParticles[ParticleIndex];

		float uj = u0 + (ParticleIndex + 0.) / NumberOfParticles;

		while (uj > Dist[i] && i < NumberOfParticles - 1) {
			i++;
		}

		Particle->GestureID = OldGestureID[i];
		Particle->Progression = OldProgression[i];
		Particle->Dynamic = OldDynamic[i];
		Particle->Scale = OldScale[i];
		Particle->Rotation = OldRotation[i];

		// update posterior (particles' weights)
		Particle->Posterior = 1.0 / (float)NumberOfParticles;
	}

}

//--------------------------------------------------------------
void UVRGestureRecognizer::estimates() {


	int NumberOfParticles = EngineParameters.numberParticles;

	GestureManager->InitEstimates();
	for (int ParticleIndex = 0; ParticleIndex < NumberOfParticles; ParticleIndex++)
	{
		if (!GestureParticles.IsValidIndex(ParticleIndex))
		{
			UE_LOG(VRGesturePluginLog, Log, TEXT("[%s::estimates1] Failed to retrieve particle, index is invalid: %d"), *GetName(), ParticleIndex);
			continue;
		}
		FGestureParticle* Particle = &GestureParticles[ParticleIndex];

		(*GestureManager->GestureTemplates.Find(Particle->GestureID))->probabilityNormalisation += Particle->Posterior;
	}


	// compute the estimated features and likelihoods
	for (int ParticleIndex = 0; ParticleIndex < NumberOfParticles; ParticleIndex++)
	{
		if (!GestureParticles.IsValidIndex(ParticleIndex))
		{
			UE_LOG(VRGesturePluginLog, Log, TEXT("[%s::estimates2] Failed to retrieve particle, index is invalid: %d"), *GetName(), ParticleIndex);
			continue;
		}
		FGestureParticle* Particle = &GestureParticles[ParticleIndex];

		UVRGestureTemplate* Gesture = *GestureManager->GestureTemplates.Find(Particle->GestureID);

		if (Gesture == NULL)
		{
			UE_LOG(VRGesturePluginLog, Log, TEXT("[%s::estimates2] Failed to retrieve gesture with ID: %d"), *GetName(), Particle->GestureID);
			continue;
		}

		Gesture->estimatedAlignment += Particle->Progression * Particle->Posterior;

		Gesture->estimatedDynamics += Particle->Dynamic * (Particle->Posterior / Gesture->probabilityNormalisation);

		Gesture->estimatedScalings += Particle->Scale * (Particle->Posterior / Gesture->probabilityNormalisation);
		
		if (rotationsDim != 0)
			Gesture->estimatedRotations += Particle->Rotation * (Particle->Posterior / Gesture->probabilityNormalisation);

		if (!isnan(Particle->Posterior))
			Gesture->estimatedProbabilities += Particle->Posterior;

		Gesture->estimatedLikelihoods += Particle->Likelihood;
	}

	// calculate most probable index during scaling...
	float maxProbability = 0.0f;
	mostProbableIndex = -1;

	for (auto& Elem : GestureManager->GestureTemplates) {
		if (Elem.Value->estimatedProbabilities > maxProbability) {
			maxProbability = Elem.Value->estimatedProbabilities;
			mostProbableIndex = Elem.Value->GestureID;

		}
	}

	UVRGestureTemplate* MostProbableGesture = *GestureManager->GestureTemplates.Find(mostProbableIndex);

	if (MostProbableGesture != NULL)
	{
		if (MostProbableGesture->estimatedAlignment > 0.95 && MostProbableGesture->estimatedProbabilities > 0.9)
		{
			OnGestureActivated.Broadcast(mostProbableIndex);
			// Reset current gesture
			CurrentGesture->Reset();
		}
	}


}

//--------------------------------------------------------------
// Update the number of particles
void UVRGestureRecognizer::setNumberOfParticles(int numberOfParticles) {

	EngineParameters.numberParticles = numberOfParticles;

	if (EngineParameters.numberParticles < 4)     // minimum number of particles allowed
		EngineParameters.numberParticles = 4;

	train();

	if (EngineParameters.numberParticles <= EngineParameters.resamplingThreshold) {
		EngineParameters.resamplingThreshold = EngineParameters.numberParticles / 4;
	}

}

//--------------------------------------------------------------
int UVRGestureRecognizer::getNumberOfParticles() {
	return EngineParameters.numberParticles; // Return the number of particles
}

//--------------------------------------------------------------
void UVRGestureRecognizer::setPredictionSteps(int predictionSteps)
{
	if (predictionSteps<1)
		EngineParameters.predictionSteps = 1;
	else
		EngineParameters.predictionSteps = predictionSteps;
}

//--------------------------------------------------------------
int UVRGestureRecognizer::getPredictionSteps()
{
	return EngineParameters.predictionSteps; // Return the number of particles
}

//--------------------------------------------------------------
// Update the re sampling threshold used to avoid degeneracy problem
void UVRGestureRecognizer::setResamplingThreshold(int _resamplingThreshold) {
	if (_resamplingThreshold >= EngineParameters.numberParticles)
		_resamplingThreshold = floor(EngineParameters.numberParticles / 2.0f);
	EngineParameters.resamplingThreshold = _resamplingThreshold;
}

//--------------------------------------------------------------
// Return the re sampling threshold used to avoid degeneracy problem
int UVRGestureRecognizer::getResamplingThreshold() {
	return EngineParameters.resamplingThreshold;
}

//--------------------------------------------------------------
// Update the standard deviation of the observation distribution
// this value acts as a tolerance for the algorithm
// low value: less tolerant so more precise but can diverge
// high value: more tolerant so less precise but converge more easily
void UVRGestureRecognizer::setTolerance(float _tolerance) {
	if (_tolerance <= 0.0) _tolerance = 0.1;
	EngineParameters.tolerance = _tolerance;
	tolerancesetmanually = true;
}

//--------------------------------------------------------------
float UVRGestureRecognizer::getTolerance() {
	return EngineParameters.tolerance;
}

//--------------------------------------------------------------
void UVRGestureRecognizer::setDynamicsVariance(FVector dynVariance)
{
	EngineParameters.dynamicsVariance = dynVariance;
}
//--------------------------------------------------------------
FVector UVRGestureRecognizer::getDynamicsVariance()
{
	return EngineParameters.dynamicsVariance;
}

//--------------------------------------------------------------
void UVRGestureRecognizer::setScalingsVariance(FVector scaleVariance)
{
	EngineParameters.scalingsVariance = scaleVariance;
}

//--------------------------------------------------------------
FVector UVRGestureRecognizer::getScalingsVariance()
{
	return EngineParameters.scalingsVariance;
}

//--------------------------------------------------------------
void UVRGestureRecognizer::setRotationsVariance(FVector rotationVariance)
{
	EngineParameters.scaleVariance = rotationVariance;
}

//--------------------------------------------------------------
FVector UVRGestureRecognizer::getRotationsVariance()
{
	return EngineParameters.rotationsVariance;
}

//--------------------------------------------------------------
void UVRGestureRecognizer::setSpreadDynamics(float center, float range, int dim)
{
	EngineParameters.dynamicsSpreadingCenter = center;
	EngineParameters.dynamicsSpreadingRange = range;
}

//--------------------------------------------------------------
void UVRGestureRecognizer::setSpreadScalings(float center, float range, int dim)
{
	EngineParameters.scalingsSpreadingCenter = center;
	EngineParameters.scalingsSpreadingRange = range;
}

//--------------------------------------------------------------
void UVRGestureRecognizer::setSpreadRotations(float center, float range, int dim)
{
	EngineParameters.rotationsSpreadingCenter = center;
	EngineParameters.rotationsSpreadingRange = range;
}

//--------------------------------------------------------------
void UVRGestureRecognizer::translate(bool translateFlag)
{
	RecognizerConfig.bTranslate = translateFlag;
}

//--------------------------------------------------------------
void UVRGestureRecognizer::segmentation(bool segmentationFlag)
{
	RecognizerConfig.bSegmentation = segmentationFlag;
}