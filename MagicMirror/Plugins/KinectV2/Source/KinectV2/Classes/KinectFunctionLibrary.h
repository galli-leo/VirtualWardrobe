//------------------------------------------------------------------------------
// 
//     The Kinect for Windows APIs used here are preliminary and subject to change
// 
//------------------------------------------------------------------------------
#pragma once
#include "Core.h"
#include "CoreUObject.h"
#include "Engine.h"
#include "Engine/EngineTypes.h"
#include "AllowWindowsPlatformTypes.h"
#include "Kinect.h"
#include "HideWindowsPlatformTypes.h"
#include "BoneOrientationDoubleExponentialFilter.h"
#include "KinectFunctionLibrary.generated.h"


#pragma region Kinect Types

UENUM(BlueprintType)
namespace EJoint{
	enum Type
	{
		JointType_SpineBase = 0 UMETA(DisplayName = "JointType_SpineBase"),
		JointType_SpineMid = 1 UMETA(DisplayName = "JointType_SpineMid"),
		JointType_Neck = 2 UMETA(DisplayName = "JointType_Neck"),
		JointType_Head = 3 UMETA(DisplayName = "JointType_Head"),
		JointType_ShoulderLeft = 4 UMETA(DisplayName = "JointType_ShoulderLeft"),
		JointType_ElbowLeft = 5 UMETA(DisplayName = "JointType_ElbowLeft"),
		JointType_WristLeft = 6 UMETA(DisplayName = "JointType_WristLeft"),
		JointType_HandLeft = 7 UMETA(DisplayName = "JointType_HandLeft"),
		JointType_ShoulderRight = 8 UMETA(DisplayName = "JointType_ShoulderRight"),
		JointType_ElbowRight = 9 UMETA(DisplayName = "JointType_ElbowRight"),
		JointType_WristRight = 10 UMETA(DisplayName = "JointType_WristRight"),
		JointType_HandRight = 11 UMETA(DisplayName = "JointType_HandRight"),
		JointType_HipLeft = 12 UMETA(DisplayName = "JointType_HipLeft"),
		JointType_KneeLeft = 13 UMETA(DisplayName = "JointType_KneeLeft"),
		JointType_AnkleLeft = 14 UMETA(DisplayName = "JointType_AnkleLeft"),
		JointType_FootLeft = 15 UMETA(DisplayName = "JointType_FootLeft"),
		JointType_HipRight = 16 UMETA(DisplayName = "JointType_HipRight"),
		JointType_KneeRight = 17 UMETA(DisplayName = "JointType_KneeRight"),
		JointType_AnkleRight = 18 UMETA(DisplayName = "JointType_AnkleRight"),
		JointType_FootRight = 19 UMETA(DisplayName = "JointType_FootRight"),
		JointType_SpineShoulder = 20 UMETA(DisplayName = "JointType_SpineShoulder"),
		JointType_HandTipLeft = 21 UMETA(DisplayName = "JointType_HandTipLeft"),
		JointType_ThumbLeft = 22 UMETA(DisplayName = "JointType_ThumbLeft"),
		JointType_HandTipRight = 23 UMETA(DisplayName = "JointType_HandTipRight"),
		JointType_ThumbRight = 24 UMETA(DisplayName = "JointType_ThumbRight"),
		JointType_Count = 25 UMETA(DisplayName = "JointType_Count", Hidden)
	};
}

UENUM(BlueprintType)
namespace ETrackingState{

	enum Type{
		TrackingState_NotTracked = 0 UMETA(DisplayName = "TrackingState_NotTracked"),
		TrackingState_Inferred = 1 UMETA(DisplayName = "TrackingState_Inferred"),
		TrackingState_Tracked = 2 UMETA(DisplayName = "TrackingState_Tracked")
	};

}

UENUM(BlueprintType)
namespace EHandState {

	enum Type{
		HandState_Unknown = 0 UMETA(DisplayName = "HandState_Unknown"),
		HandState_NotTracked = 1 UMETA(DisplayName = "HandState_NotTracked"),
		HandState_Open = 2 UMETA(DisplayName = "HandState_Open"),
		HandState_Closed = 3 UMETA(DisplayName = "HandState_Closed"),
		HandState_Lasso = 4 UMETA(DisplayName = "HandState_Lasso")
	};

}

UENUM()
enum class EKinectStreamType : uint8
{
	KST_Color UMETA(DisplayName = "Color Stream"),
	KST_Depth UMETA(DisplayName = "Depth Stream"),
	KST_IR UMETA(DisplayName = "IR Stream"),
	KST_BodyIndex UMETA(DisplayName = "Body Index Stream"),
};

