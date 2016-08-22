// Fill out your copyright notice in the Description page of Project Settings.

#include "VRGesturePluginPrivatePCH.h"
#include "VRGestureTemplate.h"




void UVRGestureTemplateManager::InitEstimates()
{
	for (auto& Elem : GestureTemplates)
	{
		Elem.Value->InitEstimates(GestureTemplates.Num());
	}
}

bool UVRGestureTemplateManager::IsValidGestureID(int32 GestureID)
{
	return !GestureTemplates.Contains(GestureID);
}

bool UVRGestureTemplateManager::AddNewGesture(UVRGestureTemplate* CurrentGesture)
{
	if (GestureTemplates.Contains(CurrentGesture->GestureID))
		return false;

	if (GestureTemplates.Add(CurrentGesture->GestureID, CurrentGesture) == NULL)
	{
		return false;
	}

	return true; 
}

float UVRGestureTemplateManager::GetTemplateLength(int32 GestureID)
{
	UVRGestureTemplate* Gesture = *GestureTemplates.Find(GestureID); 
	if (Gesture)
	{
		return Gesture->getTemplateLength();
	}
	return 0; 
}

int32 UVRGestureTemplateManager::GetGestureIDFromParticleIndex(int32 ParticleIndex)
{
	int32 NewIndex = ParticleIndex % GestureTemplates.Num(); 
	int32 cpt = 0; 
	for (auto& Elem : GestureTemplates)
	{
		if (cpt == NewIndex)
			return Elem.Key; 
		cpt++;
	}

	UE_LOG(VRGesturePluginLog, Log, TEXT("[%s::GetGestureIDFromParticleIndex]  Failed to find a correct ID for particle %d"), *GetName(), ParticleIndex);
	return -1;
}
