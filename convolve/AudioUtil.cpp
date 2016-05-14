//
//  AudioUtil.cpp
//  WriteSamplesToAudioFile
//
//  Created by Graham Barab on 3/17/16.
//  Copyright © 2016 Graham Barab. All rights reserved.
//

#include "AudioUtil.h"
#include "fft.h"
#include <stdexcept>

using namespace Gmb;
using namespace std;


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