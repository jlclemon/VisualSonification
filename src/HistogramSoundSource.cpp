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

#include "HistogramSoundSource.h"

void HistogramSoundSource::DisplayALError(string text,ALenum error)
{
	cout << text;
	switch(error)
	{
		case AL_NO_ERROR:
			cout << "No Error.";
			break;
		case AL_INVALID_NAME:
			cout << "Invalid Name Error.";
			break;
		case AL_INVALID_ENUM:

			cout << "Invalid enum Error.";
			break;
		case AL_INVALID_VALUE:
			cout << "Invalid value Error.";
			break;
		case AL_INVALID_OPERATION:
			cout << "Invalid Operation Error.";
			break;
		case AL_OUT_OF_MEMORY:
			cout << "Out Of memory Error.";
			break;
		default:
			cout << "Unkown Error.";
			break;

	}

	cout << endl;
}



HistogramSoundSource::HistogramSoundSource(void)
{



	alGenSources (1, &(this->histogramSource));
	ALBuffersCreated = false;
	modALBuffersCreated = false;
	this->trueModulatedSignal = false;
	this->normalizeHistogram = false;
	ALenum error;
	error = alGetError();
	if(error != AL_NO_ERROR)
	{
		DisplayALError("Gen Source Error: ",error);
	}

#ifdef VERBOSE_OUPUT
	cout << "Source generated: " << this->histogramSource << endl;
#endif

	this->histogramLength = 0;
	for(int i =0; i <3; i++)
	{
		this->location[i] = 0;
		this->direction[i] = 0;
		this->velocity[i] = 0;

	}

#ifndef HARMONIC_OUTPUT
	#ifdef TRANSITION_OUTPUT
		currentGenerationTechnique = SOUND_GENERATION_PURE_SINEWAVE_WITH_TRANSITION;

	#else

		currentGenerationTechnique = SOUND_GENERATION_PURE_SINEWAVE;

	#endif
#else

	#ifdef HARMONICS_PIANO
		currentGenerationTechnique =SOUND_GENERATION_HARMONICS_PIANO;

	#else
		currentGenerationTechnique = SOUND_GENERATION_HARMONICS;
	#endif
#endif
#ifdef TRUE_MODULATED
	currentGenerationTechnique = SOUND_GENERATION_CONVOLUTION;
#endif
	//this->waveform = ALUT_WAVEFORM_SINE;
	this->sampleFreq = HISTOGRAM_SOUND_SOURCE_DEFAULT_AUDIO_SAMPLING_RATE;


}

HistogramSoundSource::HistogramSoundSource(const HistogramSoundSource & other)
{


	alGenSources (1, &(this->histogramSource));
	ALBuffersCreated =false;
	modALBuffersCreated = false;
	this->trueModulatedSignal = other.trueModulatedSignal;
	this->normalizeHistogram = other.normalizeHistogram;
	this->currentGenerationTechnique = other.currentGenerationTechnique;
	this->sampleFreq = other.sampleFreq;
	ALenum error;
	error = alGetError();
	if(error != AL_NO_ERROR)
	{
		DisplayALError("Gen Source Error: ",error);
	}
#ifdef VERBOSE_OUPUT
	cout << "Source generated: " << this->histogramSource << endl;
#endif
	this->histogramLength = other.histogramLength;
	this->histogram.resize(other.histogramLength);
	for(int i = 0; i <other.histogramLength; i++)
	{
		histogram[i] = other.histogram[i];

	}

	for(int i =0; i <3; i++)
	{
		this->location[i] = other.location[i];
		this->direction[i] = other.direction[i];
		this->velocity[i] = other.velocity[i];

	}
	alSource3i(this->histogramSource,AL_VELOCITY, velocity[0],velocity[1],velocity[2]);
	//ALenum error;
	error = alGetError();
	if(error != AL_NO_ERROR)
	{
		DisplayALError("Velocity Set Error: ",error);
	}


	alSource3i(this->histogramSource,AL_DIRECTION, direction[0],direction[1],direction[2]);
//	ALenum error;
	error = alGetError();
	if(error != AL_NO_ERROR)
	{
		DisplayALError("Direction Set Error: ",error);
	}


	alSource3i(this->histogramSource,AL_POSITION, location[0],location[1],location[2]);
//	ALenum error;
	error = alGetError();
	if(error != AL_NO_ERROR)
	{
		DisplayALError("Position Error: ",error);
	}


	this->relativeToListener = other.relativeToListener;
	alSourcei(this->histogramSource,AL_SOURCE_RELATIVE,this->relativeToListener?AL_TRUE:AL_FALSE);
	//ALenum error;
	error = alGetError();
	if(error != AL_NO_ERROR)
	{
		DisplayALError("Set Relative Error: ",error);
	}


	this->maxFrequencyInHz =  other.maxFrequencyInHz;
	this->minFrequencyInHz=  other.minFrequencyInHz;

	this->histogramMax = other.histogramMax;

	this->waveform = other.waveform;

	this->histogramOutputTotalTime = other.histogramEntryOutputTime;


	this->histogramEntryOutputTime = (this->histogramOutputTotalTime/(float)this->histogramLength);
	this->frequencies.resize(this->histogramLength);
	this->histogramBuffers.resize(this->histogramLength);
	
	if(other.ALBuffersCreated)
	{

		createHistogramOpenALBuffers(&this->histogram[0], this->histogramLength);
	}



}


HistogramSoundSource & HistogramSoundSource::operator=(const HistogramSoundSource & other)
{

	this->currentGenerationTechnique = other.currentGenerationTechnique;
	this->histogramLength = other.histogramLength;
	this->histogram.resize(other.histogramLength);
	for(int i = 0; i <other.histogramLength; i++)
	{
		histogram[i] = other.histogram[i];

	}

	for(int i =0; i <3; i++)
	{
		this->location[i] = other.location[i];
		this->direction[i] = other.direction[i];
		this->velocity[i] = other.velocity[i];

	}
	alSource3i(this->histogramSource,AL_VELOCITY, velocity[0],velocity[1],velocity[2]);
	ALenum error;
	error = alGetError();
	if(error != AL_NO_ERROR)
	{
		DisplayALError("Velocity Set Error: ",error);
	}

	alSource3i(this->histogramSource,AL_DIRECTION, direction[0],direction[1],direction[2]);
	//ALenum error;
	error = alGetError();
	if(error != AL_NO_ERROR)
	{
		DisplayALError("Direction Set Error: ",error);
	}

	alSource3i(this->histogramSource,AL_POSITION, location[0],location[1],location[2]);
	//ALenum error;
	error = alGetError();
	if(error != AL_NO_ERROR)
	{
		DisplayALError("Position Set Error: ",error);
	}


	this->relativeToListener = other.relativeToListener;
	alSourcei(this->histogramSource,AL_SOURCE_RELATIVE,this->relativeToListener?AL_TRUE:AL_FALSE);
	//ALenum error;
	error = alGetError();
	if(error != AL_NO_ERROR)
	{
		DisplayALError("Relative Set Error: ",error);
	}
	this->sampleFreq = other.sampleFreq;
	this->maxFrequencyInHz =  other.maxFrequencyInHz;
	this->minFrequencyInHz=  other.minFrequencyInHz;

	this->histogramMax = other.histogramMax;

	this->waveform = other.waveform;

	this->histogramOutputTotalTime = other.histogramEntryOutputTime;
	this->trueModulatedSignal = other.trueModulatedSignal;
	this->normalizeHistogram = other.normalizeHistogram;

	this->histogramEntryOutputTime = (this->histogramOutputTotalTime/(float)this->histogramLength);
	this->frequencies.resize(this->histogramLength);
	this->histogramBuffers.resize(this->histogramLength);

	releaseAudioBuffers();
	if(other.ALBuffersCreated)
	{

		createHistogramOpenALBuffers(&this->histogram[0], this->histogramLength);
	}



	return *this;
}


