#pragma once

#include "BoneOrientationDoubleExponentialFilter.generated.h"


namespace EJoint { enum Type; }

USTRUCT(BlueprintType)
struct KINECTV2_API FFilterDoubleExponentialData
{

	
	GENERATED_USTRUCT_BODY()

public:


	FQuat RawBoneOrientation;

	FQuat FilteredBoneOrientation;

	FQuat Trend;

	uint32 FrameCount;

};


USTRUCT(BlueprintType)
struct KINECTV2_API FTransformSmoothParameters
{


	GENERATED_USTRUCT_BODY();

public:

	/**********************************************************************************************//**
	 * Constructor.
	 *
	 * @author	Leon Rosengarten
	 * @date	26-16-2015
	 *
	 * @param	SmoothingValue		   	The smoothing value.
	 * @param	CorrectionValue		   	The correction value.
	 * @param	PredictionValue		   	The prediction value.
	 * @param	JitterRadiusValue	   	The jitter radius value.
	 * @param	MaxDeviationRadiusValue	The maximum deviation radius value.
	 **************************************************************************************************/

	FTransformSmoothParameters(float SmoothingValue = 0.5f, float CorrectionValue = 0.8f, float PredictionValue = 0.75f, float JitterRadiusValue = 0.1f, float MaxDeviationRadiusValue = 0.1f);

	/**********************************************************************************************//**
	 * Assignment operator.
	 *
	 * @author	Leon Rosengarten
	 * @date	26-18-2015
	 *
	 * @param	Other	The other.
	 *
	 * @return	A shallow copy of this object.
	 **************************************************************************************************/

	FTransformSmoothParameters& operator=(const FTransformSmoothParameters& Other);

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Parameters)
	float Correction; // How much to correct back from prediction.  Can make things springy
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Parameters)
	float JitterRadius; // Size of the radius where jitter is removed. Can do too much smoothing when too high
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Parameters)
	float MaxDeviationRadius; // Size of the max prediction radius Can snap back to noisy data when too high
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Parameters)
	float Prediction; // Amount of prediction into the future to use. Can over shoot when too high
	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = Parameters)
	float Smoothing;  // How much soothing will occur.  Will lag when too high

};


USTRUCT(BlueprintType)
struct KINECTV2_API FBoneOrientationDoubleExponentialFilter
{

	GENERATED_USTRUCT_BODY()

public:

	/**********************************************************************************************//**
	 * Default constructor.
	 *
	 * @author	Leon Rosengarten
	 * @date	26-18-2015
	 **************************************************************************************************/

	FBoneOrientationDoubleExponentialFilter();

	/**********************************************************************************************//**
	 * Initializes this object.
	 *
	 * @author	Leon Rosengarten
	 * @date	26-18-2015
	 **************************************************************************************************/

	void Init();

	/**********************************************************************************************//**
	 * Initializes this object.
	 *
	 * @author	Leon Rosengarten
	 * @date	26-19-2015
	 *
	 * @param	SmoothingValue				The smoothing value.
	 * @param	CorrectionValue				The correction value.
	 * @param	PredictionValue				The prediction value.
	 * @param	JitterRadiusValue			The jitter radius value.
	 * @param	MmaxDeviationRadiusValue	The mmax deviation radius value.
	 **************************************************************************************************/

	void Init(float SmoothingValue, float CorrectionValue, float PredictionValue, float JitterRadiusValue, float MmaxDeviationRadiusValue);

	/**********************************************************************************************//**
	 * Initialises this object.
	 *
	 * @author	Leon Rosengarten
	 * @date	26-19-2015
	 *
	 * @param	SmoothingParameters	Options for controlling the smoothing.
	 **************************************************************************************************/

	void Init(const FTransformSmoothParameters& SmoothingParameters);

	/**********************************************************************************************//**
	 * Resets this object.
	 *
	 * @author	Leon Rosengarten
	 * @date	26-19-2015
	 **************************************************************************************************/

	void Reset();

	struct FBody UpdateFilter(const struct FBody& InBody);

public:

	UPROPERTY(EditAnywhere, BlueprintReadWrite, Category = "Filter|Parameters")
	FTransformSmoothParameters SmoothParameters;

protected:

	/**********************************************************************************************//**
	 * Filter joint.
	 *
	 * @author	Leon Rosengarten
	 * @date	26-19-2015
	 *
	 * @param [in,out]	Body			 	The body.
	 * @param	Jt						 	The jt.
	 * @param	TransformSmoothParameters	Options for controlling the transform smooth.
	 **************************************************************************************************/

	void FilterJoint(struct FBody& Body, EJoint::Type Jt, const FTransformSmoothParameters& TransformSmoothParameters);

private:


	bool IsInit;

	TArray<FFilterDoubleExponentialData> History;



};