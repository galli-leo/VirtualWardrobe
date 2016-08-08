//------------------------------------------------------------------------------
// 
//     The Kinect for Windows APIs used here are preliminary and subject to change
// 
//------------------------------------------------------------------------------

#include "IKinectV2PluginPCH.h"
#include "KinectV2InputDevice.h"
//#include "KinectListener.h"
#include "KinectSensor.h"
#include "AllowWindowsPlatformTypes.h"
#include <WinUser.h>
#include "HideWindowsPlatformTypes.h"
//TArray<FKinectListenerObject*> FKinectListenerObject::KinectListenersObjects;

#define LOCTEXT_NAMESPACE "KinectV2InputDevice"

const FKey EKinectKeys::KinectBodyJoystickX("KinectBodyJoystickX");
const FKey EKinectKeys::KinectBodyJoystickY("KinectBodyJoystickY");
const FKey EKinectKeys::KinectLeftHandPos("KinectLeftHandPos");
const FKey EKinectKeys::KinectRightHandPos("KinectRightHandPos");


#if PLATFORM_WINDOWS
HMODULE FaceHandle;
#endif


class KINECTV2_API FKinectV2Plugin : public IKinectV2Plugin
{


	

	virtual TSharedPtr< class IInputDevice > CreateInputDevice(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler);
	//virtual TSharedPtr<class IInputDevice > CreateCustomHardwareInput();
	//TSharedPtr<class KinectSensor> m_pKinectSensor;

	virtual void ShutdownModule() override;


	virtual void StartupModule() override;

};

IMPLEMENT_MODULE(FKinectV2Plugin, KinectV2)
/*
TSharedPtr< class IInputDevice > FKinectV2Plugin::CreateCustomHardwareInput()
{

TSharedPtr< FKinectV2CHI > KinectV2CHI(new FKinectV2CHI());

return KinectV2CHI;
}
*/
TSharedPtr< class IInputDevice > KINECTV2_API FKinectV2Plugin::CreateInputDevice(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler)
{
	return MakeShareable(new FKinectV2InputDevice(InMessageHandler));

}


void FKinectV2Plugin::ShutdownModule()
{

	IKinectV2Plugin::ShutdownModule();
	FreeLibrary(FaceHandle);
	//if (KinectSensor.)
	//FKinectSensor::Get().Stop();

}

void FKinectV2Plugin::StartupModule()
{
	IKinectV2Plugin::StartupModule();
	IConsoleManager::Get().RegisterConsoleCommand(TEXT("sensoron"), TEXT("Turn on kinect sensor"));
#if PLATFORM_WINDOWS
	FString LibRootPath = FPaths::EnginePluginsDir() / TEXT("Runtime/KinectV2/Source/ThirdParty/Redist/Face/x64/");

	FaceHandle = LoadLibraryW(*(LibRootPath + "Kinect20.Face.dll"));

#endif
}


FKinectV2InputDevice::FKinectV2InputDevice(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler) :
				Active(true), 
				MessageHandler(InMessageHandler), 
				BodyJoystickEnabled(false), 
				ColorFrame(nullptr),
				InfraredFrame(nullptr),
				DepthFrame(nullptr),
				KinectManeger(nullptr)
{

	UKinectFunctionLibrary::EnableBodyJoystickEvent.BindRaw(this, &FKinectV2InputDevice::EnableBodyJoystick);
	
	ColorFrame = UTexture2D::CreateTransient(1920, 1080);
	InfraredFrame = UTexture2D::CreateTransient(512, 424);
	DepthFrame = UTexture2D::CreateTransient(512, 424);
	BodyIndexFrame = UTexture2D::CreateTransient(512, 424);

	if (ColorFrame){

		ColorFrame->AddToRoot();
		ColorFrame->UpdateResource();

	}

	if (InfraredFrame){
		InfraredFrame->AddToRoot();
		InfraredFrame->UpdateResource();
	}
	if (DepthFrame){
		DepthFrame->AddToRoot();
		DepthFrame->UpdateResource();
	}

	if (BodyIndexFrame)
	{
		BodyIndexFrame->AddToRoot();
		BodyIndexFrame->UpdateResource();
	}

	KinectSensor = TSharedPtr<FKinectSensor>(new FKinectSensor());

	UKinectFunctionLibrary::StartSensorEvent.BindRaw(KinectSensor.Get(), &FKinectSensor::StartSensor);
	UKinectFunctionLibrary::ShutdownSensorEvent.BindRaw(KinectSensor.Get(), &FKinectSensor::ShutDownSensor);
	UKinectFunctionLibrary::MapBodyCoordToScreenCoordEvent.BindRaw(KinectSensor.Get(), &FKinectSensor::BodyToScreen);
	UKinectFunctionLibrary::MapColorFrameToDepthSpaceEvent.BindRaw(KinectSensor.Get(), &FKinectSensor::MapColorFrameToDepthSpace);

	UClass* temp = UKinectEventManager::StaticClass();

	if (temp)
		KinectManeger = NewObject<UKinectEventManager>(temp);

	if (KinectManeger){
		KinectManeger->AddToRoot();
	}

	EKeys::AddKey(FKeyDetails(EKinectKeys::KinectBodyJoystickX, LOCTEXT("KinectBodyJoystickX", "Kinect Body Joystick X"), FKeyDetails::FloatAxis));
	EKeys::AddKey(FKeyDetails(EKinectKeys::KinectBodyJoystickY, LOCTEXT("KinectBodyJoystickY", "Kinect Body Joystick Y"), FKeyDetails::FloatAxis));
	EKeys::AddKey(FKeyDetails(EKinectKeys::KinectLeftHandPos, LOCTEXT("KinectLeftHandPos", "Kinect Left Hand Pos"), FKeyDetails::VectorAxis));
	EKeys::AddKey(FKeyDetails(EKinectKeys::KinectRightHandPos, LOCTEXT("KinectRightHandPos", "Kinect Right Hand Pos"), FKeyDetails::VectorAxis));

	UKinectFunctionLibrary::GetKinectManagerEvent.BindRaw(this, &FKinectV2InputDevice::GetKinectManeger);
	UKinectFunctionLibrary::GetKinectInputDeviceEvent.BindRaw(this, &FKinectV2InputDevice::GetKinectInputDevice);
}

