// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "VRGestureTypes.h"
#include "VRGestureRecognizer.h"
#include "Components/SceneComponent.h"
#include "VRGestureRecognitionComponent.generated.h"


DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnNewGestureData, FVRGROutcomes, Outcomes);

UCLASS(ClassGroup=(Custom), meta=(BlueprintSpawnableComponent))
class VRGESTUREPLUGIN_API UVRGestureRecognitionComponent : public USceneComponent
{
	GENERATED_BODY()

	bool IsRecordingGesture;
	bool IsListeningGesture;
	FVector StartRecordingPosition;
public:
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gesture)
		UVRGestureRecognizer* GestureRecognizer;

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Gesture)
		FVector OffsetInput;

	UPROPERTY(EditDefaultsOnly, Category = Gesture)
		FString TemplateFilePath = "None";

	UPROPERTY(BlueprintAssignable, Category = Gesture)
		FOnNewGestureData OnNewGestureData;



public:	
	// Sets default values for this component's properties
	UVRGestureRecognitionComponent();

	// Called when the game starts
	virtual void BeginPlay() override;
	
	// Called every frame
	virtual void TickComponent( float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction ) override;



	UFUNCTION(BlueprintCallable, Category = Gesture)
		void RecordGesture(int32 GestureID);

	UFUNCTION(BlueprintCallable, Category = Gesture)
		void StopRecordGesture();

	UFUNCTION(BlueprintCallable, Category = Gesture)
		void ClearGestures();

	UFUNCTION(BlueprintCallable, Category = Gesture)
		void ListenGestures(TArray<int> GestureIDs);

	UFUNCTION(BlueprintCallable, Category = Gesture)
		void ListenAllGestures();

	UFUNCTION(BlueprintCallable, Category = Gesture)
		void StopListenGesture();

	UFUNCTION(BlueprintCallable, Category = Gesture)
		void SaveTemplates();

	UFUNCTION(BlueprintCallable, Category = Gesture)
		void LoadTemplates();

	//UFUNCTION(BlueprintCallable, Category = Gesture)
	//	void AddGesture()

private:
	//FVRGROutcomes FromGVFToFGR(GVFOutcomes outcomes);
};
