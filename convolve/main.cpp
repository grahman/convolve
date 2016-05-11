//
//  main.cpp
//  WriteSamplesToAudioFile
//
//  Created by Graham Barab on 3/16/16.
//  Copyright © 2016 Graham Barab. All rights reserved.
//

#include <iostream>
#include <AudioToolbox/AudioToolbox.h>
#include <AudioUnit/AudioUnit.h>
#include "PublicUtility/CAXException.h"
#include "PublicUtility/CAStreamBasicDescription.h"
#include "PublicUtility/CAAudioUnit.h"
#include "CAUtilityFunctions.h"
#include "AudioUtil.h"
#include "AudioBuffer.h"
#include <cstdlib>
#include <stdlib.h>
#include "SlowConvolve.h"
#include "FastConvolve.h"
#include "converter.h"
#include "gmbutil.h"

using namespace std;

struct SimpleSignal {
    double *data;
    unsigned int N;
    unsigned int i;
};

/* convolveFilesSlowly() uses direct form convolution, extremely slow but useful for verifying results of FFT convolution */
void convolveFilesSlowly(const char *impulsePath, const char *signalPath, const char *outpath);

/* convolveFiles() uses FFT convolution -- this should be the default usually */
void convolveFiles(const char *impulsePath, const char *signalPath, const char *outpath);

static double *BUF;
static unsigned int BUF_SIZE;
int main(int argc, const char * argv[]) {
    // insert code here...
    if (argc != 5) {
        cerr << "Usage: convolve <impulseResponse> <signal> <outFileName>" << endl;
        exit(1);
    }
    
    const char *impulsePath = argv[1];
    const char *signalPath = argv[2];
    const char *outputPath = argv[3];
    CAStreamBasicDescription asbd;
    
    if (atoi(argv[4]) == 0) {
        convolveFilesSlowly(impulsePath, signalPath, outputPath);
    } else if (atoi(argv[4]) == 1) {
        convolveFiles(impulsePath, signalPath, outputPath);
    }

    return 0;
}



OSStatus copyOutputSimpleSignal(AudioConverterRef              converterRef,
                                UInt32                         *ioNumberDataPackets,
                                AudioBufferList                *ioData,
                                AudioStreamPacketDescription   **outDataPacketDescription,
                                void                           *inUserData)
{
    struct SimpleSignal * sig = (struct SimpleSignal *)inUserData;
    unsigned int n = *ioNumberDataPackets;
    unsigned int i = sig->i;
    
    ioData->mBuffers->mData = &sig->data[i];
    sig->i += n;
    
    ioData->mBuffers->mDataByteSize = n * sizeof(double);
    ioData->mBuffers->mNumberChannels = 1;
    return noErr;
}


OSStatus copyOutputFastConvolver(AudioConverterRef              converterRef,
                     UInt32                         *ioNumberDataPackets,
                     AudioBufferList                *ioData,
                     AudioStreamPacketDescription   **outDataPacketDescription,
                     void                           *inUserData)
{
    Gmb::FastConvolver *conv = (Gmb::FastConvolver *)inUserData;
    memset(::BUF, 0, *ioNumberDataPackets * sizeof(double));
    conv->render(::BUF, *ioNumberDataPackets);
    ioData->mBuffers->mData = ::BUF;
    ioData->mBuffers->mDataByteSize = *ioNumberDataPackets * sizeof(double);
    return noErr;
}

