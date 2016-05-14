//
//  gmbutil.hpp
//  convolve
//
//  Created by Graham Barab on 3/20/16.
//  Copyright © 2016 Graham Barab. All rights reserved.
//

#ifndef gmbutil_hpp
#define gmbutil_hpp

#include <iostream>
#include <AudioToolbox/AudioToolbox.h>

namespace Gmb {
    unsigned long ceilPow2(unsigned long x);
    
    template<typename t>
    void printbuf(t *x, unsigned int n);

}

std::ostream& operator << (std::ostream& os, AudioStreamPacketDescription const &asbd);


template<typename t>
void Gmb::printbuf(t *x, unsigned int n)
{
    unsigned int i;
    
    for (i = 0; i < n; ++i) {
        std::cout << x[i] << std::endl;
    }
}

#endif /* gmbutil_hpp */