HistogramSoundSource::~HistogramSoundSource(void)
{

	while(!this->checkPlayingComplete())
	{
		;
	}

	this->releaseAudioBuffers();
#ifdef VERBOSE_OUPUT
	cout << "Source Deleted: " << this->histogramSource << endl;
#endif
	alDeleteSources(1,&(this->histogramSource));
	ALenum error;
	error = alGetError();
	if(error != AL_NO_ERROR)
	{
		DisplayALError("Source Delete Error: ",error);
	}

}

int HistogramSoundSource::releaseAudioBuffers()
{
	int numberReleased = this->histogramBuffers.size();
	if(ALBuffersCreated == true)
	{
		if(this->histogramBuffers.size() != 0)
		{
		
			alDeleteBuffers(this->histogramBuffers.size(), &(this->histogramBuffers[0]));
				ALenum error;
				error = alGetError();
				if(error != AL_NO_ERROR)
				{
					DisplayALError("Buffere Delete Error: ",error);
				}


			this->ALBuffersCreated = false;

		}
	}
	if(this->modALBuffersCreated)
	{
		alDeleteBuffers(1, &(this->modulatedHistogramBuffer));
				ALenum error;
				error = alGetError();
				if(error != AL_NO_ERROR)
				{
					DisplayALError("Buffere Delete Error: ",error);
				}


			this->modALBuffersCreated = false;


	}
	return numberReleased;
}


HistogramSoundSource::HistogramSoundSource(int _histogramLength, float * _histogram, int _location[3],int _direction[3],int _velocity[3],bool _relativeToListener,	float _outputDuration,	int _maxFrequencyInHz, int _minFrequencyInHz,float _histogramMax,int _sampleFreq,enum HistogramSoundSource::SoundGenerationTechniques newTechnique)
{

	alGenSources (1, &(this->histogramSource));
	ALenum error;
	this->trueModulatedSignal = false;
	this->normalizeHistogram = false;
	error = alGetError();
	if(error != AL_NO_ERROR)
	{
		DisplayALError("Gen Source Error: ",error);
	}


	ALBuffersCreated = false;
	this->histogramLength = _histogramLength;
	this->histogram.resize(_histogramLength);
	for(int i = 0; i < _histogramLength; i++)
	{
		histogram[i] = _histogram[i];

	}
	for(int i =0; i <3; i++)
	{
		this->location[i] = _location[i];
		this->direction[i] = _direction[i];
		this->velocity[i] = _velocity[i];

	}
	alSource3i(this->histogramSource,AL_VELOCITY, velocity[0],velocity[1],velocity[2]);
	//ALenum error;
	error = alGetError();
	if(error != AL_NO_ERROR)
	{
		DisplayALError("Set Velocity Error: ",error);
	}


	alSource3i(this->histogramSource,AL_DIRECTION, direction[0],direction[1],direction[2]);
	//ALenum error;
	error = alGetError();
	if(error != AL_NO_ERROR)
	{
		DisplayALError("Set Direction Error: ",error);
	}


	alSource3i(this->histogramSource,AL_POSITION, location[0],location[1],location[2]);
	//ALenum error;
	error = alGetError();
	if(error != AL_NO_ERROR)
	{
		DisplayALError("Position Set Error: ",error);
	}


	this->relativeToListener = _relativeToListener;
	alSourcei(this->histogramSource,AL_SOURCE_RELATIVE,_relativeToListener?AL_TRUE:AL_FALSE);
	//ALenum error;
	error = alGetError();
	if(error != AL_NO_ERROR)
	{
		DisplayALError("Set Relative  Error: ",error);
	}


	this->sampleFreq = _sampleFreq;
	this->maxFrequencyInHz =  _maxFrequencyInHz;
	this->minFrequencyInHz=  _minFrequencyInHz;

	this->histogramMax = _histogramMax;

	//this->waveform = ALUT_WAVEFORM_SINE;

	this->histogramOutputTotalTime = _outputDuration;

	this->histogramEntryOutputTime = (this->histogramOutputTotalTime/(float)_histogramLength);
	this->frequencies.resize(_histogramLength);
	this->histogramBuffers.resize(_histogramLength);

	this->currentGenerationTechnique = newTechnique;
/*
	#ifndef HARMONIC_OUTPUT
		#ifdef TRANSITION_OUTPUT
			currentGenerationTechnique = SOUND_GENERATION_PURE_SINEWAVE_WITH_TRANSITION;

		#else
			currentGenerationTechnique = SOUND_GENERATION_PURE_SINEWAVE;
		#endif
	#else
		currentGenerationTechnique = SOUND_GENERATION_HARMONICS;
	#endif
	#ifdef TRUE_MODULATED
		currentGenerationTechnique = SOUND_GENERATION_CONVOLUTION;
	#endif
*/
	createHistogramOpenALBuffers(&this->histogram[0], this->histogramLength);
}

void HistogramSoundSource::setHistogram(float * histogram, int histogramSize)
{
	this->histogramLength = histogramSize; 
	this->histogram.resize(this->histogramLength);
	for(int i = 0; i < this->histogramLength; i++)
	{
		this->histogram[i] = histogram[i];

	}


}

void HistogramSoundSource::getHistogram(float * histogram, int & bufferSize)
{
	
	for(int i = 0; i < this->histogramLength && i <bufferSize; i++)
	{
		 histogram[i] = this->histogram[i];

	}
	
	bufferSize = (this->histogramLength < bufferSize? this->histogramLength:bufferSize);
}

void HistogramSoundSource::setTrueModulated(bool trueModulate)
{

	this->trueModulatedSignal = trueModulate;

}

void HistogramSoundSource::createHistogramOpenALBuffers()
{

	createHistogramOpenALBuffers(&this->histogram[0], this->histogramLength);

}


void HistogramSoundSource::setSoundGenerationTechnique(enum HistogramSoundSource::SoundGenerationTechniques newTechnique)
{


	this->currentGenerationTechnique = newTechnique;


}
enum HistogramSoundSource::SoundGenerationTechniques HistogramSoundSource::getSoundGenerationTechnique()
{




