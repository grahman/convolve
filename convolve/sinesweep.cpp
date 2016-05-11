//
//  sinesweep.cpp
//  WriteSamplesToAudioFile
//
//  Created by Graham Barab on 3/17/16.
//  Copyright © 2016 Graham Barab. All rights reserved.
//

#include "sinesweep.h"
#include <cmath>
#include "AudioUtil.h"
#include "fft.h"
#include "gmbutil.h"

using namespace Gmb;
Gmb::SweepInfo::SweepInfo(double samplerate, double frequency1, double frequency2, double duration)
{
    this->omega1 = 2.0 * M_PI * frequency1;
    this->omega2 = 2.0 * M_PI * frequency2;
    this->i = 0;
    this->duration = duration;
    this->M = samplerate * duration;
    this->N = (unsigned int)ceilPow2(this->M);
    this->sr = samplerate;
    this->denom = logf(this->omega2 / this->omega1);
    this->pos = 0;
}

void Gmb::SweepInfo::reset()
{
    this->i = 0;
    this->pos = 0;
}

OSStatus Gmb::provideSineSweepToConverter(AudioConverterRef              converterRef,
                                     UInt32                         *ioNumberDataPackets,
                                     AudioBufferList                *ioData,
                                     AudioStreamPacketDescription   **outDataPacketDescription,
                                     void                           *inUserData)
{
    Gmb::Environment *env = (Gmb::Environment *)inUserData;
    struct Gmb::SweepInfo *swp = (struct Gmb::SweepInfo *)env->sweepInfo;
    AudioBuffer *buffer = env->bufferLists[0]->mBuffers;
    double *out = (double *)buffer->mData;
    double xn;
 
    unsigned int n = *ioNumberDataPackets;
    
    while (n--) {
        if (swp->i >= swp->M) {
            *out++ = 0.0001;    /* Silence */
        } else {
            *out++ = xn = sin( ((swp->duration * swp->omega1) / swp->denom) * (exp((swp->i/ (double)swp->M) * swp->denom) - 1) );
        }
        
        ++swp->i;
    }
    
    ioData->mNumberBuffers = 1;
    ioData->mBuffers->mDataByteSize = *ioNumberDataPackets * sizeof(double);
    ioData->mBuffers->mData = buffer->mData;
    
    return noErr;
}

void Gmb::createSweepSignal(struct SweepInfo *swp, double *outSamples, unsigned int numSamples)
{
    unsigned int n = numSamples;
    double *out = outSamples;
    while (n--) {
        if (swp->i >= swp->M) {
            *out++ = 0.0001;    /* Silence */
        } else {
            *out++ = sin( ((swp->duration * swp->omega1) / swp->denom) * (exp((swp->i/ (double)swp->M) * swp->denom) - 1) );
        }
        
        ++swp->i;
    }
}


void Gmb::createInverseFilter(struct SweepInfo *swp, double *outSamplesArray, unsigned int numSamples)
{
    double *out = outSamplesArray;
    unsigned int n = numSamples;
    unsigned int startsample =  swp->N - swp->M + (swp->M / 2);
    int m = swp->M;

    while (n--) {
        if ( (n >= startsample) || (m <= 0)) {
            *out++ = 0.0001;    /* Silence */
        } else {
            *out++ = sin( ((swp->duration * swp->omega1) / swp->denom) * (exp((m - 1) / ((double)swp->M) * swp->denom) - 1)) ;
            --m;
        }
        
    }
}