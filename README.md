#Convolve#

##Author##

Graham Barab

gbarab@mac.com

##Description##

Computes the convolution of two input audio files, and writes the result to an output file in 24-bit AIF at the specified sample rate.

Input files do not need to be of the same data format or sample rate as eachother or the output file (although with fewer sample rate conversions comes better quality).

##Usage##
	convolve audiofile1 audiofile2 outputfile out_samplerate mode

##Notes##

•The last argument, mode, is optional. It can be 0 for direct form (slow) convolution, or 1 for FFT convolution (fast). Default is 1 if mode is unspecified. Output should be the same regardless.

•Types of input files tested so far: uncompressed AIF, uncompressed CAF, AAC (vbr), MP3 (cbr, vbr), WAVE