FKinectV2InputDevice::~FKinectV2InputDevice(){


	if (KinectManeger)
	{
		KinectManeger->RemoveFromRoot();
	}

	if (ColorFrame){
		ColorFrame->RemoveFromRoot();
	}
	if (DepthFrame)
	{
		DepthFrame->RemoveFromRoot();
	}
	if (InfraredFrame)
	{
		InfraredFrame->RemoveFromRoot();
	}

	if (BodyIndexFrame)
	{
		BodyIndexFrame->RemoveFromRoot();
	}

	if (KinectSensor.IsValid()){
		KinectSensor->Stop();
	}
	//	FKinectSensor::Get().Stop();

}

void FKinectV2InputDevice::EnableBodyJoystick(const bool& enable){

	BodyJoystickEnabled = enable;

}

void FKinectV2InputDevice::Tick(float dt){

}

/** Poll for controller state and send events if needed */
void FKinectV2InputDevice::SendControllerEvents(){

	if (KinectSensor.IsValid() && KinectSensor->IsRunning())
	{
	
		FBodyFrame Frame;
		bool newBodyFrame = KinectSensor->GetBodyFrame(Frame);
		bool newColorFrame = false;
		bool newInfraRedFrame = false;
		bool newDepthFrame = false;

		if (KinectManeger){

			if (newBodyFrame){
				KinectManeger->ProcessNewBodyFrameEvent(Frame, MessageHandler);
				if (KinectManeger->GetControlMouse())
				{
					MessageHandler->OnMouseMove();
				}
			}
			if (KinectManeger->OnNewKinectColorFrame.IsBound()){
				KinectSensor->UpdateColorTexture(ColorFrame);
				KinectManeger->OnNewKinectColorFrame.Broadcast(ColorFrame);
			}

			if (KinectManeger->OnNewKinectDepthFrame.IsBound())
			{
				KinectSensor->UpdateDepthFrameTexture(DepthFrame);
				KinectManeger->OnNewKinectDepthFrame.Broadcast(DepthFrame);

			}

			if (KinectManeger->OnNewKinectInfraredFrame.IsBound())
			{
				KinectSensor->UpdateInfraredTexture(InfraredFrame);
				KinectManeger->OnNewKinectInfraredFrame.Broadcast(InfraredFrame);
			}

			if (KinectManeger->OnBodyIndexFrameEvent.IsBound())
			{
				KinectSensor->UpdateBodyIndexTexture(BodyIndexFrame);
				KinectManeger->OnBodyIndexFrameEvent.Broadcast(BodyIndexFrame);
			}
		}
	}
}

/** Set which MessageHandler will get the events from SendControllerEvents. */
void FKinectV2InputDevice::SetMessageHandler(const TSharedRef< FGenericApplicationMessageHandler >& InMessageHandler) {

	MessageHandler = InMessageHandler;

}

/** Exec handler to allow console commands to be passed through for debugging */
bool FKinectV2InputDevice::Exec(UWorld* InWorld, const TCHAR* Cmd, FOutputDevice& Ar){
	

	if (FParse::Command(&Cmd, TEXT("kinect")))
	{
		if (FParse::Command(&Cmd, TEXT("on")) || FParse::Command(&Cmd, TEXT("1")))
		{
			KinectSensor->StartSensor();
		}
		else if (FParse::Command(&Cmd, TEXT("off")) || FParse::Command(&Cmd, TEXT("0")))
		{
			KinectSensor->ShutDownSensor();
		}
		

	}

	return true;

}

/**
* IForceFeedbackSystem pass through functions
*/
void FKinectV2InputDevice::SetChannelValue(int32 ControllerId, FForceFeedbackChannelType ChannelType, float Value){

}
void FKinectV2InputDevice::SetChannelValues(int32 ControllerId, const FForceFeedbackValues &values){

}

class UKinectEventManager* FKinectV2InputDevice::GetKinectManeger() const
{
	return KinectManeger;
}

FKinectV2InputDevice* FKinectV2InputDevice::GetKinectInputDevice() 
{
	return this;
}
