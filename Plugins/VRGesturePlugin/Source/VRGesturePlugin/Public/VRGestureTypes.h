
#include <vector> 
#include "VRGestureTypes.generated.h"
#pragma once

using namespace std; 

USTRUCT()
struct FVRGestureRecognizerConfig
{
	GENERATED_USTRUCT_BODY()

	// Dimension of input points [2;3]
	UPROPERTY(EditDefaultsOnly)
	int32 Dimensions;

	// If should use translation for each new segment
	UPROPERTY(EditDefaultsOnly)
	bool bTranslate;

	// If should segment after a completed gesture 
	UPROPERTY(EditDefaultsOnly)
	bool bSegmentation;
}; 


USTRUCT()
struct FGREngineParameters
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditDefaultsOnly)
	float tolerance;
	UPROPERTY(EditDefaultsOnly)
	float distribution;
	UPROPERTY(EditDefaultsOnly)
	int32 numberParticles;
	UPROPERTY(EditDefaultsOnly)
	int32 resamplingThreshold;
	UPROPERTY(EditDefaultsOnly)
	float alignmentVariance;
	UPROPERTY(EditDefaultsOnly)
	float speedVariance;
	UPROPERTY(EditDefaultsOnly)
	FVector scaleVariance;
	UPROPERTY(EditDefaultsOnly)
	FVector dynamicsVariance;
	UPROPERTY(EditDefaultsOnly)
	FVector scalingsVariance;
	UPROPERTY(EditDefaultsOnly)
	FVector rotationsVariance;

	// spreadings
	UPROPERTY(EditDefaultsOnly)
	float alignmentSpreadingCenter;
	UPROPERTY(EditDefaultsOnly)
	float alignmentSpreadingRange;
	UPROPERTY(EditDefaultsOnly)
	float dynamicsSpreadingCenter;
	UPROPERTY(EditDefaultsOnly)
	float dynamicsSpreadingRange;
	UPROPERTY(EditDefaultsOnly)
	float scalingsSpreadingCenter;
	UPROPERTY(EditDefaultsOnly)
	float scalingsSpreadingRange;
	UPROPERTY(EditDefaultsOnly)
	float rotationsSpreadingCenter;
	UPROPERTY(EditDefaultsOnly)
	float rotationsSpreadingRange;
	UPROPERTY(EditDefaultsOnly)
	int32 predictionSteps;
	UPROPERTY(EditDefaultsOnly)
	FVector dimWeights;
};

UENUM()
enum class EVRGestureRecognizerState : uint8
{
	Idle,
	Listening,
	Recording
};


USTRUCT()
struct FGestureParticle
{
	GENERATED_USTRUCT_BODY()

	// ID of the associated gesture
	UPROPERTY()
	int32 GestureID;

	// Instantaneous progression of the gesture [0;1]
	UPROPERTY()
	float Progression; 

	// Instantaneous estimation of the dynamic parameter
	UPROPERTY()
	FVector Dynamic; 

	// Instantaneous estimation of the scale
	UPROPERTY()
	FVector Scale; 

	// Instantaneous estimation of the rotation 
	UPROPERTY()
	FVector Rotation;

	UPROPERTY()
	FVector Offset;

	UPROPERTY()
	float Weight;

	UPROPERTY()
	float Likelihood; 

	UPROPERTY()
	float Prior;

	UPROPERTY()
	float Posterior;
};

USTRUCT(Blueprintable)
struct FGestureOutcome
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gesture")
	int32 GestureIndex;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gesture")
	float likelihood;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gesture")
	float alignment;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gesture")
	FVector dynamic;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gesture")
	FVector scaling;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gesture")
	FVector rotation;
};

USTRUCT(Blueprintable)
struct FVRGROutcomes
{
	GENERATED_USTRUCT_BODY()

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gesture")
	FGestureOutcome likeliestGesture;
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Gesture")
	TArray<FGestureOutcome> Gestures; 
};


template <typename T>
inline void initVec(vector<T> & V, int rows) {
	V.resize(rows);
}

//--------------------------------------------------------------
//--------------------------------------------------------------
// init matrix by allocating memory
template <typename T>
inline void initMat(vector< vector<T> > & M, int rows, int cols) {
	M.resize(rows);
	for (int n = 0; n < rows; n++) {
		M[n].resize(cols);
	}
}

template <typename T>
inline vector<vector<float> > getRotationMatrix3d(T phi, T theta, T psi)
{
	vector< vector<float> > M;
	initMat(M, 3, 3);

	M[0][0] = cos(theta)*cos(psi);
	M[0][1] = -cos(phi)*sin(psi) + sin(phi)*sin(theta)*cos(psi);
	M[0][2] = sin(phi)*sin(psi) + cos(phi)*sin(theta)*cos(psi);

	M[1][0] = cos(theta)*sin(psi);
	M[1][1] = cos(phi)*cos(psi) + sin(phi)*sin(theta)*sin(psi);
	M[1][2] = -sin(phi)*cos(psi) + cos(phi)*sin(theta)*sin(psi);

	M[2][0] = -sin(theta);
	M[2][1] = sin(phi)*cos(theta);
	M[2][2] = cos(phi)*cos(theta);

	return M;
}

//--------------------------------------------------------------
template <typename T>
inline vector<T> multiplyMat(vector< vector<T> > & M1, vector< T> & Vect) {
	//assert(Vect.size() == M1[0].size()); // columns in M1 == rows in Vect
	vector<T> multiply;
	initVec(multiply, Vect.size());
	for (int i = 0; i < M1.size(); i++) {
		multiply[i] = 0.0f;
		for (int j = 0; j < M1[i].size(); j++) {
			multiply[i] += M1[i][j] * Vect[j];
		}
	}
	return multiply;
}
//--------------------------------------------------------------
template <typename T>
float distance_weightedEuclidean(vector<T> x, vector<T> y, vector<T> w)
{
	int count = x.size();
	if (count <= 0) return 0;
	float dist = 0.0;
	for (int k = 0; k < count; k++) dist += w[k] * pow((x[k] - y[k]), 2);
	return dist;
}

// Example usage
//GetEnumValueAsString<EVictoryEnum>("EVictoryEnum", VictoryEnum)));
// from: https://wiki.unrealengine.com/Enums_For_Both_C%2B%2B_and_BP#Get_Name_of_Enum_as_String
template<typename TEnum>
static FORCEINLINE FString GetEnumValueToString(const FString& Name, TEnum Value)
{
	const UEnum* enumPtr = FindObject<UEnum>(ANY_PACKAGE, *Name, true);
	if (!enumPtr)
	{
		return FString("Invalid");
	}

	return enumPtr->GetEnumName((int32)Value);
}

