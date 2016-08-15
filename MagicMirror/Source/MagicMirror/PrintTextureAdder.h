#pragma once
#ifndef PRINTTEXTURE_H
#define PRINTTEXTURE_H
#include "MagicMirror.h"
//#include "Structs.h"
//#include "WardrobeManager.h"
//#include "PythonUtils.h"

class UWardrobeManager;

//~~~~~ Multi Threading ~~~
class FPrintTextureCreator : public FRunnable
{
public:
	/** Singleton instance, can access the thread any time via static accessor, if it is active! */
	static  FPrintTextureCreator* PrintCreator;

	/** Thread to run the worker FRunnable on */
	FRunnableThread* Thread;

	/** Stop this thread? Uses Thread Safe Counter */
	FThreadSafeCounter StopTaskCounter;

	/** The Wardrobe Manager containing the Delegate*/
	UWardrobeManager* manager;

	/**ID of clothing item to add print texture to*/
	uint32 id;

private:
	UTexture2D* finalTexture;

	UTexture2D* LoadImageFromFile(FString file);
public:

	//Done?
	bool IsFinished = false;

	//~~~ Thread Core Functions ~~~

	//Constructor / Destructor
	FPrintTextureCreator(UWardrobeManager* manager, uint32 id);
	virtual ~FPrintTextureCreator();

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
	static FPrintTextureCreator* JoyInit(UWardrobeManager* manager, uint32 id);

	/** Shuts down the thread. Static so it can easily be called from outside the thread context */
	static void Shutdown();

	static bool IsThreadFinished();

};

#endif