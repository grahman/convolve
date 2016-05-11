//
//  SlowConvolve.hpp
//  convolve
//
//  Created by Graham Barab on 4/14/16.
//  Copyright © 2016 Graham Barab. All rights reserved.
//

#ifndef SlowConvolve_hpp
#define SlowConvolve_hpp

#include <MacTypes.h>
#import <AudioToolbox/AudioToolbox.h>


namespace Gmb {
    void convolve_slowly(double *x, unsigned long nx, double *h, unsigned long nh, double *y, unsigned long ny);
    void generate_windowsinc_lpf(double *h, unsigned long nh, double fc);
}
#endif /* SlowConvolve_hpp */
