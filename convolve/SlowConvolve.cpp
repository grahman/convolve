//
//  SlowConvolve.cpp
//  convolve
//
//  Created by Graham Barab on 4/14/16.
//  Copyright © 2016 Graham Barab. All rights reserved.
//

#include "SlowConvolve.h"
#include <iostream>

using namespace std;

void Gmb::convolve_slowly(double *x, unsigned long nx, double *h, unsigned long nh, double *y, unsigned long ny)
{
    if (ny < (nx + nh - 1)) {
        throw std::invalid_argument("Output buffer is of insufficient length");
    }
    
    unsigned long i, j;
    
    for (i = 0; i < nx + nh; ++i) {
        for (j = 0; j < nh; ++j) {
            if (j > i) {
                y[i] += 0.0;
            } else {
                y[i] += (h[j] * x[i - j]);
            }
        }

    }
}


void Gmb::generate_windowsinc_lpf(double *h, unsigned long nh, double fc)
{
    unsigned long i;
    double sum = 0;
    
    for (i = 0; i < nh; ++i) {
        if (i == nh / 2) {
            h[i] = (sin(2.0 * M_PI * fc)) * (0.42 - (0.5 * cos(2.0 * M_PI * i) / (double)nh) + (0.08 * cos(4.0 * M_PI * i / (double)nh)));
        } else {
            h[i] = (sin(2.0 * M_PI * fc * (i - (nh / 2.0))) / (i - (nh / 2.0))) * (0.42 - (0.5 * cos(2.0 * M_PI * i / (double)nh) + (0.08 * cos(4.0 * M_PI * i / (double)nh))));
        }
        sum += h[i];
    }
    
    for (i = 0; i < nh; ++i) {
        h[i] /= sum;
    }
}

