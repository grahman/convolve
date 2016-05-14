//
//  converter.hpp
//  convolve
//
//  Created by Graham Barab on 5/7/16.
//  Copyright © 2016 Graham Barab. All rights reserved.
//

#ifndef converter_hpp
#define converter_hpp

#include "AudioModule.h"
#include <AudioToolbox/AudioToolbox.h>

namespace Gmb {
    class Converter : public AudioModule {
        
    protected:
        friend OSStatus complexInputDataProc ( AudioConverterRef inAudioConverter, UInt32 *ioNumberDataPackets, AudioBufferList *ioData, AudioStreamPacketDescription **outDataPacketDescription, void *inUserData );
        friend void converterRenderProc(double *out, unsigned int n, void *module);

        AudioStreamBasicDescription inFormat;
        AudioStreamBasicDescription outFormat;
        char *converted;
        char *unconverted;              /* Only used if informat is cbr */
        char *singlePacket;             /* Only used if informat is vbr */
        AudioStreamPacketDescription currentPacketDescription;
        unsigned int numPacketDescriptions;
        AudioFileID audioFile;
        AudioConverterRef converterRef;
        AudioBufferList *converterOutput;
        
        unsigned int outNumFrames;
        unsigned int inNumFrames;
        unsigned int numBytes;          /* in "data" section of audio file, > unconvertedSize for vbr */
        UInt64 numPackets;
        unsigned int convertedSize;
        unsigned int unconvertedSize;   /* Only used if informat is cbr */
        unsigned int unconvertedOffset;
        unsigned int i;                 /* Offset into the file, for use in the _render function */
        unsigned int currentPacket;     /* Only used if informat is vbr */
        unsigned int maxPacketSize;     /* Only used if informat is vbr */
        char *filepath;
        bool vbr;
        
        /* MP3 specific state variables */
        bool mp3;                       /* If mp3 and informat.sampleRate != outformat.samplerate, audioConverterRef hangs */
        bool srConversion;              /* True if performing sample rate conversion in an mp3 file */
        Gmb::Converter *mp3ToLPCMConverter; /* This is a workaround for above problem, only necessary if format is mp3 and 
                                             * we are performing sample rate conversion. Output of this will be at the original
                                             * format's sample rate .*/
        
        bool outputUsesOrigSampleRate() const {return inFormat.mSampleRate == outFormat.mSampleRate;};
    public:
        Converter(const char *filepath, const AudioStreamBasicDescription &outAsbd, unsigned int bufsize);
        ~Converter();
        unsigned int n() const {return this->outNumFrames;};
        const AudioStreamBasicDescription &outStreamFormat() const {return outFormat;};
        const AudioStreamBasicDescription &inStreamFormat() const {return inFormat;};
    };
}



#endif /* converter_hpp */
