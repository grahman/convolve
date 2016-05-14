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

#ifdef DEBUG
#include <iostream>
#endif

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
    bool mp3 = converter->mp3;
    unsigned int N = *ioNumberDataPackets;
    unsigned int numBytes;
    
    if (mp3) {
        Gmb::Converter *mp3conv = converter->mp3ToLPCMConverter;
        mp3conv->render((double *)converter->unconverted, N);
        ioData->mBuffers->mData = converter->unconverted;
        ioData->mBuffers->mDataByteSize = N * sizeof(double);
        ioData->mBuffers->mNumberChannels = 1;
        ioData->mNumberBuffers = 1;
    } else if (vbr) {
        numBytes = converter->maxPacketSize;
        AudioStreamPacketDescription &pckd = converter->currentPacketDescription;
        memset(&converter->currentPacketDescription, 0, sizeof(AudioStreamPacketDescription));
        memset(converter->singlePacket, 0, converter->maxPacketSize);
        XThrowIfError(AudioFileReadPacketData(converter->audioFile,
                                              false,
                                              &numBytes,
                                              &pckd,
                                              (SInt64)converter->currentPacket,
                                              &N,
                                              converter->singlePacket), "AudioFileReadPacketData");
        
        ioData->mBuffers->mData = converter->singlePacket;
        ioData->mBuffers->mDataByteSize = numBytes;
        ioData->mBuffers->mNumberChannels = (numBytes) ? converter->inFormat.mChannelsPerFrame : 0;

        converter->currentPacket += N;
        *ioNumberDataPackets = N;
        if (outDataPacketDescription)
            *outDataPacketDescription = &pckd;
    } else {
        unsigned int numBytesRequested = N * converter->inFormat.mBytesPerFrame;
        numBytes = converter->unconvertedSize;
        XThrowIfError(AudioFileReadPacketData(converter->audioFile,
                                              false,
                                              &numBytes,
                                              NULL,
                                              (SInt64)converter->currentPacket,
                                              &N,
                                              converter->unconverted), "AudioFileReadPacketData");
        if (numBytes < numBytesRequested) {
            memset(&converter->unconverted[numBytes], 0, numBytesRequested - numBytes);
        }
        ioData->mBuffers->mData = converter->unconverted;
        ioData->mBuffers->mDataByteSize = numBytesRequested;
        ioData->mBuffers->mNumberChannels = converter->inFormat.mChannelsPerFrame;
        converter->currentPacket += N;
    }
    
    return noErr;
}
Gmb::Converter::Converter(const char *_filepath, const AudioStreamBasicDescription &outputFormat, unsigned int bufsize)
:AudioModule((Gmb::renderProc)&Gmb::converterRenderProc, NULL, bufsize)
{
    outFormat = outputFormat;
    OSStatus error = noErr;
    currentPacket = 0;
    unconvertedOffset = 0;
    this->i = 0;
    filepath = (char *)malloc(strlen(_filepath) + 1);
    if (!filepath) {
        throw std::exception();
    }
    strcpy(filepath, _filepath);

    CFURLRef URL = CFURLCreateFromFileSystemRepresentation(kCFAllocatorDefault, (UInt8*)filepath, strlen(filepath), FALSE);
    error = AudioFileOpenURL(URL, kAudioFileReadPermission, 0, &this->audioFile);
    assert(error == noErr);
    
    UInt32 propSize = sizeof(inFormat);
    double durationInSeconds = 0;
    error = AudioFileGetProperty(audioFile, kAudioFilePropertyDataFormat, &propSize, &inFormat);
    assert(error==noErr);
    if (inFormat.mBytesPerFrame == 0 && inFormat.mBytesPerPacket == 0)
        vbr = true;
    else
        vbr = false;
    
    if (inFormat.mFormatID == kAudioFormatMPEGLayer3 && (inFormat.mSampleRate != outFormat.mSampleRate)) {
        /* I wish there were a better solution than this hack but audio converter
         services just doesn't want to convert sample rates for mp3s. So convert to lpcm
         without sr conversion first, then feed the output of mp3ToLPCMConverter's render
         function to the converter belonging to this constructor */
        mp3 = true;
        srConversion = true;
        AudioStreamBasicDescription tempAsbd = outputFormat;
        tempAsbd.mSampleRate = inFormat.mSampleRate;
        inFormat = tempAsbd;
        mp3ToLPCMConverter = new Gmb::Converter(_filepath, inFormat, bufsize);
        unconvertedSize = mp3ToLPCMConverter->n() * sizeof(double);
    } else {
        mp3 = false;    /* It's ok if the file really is mp3, as long as there is no 
                         sr conversion the 'mp3 = false' path will work out fine */
        mp3ToLPCMConverter = NULL;
    }
    
    XThrowIfError(AudioFileGetPropertyInfo(audioFile, kAudioFilePropertyEstimatedDuration, &propSize, NULL), "AudioFileGetPropertyInfo - duration");
    XThrowIfError(AudioFileGetProperty(audioFile, kAudioFilePropertyEstimatedDuration, &propSize, &durationInSeconds), "AudioFileGetProperty - duration");
    outNumFrames = outputFormat.mSampleRate * (durationInSeconds);
    inNumFrames = inFormat.mSampleRate * durationInSeconds;
    XThrowIfError(AudioFileGetPropertyInfo(audioFile,
                                           kAudioFilePropertyAudioDataByteCount,
                                           &propSize,
                                           nullptr), "AudioFileGetProperty - Audio data byte count");
    XThrowIfError(AudioFileGetProperty(audioFile,
                                       kAudioFilePropertyAudioDataByteCount,
                                       &propSize,
                                       &numBytes), "AudioFileGetProperty - data byte count");
    

    UInt32 numPacketsSize = sizeof(numPackets);
    XThrowIfError(AudioFileGetProperty(audioFile,
                                       kAudioFilePropertyAudioDataPacketCount,
                                       &numPacketsSize,
                                       &numPackets), "AudioFileGetProperty - number of packets");

    numPacketDescriptions = (vbr) ? (unsigned int)numPackets: outNumFrames;
    UInt32 maxPacketSizeSize = sizeof(maxPacketSize);
    XThrowIfError(AudioFileGetProperty(audioFile,
                                       kAudioFilePropertyMaximumPacketSize,
                                       &maxPacketSizeSize,
                                       &maxPacketSize), "AudioFileGetProperty - max packet size");
    
    /* We want to skip setting unconvertedSize if we are dealing with the mp3 edge case, since
     it has already been set */
    if (vbr && !mp3)
        unconvertedSize = (unsigned int)(maxPacketSize * numPackets);
    else if (!mp3) /* Use max number of frames in case the audio converter wants zero padding */
        unconvertedSize = MAX(inNumFrames, outNumFrames) * inFormat.mBytesPerFrame;
    
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
    
    if (inFormat.mSampleRate != outFormat.mSampleRate) {
        /* My ears can't discern a difference between these settings, but why not go for the
         maximum possible sample rate conversion quality */
        UInt32 srConversionQuality = kAudioConverterQuality_Max;
        UInt32 srConversionComplexity = kAudioConverterSampleRateConverterComplexity_Mastering;
        XThrowIfError(AudioConverterSetProperty(converterRef,
                                                kAudioConverterSampleRateConverterQuality,
                                                sizeof(srConversionQuality),
                                                &srConversionQuality), "AudioConverterSetProperty - SR Conversion Quality");
        XThrowIfError(AudioConverterSetProperty(converterRef,
                                                kAudioConverterSampleRateConverterComplexity,
                                                sizeof(srConversionComplexity),
                                                &srConversionComplexity), "AudioConverterSetProperty - SR Conversion Complexity");
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
    if (mp3ToLPCMConverter) {
        delete mp3ToLPCMConverter;
        mp3ToLPCMConverter = NULL;
    }
}


Gmb::Converter::~Converter()
{
    if (converted) {
        delete converted;
    }
    if (unconverted) {
        delete unconverted;
    }
    if (singlePacket) {
        delete singlePacket;
    }
    if (filepath) {
        free(filepath);
    }
}