USTRUCT(BlueprintType)
struct KINECTV2_API FKinectBone
{
	GENERATED_USTRUCT_BODY()

public:

	/**************************************************************************************************
	 * Default constructor.
	 *
	 * @author	Leon Rosengarten
	 **************************************************************************************************/

	FKinectBone();

	FKinectBone(const JointOrientation& jointOrientation, const Joint& joint);

	FKinectBone& operator = (const FKinectBone& Other);


	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kinect")
		TEnumAsByte<EJoint::Type> JointTypeStart;   ///< The joint type start
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kinect")
		TEnumAsByte<EJoint::Type> JointTypeEnd; ///< The joint type end

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kinect")
		FQuat Orientation;  ///< The orientation
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kinect")
		FVector CameraSpacePoint;   ///< The camera space point
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kinect")
		TEnumAsByte<ETrackingState::Type> TrackingState;	///< State of the tracking

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kinect")
		FTransform JointTransform;  ///< The joint transform

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kinect")
		FTransform MirroredJointTransform;  ///< The joint transform

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kinect")
		FVector Normal;

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kinect")
		TArray<TEnumAsByte<EJoint::Type>> Children;

};

USTRUCT(BlueprintType)
struct KINECTV2_API FBody
{
	GENERATED_USTRUCT_BODY()

public:

	/**************************************************************************************************
	 * Default constructor.
	 *
	 * @author	Leon Rosengarten
	 **************************************************************************************************/

	FBody();

	/**************************************************************************************************
	 * Constructor.
	 *
	 * @author	Leon Rosengarten
	 * @param [in,out]	pBody	If non-null, the body.
	 **************************************************************************************************/

	FBody(IBody* pBody);

	/**************************************************************************************************
	 * Constructor.
	 *
	 * @author	Leon Rosengarten
	 * @param	body	The body.
	 **************************************************************************************************/

	FBody(const FBody& body);

	/**************************************************************************************************
	 * Assignment operator.
	 *
	 * @author	Leon Rosengarten
	 * @param	OtherBody	The other body.
	 *
	 * @return	A shallow copy of this object.
	 **************************************************************************************************/

	FBody& operator = (const FBody & OtherBody);

	UPROPERTY(VisibleAnywhere,BlueprintReadOnly, EditFixedSize, Category = "Kinect")
		TArray<FKinectBone> KinectBones;	///< The kinect bones
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kinect")
		TEnumAsByte<ETrackingState::Type> BodyTrackingState;	///< State of the body tracking
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kinect")
		TEnumAsByte<EHandState::Type> LeftHandState;	///< State of the left hand
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kinect")
		TEnumAsByte<EHandState::Type> RightHandState;   ///< State of the right hand

	

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kinect")
		FVector Lean;

	UPROPERTY()
		uint64 TrackingId;  ///< Identifier for the tracking

	UPROPERTY(BlueprintReadOnly,  Category = "Kinect")
		bool bIsTracked;	///< true if this object is tracked
};


USTRUCT(BlueprintType)
struct KINECTV2_API FBodyFrame{

	GENERATED_USTRUCT_BODY()

public:

	/**************************************************************************************************
	 * Default constructor.
	 *
	 * @author	Leon Rosengarten
	 **************************************************************************************************/

	FBodyFrame();

	/**************************************************************************************************
	 * Constructor.
	 *
	 * @author	Leon Rosengarten
	 * @param	bodyFrame	The body frame.
	 **************************************************************************************************/

	FBodyFrame(const FBodyFrame& bodyFrame);

	/**************************************************************************************************
	 * Constructor.
	 *
	 * @author	Leon Rosengarten
	 * @param [in,out]	ppBodies	If non-null, the bodies.
	 * @param	floorPlane			The floor plane.
	 **************************************************************************************************/

	FBodyFrame(IBody** ppBodies, const Vector4& floorPlane);

	/**************************************************************************************************
	 * Assignment operator.
	 *
	 * @author	Leon Rosengarten
	 * @param	OtherBodyFrame	The other body frame.
	 *
	 * @return	A shallow copy of this object.
	 **************************************************************************************************/

	FBodyFrame& operator = (const FBodyFrame& OtherBodyFrame);

