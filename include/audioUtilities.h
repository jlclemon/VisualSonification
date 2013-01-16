/*Copyright (c) 2011, Texas Instruments Inc.

All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted

provided that the following conditions are met:

• Redistributions of source code must retain the above copyright notice, this list of

conditions and the following disclaimer.

• Redistributions in binary form must reproduce the above copyright notice, this list of

conditions and the following disclaimer in the documentation and/or other materials provided

with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS

IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE

IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE

ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS

BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR

CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF

SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS

INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN

CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING

IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY

OF SUCH DAMAGE.  */

#ifndef AUDIO_UTILITIES_H
#define AUDIO_UTILITIES_H

#if defined(_MSC_VER)
#include <alc.h>
#include <al.h>
#include <Windows.h>
#elif defined(__APPLE__)
#include <OpenAL/alc.h>
#include <OpenAL/al.h>
#else
#include <AL/al.h>
#include <AL/alc.h>
#endif
#include <math.h>
#include <float.h>
#include <vector>
#include <stdlib.h>
#include <stdio.h>
#include <iostream>
using namespace std;



void DisplayALError(char *msg, ALuint error);

signed short int * generateSumOfSineWavesPianoStyle(int lengthInMilliseconds, int numberOfFrequencies,int * freqs, int sampleFreq, signed short int * scaleFactors,float * omegaBuffer,int & bufferLength);
signed short int * generateSumOfSinWaves(int lengthInMilliseconds,int attackLength, int decayLength, int numberOfSineWaves,int * sineWaveFreqs, int sampleFreq, float* phases, signed short int * scaleFactors,float * omegaBuffer,int & bufferLength);
signed short int * generateSineWave(int lengthInMilliseconds,int attackLength, int decayLength, int sineWaveFreq, int sampleFreq, float phase, signed short int scaleFactor, int & bufferLength);
signed short int * generateFMModulatedSineWave(int lengthInMilliseconds, float * signal, int signalCount, int sineWaveFreq, int sampleFreq, signed short int scaleFactor, int & bufferLength);
signed short int * generateSineWaveWithTransition(int lengthInMilliseconds,int transitionLength, int sineWaveFreq, int sampleFreq, float phase, signed short int scaleFactor, int prevFreq, int & bufferLength);
ALuint createSumOfSineWaveBuffer(int lengthInMilliseconds, int attackLength,int decayLength,int numberOfSineWaves,int * sineWaveFreqs, int sampleFreq, float* phases, signed short int * scaleFactors,float * omegaBuffer);
ALuint createSineWaveBuffer(int lengthInMilliseconds, int attackLength,int decayLength,int sineWaveFreq, int sampleFreq, float phase, signed short int  scaleFactor);
ALuint createFMModulatedSineWaveBuffer(int lengthInMilliseconds, float * signal, int signalCount,int sineWaveFreq, int sampleFreq,signed short int  scaleFactor);
ALuint createSineWaveWithTransitionBuffer(int lengthInMilliseconds, int transitionLength, int sineWaveFreq, int sampleFreq, float phase, signed short int  scaleFactor, int prevFreq);
ALuint createSumOfSineWavesPianoStyleBuffer(int lengthInMilliseconds, int numberOfFrequencies,int * freqs, int sampleFreq, signed short int * scaleFactors,float * omegaBuffer);
void saveSineWaveToFile( char * filename,signed short int * dataBuffer,int bufferLength, int bufferEntrySize);
void testSineWave();
void testToneWave();
void testToneWave(int i);

ALuint createSoundWaveBuffer(signed short int * sourceData, int bufferLength, int sampleFreq);


ALboolean auExit (void);
ALboolean auInit ();



#endif
