#pragma once
#ifndef PYTHONLOADING_H
#define PYTHONLOADING_H
#include "MagicMirror.h"
#include "WardrobeManager.h"
//#include "Structs.h"
//#include "WardrobeManager.h"
//#include "PythonUtils.h"

class UWardrobeManager;

//~~~~~ Multi Threading ~~~
class FPythonLoading : public FRunnable
{
public:
	/** Singleton instance, can access the thread any time via static accessor, if it is active! */
	static  FPythonLoading* Runnable;

	/** Thread to run the worker FRunnable on */
	FRunnableThread* Thread;

	/** Stop this thread? Uses Thread Safe Counter */
	FThreadSafeCounter StopTaskCounter;

	/** The Wardrobe Manager containing the Delegate*/
	UWardrobeManager* manager;

	FString dir;

private:
#if PLATFORM_WINDOWS
	PyGILState_STATE gstate;
#endif

public:

	int32				TotalPrimesToFind;

	//Done?
	bool IsFinished = false;

	//~~~ Thread Core Functions ~~~

	//Constructor / Destructor
	FPythonLoading(UWardrobeManager* manager, FString theDir);
	virtual ~FPythonLoading();

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
	static FPythonLoading* JoyInit(UWardrobeManager* manager, FString theDir);

	/** Shuts down the thread. Static so it can easily be called from outside the thread context */
	static void Shutdown();

	static bool IsThreadFinished();

};

#endif
