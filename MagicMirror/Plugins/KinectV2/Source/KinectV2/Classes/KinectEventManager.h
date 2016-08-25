#pragma once

#include "Engine/EngineTypes.h"
//#include "KinectBodyFrameListenerInterface.h"

UENUM(BlueprintType)
namespace EKinectPlayer
{
	enum Type
	{
		Player0,
		Player1,
		Player2,
		Player3,
		Player4,
		Player5
	};
}
#include "KinectFunctionLibrary.h"
#include "KinectEventManager.generated.h"



DECLARE_DYNAMIC_MULTICAST_DELEGATE_TwoParams(FNewSkeletonDetectedEvent, const struct FBody&, NewSkeleton,int32,SkeletonIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FSkeletonLostEvent, EAutoReceiveInput::Type, KinectPlayerLost);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNewKinectColorFrameEvent, const class UTexture2D*, ColorFrameTexture);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNewKinectDepthFrameEvent, const class UTexture2D*, DepthFrameTexture);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNewKinectInfraredFrameEvent, const class UTexture2D*, InfraredFrameTexture);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FNewKinectBodyIndexFrameEvent, const class UTexture2D*, BodyIndexFrameTexture);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FKinectBodyEventSigneture, EAutoReceiveInput::Type, KinectPlayer, const struct FBody&, Skeleton,int32,SkeletonIndex);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FKinectRawBodyFrameEvent, const struct FBodyFrame&, BodyFrame);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_OneParam(FKinSkeletonEvent, EKinectPlayer::Type, KinectPlayer);

UCLASS()
class KINECTV2_API UKinectEventManager : public UObject//, public IKinectBodyFrameListenerInterface
{

	GENERATED_UCLASS_BODY()

		//UPROPERTY(BlueprintAssignable, Category = "Kinect")
		//	FSkeletonEvent SkeletonEvent;
public:


	//UKinectEventManager(const class FObjectInitializer& PCIP);

	/**************************************************************************************************
	 * Process the new body frame event described by Body.
	 *
	 * @author	Leon Rosengarten
	 * @param	Body	The body.
	 **************************************************************************************************/

	void ProcessNewBodyFrameEvent(const struct FBodyFrame & NewBodyFrame, TSharedRef<FGenericApplicationMessageHandler> MessageHandler);


	UFUNCTION(BlueprintImplementableEvent)

		/**************************************************************************************************
		 * Creates a new skeleton detected.
		 *
		 * @author	Leon Rosengarten
		 * @param	Skeleton	The skeleton.
		 **************************************************************************************************/

		 void NewSkeletonDetected(const struct FBody& Skeleton);

	UFUNCTION(BlueprintImplementableEvent)

		/**************************************************************************************************
		 * Skeleton lost.
		 *
		 * @author	Leon Rosengarten
		 * @param	KinectPlayer	The kinect player.
		 **************************************************************************************************/

		 void SkeletonLost(EAutoReceiveInput::Type KinectPlayer);

	UFUNCTION(BlueprintCallable, Category = "Kinect")

		/**************************************************************************************************
		 * Assign skeleton to player.
		 *
		 * @author	Leon Rosengarten
		 * @param	Body					The body.
		 * @param	Player					The player.
		 * @param	SetAsMouseController	true to set as mouse controller.
		 **************************************************************************************************/

		 void AssigneSkeletonToPlayer(const struct FBody& Body, TEnumAsByte<EAutoReceiveInput::Type> Player, bool SetAsMouseController);

	UFUNCTION(BlueprintCallable, Category = "Kinect")

		/**************************************************************************************************
		 * Enables the mouse control.
		 *
		 * @author	Leon Rosengarten
		 * @param	MouseControl	true to mouse control.
		 **************************************************************************************************/

		 void EnableMouseControl(bool MouseControl);
	
	UPROPERTY(BlueprintAssignable, Category = "Kinect|EventDispatchers")
		FNewSkeletonDetectedEvent NewSkeletonDetectedEvent; ///< Called when skeleton with new TrackingId Is detected

	UPROPERTY(BlueprintAssignable, Category = "Kinect|EventDispatchers")
		FKinectBodyEventSigneture KinectBodyEvent;  ///< Called when new body frame is generated and the TrackingId of that body is assigned

	UPROPERTY(BlueprintAssignable, Category = "Kinect|EventDispatchers")
		FNewKinectColorFrameEvent OnNewKinectColorFrame;	///< Called when new color frame is generated

	UPROPERTY(BlueprintAssignable, Category = "Kinect|EventDispatchers")
		FNewKinectDepthFrameEvent OnNewKinectDepthFrame;	///< Called when new depth frame is generated

	UPROPERTY(BlueprintAssignable, Category = "Kinect|EventDispatchers")
		FNewKinectInfraredFrameEvent OnNewKinectInfraredFrame;  ///< Called when new IR frame is generated

	UPROPERTY(BlueprintAssignable, Category = "Kinect|EventDispatchers")
		FKinectRawBodyFrameEvent RawBodyFrameEvent; ///< Called for every body frame generated

	UPROPERTY(BlueprintAssignable, Category = "Kinect|EventDispatchers")
		FSkeletonLostEvent SkeletonLostEvent;   ///< Called when a skeleton with assigned TrackingId is no longer tracked

	UPROPERTY(BlueprintAssignable, Category = "Kinect|EventDispatchers")
		FNewKinectBodyIndexFrameEvent	OnBodyIndexFrameEvent; ///< Called when new BodyInfex frame is generated
		
	/**************************************************************************************************
	 * Gets control mouse.
	 *
	 * @author	Leon Rosengarten
	 * @return	true if it succeeds, false if it fails.
	 **************************************************************************************************/

	FORCEINLINE bool GetControlMouse()const { return ControlMouse; }
	//DECLARE_EVENT_TwoParams(UKinectBodyManeger, FSkeletonEvent, const FBody&, TEnumAsByte<EKinectPlayer::Type>);
	//FSkeletonEvent& OnNewSkeleton() { return SkeletonEvent; }

public:

	uint64 TrackingIds[6];  ///< The tracking ids[ 6]

	bool NeedsAssignment[6];	///< true to needs assignment[ 6]

	TEnumAsByte<EAutoReceiveInput::Type> CurrentAssignedPlayerIndex[6]; ///< The current assigned player index[ 6]

	TMap<uint64, TEnumAsByte<EAutoReceiveInput::Type>> AssignedKinectPlayerController;  ///< The assigned kinect player controller

	TMultiMap<TEnumAsByte<EKinectPlayer::Type>, TWeakObjectPtr<class AKinectPlayerController>> KinectPlayerActors;  ///< An enum constant representing the multi map option

protected:

	/**************************************************************************************************
	 * Begins a destroy.
	 *
	 * @author	Leon Rosengarten
	 **************************************************************************************************/

	virtual void BeginDestroy() override;
	///void BroadcastNewSkeleton(const FBody& Body, TEnumAsByte<EKinectPlayer::Type> AssingedPlayer){
	//	SkeletonEvent.Broadcast(Body, AssingedPlayer);
	//}

private:

	bool IsLeftHandTracked;

	bool IsRightHandTracked;

	FVector2D LastRightHandScreenPos;

	FVector2D LastLeftHandSrceenPos;

	FVector2D CurrRightHandScreenPos;

	FVector2D CurrLeftHandScreenPos;


	TEnumAsByte<EAutoReceiveInput::Type> MouseControllerPlayer; ///< The mouse controller player

	bool ControlMouse;  ///< true to control mouse

	INPUT* MouseInput;
};