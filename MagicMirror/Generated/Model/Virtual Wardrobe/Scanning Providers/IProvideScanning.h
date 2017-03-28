/**
 * Project Untitled
 */


#ifndef _IPROVIDESCANNING_H
#define _IPROVIDESCANNING_H

class IProvideScanning {
public: 
    int32 Width;
    int32 Height;
    bool bReturnsCanny;
    ScanningManager OwningScanningManager;
    
    void StartScanning();
    
    void StopScanning();
};

#endif //_IPROVIDESCANNING_H