void convolveFilesSlowly(const char *impulsePath, const char *signalPath, const char *outpath)
{
    AudioStreamBasicDescription outFormat = {0};
    AudioStreamBasicDescription internalFormat = {0};
    FillOutASBDForLPCM(outFormat, SAMPLERATE, 1, 24, 24, false, false);
    FillOutASBDForLPCM(internalFormat, SAMPLERATE, 1, 64, 64, true, false);
    Gmb::Converter impulse(impulsePath, internalFormat, BUF_SIZE);
    Gmb::Converter signal(signalPath, internalFormat, BUF_SIZE);
    bool swapimpulse = (impulse.n() > signal.n()) ? true : false;
    Gmb::Converter *impulsePtr = swapimpulse ? &signal: &impulse;
    Gmb::Converter *signalPtr = swapimpulse ? &impulse: &signal;
    AudioFileID outAudioFileID;
    
    XThrowIfError(Gmb::createOutputAudioFile(outpath, &outAudioFileID, outFormat), "createOutputAudioFile");
    
    double *y, *h, *x;
    unsigned long ny, nh, nx;
    
    nx = signalPtr->n();
    x = new double[nx];
    if (!x) {
        throw std::exception();
    }
    
    signalPtr->render(x, (unsigned int)nx);

    nh = impulsePtr->n();
    h = new double[nh];
    if (!h) {
        throw std::exception();
    }
    impulsePtr->render(h, (unsigned int)nh);
    ny = nx + nh - 1;
    y = (double *)calloc(ny, sizeof(double));
    if (!y) {
        cerr << "Error: calloc failed" << endl;
        throw std::exception();
    }
    
    AudioConverterRef converterRef;
    XThrowIfError(AudioConverterNew(&internalFormat, &outFormat, &converterRef), "AudioConverterNew");
    AudioBufferList *templist = Gmb::createAudioBufferList(1, (unsigned int)(ny * outFormat.mBytesPerFrame), outFormat.mChannelsPerFrame);
    
    Gmb::convolve_slowly(x, nx, h, nh, y, ny);
    
    /* Read out entire audio file */
    struct SimpleSignal sig;
    sig.data = y;
    sig.i = 0;
    sig.N = (unsigned int)ny;
    
    struct SimpleSignal ksig;
    ksig.data = h;
    ksig.i = 0;
    ksig.N = (unsigned int)nh;
    
    UInt32 numFramesToWrite = (unsigned int)ny;
    XThrowIfError(AudioConverterFillComplexBuffer(converterRef,
                                                  copyOutputSimpleSignal,
                                                  &sig,
                                                  &numFramesToWrite,
                                                  templist,
                                                  NULL), "AudioConverterFillComplexBuffer");
    
    UInt32 numBytesToWrite = numFramesToWrite * outFormat.mBytesPerFrame;
    XThrowIfError(AudioFileWriteBytes(outAudioFileID,
                                      false,
                                      0,
                                      &numBytesToWrite,
                                      (const void *)templist->mBuffers->mData), "AudioFileWriteBytes");
    Gmb::destroyAudioBufferList(templist);
    
}

void convolveFiles(const char *impulsePath, const char *signalPath, const char *outpath)
{
    AudioStreamBasicDescription outFormat = {0};
    AudioStreamBasicDescription internalFormat = {0};
    FillOutASBDForLPCM(outFormat, SAMPLERATE, 1, 24, 24, false, false);
    FillOutASBDForLPCM(internalFormat, SAMPLERATE, 1, 64, 64, true, false);
    Gmb::Converter signal(signalPath, internalFormat, BUF_SIZE);
    Gmb::Converter impulse(impulsePath, internalFormat, BUF_SIZE);

    bool swapimpulse = (impulse.n() > signal.n()) ? true: false;
    UInt32 bufsize = BUF_SIZE;
    UInt32 numSamplesOut = swapimpulse ? impulse.n() : signal.n();
    AudioFileID outAudioFileID;
    
    XThrowIfError(Gmb::createOutputAudioFile(outpath, &outAudioFileID, outFormat), "createOutputAudioFile");
    
    AudioConverterRef converterRef;
    double *h;
    unsigned long nh;
    
    nh = (swapimpulse) ? signal.n(): impulse.n();
    h = new double[nh];
    if (!h)
        throw std::exception();
    
    if (swapimpulse)
        signal.render(h, (unsigned int)nh);
    else
        impulse.render(h, (unsigned int)nh);
    
    bufsize = (unsigned int)Gmb::ceilPow2(nh);
    Gmb::AudioModule *upstream = swapimpulse ? &impulse: &signal;
    Gmb::FastConvolver conv(h, (unsigned int)nh, bufsize, upstream);

    SInt64 offset = 0;
    UInt32 bytesWritten;
    UInt32 framesWritten = 0;
    UInt32 framesToWrite;
    UInt32 bytesToWrite;
    XThrowIfError(AudioConverterNew(&internalFormat, &outFormat, &converterRef), "AudioConverterNew");
    AudioBufferList *templist = Gmb::createAudioBufferList(1, bufsize * outFormat.mBytesPerFrame, outFormat.mChannelsPerFrame);
    ::BUF_SIZE = bufsize;
    ::BUF = new double[bufsize];
    if (!::BUF) {
        throw std::exception();
    }
    memset(::BUF, 0, bufsize * internalFormat.mBytesPerFrame);
    while (framesWritten < numSamplesOut) {
        XThrowIfError(AudioConverterFillComplexBuffer(converterRef,
                                                      copyOutputFastConvolver,
                                                      &conv,
                                                      &bufsize,
                                                      templist,
                                                      NULL),
                      "AudioConverterFillComplexBuffer");
        
        bytesWritten = bufsize * outFormat.mBytesPerFrame;
        framesToWrite = MIN(bufsize, (numSamplesOut - framesWritten));
        bytesToWrite = framesToWrite * outFormat.mBytesPerFrame;
        XThrowIfError(AudioFileWriteBytes(outAudioFileID, false, offset, &bytesToWrite, templist->mBuffers->mData), "AudioFileWriteBytes");
        offset += bytesToWrite;
        framesWritten += framesToWrite;
    }

    Gmb::destroyAudioBufferList(templist);
    free(h);
}


