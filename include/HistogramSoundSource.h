#pragma once
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

#include <vector>
#include <stdlib.h>
//#include <AL/alut.h>
#include "audioUtilities.h"
#include <string>
#include <iostream>
#include <float.h>
#ifndef ANDROID
#define SAVE_WAVE_FILES_CAPABLE
#endif
#ifdef SAVE_WAVE_FILES_CAPABLE
	#include <sndfile.hh>
#endif

#define NUMBER_OF_HARMONICS 3
#define NUMBER_PIANO_FREQS 1
//#define HARMONIC_OUTPUT
#define TRANSITION_OUTPUT
#define HISTOGRAM_SOUND_SOURCE_DEFAULT_AUDIO_SAMPLING_RATE 44100

using namespace std;

class HistogramSoundSource
{
public:
	enum SoundGenerationTechniques
	{
		SOUND_GENERATION_PURE_SINEWAVE,
		SOUND_GENERATION_PURE_SINEWAVE_WITH_TRANSITION,
		SOUND_GENERATION_HARMONICS,
		SOUND_GENERATION_CONVOLUTION,
		SOUND_GENERATION_HARMONICS_PIANO,
		SOUND_GENERATION_NUMBER_OF_TECHNIQUES
	};

	struct AudioGenerationInformation
	{
		float histogramOutputTotalTime;
		int maxFrequencyInHz;
		int minFrequencyInHz;
		int sampleFrequency;
		enum HistogramSoundSource::SoundGenerationTechniques currentSonificationTechnique;
	};

	
	HistogramSoundSource(void);
	HistogramSoundSource(int _histogramLength, float * _histogram, int _location[3],int _direction[3],int _velocity[3],bool _relaitveToListener,	float _outputDuration,	int _maxFrequencyInHz, int _minFrequencyInHz, float _histogramMax,	int sampleFreq,enum HistogramSoundSource::SoundGenerationTechniques newTechnique);
	HistogramSoundSource(const HistogramSoundSource & oldSource);
	HistogramSoundSource & operator=(const HistogramSoundSource & other);

	virtual ~HistogramSoundSource(void);



	static void generateWaveBufferDataFromHistogram(int histogramLength, float * histogram,float outputDuration,int sampleFreq,int maxFrequencyInHz, int minFrequencyInHz, float histogramMax,bool normalizeHistogram,bool trueModulatedSignal,short int * waveBufferOut, unsigned int & waveBufferLength);
	static void generateWaveBufferDataFromHistogram(int histogramLength, float * histogram,float outputDuration,int sampleFreq,int maxFrequencyInHz, int minFrequencyInHz, float histogramMax,bool normalizeHistogram,enum HistogramSoundSource::SoundGenerationTechniques technique, short int * waveBufferOut, unsigned int & waveBufferLength);
	static void playWaveBufferData(int sampleFreq, signed short int * waveBuffer, int waveBufferLength, int location[3], int velocity[3], bool relativeToListener, bool blocking);
	static void saveWaveBufferDataToAudioFile(string filename,int sampleFreq, signed short int * waveBuffer, int waveBufferLength);






	//void DisplayALError(string text,ALenum error);
	static void DisplayALError(string text,ALenum error);	

	void setSoundGenerationTechnique(enum HistogramSoundSource::SoundGenerationTechniques newTechnique);
	enum HistogramSoundSource::SoundGenerationTechniques getSoundGenerationTechnique();

	void setTrueModulated(bool trueModulate);
	void setHistogramDuration(float _outputDuration);
	float getHistogramDuration();
	void setHistogramMaxFreq(int _maxFrequencyInHz);
	int getHistogramMaxFreq();
	void setHistogramMinFreq(int _minFrequencyInHz);
	int getHistogramMinFreq();
	void setHistogramClippingMax(float _histogramMax);
	float getHistogramClippingMax();
	void createHistogramOpenALBuffers(float * histogram, int histogramSize);
	void createHistogramOpenALBuffers();
	ALuint * getHistogramOpenALBuffers(int &numberOfBuffers );

	void setSampleFreq(int _sampleFreqInHz);
	int getSampleMaxFreq();



	void setHistogramSize(int _histogramLength);
	int getHistogramSize(void);

	void setNormalizeHistogram(bool _normalizeHist);
	bool getNormalizeHistogram();
	void setLocation(int x, int y, int z);
	void getLocation(int &x, int& y, int& z);
	void setDirection(int u, int v, int w);
	void setDirectionTowardListner();
	void getDirection(int &u, int& v, int& w);

	void setVelocity(int dx, int dy, int dz);
	void getVelocity(int &dx, int& dy, int& dz);
	
	void setRelativeToListener(bool _relativeToListener);
	bool getRelativeToListener(void);
	void setWaveform(ALenum _waveform);
	ALenum getWaveform(void);

	void attachBuffersToInternalSource(void);
	void play(void);
	bool checkPlayingComplete(void);
	void dettachBuffersFromInternalSource(void);
	void setHistogram(float * histogram, int histogramSize);
	void getHistogram(float * histogram, int & bufferSize);
	int releaseAudioBuffers();
protected:
	vector<float> histogram;
	int histogramLength;
	int location[3];
	int direction[3];
	int velocity[3];
	bool relativeToListener;
	bool trueModulatedSignal;
	bool normalizeHistogram;
	int maxFrequencyInHz;
	int minFrequencyInHz;

	float histogramMax;
	int sampleFreq;


	ALfloat histogramOutputTotalTime;
	ALfloat histogramEntryOutputTime;
	vector<ALfloat> frequencies;
	ALenum waveform;

	vector<ALuint> histogramBuffers;
	ALuint modulatedHistogramBuffer;

	ALuint histogramSource;

	bool ALBuffersCreated;
	bool modALBuffersCreated;
	enum SoundGenerationTechniques currentGenerationTechnique;
};



struct HistogramSoundSourceManager
{
	HistogramSoundSource * histogramSoundSources;
	int amountInUse;
	int capacity;
	int automaticCapacityIncreaseAmount;

};




