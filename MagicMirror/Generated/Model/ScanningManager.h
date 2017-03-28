/**
 * Project Untitled
 */


#ifndef _SCANNINGMANAGER_H
#define _SCANNINGMANAGER_H

class ScanningManager {
public: 
    IProvideScanning scanningProvider;
    RGBQUAD latestFrame;
    RGBQUAD latestCannyEdge;
    float currentFrameRate;
    
    static void Get();
    
    /**
     * @param frame
     */
    void OnReceiveFrame(RGBQUAD frame);
    
    /**
     * @param frame
     * @param canny
     */
    void OnReceiveFrameAndCanny(RGBQUAD frame, RGBQUAD canny);
};

#endif //_SCANNINGMANAGER_H