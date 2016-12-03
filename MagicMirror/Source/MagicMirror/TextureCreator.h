#pragma once
#ifndef TEXTURECREATOR_H
#define TEXTURECREATOR_H
#include "MagicMirror.h"
#include "WardrobeManager.h"
//#include "Structs.h"
//#include "WardrobeManager.h"
//#include "PythonUtils.h"

class UWardrobeManager;

//~~~~~ Multi Threading ~~~
class FTextureCreator : public FRunnable
{
public:
	/** Singleton instance, can access the thread any time via static accessor, if it is active! */
	static  FTextureCreator* Runnable;

	/** Thread to run the worker FRunnable on */
	FRunnableThread* Thread;

	/** Stop this thread? Uses Thread Safe Counter */
	FThreadSafeCounter StopTaskCounter;

	/** The Wardrobe Manager containing the Delegate*/
	UWardrobeManager* manager;

	FCategory category;

private:
	UTexture2D* finalTexture;
	PyGILState_STATE gstate;
	UTexture2D* LoadImageFromFile(FString file);
public:

	int32				TotalPrimesToFind;

	//Done?
	bool IsFinished = false;

	//~~~ Thread Core Functions ~~~

	//Constructor / Destructor
	FTextureCreator(UWardrobeManager* manager, FCategory theCat);
	virtual ~FTextureCreator();

	// Begin FRunnable interface.
	virtual bool Init();
	virtual uint32 Run();
	virtual void Stop();
	// End FRunnable interface

	/** Makes sure this thread has stopped properly */
	void EnsureCompletion();



	//~~~ Starting and Stopping Thread ~~~



	/*
	Start the thread and the worker from static (easy access)!
	This code ensures only 1 Prime Number thread will be able to run at a time.
	This function returns a handle to the newly started instance.
	*/
	static FTextureCreator* JoyInit(UWardrobeManager* manager, FCategory theCat);

	/** Shuts down the thread. Static so it can easily be called from outside the thread context */
	static void Shutdown();

	static bool IsThreadFinished();

};

#endif