	UPROPERTY(VisibleAnywhere, BlueprintReadOnly,EditFixedSize, Category = "Kinect")
		TArray<FBody> Bodies;   ///< The bodies
	UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category = "Kinect")
		FPlane FloorPlane;  ///< The floor plane


};

#pragma endregion

#pragma region Delegates

DECLARE_MULTICAST_DELEGATE_OneParam(FBodyFrameEventSigneture, const FBodyFrame&);

DECLARE_DELEGATE_OneParam(FEnableBodyJoystick, const bool&);

DECLARE_DELEGATE_RetVal_ThreeParams(FVector2D, FMapBodyCoordToScreenCoord, const FVector&, int32, int32);

DECLARE_DELEGATE_RetVal(class UKinectEventManager*, FGetKinectManegerEvent);

DECLARE_DELEGATE_RetVal(class FKinectV2InputDevice*, FGetKinectInputDevice);

DECLARE_DELEGATE(FStartSensorEvent);

DECLARE_DELEGATE(FShutdownSensorEvent);

DECLARE_DELEGATE_OneParam(FMapColorFrameToDepthSpace, TArray<FVector2D>&);

#pragma endregion


UCLASS()
class KINECTV2_API UKinectFunctionLibrary : public UBlueprintFunctionLibrary
{

	GENERATED_UCLASS_BODY()

public:

	friend struct FBody;
	friend struct FKinectBone;
	friend struct FBodyFrame;
	friend class UVisualGestureImporterFactory;


	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Kinect|Math", meta = (NativeMakeFunc))

		/**************************************************************************************************
		 * \fn	static FQuat UKinectFunctionLibrary::MakeQuat(float x, float y, float z, float w);
		 *
		 * \brief	Makes a quaternion.
		 *
		 * \param	x	The x coordinate. \param	y	The y coordinate. \param	z	The z coordinate. \param	w	The
		 * scalar.
		 *
		 * \return	A FQuat.
		 *
		 * @author	Leon Rosengarten
		 * @param	x	The x coordinate.
		 * @param	y	The y coordinate.
		 * @param	z	The z coordinate.
		 * @param	w	The width.
		 *
		 * @return	A FQuat.
		 **************************************************************************************************/

		 static FQuat MakeQuat(float x, float y, float z, float w);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Kinect|Math", meta = (NativeBreakFunc))

		/**************************************************************************************************
		 * \fn	static void UKinectFunctionLibrary::BreakQuat(const FQuat&amp; InQuat, float&amp; x,
		 * float&amp; y, float&amp; z, float&amp; w);
		 *
		 * \brief	Break quaternion.
		 *
		 * \param	InQuat   	The in quaternion. \param [in,out]	x	The float&amp; to process. \param [in,
		 * out]	y	The float&amp; to process. \param [in,out]	z	The float&amp; to process. \param [in,out]
		 * w	The float&amp; to process.
		 *
		 * @author	Leon Rosengarten
		 * @param	InQuat   	The in quaternion.
		 * @param [in,out]	x	The float&amp; to process.
		 * @param [in,out]	y	The float&amp; to process.
		 * @param [in,out]	z	The float&amp; to process.
		 * @param [in,out]	w	The float&amp; to process.
		 **************************************************************************************************/

		 static void BreakQuat(const FQuat& InQuat, float& x, float& y, float& z, float& w);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Kinect|Math")

		/**************************************************************************************************
		 * Convert quaternion to rotator.
		 *
		 * @author	Leon Rosengarten
		 * @param	InQuat	The in quaternion.
		 *
		 * @return	The quaternion converted to rotator.
		 **************************************************************************************************/

		 static FRotator Conv_QuatToRotator(const FQuat& InQuat);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Kinect|Math")

		/**************************************************************************************************
		 * \fn	static FQuat UKinectFunctionLibrary::Conv_Vector4ToQuat(const FVector4&amp; InVec);
		 *
		 * \brief	Convert vector 4 to quaternion.
		 *
		 * \param	InVec	The in vector.
		 *
		 * \return	The vector converted 4 to quaternion.
		 *
		 * @author	Leon Rosengarten
		 * @param	InVec	The in vector.
		 *
		 * @return	The vector converted 4 to quaternion.
		 **************************************************************************************************/

		 static FQuat Conv_Vector4ToQuat(const FVector4& InVec);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Kinect|Math")

