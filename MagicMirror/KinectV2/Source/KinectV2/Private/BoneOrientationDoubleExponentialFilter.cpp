#include "IKinectV2PluginPCH.h"
#include "BoneOrientationDoubleExponentialFilter.h"
#include "KinectFunctionLibrary.h"

FTransformSmoothParameters& FTransformSmoothParameters::operator=(const FTransformSmoothParameters& Other)
{
	this->Correction = Other.Correction;
	this->JitterRadius = Other.JitterRadius;
	this->MaxDeviationRadius = Other.MaxDeviationRadius;
	this->Prediction = Other.Prediction;
	this->Smoothing = Other.Smoothing;

	return *this;
}

FTransformSmoothParameters::FTransformSmoothParameters(float SmoothingValue, float CorrectionValue, float PredictionValue, float JitterRadiusValue, float MaxDeviationRadiusValue)
{
	this->MaxDeviationRadius = MaxDeviationRadiusValue; 
	this->Smoothing = SmoothingValue;                  
	this->Correction = CorrectionValue;                 
	this->Prediction = PredictionValue;                 
	this->JitterRadius = JitterRadiusValue;             
}

FBoneOrientationDoubleExponentialFilter::FBoneOrientationDoubleExponentialFilter() : IsInit(false), SmoothParameters(0.5f, 0.8f, 0.75f, 0.1f, 0.1f)
{

}

/*
FBoneOrientationDoubleExponentialFilter::FBoneOrientationDoubleExponentialFilter(const FObjectInitializer& ObjectInitializer) :Super(ObjectInitializer), IsInit(false)
{

}
*/
void FBoneOrientationDoubleExponentialFilter::Init()
{
	Init(0.5f, 0.8f, 0.75f, 0.1f, 0.1f);
}

void FBoneOrientationDoubleExponentialFilter::Init(float SmoothingValue, float CorrectionValue, float PredictionValue, float JitterRadiusValue, float MaxDeviationRadiusValue)
{
	SmoothParameters = FTransformSmoothParameters();

	SmoothParameters.MaxDeviationRadius = MaxDeviationRadiusValue; // Size of the max prediction radius Can snap back to noisy data when too high
	SmoothParameters.Smoothing = SmoothingValue;                   // How much soothing will occur.  Will lag when too high
	SmoothParameters.Correction = CorrectionValue;                 // How much to correct back from prediction.  Can make things springy
	SmoothParameters.Prediction = PredictionValue;                 // Amount of prediction into the future to use. Can over shoot when too high
	SmoothParameters.JitterRadius = JitterRadiusValue;             // Size of the radius where jitter is removed. Can do too much smoothing when too high
	Reset();
	IsInit = true;
}

void FBoneOrientationDoubleExponentialFilter::Init(const FTransformSmoothParameters& TransformSmoothParameters)
{
	SmoothParameters = TransformSmoothParameters;
	Reset();
	IsInit = true;

}

void FBoneOrientationDoubleExponentialFilter::Reset()
{
	History.Empty();

	History.AddDefaulted(25);
}

FBody FBoneOrientationDoubleExponentialFilter::UpdateFilter(const struct FBody& InBody)
{

	FBody OutBody = InBody;

	if (!OutBody.bIsTracked)
	{
		return OutBody;
	}

	if (!IsInit)
	{
		Init(SmoothParameters);
	}

	FTransformSmoothParameters TempSmoothingParams = FTransformSmoothParameters();

	// Check for divide by zero. Use an epsilon of a 10th of a millimeter
	SmoothParameters.JitterRadius = FMath::Max(0.0001f, SmoothParameters.JitterRadius);

	TempSmoothingParams.Smoothing = SmoothParameters.Smoothing;
	TempSmoothingParams.Correction = SmoothParameters.Correction;
	TempSmoothingParams.Prediction = SmoothParameters.Prediction;



	for(auto Bone : OutBody.KinectBones)
	{
		// If not tracked, we smooth a bit more by using a bigger jitter radius
		// Always filter feet highly as they are so noisy
		if (Bone.TrackingState != ETrackingState::TrackingState_Tracked || Bone.JointTypeEnd == EJoint::JointType_FootLeft || Bone.JointTypeEnd == EJoint::JointType_FootRight)
		{
			TempSmoothingParams.JitterRadius *= 2.0f;
			TempSmoothingParams.MaxDeviationRadius *= 2.0f;
		}
		else
		{
			TempSmoothingParams.JitterRadius = SmoothParameters.JitterRadius;
			TempSmoothingParams.MaxDeviationRadius = SmoothParameters.MaxDeviationRadius;
		}

		FilterJoint(OutBody, Bone.JointTypeEnd, TempSmoothingParams);
	}

	return OutBody;
}

FQuat EnsureQuaternionNeighborhood(const FQuat& quaternionA, const FQuat& quaternionB)
{
	if ((quaternionA | quaternionB) < 0.f)
	{
		// Negate the second quaternion, to place it in the opposite 3D sphere.
		return quaternionB*-1;
	}

	return quaternionB;
}
FQuat EnhancedQuaternionSlerp(const FQuat& quaternionA, const FQuat& quaternionB, float amount)
{
	FQuat modifiedB = EnsureQuaternionNeighborhood(quaternionA, quaternionB);
	return FQuat::Slerp(quaternionA, modifiedB, amount);
}