	return this->currentGenerationTechnique;

}


void HistogramSoundSource::createHistogramOpenALBuffers(float * histogram, int histogramSize)
{



	if(histogram != &this->histogram[0])
	{
		setHistogram(histogram, histogramSize);
	}
	if(this->normalizeHistogram)
	{
		float histogramMaxVal = -FLT_MAX;
		for(int i = 0; i < (int)this->histogram.size(); i++)
		{
			if(histogramMaxVal < this->histogram[i])
			{
				histogramMaxVal = this->histogram[i];

			}
		}

		

		for(int i = 0; i < (int)this->histogram.size(); i++)
		{
			this->histogram[i] = (this->histogram[i]/histogramMaxVal)* this->histogramMax;
		}


	}


	int frequencyBand = maxFrequencyInHz - minFrequencyInHz;

	releaseAudioBuffers();
	this->histogramBuffers.resize(this->histogramLength);

#ifdef SAVE_WAVE_FILES_CAPABLE

	SndfileHandle sndFile("regionFileOut.wav",SFM_WRITE,SF_FORMAT_WAV|SF_FORMAT_PCM_16,1,this->sampleFreq);
	

#endif
	for(int i = 0; i <histogramSize; i++)
	{
		float currentHistogramBinVal = (float)this->histogram[i];
		
		if(currentHistogramBinVal >= this->histogramMax)
		{
			currentHistogramBinVal = (float)this->histogramMax;

		}
		frequencies[i] = (currentHistogramBinVal/(float)this->histogramMax) * frequencyBand + this->minFrequencyInHz;

		switch(this->currentGenerationTechnique)
		{
			case SOUND_GENERATION_PURE_SINEWAVE_WITH_TRANSITION:
			{

				histogramBuffers[i] = createSineWaveWithTransitionBuffer((int) (this->histogramEntryOutputTime*1000), (int) (this->histogramEntryOutputTime*1000)/2, (int)frequencies[i], this->sampleFreq, 0.0f, 32767, (i==0?0:(int)frequencies[i-1]));//createSineWaveBuffer((int) (this->histogramEntryOutputTime*1000), 0,0,(int)frequencies[i], AUDIO_SAMPLING_RATE, 0.0f, 32767);//alutCreateBufferWaveform (this->waveform,frequencies[i],0.0f,this->histogramEntryOutputTime);
				//histogramBuffers[i] = createSineWaveBuffer((int) (this->histogramEntryOutputTime*1000), 0,0,(int)frequencies[i], AUDIO_SAMPLING_RATE, 0.0f, 32767);

				#ifdef SAVE_WAVE_FILES_CAPABLE

				int bufferLength;
				signed short int * sineWaveBuffer = generateSineWaveWithTransition((int) (this->histogramEntryOutputTime*1000), (int) (this->histogramEntryOutputTime*1000)/2, (int)frequencies[i], this->sampleFreq, 0.0f, 32767, (i==0?0:(int)frequencies[i-1]),bufferLength);//generateSineWave((int) (this->histogramEntryOutputTime*1000),0 ,0,(int)frequencies[i], AUDIO_SAMPLING_RATE, 0.0f, 32767,bufferLength);
				sndFile.write(sineWaveBuffer,bufferLength/sizeof(signed short int));
				free(sineWaveBuffer);



				#endif

				break;
			}
			case SOUND_GENERATION_PURE_SINEWAVE:
			{

				//histogramBuffers[i] = createSineWaveWithTransitionBuffer((int) (this->histogramEntryOutputTime*1000), (int) (this->histogramEntryOutputTime*1000)/2, (int)frequencies[i], AUDIO_SAMPLING_RATE, 0.0f, 32767, (i==0?0:(int)frequencies[i-1]));//createSineWaveBuffer((int) (this->histogramEntryOutputTime*1000), 0,0,(int)frequencies[i], AUDIO_SAMPLING_RATE, 0.0f, 32767);//alutCreateBufferWaveform (this->waveform,frequencies[i],0.0f,this->histogramEntryOutputTime);
				histogramBuffers[i] = createSineWaveBuffer((int) (this->histogramEntryOutputTime*1000), 0,0,(int)frequencies[i], this->sampleFreq, 0.0f, 32767);

				#ifdef SAVE_WAVE_FILES_CAPABLE

				int bufferLength;
				signed short int * sineWaveBuffer = generateSineWave((int) (this->histogramEntryOutputTime*1000),0 ,0,(int)frequencies[i], this->sampleFreq, 0.0f, 32767,bufferLength);
				sndFile.write(sineWaveBuffer,bufferLength/sizeof(signed short int));
				free(sineWaveBuffer);



				#endif

				break;
			}

			case SOUND_GENERATION_HARMONICS:
			{
				int currentFreqWithHarmonics[NUMBER_OF_HARMONICS];
				float phasesArray[NUMBER_OF_HARMONICS];
				signed short int scaleFactorsArray[NUMBER_OF_HARMONICS];
				float * omegaBuffer = NULL;
				for(int l = 0; l < NUMBER_OF_HARMONICS; l++)
				{
					currentFreqWithHarmonics[l] = (l+1) * (int)frequencies[i];
					phasesArray[l] = 0;
					scaleFactorsArray[l] = (int)(32767/NUMBER_OF_HARMONICS);
				}

				histogramBuffers[i] = createSumOfSineWaveBuffer((int) (this->histogramEntryOutputTime*1000), 0,0,NUMBER_OF_HARMONICS,currentFreqWithHarmonics, this->sampleFreq, phasesArray, scaleFactorsArray,omegaBuffer);
				#ifdef SAVE_WAVE_FILES_CAPABLE

				int bufferLength;
				signed short int * sineWaveBuffer = generateSumOfSinWaves((int) (this->histogramEntryOutputTime*1000), 0,0,NUMBER_OF_HARMONICS,currentFreqWithHarmonics, this->sampleFreq, phasesArray, scaleFactorsArray,omegaBuffer, bufferLength);
					//generateSineWaveWithTransition((int) (this->histogramEntryOutputTime*1000), (int) (this->histogramEntryOutputTime*1000)/2, (int)frequencies[i], AUDIO_SAMPLING_RATE, 0.0f, 32767, (i==0?0:(int)frequencies[i-1]),bufferLength);//generateSineWave((int) (this->histogramEntryOutputTime*1000),0 ,0,(int)frequencies[i], AUDIO_SAMPLING_RATE, 0.0f, 32767,bufferLength);
				sndFile.write(sineWaveBuffer,bufferLength/sizeof(signed short int));
				free(sineWaveBuffer);

				#endif
				break;
			}
			case SOUND_GENERATION_HARMONICS_PIANO:
			{
				int currentFreq[NUMBER_PIANO_FREQS];
				signed short int scaleFactorsArray[NUMBER_PIANO_FREQS];
				float * omegaBuffer = NULL;
				for(int l = 0; l < NUMBER_PIANO_FREQS; l++)
				{
					currentFreq[l] = (l+1) * (int)frequencies[i];

					scaleFactorsArray[l] = (int)(32767/NUMBER_PIANO_FREQS);
				}

				histogramBuffers[i] =  createSumOfSineWavesPianoStyleBuffer((int) (histogramEntryOutputTime*1000), 1,currentFreq, sampleFreq, scaleFactorsArray,omegaBuffer);

				#ifdef SAVE_WAVE_FILES_CAPABLE
				int bufferLength;
				signed short int * sineWaveBuffer = generateSumOfSineWavesPianoStyle((int) (histogramEntryOutputTime*1000), 1,currentFreq, sampleFreq, scaleFactorsArray,omegaBuffer,bufferLength);
				sndFile.write(sineWaveBuffer,bufferLength/sizeof(signed short int));


				free(sineWaveBuffer);
				#endif
				break;
			}
			case SOUND_GENERATION_CONVOLUTION:
			default:
			{


				break;
			}

		}

#ifndef HARMONIC_OUTPUT

	#ifdef TRANSITION_OUTPUT
	#else


	#endif

#else






#endif



		ALenum error;
		error = alGetError();
		if(error != AL_NO_ERROR)
		{
			DisplayALError("Create Waveform Buffer Error: ",error);
		}

	}


	switch(this->currentGenerationTechnique)
	{
		case	SOUND_GENERATION_CONVOLUTION:
		{
			this->modulatedHistogramBuffer = createFMModulatedSineWaveBuffer((int) (this->histogramOutputTotalTime  *1000), (float *)&(this->histogram[0]), this->histogram.size(),3000, this->sampleFreq,32767);
			#ifdef SAVE_WAVE_FILES_CAPABLE



			SndfileHandle sndFileMod("regionModFileOut.wav",SFM_WRITE,SF_FORMAT_WAV|SF_FORMAT_PCM_16,1,this->sampleFreq);





			int bufferLength;
			float * tmpHistPtr = (float *)&this->histogram[0];
			signed short int * sineWaveBuffer = generateFMModulatedSineWave((int) (this->histogramOutputTotalTime  *1000), tmpHistPtr, this->histogram.size(),3000, this->sampleFreq,32767,bufferLength);


			sndFileMod.write(sineWaveBuffer,bufferLength/sizeof(signed short int));
			free(sineWaveBuffer);
			sndFileMod.writeSync();
		
	
			#endif




			modALBuffersCreated = true;
			break;
		}

		default:
		{
			
			break;
		}
	}

	#ifdef SAVE_WAVE_FILES_CAPABLE

	sndFile.writeSync();

	#endif


	ALBuffersCreated = true;


}

