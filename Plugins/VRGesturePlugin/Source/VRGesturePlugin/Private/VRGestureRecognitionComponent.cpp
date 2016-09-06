// Fill out your copyright notice in the Description page of Project Settings.

#include "VRGesturePluginPrivatePCH.h"
#include "VRGestureRecognitionComponent.h"


// Sets default values for this component's properties
UVRGestureRecognitionComponent::UVRGestureRecognitionComponent()
{
	// Set this component to be initialized when the game starts, and to be ticked every frame.  You can turn these features
	// off to improve performance if you don't need them.
	bWantsBeginPlay = true;
	PrimaryComponentTick.bCanEverTick = true;

	// Create recognizer
	GestureRecognizer = CreateDefaultSubobject<UVRGestureRecognizer>("GestureRecognizer");
	// Ensure we are not listening or recording at start
	IsRecordingGesture = false;
	IsListeningGesture = false;

}


// Called when the game starts
void UVRGestureRecognitionComponent::BeginPlay()
{
	Super::BeginPlay();

	// Load saved templates
	if (TemplateFilePath != "None")
	{
		LoadTemplates();
	}
	
}


// Called every frame
void UVRGestureRecognitionComponent::TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction )
{
	Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

	if (GetOwner() && GestureRecognizer)
	{
		FVector Position = this->GetComponentLocation();
		GestureRecognizer->Tick(Position);
	}
}


void UVRGestureRecognitionComponent::RecordGesture(int32 GestureID)
{
	UE_LOG(VRGesturePluginLog, Log, TEXT("RecordGesture Starting recording gesture with ID: %d"), GestureID);
	GestureRecognizer->StartRecordingNewGesture(GestureID);
}

void UVRGestureRecognitionComponent::StopRecordGesture()
{
	GestureRecognizer->StopRecordingGesture();
	UE_LOG(VRGesturePluginLog, Log, TEXT("[%s::StopRecordGesture]"), *GetName());
}

void UVRGestureRecognitionComponent::ClearGestures()
{
	GestureRecognizer->ClearAllGestures();
}

void UVRGestureRecognitionComponent::ListenGestures(TArray<int> GestureIDs)
{
	GestureRecognizer->StartListening(GestureIDs);
}


void UVRGestureRecognitionComponent::ListenAllGestures()
{
	GestureRecognizer->StartListening();
}

void UVRGestureRecognitionComponent::StopListenGesture()
{
	GestureRecognizer->StopListening();
}

void UVRGestureRecognitionComponent::SaveTemplates()
{
	if (GestureRecognizer)
	{
		FString FullPath = FPaths::GameContentDir() + TemplateFilePath;
		//gvf->saveTemplates(TCHAR_TO_UTF8(*FullPath));
	}
}
void UVRGestureRecognitionComponent::LoadTemplates()
{
	if (GestureRecognizer)
	{
		FString FullPath = FPaths::GameContentDir() + TemplateFilePath;
		//gvf->loadTemplates(TCHAR_TO_UTF8(*FullPath));
	}
}