bool JointPositionIsValid(FVector jointPosition)
{
	return jointPosition.X != 0.0f || jointPosition.Y != 0.0f || jointPosition.Z != 0.0f;
}

bool IsTrackedOrInferred(const FBody& skeleton, EJoint::Type jt)
{


	return skeleton.KinectBones[jt].TrackingState != ETrackingState::TrackingState_NotTracked;
}


FQuat RotationBetweenQuaternions(const FQuat& quaternionA, const FQuat& quaternionB)
{
	FQuat modifiedB = EnsureQuaternionNeighborhood(quaternionA, quaternionB);
	return quaternionA.Inverse()*modifiedB; 
}

float QuaternionAngle(const FQuat& rotation)
{

	FQuat TempRotation = rotation;
	TempRotation.Normalize();
	float angle = 2.0f * FMath::Acos(TempRotation.W);
	return angle;
}

void FBoneOrientationDoubleExponentialFilter::FilterJoint(FBody& Body, EJoint::Type Jt, const FTransformSmoothParameters& TransformSmoothParameters)
{

	FQuat filteredOrientation;
	FQuat trend;

	uint32 jointIndex = TEnumAsByte<EJoint::Type>(Jt).GetValue();

	FQuat rawOrientation = Body.KinectBones[Jt].MirroredJointTransform.GetRotation();
	FQuat prevFilteredOrientation = History[jointIndex].FilteredBoneOrientation;
	FQuat prevTrend = History[jointIndex].Trend;
	FVector rawPosition = Body.KinectBones[Jt].MirroredJointTransform.GetLocation();
	bool orientationIsValid = JointPositionIsValid(rawPosition) && IsTrackedOrInferred(Body, Jt) && !rawOrientation.ContainsNaN();

	if (!orientationIsValid)
	{
		if (History[jointIndex].FrameCount > 0)
		{
			rawOrientation = History[jointIndex].FilteredBoneOrientation;
			History[jointIndex].FrameCount = 0;
		}
	}

	// Initial start values or reset values
	if (History[jointIndex].FrameCount == 0)
	{
		// Use raw position and zero trend for first value
		filteredOrientation = rawOrientation;
		trend = FQuat::Identity;
	}
	else if (History[jointIndex].FrameCount == 1)
	{
		// Use average of two positions and calculate proper trend for end value
		FQuat prevRawOrientation = History[jointIndex].RawBoneOrientation;
		filteredOrientation = EnhancedQuaternionSlerp(prevRawOrientation, rawOrientation, 0.5f);

		FQuat diffStarted = RotationBetweenQuaternions(filteredOrientation, prevFilteredOrientation);
		trend = EnhancedQuaternionSlerp(prevTrend, diffStarted, TransformSmoothParameters.Correction);
	}
	else
	{
		// First apply a jitter filter
		FQuat diffJitter = RotationBetweenQuaternions(rawOrientation, prevFilteredOrientation);
		float diffValJitter = FMath::Abs(QuaternionAngle(diffJitter));

		if (diffValJitter <= TransformSmoothParameters.JitterRadius)
		{
			filteredOrientation = EnhancedQuaternionSlerp(prevFilteredOrientation, rawOrientation, diffValJitter / TransformSmoothParameters.JitterRadius);
		}
		else
		{
			filteredOrientation = rawOrientation;
		}

		// Now the double exponential smoothing filter
		filteredOrientation = EnhancedQuaternionSlerp(filteredOrientation, prevFilteredOrientation*prevTrend, TransformSmoothParameters.Smoothing);

		diffJitter = RotationBetweenQuaternions(filteredOrientation, prevFilteredOrientation);
		trend = EnhancedQuaternionSlerp(prevTrend, diffJitter, TransformSmoothParameters.Correction);
	}

	// Use the trend and predict into the future to reduce latency
	FQuat predictedOrientation = filteredOrientation*  EnhancedQuaternionSlerp(FQuat::Identity, trend, TransformSmoothParameters.Prediction);

	// Check that we are not too far away from raw data
	FQuat diff = RotationBetweenQuaternions(predictedOrientation, filteredOrientation);
	float diffVal = FMath::Abs(QuaternionAngle(diff));

	if (diffVal > TransformSmoothParameters.MaxDeviationRadius)
	{
		predictedOrientation = EnhancedQuaternionSlerp(filteredOrientation, predictedOrientation, TransformSmoothParameters.MaxDeviationRadius / diffVal);
	}

	predictedOrientation.Normalize();
	filteredOrientation.Normalize();
	trend.Normalize();

	// Save the data from this frame
	History[jointIndex].RawBoneOrientation = rawOrientation;
	History[jointIndex].FilteredBoneOrientation = filteredOrientation;
	History[jointIndex].Trend = trend;
	History[jointIndex].FrameCount++;

	// Set the filtered and predicted data back into the bone orientation
	
	Body.KinectBones[Jt].MirroredJointTransform.SetRotation(predictedOrientation);

}

