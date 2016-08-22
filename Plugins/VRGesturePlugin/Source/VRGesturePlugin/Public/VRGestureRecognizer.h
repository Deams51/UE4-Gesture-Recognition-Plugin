// Fill out your copyright notice in the Description page of Project Settings.

#pragma once

#include "Object.h"
#include "VRGestureTypes.h"
#include "VRGestureTemplateManager.h"
#include "VRGestureRecognizer.generated.h"

DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FOnGestureActivated, int32, GestureID);

/**
 * 
 */
UCLASS()
class VRGESTUREPLUGIN_API UVRGestureRecognizer : public UObject
{
	GENERATED_BODY()
	
public:

	UVRGestureRecognizer(const FObjectInitializer& X);

	/**
	* Define a subset of gesture templates on which to perform the recognition
	* and variation tracking
	*
	* @details By default every recorded gesture template is considered
	* @param set of gesture template index to consider
	*/
	//void setActiveGestures(TArray<int> activeGestureIds);

	/**
	* Restart GVF
	* @details re-sample particles at the origin (i.e. initial prior)
	*//*
	void restart();*/

	/**
	* Clear GVF
	//* @details delete templates
	//*/
	//void clear();

	/**
	* Translate data according to the first point
	* @details substract each gesture feature by the first point of the gesture
	* @param boolean to activate or deactivate translation
	*/
	void translate(bool translateFlag);

	/**
	* Segment gestures within a continuous gesture stream
	* @details if segmentation is true, the method will segment a continuous gesture into a sequence
	* of gestures. In other words no need to call the method startGesture(), it is done automatically
	* @param segmentationFlag boolean to activate or deactivate segmentation
	*/
	void segmentation(bool segmentationFlag);

	//#pragma mark - [ Accessors ]
	//#pragma mark > Parameters
	/**
	* Set tolerance between observation and estimation
	* @details tolerance depends on the range of the data
	* typially tolerance = (data range)/3.0;
	* @param tolerance value
	*/
	void setTolerance(float tolerance);

	/**
	* Get the obervation tolerance value
	* @details see setTolerance(float tolerance)
	* @return the current toleranc value
	*/
	float getTolerance();

	/**
	* Set number of particles used in estimation
	* @details default valye is 1000, note that the computational
	* cost directly depends on the number of particles
	* @param new number of particles
	*/
	void setNumberOfParticles(int numberOfParticles);

	/**
	* Get the current number of particles
	* @return the current number of particles
	*/
	int getNumberOfParticles();

	/**
	* Number of prediciton steps
	* @details it is possible to leave GVF to perform few steps of prediction
	* ahead which can be useful to estimate more fastly the variations. Default value is 1
	* which means no prediction ahead
	* @param the number of prediction steps
	*/
	void setPredictionSteps(int predictionSteps);

	/**
	* Get the current number of prediction steps
	* @return current number of prediciton steps
	*/
	int getPredictionSteps();

	/**
	* Set resampling threshold
	* @details resampling threshold is the minimum number of active particles
	* before resampling all the particles by the estimated posterior distribution.
	* in other words, it re-targets particles around the best current estimates
	* @param the minimum number of particles (default is (number of particles)/2)
	*/
	void setResamplingThreshold(int resamplingThreshold);

	/**
	* Get the current resampling threshold
	* @return resampling threshold
	*/
	int getResamplingThreshold();

	//#pragma mark > Dynamics
	/**
	* Change variance of adaptation in dynamics
	* @details if dynamics adaptation variance is high the method will adapt faster to
	* fast changes in dynamics. Dynamics is 2-dimensional: the first dimension is the speed
	* The second dimension is the acceleration.
	*
	* Typically the variance is the average amount the speed or acceleration can change from
	* one sample to another. As an example, if the relative estimated speed can change from 1.1 to 1.2
	* from one sample to another, the variance should allow a change of 0.1 in speed. So the variance
	* should be set to 0.1*0.1 = 0.01
	*
	* @param dynVariance dynamics variance value
	* @param dim optional dimension of the dynamics for which the change of variance is applied (default value is 1)
	*/
	//void setDynamicsVariance(float dynVariance, int dim = -1);

	/**
	* Change variance of adaptation in dynamics
	* @details See setDynamicsVariance(float dynVariance, int dim) for more details
	* @param dynVariance vector of dynamics variances, each vector index is the variance to be applied to
	* each dynamics dimension (consequently the vector should be 2-dimensional).
	*/
	void setDynamicsVariance(FVector dynVariance);

	/**
	* Get dynamics variances
	* @return the vector of variances (the returned vector is 2-dimensional)
	*/
	FVector getDynamicsVariance();

	//#pragma mark > Scalings
	/**
	* Change variance of adaptation in scalings
	* @details if scalings adaptation variance is high the method will adapt faster to
	* fast changes in relative sizes. There is one scaling variance for each dimension
	* of the input gesture. If the gesture is 2-dimensional, the scalings variances will
	* also be 2-dimensional.
	*
	* Typically the variance is the average amount the size can change from
	* one sample to another. As an example, if the relative estimated size changes from 1.1 to 1.15
	* from one sample to another, the variance should allow a change of 0.05 in size. So the variance
	* should be set to 0.05*0.05 = 0.0025
	*
	* @param scalings variance value
	* @param dimension of the scalings for which the change of variance is applied
	*/
	void setScalingsVariance(float scaleVariance, int dim = -1);

