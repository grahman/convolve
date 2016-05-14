//
//  AudioBuffer.hpp
//  WriteSamplesToAudioFile
//
//  Created by Graham Barab on 3/17/16.
//  Copyright © 2016 Graham Barab. All rights reserved.
//

#ifndef AudioBuffer_hpp
#define AudioBuffer_hpp

#include <MacTypes.h>
#import <AudioToolbox/AudioToolbox.h>
#import <AudioUnit/AudioUnit.h>
#import <CoreMedia/CoreMedia.h>
#include <vector>

namespace Gmb {
    AudioBufferList *createAudioBufferList(unsigned int numBuffers, unsigned int byteSize, unsigned int numChannels);
    void destroyAudioBufferList(AudioBufferList *list);
}

#endif /* AudioBuffer_hpp */