ALuint * HistogramSoundSource::getHistogramOpenALBuffers(int &numberOfBuffers )
{
	numberOfBuffers = this->histogramBuffers.size();
	
	return &(this->histogramBuffers[0]);
}


void HistogramSoundSource::setHistogramSize(int _histogramLength)
{
		this->histogram.resize(_histogramLength);
		this->histogramEntryOutputTime = (this->histogramOutputTotalTime/(float)_histogramLength);
		this->frequencies.resize(_histogramLength);
		if(this->histogramBuffers.size() != 0)
		{
			alDeleteBuffers(this->histogramBuffers.size(), &(this->histogramBuffers[0]));
			ALenum error;
			error = alGetError();
			if(error != AL_NO_ERROR)
			{
				DisplayALError("Delete Buffers Error: ",error);
			}

		}


		this->histogramBuffers.resize(_histogramLength);

}


int HistogramSoundSource::getHistogramSize(void)
{
	return this->histogram.size();

}


void HistogramSoundSource::setLocation(int x, int y, int z)
{
	this->location[0] = x;
	this->location[1] = y;
	this->location[2] = z;

	alSource3i(this->histogramSource,AL_POSITION, location[0],location[1],location[2]);
	ALenum error;
	error = alGetError();
	if(error != AL_NO_ERROR)
	{
		DisplayALError("Set Location Error: ",error);
	}

}

void HistogramSoundSource::getLocation(int &x, int& y, int& z)
{
	x = this->location[0];
	y = this->location[1];
	z = this->location[2];




}
void HistogramSoundSource::setDirection(int u, int v, int w)
{
	this->direction[0] = u;
	this->direction[1] = v;
	this->direction[2] = w;
	alSource3i(this->histogramSource,AL_DIRECTION, direction[0],direction[1],direction[2]);
	ALenum error;
	error = alGetError();
	if(error != AL_NO_ERROR)
	{
		DisplayALError("Set Direction Error: ",error);
	}

}

void HistogramSoundSource::setDirectionTowardListner()
{

	int lisPos[3];


	alGetListeneriv(AL_POSITION,lisPos);
	ALenum error;
	error = alGetError();
	if(error != AL_NO_ERROR)
	{
		DisplayALError("Get Listener Error: ",error);
	}


	this->direction[0] = lisPos[0] -this->location[0];
	this->direction[1] = lisPos[1] -this->location[1];
	this->direction[2] = lisPos[2] -this->location[2];

	alSource3i(this->histogramSource,AL_DIRECTION, direction[0],direction[1],direction[2]);
	//ALenum error;
	error = alGetError();
	if(error != AL_NO_ERROR)
	{
		DisplayALError("Set Direction Error: ",error);
	}

}
void HistogramSoundSource::getDirection(int &u, int& v, int& w)
{

	u = this->direction[0];
	v = this->direction[1];
	w = this->direction[2];



}



void HistogramSoundSource::setVelocity(int dx, int dy, int dz)
{
	this->velocity[0] = dx;
	this->velocity[1] = dy;
	this->velocity[2] = dz;
	alSource3i(this->histogramSource,AL_VELOCITY, velocity[0],velocity[1],velocity[2]);
	ALenum error;
	error = alGetError();
	if(error != AL_NO_ERROR)
	{
		DisplayALError("Velocity Set Error: ",error);
	}

}
void HistogramSoundSource::getVelocity(int &dx, int& dy, int& dz)
{
	dx = this->velocity[0];
	dy = this->velocity[1];
	dz = this->velocity[2];



}
	
void HistogramSoundSource::setRelativeToListener(bool _relativeToListener)
{
	this->relativeToListener = _relativeToListener;
	alSourcei(this->histogramSource,AL_SOURCE_RELATIVE,_relativeToListener?AL_TRUE:AL_FALSE);
	ALenum error;
	error = alGetError();
	if(error != AL_NO_ERROR)
	{
		DisplayALError("Set Relative Error: ",error);
	}

}
bool HistogramSoundSource::getRelativeToListener(void)
{
	return this->relativeToListener;
}