		/**************************************************************************************************
		 * \fn	static FRotator UKinectFunctionLibrary::Vec4QuatToRotator(const FVector4&amp; TheVec);
		 *
		 * \brief	Vector 4 quaternion to rotator.
		 *
		 * \param	TheVec	the vector.
		 *
		 * \return	A FRotator.
		 *
		 * @author	Leon Rosengarten
		 * @param	TheVec	the vector.
		 *
		 * @return	A FRotator.
		 **************************************************************************************************/

		 static FRotator Vec4QuatToRotator(const FVector4& TheVec);

	UFUNCTION(BlueprintCallable, BlueprintPure, Category = "Kinect|Math")

		/**************************************************************************************************
		 * \fn	static FQuat UKinectFunctionLibrary::RotatorToQuat(const FRotator&amp; TheRotator);
		 *
		 * \brief	Rotator to quaternion.
		 *
		 * \param	TheRotator	the rotator.
		 *
		 * \return	A FQuat.
		 *
		 * @author	Leon Rosengarten
		 * @param	TheRotator	the rotator.
		 *
		 * @return	A FQuat.
		 **************************************************************************************************/

		 static FQuat	RotatorToQuat(const FRotator& TheRotator);

	UFUNCTION(BlueprintPure, Category = "Kinect", meta = (NativeBreakFunc))

		/**************************************************************************************************
		 * Break body frame.
		 *
		 * @author	Leon Rosengarten
		 *
		 * @param	InBodyFrame   	The in body frame.
		 * @param [in,out]	Bodies	The bodies.
		 **************************************************************************************************/

		 static void BreakBodyFrame(const FBodyFrame& InBodyFrame, TArray<FBody> &Bodies);

	UFUNCTION(BlueprintPure, Category = "Kinect|Body", meta = (NativeBreakFunc))

		/**************************************************************************************************
		 * Break vector 4.
		 *
		 * @author	Leon Rosengarten
		 *
		 * @param	InVector	 	The in vector.
		 * @param [in,out]	X	The float&amp; to process.
		 * @param [in,out]	Y	The float&amp; to process.
		 * @param [in,out]	Z	The float&amp; to process.
		 * @param [in,out]	W	The float&amp; to process.
		 **************************************************************************************************/

		 static void BreakVector4(const FVector4& InVector, float& X, float& Y, float& Z, float& W);

	UFUNCTION(BlueprintPure, Category = "Kinect|Math", meta = (NativeMakeFunc))

		/**************************************************************************************************
		 * Makes vector 4.
		 *
		 * @author	Leon Rosengarten
		 *
		 * @param	X	The X coordinate.
		 * @param	Y	The Y coordinate.
		 * @param	Z	The Z coordinate.
		 * @param	W	The width.
		 *
		 * @return	A FVector4.
		 **************************************************************************************************/

		 static FVector4 MakeVector4(const float& X, const float& Y, const float& Z, const float& W);

	UFUNCTION(BlueprintPure, Category = "Kinect|Math", meta = (NativeBreakFunc))

		/**************************************************************************************************
		 * Break body.
		 *
		 * @author	Leon Rosengarten
		 *
		 * @param	InBody				  	The in body.
		 * @param [in,out]	BoneTransforms	The bone transforms.
		 * @param [in,out]	IsTracked	  	The is tracked.
		 **************************************************************************************************/

		 static void BreakBody(const FBody& InBody, TArray<FTransform> &BoneTransforms, bool &IsTracked);

	UFUNCTION(BlueprintPure, Category = "Kinect|Body", meta = (NativeBreakFunc))

		/**************************************************************************************************
		 * Break kinect bone.
		 *
		 * @author	Leon Rosengarten
		 *
		 * @param	InKinectBone				The in kinect bone.
		 * @param [in,out]	JointTypeStart  	The joint type start.
		 * @param [in,out]	JointTypeEnd		The joint type end.
		 * @param [in,out]	Orientation			The orientation.
		 * @param [in,out]	CameraSpacePoint	The camera space point.
		 * @param [in,out]	TrackingState   	State of the tracking.
		 **************************************************************************************************/

		 static void BreakKinectBone(const FKinectBone& InKinectBone, TEnumAsByte<EJoint::Type> &JointTypeStart,
		 TEnumAsByte<EJoint::Type>& JointTypeEnd,
		 FQuat& Orientation, FVector& CameraSpacePoint, TEnumAsByte<ETrackingState::Type>& TrackingState);

	UFUNCTION(BlueprintPure, Category = "Kinect|Math")

