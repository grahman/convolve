//
//  CAUtilityFunctions.h
//  CAPractice
//
//  Created by Graham on 5/16/13.
//  Copyright (c) 2013 Graham. All rights reserved.
//

#ifndef CAPractice_CAUtilityFunctions_h
#define CAPractice_CAUtilityFunctions_h

#import <AudioToolbox/AudioToolbox.h>
#import <AudioUnit/AudioUnit.h>
#import <CoreMedia/CoreMedia.h>

void CheckError(OSStatus error, const char *operation);

void MyCopyEncoderCookieToFile(AudioQueueRef queue,
							AudioFileID theFile);


//CalculateLPCMFlags is adapted from the c++ inline function defined in Apple's Core Audio Data Types Reference
UInt32 GMBCalculateLPCMFlags (
							UInt32 inValidBitsPerChannel,
							UInt32 inTotalBitsPerChannel,
							bool inIsFloat,
							bool inIsBigEndian,
							bool inIsNonInterleaved
							);

//FillOutASBDForLPCM is adapted from the c++ inline function defined in Apple's Core Audio Data Types Reference
void GMBFillOutASBDForLPCM (
							AudioStreamBasicDescription *outASBD,
							Float64 inSampleRate,
							UInt32 inChannelsPerFrame,
							UInt32 inValidBitsPerChannel,
							UInt32 inTotalBitsPerChannel,
							bool inIsFloat,
							bool inIsBigEndian,
							bool inIsNonInterleaved
							);

#endif


