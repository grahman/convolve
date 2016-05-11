//
//  AudioBuffer.cpp
//  WriteSamplesToAudioFile
//
//  Created by Graham Barab on 3/17/16.
//  Copyright © 2016 Graham Barab. All rights reserved.
//

#include "AudioBuffer.h"
#include <exception>

AudioBuffer *Gmb::createAudioBuffer(unsigned int numChannels, unsigned int dataByteSize)
{
    AudioBuffer *buf = new AudioBuffer();
    
    buf->mDataByteSize = dataByteSize;
    buf->mNumberChannels = numChannels;
    buf->mData = calloc(dataByteSize, 1);
    if (!buf->mData)
        throw std::exception();
    
    return buf;
}

void Gmb::destroyAudioBuffer(AudioBuffer *buf)
{
    free (buf->mData);
    delete buf;
}


AudioBufferList *Gmb::createAudioBufferList(std::vector<AudioBuffer *>buflist)
{
    size_t numBuffers = buflist.size();
    AudioBufferList *list = (AudioBufferList *)malloc(sizeof(AudioBufferList) + (numBuffers - 1) * sizeof(list->mBuffers[0]));
    if (!list) {
        throw std::exception();
    }
    
    size_t i;
    list->mNumberBuffers = (unsigned int)numBuffers;
    
    for (i = 0; i < numBuffers; ++i) {
        list->mBuffers[i].mData = buflist[i]->mData;
        list->mBuffers[i].mDataByteSize = buflist[i]->mDataByteSize;
        list->mBuffers[i].mNumberChannels = buflist[i]->mNumberChannels;
        list->mBuffers[i].mData = buflist[i]->mData;
    }
    
    return list;
}

AudioBufferList *Gmb::createAudioBufferList(unsigned int numBuffers, unsigned int bytesSize, unsigned int numChannels)
{
    AudioBufferList *list = NULL;
    unsigned int i;
    
    if (numBuffers <= 0)
        throw std::invalid_argument("numBuffers must be > 0");
    
    unsigned long malloc_size = sizeof(AudioBufferList) + (numBuffers - 1) * sizeof(list->mBuffers[0]);
    list = (AudioBufferList *)malloc(malloc_size);
    if (!list) {
        throw std::exception();
    }
    
    list->mNumberBuffers = numBuffers;
    for (i = 0; i < numBuffers; ++i) {
        list->mBuffers[i].mData = (void *)calloc(bytesSize, 1);
        if (!list->mBuffers[i].mData)
            throw std::exception();
        list->mBuffers[i].mDataByteSize = bytesSize;
        list->mBuffers[i].mNumberChannels = numChannels;
    }
    return list;
}


void Gmb::destroyAudioBufferList(AudioBufferList *list)
{
    unsigned int n = list->mNumberBuffers;
    unsigned int i;
    
    for (i = 0; i < n; ++i) {
        free(list->mBuffers[i].mData);
    }
    
    free (list);
}