		/**************************************************************************************************
		 * \fn	static FQuat UKinectFunctionLibrary::MultiplyQuat(const FQuat&amp; A, const FQuat&amp;
		 * B);
		 *
		 * \brief	Multiply quaternion.
		 *
		 * \param	A	The const FQuat&amp; to process. \param	B	The const FQuat&amp; to process.
		 *
		 * \return	A FQuat.
		 *
		 * @author	Leon Rosengarten
		 * @param	A	The const FQuat&amp; to process.
		 * @param	B	The const FQuat&amp; to process.
		 *
		 * @return	A FQuat.
		 **************************************************************************************************/

		 static FQuat MultiplyQuat(const FQuat& A, const FQuat& B);

	UFUNCTION(BlueprintPure, Category = "Kinect|Math")

		/**************************************************************************************************
		 * \fn	static void UKinectFunctionLibrary::QuatToAxisOutAngle(const FQuat&amp; InQuat,
		 * FVector&amp; OutAxis, float &amp;OutAngle);
		 *
		 * \brief	Quaternion to axis out angle.
		 *
		 * \param	InQuat				The in quaternion. \param [in,out]	OutAxis 	The out axis. \param [in,out]
		 * OutAngle	The out angle.
		 *
		 * @author	Leon Rosengarten
		 * @param	InQuat				The in quaternion.
		 * @param [in,out]	OutAxis 	The out axis.
		 * @param [in,out]	OutAngle	The out angle.
		 **************************************************************************************************/

		 static void QuatToAxisOutAngle(const FQuat& InQuat, FVector& OutAxis, float &OutAngle);

	UFUNCTION(BlueprintPure, Category = "Kinect|Math")

		/**************************************************************************************************
		 * \fn	static float UKinectFunctionLibrary::DegreesToRadians(const float &amp;InDegrees);
		 *
		 * \brief	Degrees to radians.
		 *
		 * \param	InDegrees	The in in degrees.
		 *
		 * \return	A float.
		 *
		 * @author	Leon Rosengarten
		 * @param	InDegrees	The in in degrees.
		 *
		 * @return	A float.
		 **************************************************************************************************/

		 static float DegreesToRadians(const float &InDegrees);

	UFUNCTION(BlueprintPure, Category = "Kinect|Math")

		/**************************************************************************************************
		 * \fn	static float UKinectFunctionLibrary::RadiansToDegrees(const float &amp;InRadians);
		 *
		 * \brief	Radians to degrees.
		 *
		 * \param	InRadians	The in in radians.
		 *
		 * \return	A float.
		 *
		 * @author	Leon Rosengarten
		 * @param	InRadians	The in in radians.
		 *
		 * @return	A float.
		 **************************************************************************************************/

		 static float RadiansToDegrees(const float &InRadians);

	UFUNCTION(BlueprintPure, Category = "Kinect|Body")

		/**************************************************************************************************
		 * Gets the parent of this item.
		 *
		 * @author	Leon Rosengarten
		 *
		 * @param	InBone	The in bone.
		 *
		 * @return	The bone parent.
		 **************************************************************************************************/

		 static TEnumAsByte<EJoint::Type> GetBoneParent(const TEnumAsByte<EJoint::Type> &InBone);

	UFUNCTION(BlueprintPure, Category = "Kinect|Math")

		/**************************************************************************************************
		 * Inverse transform.
		 *
		 * @author	Leon Rosengarten
		 * @param	InTransform	The in transform.
		 *
		 * @return	A FTransform.
		 **************************************************************************************************/

		 static FTransform InverseTransform(const FTransform& InTransform);

	UFUNCTION(BlueprintPure, Category = "Kinect|Math")
		static FRotator ConvertRotatorToLocal(const FRotator& WorldRotator, const FTransform& ParentTransform);


	UFUNCTION(BlueprintCallable, Category = "Kinect|Math")

		/**************************************************************************************************
		 * Inverse quaternion.
		 *
		 * @author	Leon Rosengarten
		 * @param	InQuat	The in quaternion.
		 *
		 * @return	A FQuat.
		 **************************************************************************************************/

		 static FQuat InverseQuat(const FQuat& InQuat);

	UFUNCTION(BlueprintCallable, Category = "Kinect|Math")

		/**************************************************************************************************
		 * Enables the body joystick.
		 *
		 * @author	Leon Rosengarten
		 * @param	Enable	The enable.
		 **************************************************************************************************/

		 static void EnableBodyJoystick(const bool& Enable);

	UFUNCTION(BlueprintCallable, Category = "Kinect|CoordianteMapper")

