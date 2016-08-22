// Fill out your copyright notice in the Description page of Project Settings.

#include "VRGesturePluginPrivatePCH.h"
#include "VRGestureTemplate.h"


UVRGestureTemplate::UVRGestureTemplate() 
	:Super()
{
	inputDimensions = 3;
	templateNormal = TArray<FVector>();
	templateRaw = TArray<FVector>();

	setAutoAdjustRanges(true);

	Reset();
}