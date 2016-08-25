//------------------------------------------------------------------------------
// 
//     The Kinect for Windows APIs used here are preliminary and subject to change
// 
//------------------------------------------------------------------------------

#pragma warning(disable:4653)
#pragma once

#include "ModuleManager.h"
#include "IInputDeviceModule.h"

#ifdef _UNICODE
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#endif


//#include "ICustomHardwareInputModule.h"
/**
 * The public interface to this module.  In most cases, this interface is only public to sibling modules 
 * within this plugin.
 */


extern KINECTV2_API class UKinectEventManager*			GKinectManeger;

class KINECTV2_API IKinectV2Plugin : public IInputDeviceModule
{

public:

	/**
	 * Singleton-like access to this module's interface.  This is just for convenience!
	 * Beware of calling this during the shutdown phase, though.  Your module might have been unloaded already.
	 *
	 * @return Returns singleton instance, loading the module on demand if needed
	 */
	static inline IKinectV2Plugin& Get()
	{
		return FModuleManager::LoadModuleChecked< IKinectV2Plugin >("KinectV2");
	}

	/**
	 * Checks to see if this module is loaded and ready.  It is only valid to call Get() if IsAvailable() returns true.
	 *
	 * @return True if the module is loaded and ready to use
	 */
	static inline bool IsAvailable()
	{
		return FModuleManager::Get().IsModuleLoaded( "KinectV2" );
	}
};

DECLARE_LOG_CATEGORY_EXTERN(LogKinectV2Plugin, Log, All);
