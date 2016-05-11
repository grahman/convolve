//
//  AudioUtil.hpp
//  WriteSamplesToAudioFile
//
//  Created by Graham Barab on 3/17/16.
//  Copyright © 2016 Graham Barab. All rights reserved.
//

#ifndef AudioUtil_hpp
#define AudioUtil_hpp

#include <MacTypes.h>
#import <AudioToolbox/AudioToolbox.h>
#import <AudioToolbox/AudioFile.h>
#import <AudioUnit/AudioUnit.h>
#import <CoreMedia/CoreMedia.h>
#include <vector>
#include "sinesweep.h"

#ifndef MOD
#define MOD(a, b) ({(a % b + b) % b;})
#endif
#define SAMPLERATE 44100.0

namespace Gmb {
    unsigned int samplesForDuration(double duration, double sr);
    
    void calculateNumPackets(AudioFileID audioFile,
                             AudioStreamBasicDescription inDesc,
                             float inSeconds,
                             UInt32 *outBufferSize,
                             UInt32 *outNumPackets);
    
    OSStatus createOutputAudioFile(const char *name, AudioFileID *outAudioFile, const AudioStreamBasicDescription &format);


    void getMagicCookie(AudioFileID audioFile,
                                    void **cookiePtr,
                                    UInt32 &propertySize);
}

#endif /* AudioUtil_hpp */
