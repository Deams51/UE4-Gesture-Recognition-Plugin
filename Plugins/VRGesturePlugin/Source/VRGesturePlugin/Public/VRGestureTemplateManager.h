// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Object.h"
#include "VRGestureTemplate.h"
#include "VRGestureTemplateManager.generated.h"

/**
*
*/
UCLASS(BlueprintType, Blueprintable, meta = (ShortTooltip = "A manager of gesture templates."))
class VRGESTUREPLUGIN_API UVRGestureTemplateManager : public UObject
{
	GENERATED_BODY()


public:
	UPROPERTY(VisibleAnywhere, Category = Gesture)
	int inputDimensions;

	UPROPERTY(EditAnywhere, Category = Gesture)
	TMap<int32, UVRGestureTemplate*> GestureTemplates;

	UVRGestureTemplateManager(const FObjectInitializer& X)
		:Super(X)
	{
		inputDimensions = 3;
	}

	/*// Add a new input data to a gesture template, or create a new gesture if doesn't exist 
	void addObservation(FVector observation, int32 templateID = 0) {
		//if (observation.size() != inputDimensions)
		//	inputDimensions = observation.size();

		// check if templateID exists
		if (!GestureTemplates.Contains(templateID))
		{
			UE_LOG(GameDebug, Warning, TEXT("[%s::addObservation] Cannot add observation to template #%d. Template not found."), *GetName(), templateID);
			
			// Create a new one 
			UVRGestureTemplate* NewGesture = NewObject<UVRGestureTemplate>(this, UVRGestureTemplate::StaticClass());
			NewGesture->GestureID = templateID;
			// Store
			GestureTemplates.Add(templateID, NewGesture); 
		}

		UVRGestureTemplate* GestureTemplate = *GestureTemplates.Find(templateID); 
		if (GestureTemplate == NULL)
		{
			UE_LOG(GameDebug, Error, TEXT("[%s::addObservation] Failed to retrieve template with ID = %d"), *GetName(), templateID);
			return;
		}

		GestureTemplate->addObservation(observation);
	}*/


	//void setTemplate(vector< FVector > & observations, int templateIndex = 0) {
	//	for (int i = 0; i < observations.size(); i++) {
	//		addObservation(observations[i], templateIndex);
	//	}
	//}

	UFUNCTION(BlueprintCallable, Category = "Gesture")
	TArray<UVRGestureTemplate*> GetAllTemplates() {
		TArray<UVRGestureTemplate*> Templates; 
		for (auto& Elem : GestureTemplates)
		{
			Templates.Add(Elem.Value);
		}
		return Templates;
	}


	TArray<FVector>& getTemplate(int templateIndex = 0) {
		return (*GestureTemplates.Find(templateIndex))->templateRaw; 
	}

	int getNumberOfTemplates() {
		return GestureTemplates.Num();
	}

	int getNumberDimensions() {
		return inputDimensions;
	}

	int getTemplateLength(int templateIndex = 0) {
		return (*GestureTemplates.Find(templateIndex))->getTemplateLength();
	}

	int getTemplateDimension() {
		return inputDimensions;
	}

	FVector& getLastObservation(int templateIndex = 0) {
		return (*GestureTemplates.Find(templateIndex))->getLastObservation(); 
	}
	/*
	vector< vector< FVector > >& getTemplates() {
		return NULL; // templatesRaw;
	}
	*/

	void deleteTemplate(int templateIndex = 0)
	{
		GestureTemplates.Remove(templateIndex);
	}

	void clear()
	{
		GestureTemplates.Empty(); 
	}
	void InitEstimates();

	TArray<int32> GetAllGestureIDs()
	{
		TArray<int32> GestureIDs;

		for (auto& Elem : GestureTemplates)
		{
			GestureIDs.Add(Elem.Key);
		}

		return GestureIDs;
	}
	bool IsValidGestureID(int32 GestureID);
	
	bool AddNewGesture(UVRGestureTemplate* CurrentGesture);
	float GetTemplateLength(int32 GestureId);
	int32 GetGestureIDFromParticleIndex(int32 ParticleIndex);
};
