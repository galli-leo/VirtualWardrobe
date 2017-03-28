/**
 * Project Untitled
 */


#ifndef _IOSSCANNINGPROVIDER_H
#define _IOSSCANNINGPROVIDER_H

#include "Virtual Wardrobe/Scanning Providers/IProvideScanning.h"


class IOSScanningProvider: public IProvideScanning {
public: 
    IOSScanningBridge scanningBridge;
    
    /**
     * @param frame
     * @param canny
     */
    void bridgeCallback(RGBQUAD frame, RGBQUAD canny);
};

#endif //_IOSSCANNINGPROVIDER_H