void HistogramSoundSource::setWaveform(ALenum _waveform)
{
	this->waveform = _waveform;

}
ALenum HistogramSoundSource::getWaveform(void)
{
	return this->waveform;

}
void HistogramSoundSource::attachBuffersToInternalSource(void)
{

	if(!this->trueModulatedSignal)
	{
		alSourceQueueBuffers(this->histogramSource,this->histogramBuffers.size(),&(this->histogramBuffers[0]));
		ALenum error;
		error = alGetError();
		if(error != AL_NO_ERROR)
		{
			DisplayALError("Source Queue Buffer Error: ",error);
		}
	}
	else
	{
		alSourceQueueBuffers(this->histogramSource,1,&(this->modulatedHistogramBuffer));
		ALenum error;
		error = alGetError();
		if(error != AL_NO_ERROR)
		{
			DisplayALError("Source Queue Buffer Error: ",error);
		}


	}
}

void HistogramSoundSource::dettachBuffersFromInternalSource(void)
{
	if(!this->trueModulatedSignal)
	{

		alSourceUnqueueBuffers(this->histogramSource,this->histogramBuffers.size(),&(this->histogramBuffers[0]));
		ALenum error;
		error = alGetError();
		if(error != AL_NO_ERROR)
		{
			DisplayALError("Source unqueue Buffer Error: ",error);
		}
	}
	else
	{
		alSourceUnqueueBuffers(this->histogramSource,1,&(this->modulatedHistogramBuffer));
		ALenum error;
		error = alGetError();
		if(error != AL_NO_ERROR)
		{
			DisplayALError("Source unqueue Buffer Error: ",error);
		}


	}

}

void HistogramSoundSource::play(void)
{
	alSourcePlay (this->histogramSource);
	ALenum error;
	error = alGetError();
	if(error != AL_NO_ERROR)
	{
		DisplayALError("Source Play Error: ",error);
	}

}

void HistogramSoundSource::setNormalizeHistogram(bool _normalizeHist)
{
	this->normalizeHistogram = _normalizeHist;



}
bool HistogramSoundSource::getNormalizeHistogram()
{
	return this->normalizeHistogram;

}


bool HistogramSoundSource::checkPlayingComplete(void)
{
	ALint currentState;
	bool returnVal = false;
	alGetSourcei(this->histogramSource,AL_SOURCE_STATE,&currentState);
	ALenum error;
	error = alGetError();
	if(error != AL_NO_ERROR)
	{
		DisplayALError("Check Source State Error: ",error);
	}

	if(currentState != AL_PLAYING ||this->histogramBuffers.size() ==0 || ALBuffersCreated==false)
	{
		returnVal =true;

	}
	return returnVal;

}

void HistogramSoundSource::setHistogramDuration(float _outputDuration)
{

	this->histogramOutputTotalTime = _outputDuration;
	if(this->histogram.size() != 0)
	{
		this->histogramEntryOutputTime = (this->histogramOutputTotalTime/(float)this->histogram.size());
	}

}

float HistogramSoundSource::getHistogramDuration()
{
	return this->histogramOutputTotalTime;


}


void HistogramSoundSource::setSampleFreq(int _sampleFreqInHz)
{

	this->sampleFreq = _sampleFreqInHz;


}
int HistogramSoundSource::getSampleMaxFreq()
{
	
	return this->sampleFreq; 
	
	
}

void HistogramSoundSource::setHistogramMaxFreq(int _maxFrequencyInHz)
{

	this->maxFrequencyInHz = _maxFrequencyInHz;


}
int HistogramSoundSource::getHistogramMaxFreq()
{
	return this->maxFrequencyInHz;


}



void HistogramSoundSource::setHistogramMinFreq(int _minFrequencyInHz)
{

	this->minFrequencyInHz = _minFrequencyInHz;

}
int HistogramSoundSource::getHistogramMinFreq()
{

	return this->minFrequencyInHz;

}
void HistogramSoundSource::setHistogramClippingMax(float _histogramMax)
{

	this->histogramMax = _histogramMax;

}
float HistogramSoundSource::getHistogramClippingMax()
{
	return this->histogramMax;

}

