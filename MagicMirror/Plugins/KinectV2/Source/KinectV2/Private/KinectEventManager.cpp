
#include "IKinectV2PluginPCH.h"

#include "SlateBasics.h"

#include "KinectV2InputDevice.h"
#include "AllowWindowsPlatformTypes.h"
#include <windows.h>
#include <winuser.h>

//#pragma comment(lib, "User32.lib" )

UKinectEventManager::UKinectEventManager(const class FObjectInitializer& PCIP)
	: Super(PCIP), MouseInput(nullptr), IsLeftHandTracked(false), IsRightHandTracked(false)
{

}



void UKinectEventManager::ProcessNewBodyFrameEvent(const struct FBodyFrame & NewBodyFrame, TSharedRef<FGenericApplicationMessageHandler> MessageHandler)
{
	
	if (RawBodyFrameEvent.IsBound()){
		RawBodyFrameEvent.Broadcast(NewBodyFrame);
	}
	
	for (int32 i = 0; i < NewBodyFrame.Bodies.Num(); ++i)
	{

		if (NewBodyFrame.Bodies[i].bIsTracked)
		{

			TrackingIds[i] = NewBodyFrame.Bodies[i].TrackingId;

			if (AssignedKinectPlayerController.Contains(NewBodyFrame.Bodies[i].TrackingId))
			{
				
				if (KinectBodyEvent.IsBound()){
					KinectBodyEvent.Broadcast(AssignedKinectPlayerController[NewBodyFrame.Bodies[i].TrackingId], NewBodyFrame.Bodies[i],i);
				}
				
				

				//if (AssignedKinectPlayerController[Body.Bodies[i].TrackingId] == MouseControllerPlayer && ControlMouse){

				//}
			}
			else
			{
				
				if (NewSkeletonDetectedEvent.IsBound())
					NewSkeletonDetectedEvent.Broadcast(NewBodyFrame.Bodies[i],i);
				this->NewSkeletonDetected(NewBodyFrame.Bodies[i]);
				
			}
		}
		else if (TrackingIds[i] != 0){
			
			if (AssignedKinectPlayerController.Contains(TrackingIds[i])){
				this->SkeletonLost(AssignedKinectPlayerController[TrackingIds[i]].GetValue());
				if (SkeletonLostEvent.IsBound())
				{
					SkeletonLostEvent.Broadcast(AssignedKinectPlayerController[TrackingIds[i]].GetValue());
				}
				AssignedKinectPlayerController.Remove(TrackingIds[i]);
				TrackingIds[i] = 0;
			}
			
		}

	}
}

void UKinectEventManager::AssigneSkeletonToPlayer(const FBody& Body, TEnumAsByte<EAutoReceiveInput::Type> Player, bool SetAsMouseController)
{

	if (Player.GetValue() != EAutoReceiveInput::Disabled && Player.GetValue() < EAutoReceiveInput::Player6)
	{

		AssignedKinectPlayerController.Add(Body.TrackingId, Player.GetValue());
		if (SetAsMouseController)
			MouseControllerPlayer = Player;
	}
}

void UKinectEventManager::BeginDestroy(){

	Super::BeginDestroy();

}

void UKinectEventManager::EnableMouseControl(bool MouseControl)
{
	ControlMouse = MouseControl;
}

#include "HideWindowsPlatformTypes.h"