		/**************************************************************************************************
		 * Convert body point to screen point.
		 *
		 * @author	Leon Rosengarten
		 *
		 * @param	BodyPoint  	The body point.
		 * @param	ScreenSizeX	The screen size x coordinate.
		 * @param	ScreenSizeY	The screen size y coordinate.
		 *
		 * @return	The body converted point to screen point.
		 **************************************************************************************************/

		 static FVector2D ConvertBodyPointToScreenPoint(const FVector& BodyPoint, int32 ScreenSizeX, int32 ScreenSizeY);


	UFUNCTION(BlueprintPure, Category = "Kinect|CoordianteMapper")

		/**************************************************************************************************
		 * Mirror kinect skeleton.
		 *
		 * @author	Leon Rosengarten
		 *
		 * @param	BodyToMirror	The body to mirror.
		 *
		 * @return	A TArray&lt;FTransform&gt;
		 **************************************************************************************************/

		 static TArray<FTransform> MirrorKinectSkeleton(const FBody& BodyToMirror, float PosLocScale = 1.f);


	UFUNCTION(BlueprintPure, Category = "Kinect|Math")

		/**************************************************************************************************
		 * A kinect event manager*.
		 *
		 * @author	Leon Rosengarten
		 **************************************************************************************************/

		 static class UKinectEventManager* GetKinectEventManager();


	UFUNCTION(BlueprintCallable, Category = "Kinect")

		/**********************************************************************************************//**
		 * Starts a sensor.
		 *
		 * @author	Leon Rosengarten
		 * @date	26-13-2015
		 **************************************************************************************************/

		static void StartSensor();

	UFUNCTION(BlueprintCallable, Category = "Kinect")

		/**********************************************************************************************//**
		 * Shutdown sensor.
		 *
		 * @author	Leon Rosengarten
		 * @date	26-13-2015
		 **************************************************************************************************/

		static void ShutdownSensor();
		
	
	UFUNCTION(BlueprintCallable, Category = "Kinect|Filters")

		/**********************************************************************************************//**
		 * Gets smoothed joint.
		 *
		 * @author	Leon
		 * @date	26-Aug-15
		 *
		 * @param [in,out]	InFilter	A filter specifying the in.
		 * @param	InBody				The in body.
		 *
		 * @return	The smoothed joint.
		 **************************************************************************************************/

		static FBody GetSmoothedJoint(UPARAM(ref) FBoneOrientationDoubleExponentialFilter& InFilter, const FBody& InBody);


	UFUNCTION(BlueprintCallable, Category = "Kinect")
		static class UTexture2D* CreateStreamTexture(EKinectStreamType StreamType);

	UFUNCTION(BlueprintCallable, Category = "Kinect")
		static UTexture2D* MapColorFrameToDepthSpace(UPARAM(ref) UTexture2D* InTexture, UPARAM(ref) UTexture2D* DepthTexture);


private:

	friend class FKinectV2InputDevice;

	static FEnableBodyJoystick EnableBodyJoystickEvent; ///< The enable body joystick event

	static FMapBodyCoordToScreenCoord MapBodyCoordToScreenCoordEvent;   ///< The map body coordinate to screen coordinate event

	static FGetKinectManegerEvent GetKinectManagerEvent;	///< The get kinect manager event

	static FGetKinectInputDevice GetKinectInputDeviceEvent;

	static FStartSensorEvent StartSensorEvent;

	static FShutdownSensorEvent ShutdownSensorEvent;

	static FMapColorFrameToDepthSpace MapColorFrameToDepthSpaceEvent;

	/**************************************************************************************************
	 * \property	static TMap&lt;TEnumAsByte&lt;EJoint::Type&gt;, TEnumAsByte&lt;EJoint::Type&gt;&gt;
	 * BoneMap
	 *
	 * \brief	Gets the bone map.
	 *
	 * \return	The bone map.
	 *
	 * @return	The bone map.
	 **************************************************************************************************/

	static TMap<TEnumAsByte<EJoint::Type>, TEnumAsByte<EJoint::Type>> BoneMap;


	/**************************************************************************************************
	 * Gets dir vector.
	 *
	 * @author	Leon Rosengarten
	 * @param	From	Source for the.
	 * @param	To  	to.
	 *
	 * @return	The dir vector.
	 **************************************************************************************************/

	static FVector GetDirVec(const FVector& From, const FVector& To);

	static TSharedPtr<class KinectSkeletonOneEuroFilter> KinectBoneFilter;

	//static FBodyFrameEvent BodyFrameEvent;

};