void HistogramSoundSource::generateWaveBufferDataFromHistogram(int histogramLength, float * histogram,float outputDuration,	int sampleFreq,int maxFrequencyInHz, int minFrequencyInHz, float histogramMax,bool normalizeHistogram,bool trueModulatedSignal, short int * waveBufferOut, unsigned int & waveBufferLength)
{
	float histogramEntryOutputTime = outputDuration/histogramLength;
	static vector <float> frequencies;
	int currentBufferEntry = 0;

	frequencies.resize(histogramLength);
	if(normalizeHistogram)
	{
		float histogramMaxVal = -FLT_MAX;
		for(int i = 0; i < (int)histogramLength; i++)
		{
			if(histogramMaxVal < histogram[i])
			{
				histogramMaxVal = histogram[i];

			}
		}

		

		for(int i = 0; i < histogramLength; i++)
		{
			histogram[i] = (histogram[i]/histogramMaxVal)* histogramMax;
		}


	}


	int frequencyBand = maxFrequencyInHz - minFrequencyInHz;


#ifdef SAVE_WAVE_FILES_CAPABLE

	SndfileHandle sndFile("regionFileOut.wav",SFM_WRITE,SF_FORMAT_WAV|SF_FORMAT_PCM_16,1,sampleFreq);
	

#endif
	for(int i = 0; i <histogramLength; i++)
	{
		float currentHistogramBinVal = (float)histogram[i];
		
		if(currentHistogramBinVal >= histogramMax)
		{
			currentHistogramBinVal = (float)histogramMax;

		}
		frequencies[i] = (currentHistogramBinVal/(float)histogramMax) * frequencyBand + minFrequencyInHz;
#ifndef HARMONIC_OUTPUT

	#ifdef TRANSITION_OUTPUT


			int bufferLength;
			signed short int * sineWaveBuffer = generateSineWaveWithTransition((int) (histogramEntryOutputTime*1000), (int) (histogramEntryOutputTime*1000)/2, (int)frequencies[i], sampleFreq, 0.0f, 32767, (i==0?0:(int)frequencies[i-1]),bufferLength);//generateSineWave((int) (this->histogramEntryOutputTime*1000),0 ,0,(int)frequencies[i], AUDIO_SAMPLING_RATE, 0.0f, 32767,bufferLength);
			int bufferEntries = bufferLength/sizeof(signed short int);

			for(int j = 0; j <bufferEntries && currentBufferEntry < (int)waveBufferLength; j++)
			{ 
				waveBufferOut[currentBufferEntry] = sineWaveBuffer[j];
				currentBufferEntry++;

			}
			#ifdef SAVE_WAVE_FILES_CAPABLE
				sndFile.write(sineWaveBuffer,bufferLength/sizeof(signed short int));
			#endif
			free(sineWaveBuffer);

	


	#else



			int bufferLength;
			
			signed short int * sineWaveBuffer = generateSineWave((int) (histogramEntryOutputTime*1000),0 ,0,(int)frequencies[i], sampleFreq, 0.0f, 32767,bufferLength);
			int bufferEntries = bufferLength/sizeof(signed short int);
			for(int j = 0; j <bufferEntries && currentBufferEntry < (int)waveBufferLength; j++)
			{ 
				waveBufferOut[currentBufferEntry] = sineWaveBuffer[j];
				currentBufferEntry++;

			}

			#ifdef SAVE_WAVE_FILES_CAPABLE
				sndFile.write(sineWaveBuffer,bufferLength/sizeof(signed short int));
			#endif

			free(sineWaveBuffer);

	




	#endif

#else
		int currentFreqWithHarmonics[NUMBER_OF_HARMONICS];
		float phasesArray[NUMBER_OF_HARMONICS];
		signed short int scaleFactorsArray[NUMBER_OF_HARMONICS];
		float * omegaBuffer = NULL;
		for(int l = 0; l < NUMBER_OF_HARMONICS; l++)
		{
			currentFreqWithHarmonics[l] = (l+1) * (int)frequencies[i];
			phasesArray[l] = 0;
			scaleFactorsArray[l] = (int)(32767/NUMBER_OF_HARMONICS);
		}
		



		int bufferLength;
		signed short int * sineWaveBuffer = generateSumOfSinWaves((int) (histogramEntryOutputTime*1000), 0,0,NUMBER_OF_HARMONICS,currentFreqWithHarmonics, sampleFreq, phasesArray, scaleFactorsArray,omegaBuffer, bufferLength);
			//generateSineWaveWithTransition((int) (this->histogramEntryOutputTime*1000), (int) (this->histogramEntryOutputTime*1000)/2, (int)frequencies[i], AUDIO_SAMPLING_RATE, 0.0f, 32767, (i==0?0:(int)frequencies[i-1]),bufferLength);//generateSineWave((int) (this->histogramEntryOutputTime*1000),0 ,0,(int)frequencies[i], AUDIO_SAMPLING_RATE, 0.0f, 32767,bufferLength);
		int bufferEntries = bufferLength/sizeof(signed short int);
		for(int j = 0; j <bufferEntries && currentBufferEntry < waveBufferLength; j++)
		{ 
			waveBufferOut[currentBufferEntry] = sineWaveBuffer[j];
			currentBufferEntry++;

		}

		#ifdef SAVE_WAVE_FILES_CAPABLE
			sndFile.write(sineWaveBuffer,bufferLength/sizeof(signed short int));
		#endif

		free(sineWaveBuffer);

	




#endif




	}

	if(trueModulatedSignal)
	{


		
		



		#ifdef SAVE_WAVE_FILES_CAPABLE			

			SndfileHandle sndFileMod("regionModFileOut.wav",SFM_WRITE,SF_FORMAT_WAV|SF_FORMAT_PCM_16,1,sampleFreq);
		#endif




		int bufferLength;
		//signed short int * sineWaveBuffer = generateFMModulatedSineWave((int) (this->histogramOutputTotalTime  *1000), &((float)this->histogram[0]), this->histogram.size(),3000, AUDIO_SAMPLING_RATE,32767,bufferLength);
		signed short int * sineWaveBuffer = generateFMModulatedSineWave((int) (outputDuration  *1000), &(histogram[0]), histogramLength,3000, sampleFreq,32767,bufferLength);
		int bufferEntries = bufferLength/sizeof(signed short int);
		for(int j = 0; j <bufferEntries && currentBufferEntry < (int)waveBufferLength; j++)
		{ 
			waveBufferOut[currentBufferEntry] = sineWaveBuffer[j];
			currentBufferEntry++;

		}


		#ifdef SAVE_WAVE_FILES_CAPABLE			
			
			sndFileMod.write(sineWaveBuffer,bufferLength/sizeof(signed short int));
		#endif
		free(sineWaveBuffer);

		#ifdef SAVE_WAVE_FILES_CAPABLE			
			sndFileMod.writeSync();
		#endif






		
	}
	#ifdef SAVE_WAVE_FILES_CAPABLE

	sndFile.writeSync();

	#endif



	
	
	
	
	
}



