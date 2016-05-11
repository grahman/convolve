//
//  gmbutil.cpp
//  convolve
//
//  Created by Graham Barab on 3/20/16.
//  Copyright © 2016 Graham Barab. All rights reserved.
//

#include "gmbutil.h"
#include <exception>
#include <stdexcept>

using namespace std;
unsigned long Gmb::ceilPow2(unsigned long x)
{
    int b = 0;
    int bitsset = 0;
    int i;
    
    for (i = 0; i < sizeof(x) * 8; ++i) {
        if (x & (1 << i)) {
            b = i;
            ++bitsset;
        }
    }
    
    if (i == sizeof(x) - 1) {
        throw std::invalid_argument("ceilPow2(): Magnitude of x is too large, result will overflow");
    }
    
    b = (bitsset > 1) ? b + 1 : b;  /* If only one bit set, then x is already a power of 2 */
    return (1 << b);
}

std::ostream& operator << (std::ostream& os, AudioStreamPacketDescription const &aspd)
{
    os << "aspd.mDataByteSize: " << aspd.mDataByteSize << endl << "mStartOffset: "  << aspd.mStartOffset << endl << "mVariableFramesInPacket: "<< aspd.mVariableFramesInPacket << endl;
    
    return os;
}
