// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Object.h"
#include "VRGestureTemplate.generated.h"

/**
*
*/
UCLASS(BlueprintType, Blueprintable, config = Game, meta = (ShortTooltip = "A motion controller gesture template."))
class VRGESTUREPLUGIN_API UVRGestureTemplate : public UObject
{
	GENERATED_BODY()
public:
		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gesture)
		int32 GestureID;

		UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gesture)
		int inputDimensions;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gesture)
		bool bAutoAdjustNormalRange;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gesture)
		FVector observationRangeMax;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gesture)
		FVector observationRangeMin;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gesture)
		FVector templateInitialObservation;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gesture)
		FVector templateInitialNormal;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gesture)
		TArray< FVector > templateRaw;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gesture)
		TArray< FVector > templateNormal;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gesture)
		float probabilityNormalisation;
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gesture)
		float   estimatedAlignment;         // ..
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gesture)
		FVector estimatedDynamics;          // ..
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gesture)
		FVector estimatedScalings;          // ..
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gesture)
		FVector	estimatedRotations;         // ..
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gesture)
		float   estimatedProbabilities;     // ..
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gesture)
		float   estimatedLikelihoods;       // ..
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gesture)
		float   absoluteLikelihoods;        // ..


public:
	UVRGestureTemplate();
	//UVRGestureTemplate(const UVRGestureTemplate& Other);

	bool operator== (const UVRGestureTemplate& Other)
	{
		return GestureID == Other.GestureID;
	}
	friend uint32 GetTypeHash(const UVRGestureTemplate& Other)
	{
		return GetTypeHash(Other.GestureID);
	}


	void setAutoAdjustRanges(bool b) {
		//        if(b) bIsRangeMinSet = bIsRangeMaxSet = false;
		bAutoAdjustNormalRange = b;
	}

	void InitEstimates(int32 TotalNumberGestures)
	{
		probabilityNormalisation = 0;
		estimatedAlignment = 0; 

		estimatedDynamics = FVector::ZeroVector;
		estimatedScalings = FVector::ZeroVector;
		estimatedRotations = FVector::ZeroVector;

		estimatedProbabilities = 0; 
		estimatedLikelihoods = 0; 
	}

	//void setMax(float x, float y, float z) {
	//	assert(inputDimensions == 3);
	//	FVector r;
	//	r.X = x; r.Y = y; r.Z = z;
	//	setMaxRange(r);
	//}

	//void setMin(float x, float y, float z) {
	//	assert(inputDimensions == 3);
	//	FVector r;
	//	r.X = x; r.Y = y; r.Z = z;
	//	setMinRange(r);
	//}

	void normalise()
	{ 
		templateNormal.SetNumUninitialized(templateRaw.Num());

		FVector MaxMin = observationRangeMax - observationRangeMin; 
		for (int i = 0; i < templateRaw.Num(); i++)
		{
			templateNormal[i] = templateRaw[i] / MaxMin;
		}
		templateInitialNormal = templateInitialObservation / MaxMin;
	}

	void setMaxRange(FVector observationRangeMax) {
		this->observationRangeMax = observationRangeMax;
		//        bIsRangeMaxSet = true;
		normalise();
	}

	void setMinRange(FVector observationRangeMin) {
		this->observationRangeMin = observationRangeMin;
		//        bIsRangeMinSet = true;
		normalise();
	}

	FVector& getMaxRange() {
		return observationRangeMax;
	}

	FVector& getMinRange() {
		return observationRangeMin;
	}

	void ClampObservation(FVector& observation) {
		/*if (observationRangeMax.size() < inputDimensions) {
			observationRangeMax.assign(inputDimensions, -INFINITY);
			observationRangeMin.assign(inputDimensions, INFINITY);
		}*/
		observationRangeMax.X = FMath::Max(observationRangeMax.X, observation.X);
		observationRangeMin.X = FMath::Min(observationRangeMin.X, observation.X);

		observationRangeMax.Y = FMath::Max(observationRangeMax.Y, observation.Y);
		observationRangeMin.Y = FMath::Min(observationRangeMin.Y, observation.Y);

		observationRangeMax.Z = FMath::Max(observationRangeMax.Z, observation.Z);
		observationRangeMin.Z = FMath::Min(observationRangeMin.Z, observation.Z);
	}

	void addObservation(FVector observation) {

		// if it is the first observation then set initial value
		if (templateRaw.Num() == 0)
		{
			templateInitialObservation = observation;
		}

		// Offset based on initial observation
		observation = observation - templateInitialObservation;

		// store the raw observation
		templateRaw.Add(observation);

		ClampObservation(observation);

		normalise();
	}

	int getNumberDimensions() {
		return inputDimensions;
	}

	int getTemplateLength() {
		return templateRaw.Num();
	}

	FVector& getLastObservation() {
		return templateRaw.Last();
	}

	FVector& getInitialObservation() {
		return templateInitialObservation;
	}


	void Reset()
	{
		templateRaw.Empty();
		templateNormal.Empty();

		// TODO Check why -Infinity for max range :O 
		observationRangeMax = FVector(-INFINITY);
		observationRangeMin = FVector(INFINITY);

		templateInitialObservation = FVector::ZeroVector;
		templateInitialNormal = FVector::ZeroVector;
		probabilityNormalisation =0;
		estimatedAlignment = 0;
		estimatedDynamics = FVector::ZeroVector;
		estimatedScalings = FVector::ZeroVector;
		estimatedRotations = FVector::ZeroVector;
		estimatedProbabilities = 0;
		estimatedLikelihoods = 0;
		absoluteLikelihoods = 0;

	}
};
