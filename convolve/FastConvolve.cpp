//
//  FastConvolve.cpp
//  convolve
//
//  Created by Graham Barab on 5/4/16.
//  Copyright © 2016 Graham Barab. All rights reserved.
//

#include "FastConvolve.h"
#include "gmbutil.h"
#include "fft.h"

Gmb::FastConvolver::FastConvolver(double *impulseTimeDomain, unsigned int impulseLength, unsigned int system_bufsize, Gmb::AudioModule *_upstream)
:Gmb::AudioModule(Gmb::fastConvolveRenderProc, _upstream, system_bufsize)
{
    m = impulseLength;
    n = system_bufsize;
    nm1 = n + m - 1;
    fftsize = (unsigned int)Gmb::ceilPow2(nm1);
    reh = new double[fftsize];
    imh = new double[fftsize];
    y = new double[system_bufsize];
    overlap = new double[m - 1];
    temp = new double[m - 1];
    rex = new double[fftsize];
    imx = new double[fftsize];
    segcache = new double[fftsize];
    
    if (!(reh && imh && y && overlap && temp && rex && imx && segcache)) {
        throw std::exception();
    }
    
    memset(reh, 0, fftsize * sizeof(double));    /* Todo: find out if the "new" operator does this already */
    memset(imh, 0, fftsize * sizeof(double));
    memset(overlap, 0, (m - 1) * sizeof(double));
    memcpy(reh, impulseTimeDomain, impulseLength * sizeof(double));
    fft(reh, imh, fftsize);
}

void Gmb::FastConvolver::process_segment(double *input, unsigned int inputLength)
{
    if (inputLength > n) {
        throw std::invalid_argument("inputLength is greater than system_bufsize");
    }

    unsigned int i;
    double temp;
    
    if (segmentFillCount < n) {
        if (segmentFillCount + inputLength >= n) {
            unsigned int diff = n - segmentFillCount;
            for (i = 0; i < diff; ++i) {
                rex[segmentFillCount + i] = input[i];
            }
        }
    }
    
    memcpy(rex, input, inputLength * sizeof(double));
    fft(rex, imx, fftsize);
    
    for (i = 0; i < fftsize; ++i) {
        temp = (rex[i] * reh[i]) - (imx[i] * imh[i]);
        imx[i] = (rex[i] * imh[i]) + (imx[i] * reh[i]);
        rex[i] = temp;
    }
    
    ifft(rex, imx, fftsize);
    
    for (i = 0; i < m - 1; ++i) {
        rex[i] += overlap[i];
        overlap[i] = rex[i + n];    /* Set up overlap for next cycle while we're here. */
    }
    memcpy(y, rex, inputLength * sizeof(double));
    memset(rex, 0, fftsize * sizeof(double));
    memset(imx, 0, fftsize * sizeof(double));
}


Gmb::FastConvolver::~FastConvolver()
{
    if (reh) {
        delete [] reh;
    }
    if (imh) {
        delete [] imh;
    }
    if (y) {
        delete [] y;
    }
    if (overlap) {
        delete [] overlap;
    }
    if (temp) {
        delete [] temp;
    }
    if (rex) {
        delete [] rex;
    }
    if (imx) {
        delete [] imx;
    }
}

void Gmb::fastConvolveRenderProc(double *out, unsigned int n, void *module)
{
    Gmb::FastConvolver *fconv = (Gmb::FastConvolver *)module;
    Gmb::AudioModule *upstream = fconv->upstream();
    
    memset(fconv->y, 0, fconv->n * sizeof(double));
    if (!upstream) {
        /* Return silence */
        memcpy(out, fconv->y, n * sizeof(double));
    }
    
    upstream->render(out, n);
    fconv->process_segment(out, n);
    memcpy(out, fconv->y, fconv->n * sizeof(double));
}