	/**
	* Change variance of adaptation in dynamics
	* @details See setScalingsVariance(float scaleVariance, int dim) for more details
	* @param vector of scalings variances, each vector index is the variance to be applied to
	* each scaling dimension.
	* @param vector of variances (should be the size of the template gestures dimension)
	*/
	void setScalingsVariance(FVector scaleVariance);

	/**
	* Get scalings variances
	* @return the vector of variances
	*/
	FVector getScalingsVariance();

	//#pragma mark > Rotations
	/**
	* Change variance of adaptation in orientation
	* @details if rotation adaptation variance is high the method will adapt faster to
	* fast changes in relative orientation. If the gesture is 2-dimensional, there is
	* one variance value since the rotation can be defined by only one angle of rotation. If
	* the gesture is 3-dimensional, there are 3 variance values since the rotation in 3-d is
	* defined by 3 rotation angles. For any other dimension, the rotation is not defined.
	*
	* The variance is the average amount the orientation can change from one sample to another.
	* As an example, if the relative orientation in rad changes from 0.1 to 0.2 from one observation
	* to another, the variance should allow a change of 0.1 in rotation angle. So the variance
	* should be set to 0.1*0.1 = 0.01
	*
	* @param rotationsVariance rotation variance value
	* @param dim optional dimension of the rotation for which the change of variance is applied
	*/
	void setRotationsVariance(float rotationsVariance, int dim = -1);

	/**
	* Change variance of adaptation in orientation
	* @details See setRotationsVariance(float rotationsVariance, int dim) for more details
	* @param vector of rotation variances, each vector index is the variance to be applied to
	* each rotation angle (1 or 3)
	* @param vector of variances (should be 1 if the the template gestures are 2-dim or 3 if
	* they are 3-dim)
	*/
	void setRotationsVariance(FVector rotationsVariance);

	/**
	* Get rotation variances
	* @return the vector of variances
	*/
	FVector getRotationsVariance();

	/**
	* Set the interval on which the dynamics values should be spread at the beginning (before adaptation)
	* @details this interval can be used to concentrate the potential dynamics value on a narrow interval,
	* typically around 1 (the default value), for instance between -0.05 and 0.05, or to allow at the very
	* beginning, high changes in dynamics by spreading, for instance between 0.0 and 2.0
	* @param min lower value of the inital values for dynamics
	* @param max higher value of the inital values for dynamics
	* @param dim the dimension on which the change of initial interval should be applied (optional)
	*/
	void setSpreadDynamics(float min, float max, int dim = -1);

	/**
	* Set the interval on which the scalings values should be spread at the beginning (before adaptation)
	* @details this interval can be used to concentrate the potential scalings value on a narrow interval,
	* typically around 1.0 (the default value), for instance between 0.95 and 1.05, or to allow at the very
	* beginning high changes in dynamics by spreading, for instance, between 0.0 and 2.0
	* @param min lower value of the inital values for scalings
	* @param max higher value of the inital values for scalings
	* @param dim the dimension on which the change of initial interval should be applied (optional)
	*/
	void setSpreadScalings(float min, float max, int dim = -1);

	/**
	* Set the interval on which the angle of rotation values should be spread at the beginning (before adaptation)
	* @details this interval can be used to concentrate the potential angle values on a narrow interval,
	* typically around 0.0 (the default value), for instance between -0.05 and 0.05, or to allow at the very
	* beginning, high changes in orientation by spreading, for instance, between -0.5 and 0.5
	* @param min lower value of the inital values for angle of rotation
	* @param max higher value of the inital values for angle of rotation
	* @param dim the dimension on which the change of initial interval should be applied (optional)
	*/
	void setSpreadRotations(float min, float max, int dim = -1);


	void Tick(FVector& InputPoint);
	void TickListening();
	void StartRecordingNewGesture(int32 GestureID);
	void StopRecordingGesture();
	void ClearAllGestures();
	void StartListening(TArray<int32> GestureIDs = TArray<int32>());
	void StopListening();

	UPROPERTY(BlueprintAssignable)
	FOnGestureActivated OnGestureActivated; 

protected:

	UPROPERTY()
	FVRGestureRecognizerConfig  RecognizerConfig;        // Structure storing the configuration of GVF (in GVFUtils.h)

	UPROPERTY()
	FGREngineParameters			EngineParameters;    // Structure storing the parameters of GVF (in GVFUtils.h)

	UPROPERTY()
	EVRGestureRecognizerState   state;         // State (defined above)

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = Gesture)
	UVRGestureTemplateManager*  GestureManager;    // UVRGestureTemplate object to handle incoming data in learning and following modes

	UPROPERTY()
	UVRGestureTemplate*			CurrentGesture;

	FVector dimWeights;           // TOOD: to be put in parameters?
	FVector maxRange;
	FVector minRange;
	int     dynamicsDim;                // dynamics state dimension
	int     scalingsDim;                // scalings state dimension
	int     rotationsDim;               // rotation state dimension
	float   globalNormalizationFactor;          // flagged if normalization
	int     mostProbableIndex;                  // cached most probable index
	bool	tolerancesetmanually;
	
	FVector gestureProbabilities;
	TArray<FGestureParticle> GestureParticles;

private:


	//#pragma mark - Private methods for model mechanics
	void initPrior();
	void initNoiseParameters();
	void updateLikelihood(FVector obs, FGestureParticle* Particle, int32 ParticleIndex);
	void updatePrior(FGestureParticle* Particle);
	void updatePosterior(FGestureParticle* Particle);
	void resampleAccordingToWeights(FVector obs);
	void estimates();       // update estimated outcome
	void train();	
};
