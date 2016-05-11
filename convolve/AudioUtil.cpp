//
//  AudioUtil.cpp
//  WriteSamplesToAudioFile
//
//  Created by Graham Barab on 3/17/16.
//  Copyright © 2016 Graham Barab. All rights reserved.
//

#include "AudioUtil.h"
#include <iostream>
#include "fft.h"

using namespace Gmb;
using namespace std;

void Gmb::calculateNumPackets(AudioFileID audioFile,
                           AudioStreamBasicDescription inDesc,
                           float inSeconds,
                           UInt32 *outBufferSize,
                           UInt32 *outNumPackets)
{
    UInt32 maxPacketSize;
    UInt32 propSize = sizeof(maxPacketSize);
    OSStatus error = noErr;
    
    error = AudioFileGetProperty(audioFile,
                                 kAudioFilePropertyPacketSizeUpperBound,
                                 &propSize,
                                 &maxPacketSize);
    assert(error==noErr);
    
    static const int maxBufferSize = 0x10000;
    static const int minBufferSize = 0x4000;
    
    if (inDesc.mFramesPerPacket)
    {
        Float64 numPacketsForTime = inDesc.mSampleRate / inDesc.mFramesPerPacket * inSeconds;
        *outBufferSize = numPacketsForTime * maxPacketSize;
    } else {
        *outBufferSize = maxBufferSize > maxPacketSize ? maxBufferSize : maxPacketSize;
    }
    
    if (*outBufferSize > maxBufferSize && *outBufferSize > maxPacketSize)
        *outBufferSize = maxBufferSize;
    else
    {
        if (*outBufferSize < minBufferSize)
            *outBufferSize = minBufferSize;
    }
    *outNumPackets = *outBufferSize / maxPacketSize;
}


OSStatus Gmb::createOutputAudioFile(const char *name, AudioFileID *outAudioFile, const AudioStreamBasicDescription &format)
{
    
    AudioStreamBasicDescription outputFormat = format;
    CFURLRef outputFileURL = CFURLCreateFromFileSystemRepresentation (kCFAllocatorDefault, (const UInt8 *)name, strlen(name), false);
    if (!outputFileURL) {
        throw std::invalid_argument("Invalid path for output file");
    }
    OSStatus err = AudioFileCreateWithURL(outputFileURL, kAudioFileCAFType, &outputFormat, kAudioFileFlags_EraseFile, outAudioFile);
    CFRelease(outputFileURL);
    return err;
}


/* Function interpolate4:
 4-pt Lagrange interpolation implementation */
double interpolate4(double x0, double y0, double x1, double y1, double x2, double y2, double x3, double y3, double index)
{
    double L[4] = {0};
    double out = 0;
    
    L[0] = ((index - x1) * (index - x2) * (index - x3)) / ((x0 - x1) * (x0 - x2) * (x0 - x3));
    L[1] = ((index - x0) * (index - x2) * (index - x3)) / ((x1 - x0) * (x1 - x2) * (x1 - x3));
    L[2] = ((index - x0) * (index - x1) * (index - x3)) / ((x2 - x0) * (x2 - x1) * (x2 - x3));
    L[3] = ((index - x0) * (index - x1) * (index - x2)) / ((x3 - x0) * (x3 - x1) * (x3 - x2));
    
    out += (y0 * L[0]);
    out += (y1 * L[1]);
    out += (y2 * L[2]);
    out += (y3 * L[3]);
    
    return out;
}

unsigned int Gmb::samplesForDuration(double duration, double sr)
{
    unsigned int n;
    
    n = (unsigned int)(ceil(duration * sr));
    
    return n;
}

void Gmb::getMagicCookie(AudioFileID audioFile,
                                void ** magicCookie,
                                UInt32 &propertySize)
{
    assert(magicCookie);
    OSStatus error = noErr;
    OSStatus result = AudioFileGetPropertyInfo(audioFile, kAudioFilePropertyMagicCookieData, &propertySize, NULL);
    //assert(result==noErr);
    
    if (result==noErr && propertySize > 0)
    {
        *magicCookie = (UInt8*)malloc(propertySize);
        assert(magicCookie);
        error = AudioFileGetProperty(audioFile,
                                     kAudioFilePropertyMagicCookieData,
                                     &propertySize,
                                     *magicCookie);
        assert(error==noErr);
    }
}