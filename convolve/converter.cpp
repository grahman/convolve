//
//  converter.cpp
//  convolve
//
//  Created by Graham Barab on 5/7/16.
//  Copyright © 2016 Graham Barab. All rights reserved.
//

#include "converter.h"
#include "AudioBuffer.h"
#include "PublicUtility/CAXException.h"
#include "AudioUtil.h"
#include "gmbutil.h"

using namespace Gmb;
using namespace std;

void Gmb::converterRenderProc(double *out, unsigned int n, void *module)
{
    Gmb::Converter *converter = (Gmb::Converter *)module;
    unsigned int &i = converter->i;
    unsigned int j = 0;     /* Counter for this render cycle */
    unsigned int N = converter->outNumFrames;
    double *x = (double *)(converter->converted);
    while (j < n) {
        if (i < N) {
            out[j++] = x[i++];
        } else {
            /* Silence */
            out[j++] = 0.0;
        }
    }
}

OSStatus Gmb::complexInputDataProc ( AudioConverterRef inAudioConverter, UInt32 *ioNumberDataPackets, AudioBufferList *ioData, AudioStreamPacketDescription **outDataPacketDescription, void *inUserData )
{
    Gmb::Converter *converter = (Gmb::Converter *)inUserData;
    bool vbr = converter->vbr;
    unsigned int tempNumBytes = converter->maxPacketSize;
    unsigned int N = *ioNumberDataPackets;
    AudioStreamPacketDescription &pckd = converter->currentPacketDescription;
    memset(&converter->currentPacketDescription, 0, sizeof(AudioStreamPacketDescription));
    memset(converter->singlePacket, 0, converter->maxPacketSize);
    if (vbr) {
        XThrowIfError(AudioFileReadPacketData(converter->audioFile,
                                              false,
                                              &tempNumBytes,
                                              &pckd,
                                              (SInt64)converter->currentPacket,
                                              &N,
                                              converter->singlePacket), "AudioFileReadPacketData");
        unsigned int databytesize = pckd.mDataByteSize;
        
        ioData->mBuffers->mData = converter->singlePacket;
        ioData->mBuffers->mDataByteSize = databytesize;
        converter->currentPacket += N;
        if (outDataPacketDescription)
            *outDataPacketDescription = &pckd;
    } else {
        ioData->mBuffers->mDataByteSize = *ioNumberDataPackets * converter->inFormat.mBytesPerFrame;
        ioData->mBuffers->mData = (void *)&converter->unconverted[converter->unconvertedOffset];
        converter->unconvertedOffset += ioData->mBuffers->mDataByteSize;
    }

    ioData->mBuffers->mNumberChannels = converter->inFormat.mChannelsPerFrame;
    
    return noErr;
}
Gmb::Converter::Converter(const char *filepath, const AudioStreamBasicDescription &outputFormat, unsigned int bufsize)
:AudioModule((Gmb::renderProc)&Gmb::converterRenderProc, NULL, bufsize)
{
    outFormat = outputFormat;
    OSStatus error = noErr;
    currentPacket = 0;
    unconvertedOffset = 0;
    
#pragma mark open an audio file
    CFURLRef URL = CFURLCreateFromFileSystemRepresentation(kCFAllocatorDefault, (UInt8*)filepath, strlen(filepath), FALSE);
    error = AudioFileOpenURL(URL, kAudioFileReadPermission, 0, &this->audioFile);
    assert(error == noErr);
    
#pragma mark set up format
    
    UInt32 propSize = sizeof(inFormat);
    double durationInSeconds = 0;
    error = AudioFileGetProperty(audioFile, kAudioFilePropertyDataFormat, &propSize, &inFormat);
    assert(error==noErr);
    if (inFormat.mBytesPerFrame == 0 && inFormat.mBytesPerPacket == 0)
        vbr = true;
    else
        vbr = false;
    
    XThrowIfError(AudioFileGetPropertyInfo(audioFile, kAudioFilePropertyEstimatedDuration, &propSize, NULL), "AudioFileGetPropertyInfo - duration");
    XThrowIfError(AudioFileGetProperty(audioFile, kAudioFilePropertyEstimatedDuration, &propSize, &durationInSeconds), "AudioFileGetProperty - duration");
    outNumFrames = outputFormat.mSampleRate * durationInSeconds;
    inNumFrames = inFormat.mSampleRate * durationInSeconds;
    XThrowIfError(AudioFileGetPropertyInfo(audioFile,
                                           kAudioFilePropertyAudioDataByteCount,
                                           &propSize,
                                           nullptr), "AudioFileGetProperty - Audio data byte count");
    XThrowIfError(AudioFileGetProperty(audioFile,
                                       kAudioFilePropertyAudioDataByteCount,
                                       &propSize,
                                       &numBytes), "AudioFileGetProperty - data byte count");
    
    UInt32 bufferSize;
    UInt32 numPackets;
    
    Gmb::calculateNumPackets(audioFile, inFormat, durationInSeconds, &bufferSize, &numPackets);
    numPacketDescriptions = (vbr) ? numPackets: outNumFrames;
    UInt32 maxPacketSizeSize = sizeof(maxPacketSize);
    XThrowIfError(AudioFileGetProperty(audioFile,
                                       kAudioFilePropertyMaximumPacketSize,
                                       &maxPacketSizeSize,
                                       &maxPacketSize), "AudioFileGetProperty - max packet size");
    if (vbr)
        unconvertedSize = maxPacketSize * numPackets;
    else
        unconvertedSize = inNumFrames * inFormat.mBytesPerFrame;
    
    convertedSize = outNumFrames * outFormat.mBytesPerFrame;
    converted = new char[convertedSize];
    unconverted = new char[unconvertedSize];
    singlePacket = new char[maxPacketSize];
    if (!(converted && unconverted && singlePacket)) {
        throw std::exception();
    }
    memset(converted, 0, convertedSize);
    memset(unconverted, 0, unconvertedSize);
    memset(singlePacket, 0, maxPacketSize);
    
    unsigned int tempNumBytes = convertedSize;
    unsigned int N = outNumFrames;
    if (!vbr) {
        XThrowIfError(AudioFileReadPacketData(audioFile,
                                              false,
                                              &tempNumBytes,
                                              NULL,
                                              0,
                                              &N,
                                              unconverted), "AudioFileReadPacketData");
    }
    
    XThrowIfError(AudioConverterNew(&inFormat, &outFormat, &converterRef), "AudioConverterNew");
    if (vbr) {
        /* Set magic cookie data for converter */
        void *magicCookie = NULL;
        UInt32 magicCookieSize = 0;
        Gmb::getMagicCookie(audioFile, &magicCookie, magicCookieSize);
        if (magicCookieSize) {
            XThrowIfError(AudioConverterSetProperty(converterRef,
                                                    kAudioConverterDecompressionMagicCookie,
                                                    magicCookieSize,
                                                    magicCookie), "Setting magic cookie");
            free(magicCookie);  /* getMagicCookie() uses malloc, so we use free() here */
            magicCookie = NULL;
        }
    }
    
    converterOutput = Gmb::createAudioBufferList(1, outNumFrames * outFormat.mBytesPerFrame, outFormat.mChannelsPerFrame);
    XThrowIfError(AudioConverterFillComplexBuffer(converterRef,
                                                  complexInputDataProc,
                                                  this,
                                                  &outNumFrames,
                                                  converterOutput,
                                                  NULL), "AudioConverterFillComplexBuffer");
    
    memcpy(converted, converterOutput->mBuffers->mData, converterOutput->mBuffers->mDataByteSize);
    Gmb::destroyAudioBufferList(converterOutput);
}


Gmb::Converter::~Converter()
{
    if (converted) {
        delete converted;
    }
    if (unconverted) {
        delete unconverted;
    }
}
