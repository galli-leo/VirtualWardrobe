//------------------------------------------------------------------------------
// 
//     The Kinect for Windows APIs used here are preliminary and subject to change
// 
//------------------------------------------------------------------------------
// 
#pragma once

#include "InputDevice.h"


struct EKinectKeys
{
	static const FKey KinectBodyJoystickX;
	static const FKey KinectBodyJoystickY;
	static const FKey KinectLeftHandPos;
	static const FKey KinectRightHandPos;

};


/**************************************************************************************************
 * Defines an alias representing the kinect v 2 input device pointer.
 **************************************************************************************************/

typedef TSharedPtr<class FKinectV2InputDevice> FKinectV2InputDevicePtr;

/**************************************************************************************************
 * Defines an alias representing the kinect v 2 input device reference.
 **************************************************************************************************/

typedef TSharedRef<class FKinectV2InputDevice> FKinectV2InputDeviceRef;

/**************************************************************************************************
 * A kinect v 2 input device.
 *
 * @author	Leon Rosengarten
 **************************************************************************************************/

class KINECTV2_API FKinectV2InputDevice : public IInputDevice, public TSharedFromThis<FKinectV2InputDevice>, public FSelfRegisteringExec
{

public:

	/**************************************************************************************************
	 * Ticks.
	 *
	 * @author	Leon Rosengarten
	 * @param	DeltaTime	The delta time.
	 **************************************************************************************************/

	virtual void Tick(float DeltaTime) override;

	/**************************************************************************************************
	 * Sends the controller events.
	 *
	 * @author	Leon Rosengarten
	 **************************************************************************************************/

	virtual void SendControllerEvents() override;

	/**************************************************************************************************
	 * Handler, called when the set message.
	 *
	 * @author	Leon Rosengarten
	 * @param	InMessageHandler	The in message handler.
	 **************************************************************************************************/

	virtual void SetMessageHandler(const TSharedRef< class FGenericApplicationMessageHandler >& InMessageHandler) override;

	/**************************************************************************************************
	 * Execs.
	 *
	 * @author	Leon Rosengarten
	 * @param [in,out]	InWorld	If non-null, the in world.
	 * @param	Cmd			   	The command.
	 * @param [in,out]	Ar	   	The archive.
	 *
	 * @return	true if it succeeds, false if it fails.
	 **************************************************************************************************/

	virtual bool Exec(UWorld* InWorld, const TCHAR* Cmd, class FOutputDevice& Ar) override;

	/**************************************************************************************************
	 * Sets channel value.
	 *
	 * @author	Leon Rosengarten
	 * @param	ControllerId	Identifier for the controller.
	 * @param	ChannelType 	Type of the channel.
	 * @param	Value			The value.
	 **************************************************************************************************/

	virtual void SetChannelValue(int32 ControllerId,  FForceFeedbackChannelType ChannelType, float Value) override;

	/**************************************************************************************************
	 * Sets channel values.
	 *
	 * @author	Leon Rosengarten
	 * @param	ControllerId	Identifier for the controller.
	 * @param	values			The values.
	 **************************************************************************************************/

	virtual void SetChannelValues(int32 ControllerId, const FForceFeedbackValues &values) override;

	/**************************************************************************************************
	 * Destructor.
	 *
	 * @author	Leon Rosengarten
	 **************************************************************************************************/

	virtual ~FKinectV2InputDevice();

	/**************************************************************************************************
	 * Enables the body joystick.
	 *
	 * @author	Leon Rosengarten
	 * @param	enable	The enable.
	 **************************************************************************************************/

	virtual void EnableBodyJoystick(const bool& enable);

	

private:

	friend class FKinectV2Plugin;

	friend class UVisualGestureImporterFactory;

	friend class UVisualGestureDetector;
	/**************************************************************************************************
	 * Constructor.
	 *
	 * @author	Leon Rosengarten
	 * @param	MessageHandler	The message handler.
	 **************************************************************************************************/

	FKinectV2InputDevice(const TSharedRef< FGenericApplicationMessageHandler >& MessageHandler);


	/**************************************************************************************************
	 * A kinect event manager*.
	 *
	 * @author	Leon Rosengarten
	 **************************************************************************************************/

	virtual class UKinectEventManager* GetKinectManeger() const;

	virtual FKinectV2InputDevice* GetKinectInputDevice();

	bool Active;	///< true to active

	bool BodyJoystickEnabled;   ///< true to enable, false to disable the body joystick

	TSharedRef< FGenericApplicationMessageHandler > MessageHandler; ///< The message handler

	/**************************************************************************************************
	 * A texture 2 d*.
	 *
	 * @author	Leon Rosengarten
	 **************************************************************************************************/

	class UTexture2D* ColorFrame;

	/**************************************************************************************************
	 * A texture 2 d*.
	 *
	 * @author	Leon Rosengarten
	 **************************************************************************************************/

	class UTexture2D* InfraredFrame;

	/**************************************************************************************************
	 * A texture 2 d*.
	 *
	 * @author	Leon Rosengarten
	 **************************************************************************************************/

	class UTexture2D* DepthFrame;

	
	class UTexture2D* BodyIndexFrame;


	TSharedPtr<class FKinectSensor> KinectSensor;   ///< The kinect sensor

	TSharedPtr<class KinectSkeletonOneEuroFilter> KinectOneEuroFilter;  ///< A filter specifying the kinect one euro

	/**************************************************************************************************
	 * A kinect event manager*.
	 *
	 * @author	Leon Rosengarten
	 **************************************************************************************************/

	class UKinectEventManager* KinectManeger = nullptr;

};