void HistogramSoundSource::generateWaveBufferDataFromHistogram(int histogramLength, float * histogram,float outputDuration,	int sampleFreq,int maxFrequencyInHz, int minFrequencyInHz, float histogramMax,bool normalizeHistogram,enum HistogramSoundSource::SoundGenerationTechniques technique, short int * waveBufferOut, unsigned int & waveBufferLength)
{
	float histogramEntryOutputTime = outputDuration/histogramLength;
	static vector <float> frequencies;
	int currentBufferEntry = 0;

	frequencies.resize(histogramLength);
	if(normalizeHistogram)
	{
		float histogramMaxVal = -FLT_MAX;
		for(int i = 0; i < (int)histogramLength; i++)
		{
			if(histogramMaxVal < histogram[i])
			{
				histogramMaxVal = histogram[i];

			}
		}



		for(int i = 0; i < histogramLength; i++)
		{
			histogram[i] = (histogram[i]/histogramMaxVal)* histogramMax;
		}


	}


	int frequencyBand = maxFrequencyInHz - minFrequencyInHz;


#ifdef SAVE_WAVE_FILES_CAPABLE

	SndfileHandle sndFile("regionFileOut.wav",SFM_WRITE,SF_FORMAT_WAV|SF_FORMAT_PCM_16,1,sampleFreq);


#endif

	for(int i = 0; i <histogramLength; i++)
	{
		float currentHistogramBinVal = (float)histogram[i];

		if(currentHistogramBinVal >= histogramMax)
		{
			currentHistogramBinVal = (float)histogramMax;

		}
		frequencies[i] = (currentHistogramBinVal/(float)histogramMax) * frequencyBand + minFrequencyInHz;


		//Each of these techniques adds a littl to the buffer as it goes
		switch(technique)
		{
			case SOUND_GENERATION_PURE_SINEWAVE:
			{

				int bufferLength;

				signed short int * sineWaveBuffer = generateSineWave((int) (histogramEntryOutputTime*1000),0 ,0,(int)frequencies[i], sampleFreq, 0.0f, 32767,bufferLength);
				int bufferEntries = bufferLength/sizeof(signed short int);
				for(int j = 0; j <bufferEntries && currentBufferEntry < (int)waveBufferLength; j++)
				{
					waveBufferOut[currentBufferEntry] = sineWaveBuffer[j];
					currentBufferEntry++;

				}

				#ifdef SAVE_WAVE_FILES_CAPABLE
					sndFile.write(sineWaveBuffer,bufferLength/sizeof(signed short int));
				#endif

				free(sineWaveBuffer);

				break;
			}

			case SOUND_GENERATION_PURE_SINEWAVE_WITH_TRANSITION:
			{
				int bufferLength;
				signed short int * sineWaveBuffer = generateSineWaveWithTransition((int) (histogramEntryOutputTime*1000), (int) (histogramEntryOutputTime*1000)/2, (int)frequencies[i], sampleFreq, 0.0f, 32767, (i==0?0:(int)frequencies[i-1]),bufferLength);//generateSineWave((int) (this->histogramEntryOutputTime*1000),0 ,0,(int)frequencies[i], AUDIO_SAMPLING_RATE, 0.0f, 32767,bufferLength);
				int bufferEntries = bufferLength/sizeof(signed short int);

				for(int j = 0; j <bufferEntries && currentBufferEntry < (int)waveBufferLength; j++)
				{
					waveBufferOut[currentBufferEntry] = sineWaveBuffer[j];
					currentBufferEntry++;

				}
				#ifdef SAVE_WAVE_FILES_CAPABLE
					sndFile.write(sineWaveBuffer,bufferLength/sizeof(signed short int));
				#endif
				free(sineWaveBuffer);



				break;
			}
			case SOUND_GENERATION_CONVOLUTION:
			{

				int bufferLength;
				//signed short int * sineWaveBuffer = generateFMModulatedSineWave((int) (this->histogramOutputTotalTime  *1000), &((float)this->histogram[0]), this->histogram.size(),3000, AUDIO_SAMPLING_RATE,32767,bufferLength);
				signed short int * sineWaveBuffer = generateFMModulatedSineWave((int) (outputDuration  *1000), &(histogram[0]), histogramLength,3000, sampleFreq,32767,bufferLength);
				int bufferEntries = bufferLength/sizeof(signed short int);
				for(int j = 0; j <bufferEntries && currentBufferEntry < (int)waveBufferLength; j++)
				{
					waveBufferOut[currentBufferEntry] = sineWaveBuffer[j];
					currentBufferEntry++;

				}


				#ifdef SAVE_WAVE_FILES_CAPABLE

					sndFile.write(sineWaveBuffer,bufferLength/sizeof(signed short int));
				#endif
				free(sineWaveBuffer);



				break;
			}

			case SOUND_GENERATION_HARMONICS_PIANO:
			{
				int currentFreq[NUMBER_PIANO_FREQS];
				signed short int scaleFactorsArray[NUMBER_PIANO_FREQS];
				float * omegaBuffer = NULL;
				for(int l = 0; l < NUMBER_PIANO_FREQS; l++)
				{
					currentFreq[l] = (l+1) * (int)frequencies[i];
					scaleFactorsArray[l] = (int)(32000/NUMBER_PIANO_FREQS);
				}




				int bufferLength;
				signed short int * sineWaveBuffer = generateSumOfSineWavesPianoStyle((int) (histogramEntryOutputTime*1000), 1,currentFreq, sampleFreq, scaleFactorsArray,omegaBuffer,bufferLength);
				//signed short int * sineWaveBuffer = generateSumOfSinWaves(, 0,0,NUMBER_OF_HARMONICS,currentFreqWithHarmonics, sampleFreq, phasesArray, scaleFactorsArray,omegaBuffer, bufferLength);
					//generateSineWaveWithTransition((int) (this->histogramEntryOutputTime*1000), (int) (this->histogramEntryOutputTime*1000)/2, (int)frequencies[i], AUDIO_SAMPLING_RATE, 0.0f, 32767, (i==0?0:(int)frequencies[i-1]),bufferLength);//generateSineWave((int) (this->histogramEntryOutputTime*1000),0 ,0,(int)frequencies[i], AUDIO_SAMPLING_RATE, 0.0f, 32767,bufferLength);
				int bufferEntries = bufferLength/sizeof(signed short int);
				for(int j = 0; j <bufferEntries && currentBufferEntry < waveBufferLength; j++)
				{
					waveBufferOut[currentBufferEntry] = sineWaveBuffer[j];
					currentBufferEntry++;

				}

				#ifdef SAVE_WAVE_FILES_CAPABLE
					sndFile.write(sineWaveBuffer,bufferLength/sizeof(signed short int));
				#endif

				free(sineWaveBuffer);

				break;
			}
			default:
			case SOUND_GENERATION_HARMONICS:
			{
				int currentFreqWithHarmonics[NUMBER_OF_HARMONICS];
				float phasesArray[NUMBER_OF_HARMONICS];
				signed short int scaleFactorsArray[NUMBER_OF_HARMONICS];
				float * omegaBuffer = NULL;
				for(int l = 0; l < NUMBER_OF_HARMONICS; l++)
				{
					currentFreqWithHarmonics[l] = (l+1) * (int)frequencies[i];
					phasesArray[l] = 0;
					scaleFactorsArray[l] = (int)(32767/NUMBER_OF_HARMONICS);
				}




				int bufferLength;
				signed short int * sineWaveBuffer = generateSumOfSinWaves((int) (histogramEntryOutputTime*1000), 0,0,NUMBER_OF_HARMONICS,currentFreqWithHarmonics, sampleFreq, phasesArray, scaleFactorsArray,omegaBuffer, bufferLength);
					//generateSineWaveWithTransition((int) (this->histogramEntryOutputTime*1000), (int) (this->histogramEntryOutputTime*1000)/2, (int)frequencies[i], AUDIO_SAMPLING_RATE, 0.0f, 32767, (i==0?0:(int)frequencies[i-1]),bufferLength);//generateSineWave((int) (this->histogramEntryOutputTime*1000),0 ,0,(int)frequencies[i], AUDIO_SAMPLING_RATE, 0.0f, 32767,bufferLength);
				int bufferEntries = bufferLength/sizeof(signed short int);
				for(int j = 0; j <bufferEntries && currentBufferEntry < waveBufferLength; j++)
				{
					waveBufferOut[currentBufferEntry] = sineWaveBuffer[j];
					currentBufferEntry++;

				}

				#ifdef SAVE_WAVE_FILES_CAPABLE
					sndFile.write(sineWaveBuffer,bufferLength/sizeof(signed short int));
				#endif

				free(sineWaveBuffer);


				break;
			}

		}
	}




	//This is for techniques that do the whole buffer at once
	switch(technique)
	{
		case SOUND_GENERATION_CONVOLUTION:
		{

			int bufferLength;
			//signed short int * sineWaveBuffer = generateFMModulatedSineWave((int) (this->histogramOutputTotalTime  *1000), &((float)this->histogram[0]), this->histogram.size(),3000, AUDIO_SAMPLING_RATE,32767,bufferLength);
			signed short int * sineWaveBuffer = generateFMModulatedSineWave((int) (outputDuration  *1000), &(histogram[0]), histogramLength,3000, sampleFreq,32767,bufferLength);
			int bufferEntries = bufferLength/sizeof(signed short int);
			for(int j = 0; j <bufferEntries && currentBufferEntry < (int)waveBufferLength; j++)
			{
				waveBufferOut[currentBufferEntry] = sineWaveBuffer[j];
				currentBufferEntry++;

			}


			#ifdef SAVE_WAVE_FILES_CAPABLE

				sndFile.write(sineWaveBuffer,bufferLength/sizeof(signed short int));
			#endif
			free(sineWaveBuffer);



			break;
		}

		case SOUND_GENERATION_PURE_SINEWAVE:
		case SOUND_GENERATION_HARMONICS:
		case SOUND_GENERATION_PURE_SINEWAVE_WITH_TRANSITION:
		case SOUND_GENERATION_HARMONICS_PIANO:
		default:

		{
			break;
		}

	}


	#ifdef SAVE_WAVE_FILES_CAPABLE

	sndFile.writeSync();

	#endif









}





