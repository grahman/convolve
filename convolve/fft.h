#ifndef __FFT__
#define __FFT__

#include <cmath>

template <typename T>
void fft(T *REX, T *IMX, unsigned int N);
template <typename T>
void ifft(T *REX, T *IMX, unsigned int N);
template <typename T>
void mag_phase_response(T *REX, T *IMX, T *MAG, T *PHA, unsigned int N);
template <typename T>
void hann(T *REX, T *IMX, unsigned int N);

#include "fft.hpp"
#endif
