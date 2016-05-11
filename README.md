#Convolve#

##Author##

Graham Barab
gbarab@mac.com

##Description##

Computes the convolution of two input audio files, and writes the result to an output file in 24-bit, 44.1kHz AIF.

##Usage##
	convolve audiofile1 audiofile2 outputfile mode

##Notes##

•The last argument, mode, can be 0 for direct form (slow) convolution, or 1 for FFT convolution (fast). Output should be the same regardless.

•Types of input files tested so far: uncompressed aif, uncompressed caf, AAC (vbr), MP3 (cbr)