void HistogramSoundSource::playWaveBufferData(int sampleFreq, signed short int * waveBuffer, int waveBufferLength, int location[3], int velocity[3], bool relativeToListener, bool blocking)
{
	
	int bufferLength =waveBufferLength * sizeof(signed short int);
	ALuint alBuffer = createSoundWaveBuffer(waveBuffer, bufferLength, sampleFreq);
	

	




	ALuint histogramSource;
	alGenSources (1, &(histogramSource));
	ALenum error;
	error = alGetError();
	if(error != AL_NO_ERROR)
	{
		DisplayALError("Gen Source Error: ",error);
	}


	alSource3i(histogramSource,AL_VELOCITY, velocity[0],velocity[1],velocity[2]);
	//ALenum error;
	error = alGetError();
	if(error != AL_NO_ERROR)
	{
		DisplayALError("Set Velocity Error: ",error);
	}


	int lisPos[3];


	alGetListeneriv(AL_POSITION,lisPos);
	//ALenum error;
	error = alGetError();
	if(error != AL_NO_ERROR)
	{
		DisplayALError("Get Listener Error: ",error);
	}

	ALint direction[3];
	direction[0] = lisPos[0] -location[0];
	direction[1] = lisPos[1] -location[1];
	direction[2] = lisPos[2] -location[2];

	alSource3i(histogramSource,AL_DIRECTION, direction[0],direction[1],direction[2]);
	//ALenum error;
	error = alGetError();
	if(error != AL_NO_ERROR)
	{
		DisplayALError("Set Direction Error: ",error);
	}


	alSource3i(histogramSource,AL_POSITION, location[0],location[1],location[2]);
	//ALenum error;
	error = alGetError();
	if(error != AL_NO_ERROR)
	{
		DisplayALError("Position Set Error: ",error);
	}


	
	alSourcei(histogramSource,AL_SOURCE_RELATIVE,relativeToListener?AL_TRUE:AL_FALSE);
	//ALenum error;
	error = alGetError();
	if(error != AL_NO_ERROR)
	{
		DisplayALError("Set Relative  Error: ",error);
	}







	alSourceQueueBuffers(histogramSource,1,&(alBuffer));
	//ALenum error;
	error = alGetError();
	if(error != AL_NO_ERROR)
	{
		DisplayALError("Source Queue Buffer Error: ",error);
	}


	alSourcePlay (histogramSource);
	//ALenum error;
	error = alGetError();
	if(error != AL_NO_ERROR)
	{
		DisplayALError("Source Play Error: ",error);
	}



	if(blocking	)
	{
		bool donePlaying = false;
		while(!donePlaying)
		{
			ALint currentState;

			alGetSourcei(histogramSource,AL_SOURCE_STATE,&currentState);
			//ALenum error;
			error = alGetError();
			if(error != AL_NO_ERROR)
			{
				DisplayALError("Check Source State Error: ",error);
			}

			if(currentState != AL_PLAYING)
			{
				donePlaying =true;

			}
		
		}

		alSourceUnqueueBuffers(histogramSource,1,&(alBuffer));
		ALenum error;
		error = alGetError();
		if(error != AL_NO_ERROR)
		{
			DisplayALError("Source unqueue Buffer Error: ",error);
		}



		alDeleteBuffers(1, &(alBuffer));
		//ALenum error;
		error = alGetError();
		if(error != AL_NO_ERROR)
		{
			DisplayALError("Buffere Delete Error: ",error);
		}

		alDeleteSources(1,&(histogramSource));
		//ALenum error;
		error = alGetError();
		if(error != AL_NO_ERROR)
		{
			DisplayALError("Source Delete Error: ",error);
		}



	}
	
	
}

void HistogramSoundSource::saveWaveBufferDataToAudioFile(string filename,int sampleFreq, signed short int * waveBuffer, int waveBufferLength)
{

#ifdef SAVE_WAVE_FILES_CAPABLE

	SndfileHandle sndFile(filename,SFM_WRITE,SF_FORMAT_WAV|SF_FORMAT_PCM_16,1,sampleFreq);



	sndFile.write(waveBuffer,waveBufferLength);


	sndFile.writeSync();



#endif




	return;
}



void initHistogramSoundSourceManager(HistogramSoundSourceManager * manager)
{
	manager->capacity = 20;
	manager->histogramSoundSources = new HistogramSoundSource[manager->capacity];
	manager->amountInUse = 0;
	manager->automaticCapacityIncreaseAmount = manager->capacity;

}


void releaseHistogramSoundSourceManager(HistogramSoundSourceManager * manager)
{
	delete[] manager->histogramSoundSources;


}

bool increaseHistogramSoundSourceManagerCapacity(int capacityIncreaseAmount, HistogramSoundSourceManager * histogramSoundSourceManager)
{
	bool capacityIncreaseSuccessful = false;

	HistogramSoundSource * newMemory;

	newMemory = new HistogramSoundSource[histogramSoundSourceManager->capacity+histogramSoundSourceManager->automaticCapacityIncreaseAmount];
		
	//(HistogramSoundSource *)realloc(histogramSoundSourceManager->histogramSoundSources,capacityIncreaseAmount);

	if(newMemory!= NULL)
	{
		capacityIncreaseSuccessful = true;
		for(int i = 0; i < histogramSoundSourceManager->capacity; i++)
		{
			newMemory[i] = histogramSoundSourceManager->histogramSoundSources[i];

		}

		delete[] histogramSoundSourceManager->histogramSoundSources;
		histogramSoundSourceManager->histogramSoundSources = newMemory;
		histogramSoundSourceManager->capacity += histogramSoundSourceManager->automaticCapacityIncreaseAmount;

	}


	return capacityIncreaseSuccessful;
}


//Returns the index of the first added Source, if unable to add then returns negative value
int addHistogramSoundSourceToManager(int numberToAdd,HistogramSoundSourceManager * histogramSoundSourceManager)
{
	int firstAddedSource = -1;
	bool capacityAvailable = true;	
	if(histogramSoundSourceManager->amountInUse+numberToAdd > histogramSoundSourceManager->capacity)
	{
		capacityAvailable = increaseHistogramSoundSourceManagerCapacity(histogramSoundSourceManager->automaticCapacityIncreaseAmount, histogramSoundSourceManager);



	}
	if(capacityAvailable)
	{
		firstAddedSource = histogramSoundSourceManager->amountInUse;
		histogramSoundSourceManager->amountInUse+= numberToAdd;

	}


	return firstAddedSource;
}

void histogramSoundSourceManagerResize(HistogramSoundSourceManager * manager, int newSize)
{
	if(newSize<= manager->capacity)
	{
		manager->amountInUse= newSize;
	}
	else
	{

		increaseHistogramSoundSourceManagerCapacity(newSize-manager->capacity +manager->automaticCapacityIncreaseAmount , manager);
	}


}


