//
//  sinesweep.hpp
//  WriteSamplesToAudioFile
//
//  Created by Graham Barab on 3/17/16.
//  Copyright © 2016 Graham Barab. All rights reserved.
//

#ifndef sinesweep_hpp
#define sinesweep_hpp

#include <MacTypes.h>
#import <AudioToolbox/AudioToolbox.h>
#include <vector>
namespace Gmb {
    class Environment {
    public:
        ~Environment(){};
        std::vector<AudioBufferList *>bufferLists;
        struct SweepInfo *sweepInfo;
        double sr;      /* Sample rate */
    private:
        
    };
    struct SweepInfo {
        unsigned int N;     /* N is the number of samples in the signal, a power of 2 */
        unsigned int M;     /* M <= N, M is the duration of the sine sweep itself */
        unsigned int i;
        double omega1;
        double omega2;
        double denom;   /* ln(omega2/omega1 */
        double duration;    /* Total duration of sweep in seconds */
        double sr;          /* Sample rate */
        SweepInfo(double samplerate, double frequency1, double frequency2, double duration);
        unsigned int pos;
        void reset();
    };
    
    OSStatus provideSineSweepToConverter(AudioConverterRef              converterRef,
                                    UInt32                         *ioNumberDataPackets,
                                    AudioBufferList                *ioData,
                                    AudioStreamPacketDescription   **outDataPacketDescription,
                                    void                           *inUserData);
    
    void createSweepSignal(struct SweepInfo *swpinfo, double *outSamples, unsigned int numSamples);
    void createInverseFilter(struct SweepInfo *swpinfo, double *outSamples, unsigned int numSamples);
    
    void solveToeplitzSystem(double *R, double *RIM, double *G, double *GIM, unsigned int N);
}

#endif /* sinesweep_hpp */
