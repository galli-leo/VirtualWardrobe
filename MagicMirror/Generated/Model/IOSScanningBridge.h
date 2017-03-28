/**
 * Project Untitled
 */


#ifndef _IOSSCANNINGBRIDGE_H
#define _IOSSCANNINGBRIDGE_H

class IOSScanningBridge {
public: 
    IOSScanningProvider scanningProvider;
    AVCaptureSession captureSession;
    
    /**
     * This will also use CIImage for Canny Edge, which is really fast, since it's on the GPU
     * 
     * @param sampleBuffer
     */
    void didOutputSampleBuffer(CMSampleBuffer sampleBuffer);
};

#endif //_IOSSCANNINGBRIDGE_H