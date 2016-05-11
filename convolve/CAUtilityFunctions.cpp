//
//  CAUtilityFunctions.c
//  CAPractice
//
//  Created by Graham on 5/16/13.
//  Copyright (c) 2013 Graham. All rights reserved.
//

#include <stdio.h>
#include "CAUtilityFunctions.h"

void CheckError(OSStatus error, const char *operation)
{
	if (error==noErr) return;

	char errorString[20];
	//see if it appears to be a 4-character code.
	*(UInt32 *)(errorString + 1) = CFSwapInt32HostToBig(error);
	if (isprint(errorString[1] && isprint(errorString[2] &&
										isprint(errorString[3] &&
												isprint(errorString[4])))))
	{
		errorString[0] = errorString[5] = '\'';
		errorString[6] = '\0';
	} else {
		//No, format it as an integer
		sprintf(errorString, "%d", (int)error);
	}

	sprintf(errorString, "Error: %s (%s)\n", operation, errorString);
	printf("Error: %s (%s)\n", operation, errorString);

	exit(1);
}

UInt32 GMBCalculateLPCMFlags (
							UInt32 inValidBitsPerChannel,
							UInt32 inTotalBitsPerChannel,
							bool inIsFloat,
							bool inIsBigEndian,
							bool inIsNonInterleaved
							)
{
	return
	(inIsFloat ? kAudioFormatFlagIsFloat : kAudioFormatFlagIsSignedInteger) |
	(inIsBigEndian ? ((UInt32)kAudioFormatFlagIsBigEndian) : 0)			 |
	((!inIsFloat && (inValidBitsPerChannel == inTotalBitsPerChannel)) ?
	kAudioFormatFlagIsPacked : kAudioFormatFlagIsAlignedHigh)		   |
	(inIsNonInterleaved ? ((UInt32)kAudioFormatFlagIsNonInterleaved) : 0);
}

void GMBFillOutASBDForLPCM (AudioStreamBasicDescription *outASBD,
							Float64 inSampleRate,
							UInt32 inChannelsPerFrame,
							UInt32 inValidBitsPerChannel,
							UInt32 inTotalBitsPerChannel,
							bool inIsFloat,
							bool inIsBigEndian,
							bool inIsNonInterleaved
							)
{
	outASBD->mSampleRate = inSampleRate;
	outASBD->mFormatID = kAudioFormatLinearPCM;
	outASBD->mFormatFlags =	GMBCalculateLPCMFlags (
													inValidBitsPerChannel,
													inTotalBitsPerChannel,
													inIsFloat,
													inIsBigEndian,
													inIsNonInterleaved
													);
	outASBD->mBytesPerPacket =
	(inIsNonInterleaved ? 1 : inChannelsPerFrame) * (inTotalBitsPerChannel/8);
	outASBD->mFramesPerPacket = 1;
	outASBD->mBytesPerFrame =
	(inIsNonInterleaved ? 1 : inChannelsPerFrame) * (inTotalBitsPerChannel/8);
	outASBD->mChannelsPerFrame = inChannelsPerFrame;
	outASBD->mBitsPerChannel = inValidBitsPerChannel;
}


