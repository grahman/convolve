//
//  FastConvolve.hpp
//  convolve
//
//  Created by Graham Barab on 5/4/16.
//  Copyright © 2016 Graham Barab. All rights reserved.
//

#ifndef FastConvolve_hpp
#define FastConvolve_hpp

#include <stdio.h>
#include "AudioModule.h"

namespace Gmb {
    class FastConvolver: public AudioModule {
    protected:
        double *reh;            /* Real part of frequency response */
        double *imh;            /* Imaginary part of frequency response */
        double *overlap;        /* M - 1 samples, to be added to beginning of output of future render cycles */
        double *temp;           /* temporary buffer for overlap */
        double *y;              /* n samples */
        double *rex;            /* Zero padded input segment */
        double *imx;
        double *segcache;
        unsigned int m;         /* Length of time domain impulse response */
        unsigned int fftsize;   /* Power of 2 size of fft */
        unsigned int n;         /* Buffer size per render cycle */
        unsigned int nm1;       /* n + m - 1, cached just for convenience */
        unsigned int segmentFillCount;
        unsigned int segmentOverFlow;
        void process_segment(double *input, unsigned int inputLength);
    public:
        FastConvolver(double *impulseTimeDomain, unsigned int impulseLength, unsigned int system_bufsize, AudioModule *upstream = NULL);
        ~FastConvolver();
        unsigned long length() const {return nm1;};
        bool ready() const {return segmentFillCount >= n;};

        friend void fastConvolveRenderProc(double *out, unsigned int n, void *module);

    };
    
    void fastConvolveRenderProc(double *out, unsigned int n, void *module);

}
#endif /* FastConvolve_hpp */

