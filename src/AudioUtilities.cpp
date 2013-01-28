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

#include "audioUtilities.h"

#ifndef WIN32

#define Sleep(x) {usleep(x*1000);}

#endif
static enum
{
  Unintialized,                 /* ALUT has not been initialized yet or has been de-initialised */
  ALUTDeviceAndContext,         /* alutInit has been called successfully */
  ExternalDeviceAndContext      /* alutInitWithoutContext has been called */
} initialisationState = Unintialized;


/*
 * Note: alutContext contains something valid only when initialisationState
 * contains ALUTDeviceAndContext.
 */
static ALCcontext *alutContext;


void DisplayALError(const char *msg, ALuint error)
{    
	const char *errMsg = NULL;    
	switch (error)    
	{        
		case AL_NO_ERROR:     errMsg = "None";
			break;        
		case AL_INVALID_NAME: errMsg = "Invalid name.";
			break;        
		case AL_INVALID_ENUM: errMsg = "Invalid enum.";
			break;        
		case AL_INVALID_VALUE:errMsg = "Invalid value.";
			break;        // Etc.
		default:              errMsg = "Unknown error.";
			break;    
	}    
	cout << msg << errMsg;

} 


ALuint createSoundWaveBuffer(signed short int * sourceData, int bufferLength, int sampleFreq)
{

	ALuint alBuffer;
	ALenum error;
	//Create the buffers	
	alGenBuffers(1,&alBuffer);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		DisplayALError("alGenBuffers :", error);
		return AL_NONE;
	}



	ALenum format = AL_FORMAT_MONO16;
	ALsizei size = bufferLength;
	ALsizei freq = sampleFreq;

	alBufferData(alBuffer,format,(ALvoid *)sourceData,size,freq);
	if ((error = alGetError()) != AL_NO_ERROR)
	{
		DisplayALError("alBufferData buffer 0 : ", error);
		alDeleteBuffers(1, &alBuffer);
		return AL_NONE;
	}


	return alBuffer;



}




signed short int * generateSumOfSinWaves(int lengthInMilliseconds,int attackLength, int decayLength, int numberOfSineWaves,int * sineWaveFreqs, int sampleFreq, float* phases, signed short int * scaleFactors,float * omegaBuffer,int & bufferLength)
{
	int numberOfEntries = (lengthInMilliseconds*sampleFreq)/1000 +1;
	const float piVal =  3.14159265358979323846f;
	bufferLength = numberOfEntries * sizeof(signed short int);
	signed short int * sineWaveBuffer = (signed short int *)malloc(bufferLength);

	float * omega = omegaBuffer;
	if(omegaBuffer ==NULL)
	{
		omega = (float*) malloc(sizeof(float) * numberOfSineWaves); 
	}
	for(int i = 0; i < numberOfSineWaves; i++)
	{
		omega[i] = 2 * piVal * sineWaveFreqs[i];

	}
	float timeStep = 1.0f/(float)sampleFreq;
	float currentTime = -timeStep;
	float attackDecayFactor = 1.0;
	float attackLengthFloat = (float)attackLength/1000.0f;
	float decayLengthFloat = (float)decayLength/1000.0f;
	float lengthInMillisecondsFloat = (float)lengthInMilliseconds/1000.0f;
	float holdLengthFloat = lengthInMillisecondsFloat -(attackLengthFloat +decayLengthFloat);

	for(int i = 0; i < numberOfEntries; i++)
	{
		currentTime +=timeStep;
		if(currentTime <=attackLengthFloat)
		{
			if(attackLength != 0)
			{
				attackDecayFactor = (float)(currentTime)/(float)attackLengthFloat;
			}
			else
			{
				attackDecayFactor = 1.0f;
			}



		}else if(currentTime <=holdLengthFloat+attackLengthFloat)
		{
			attackDecayFactor = 1.0f;

		}
		else
		{
			if(lengthInMillisecondsFloat-(holdLengthFloat+attackLengthFloat) != 0)
			{
				attackDecayFactor = (float)(lengthInMillisecondsFloat-currentTime)/(float)(lengthInMillisecondsFloat-(holdLengthFloat+attackLengthFloat));
			}
			else
			{
				attackDecayFactor = 1.0f;
			}
		}



		sineWaveBuffer[i] = 0;
		for(int j = 0; j< numberOfSineWaves; j++)
		{
			sineWaveBuffer[i] += (signed short int)(sin(currentTime* omega[j] + phases[j]) * scaleFactors[j]* attackDecayFactor);
		}

	}

	if(omegaBuffer ==NULL)
	{
		free(omega); 
	}

	return sineWaveBuffer;




}


signed short int * generateFMModulatedSineWave(int lengthInMilliseconds, float * signal, int signalCount, int sineWaveFreq, int sampleFreq, signed short int scaleFactor, int & bufferLength)
{
	float signalIncrementPoint = ((float)lengthInMilliseconds)/(signalCount*1000);

	float signalAccumulation =0.0;

	

	int numberOfEntries = (lengthInMilliseconds*sampleFreq)/1000 +1;
	const float piVal =  3.14159265358979323846f;
	float maxSignal = FLT_MIN;

	for(int i = 0;i<signalCount;i++)
	{
		if(signal[i] > maxSignal)
		{
			maxSignal = signal[i];
		}


	}

	float modulationConstant = ((float)sineWaveFreq/(float)sampleFreq)*2*piVal/(float)maxSignal;

	bufferLength = numberOfEntries * sizeof(signed short int);
	signed short int * sineWaveBuffer = (signed short int *)malloc(bufferLength);

	float timeStep = 1.0f/(float)sampleFreq;
	float omega = 2 * piVal * sineWaveFreq;
	float currentTime = -timeStep;
	float currentSignalIncrementTime = -timeStep;

	float lengthInMillisecondsFloat = (float)lengthInMilliseconds/1000.0f;

	int currentSignalEntry = 0;

	for(int i = 0; i < numberOfEntries; i++)
	{




		currentTime +=timeStep;
		currentSignalIncrementTime += timeStep;
		if(currentSignalIncrementTime >= signalIncrementPoint)
		{
			currentSignalIncrementTime = 0;
			currentSignalEntry++;
		}

		signalAccumulation += signal[currentSignalEntry];
		sineWaveBuffer[i] = (signed short int)(sin(currentTime* omega + signalAccumulation*modulationConstant ) * scaleFactor );
		

	}
	return sineWaveBuffer;




}


signed short int * generateSineWave(int lengthInMilliseconds,int attackLength, int decayLength, int sineWaveFreq, int sampleFreq, float phase, signed short int scaleFactor, int & bufferLength)
{
	int numberOfEntries = (lengthInMilliseconds*sampleFreq)/1000 +1;
	const float piVal =  3.14159265358979323846f;
	bufferLength = numberOfEntries * sizeof(signed short int);
	signed short int * sineWaveBuffer = (signed short int *)malloc(bufferLength);

	float timeStep = 1.0f/(float)sampleFreq;
	float omega = 2 * piVal * sineWaveFreq;
	float currentTime = -timeStep;
	float attackDecayFactor = 1.0;

	float attackLengthFloat = (float)attackLength/1000.0f;
	float decayLengthFloat = (float)decayLength/1000.0f;
	float lengthInMillisecondsFloat = (float)lengthInMilliseconds/1000.0f;
	float holdLengthFloat = lengthInMillisecondsFloat -(attackLengthFloat +decayLengthFloat);
	for(int i = 0; i < numberOfEntries; i++)
	{


		currentTime +=timeStep;
		if(currentTime <=attackLengthFloat)
		{
			if(attackLength != 0)
			{
				attackDecayFactor = (float)(currentTime)/(float)attackLengthFloat;
			}
			else
			{
				attackDecayFactor = 1.0f;
			}



		}else if(currentTime <=holdLengthFloat+attackLengthFloat)
		{
			attackDecayFactor = 1.0f;

		}
		else
		{
			if(lengthInMillisecondsFloat-(holdLengthFloat+attackLengthFloat) != 0)
			{
				attackDecayFactor = (float)(lengthInMillisecondsFloat-currentTime)/(float)(lengthInMillisecondsFloat-(holdLengthFloat+attackLengthFloat));
			}
			else
			{
				attackDecayFactor = 1.0f;
			}
		}


		sineWaveBuffer[i] = (signed short int)(sin(currentTime* omega + phase) * scaleFactor * attackDecayFactor);
		

	}
	return sineWaveBuffer;
}

signed short int * generateSineWaveWithTransition(int lengthInMilliseconds,int transitionLength, int sineWaveFreq, int sampleFreq, float phase, signed short int scaleFactor, int prevFreq, int & bufferLength)
{
	int numberOfEntries = (lengthInMilliseconds*sampleFreq)/1000 +1;
	const float piVal =  3.14159265358979323846f;
	bufferLength = numberOfEntries * sizeof(signed short int);
	signed short int * sineWaveBuffer = (signed short int *)malloc(bufferLength);

	float timeStep = 1.0f/(float)sampleFreq;
	float omega = 2 * piVal * sineWaveFreq;
	float currentTime = -timeStep;


	float transitionLengthFloat = (float)transitionLength/1000.0f;
	float frequencyDelta = ((sineWaveFreq- prevFreq)/(transitionLengthFloat))* timeStep;
	
	float currentFreq = prevFreq- frequencyDelta;
	float currentOmega;

	float lengthInMillisecondsFloat = (float)lengthInMilliseconds/1000.0f;

	float currentAngle = phase;
	for(int i = 0; i < numberOfEntries; i++)
	{


		currentTime +=timeStep;
		if(currentTime <=transitionLengthFloat)
		{
			currentFreq+=frequencyDelta;
			currentOmega = 2 * piVal * currentFreq;

		}else 
		{

			currentOmega = omega;



		}
		currentAngle += currentOmega * timeStep;

		sineWaveBuffer[i] = (signed short int)(sin(currentAngle ) * scaleFactor);
		

	}
	return sineWaveBuffer;
}



struct HarmonicFactors
{
	float harmFactor;
	float decayFactor;
	float volumeFactor;



};





struct HarmonicFactors getHarmonicFactors(int frequencyInHz)
{

	HarmonicFactors harmonicFactors;
	int harmMax = 4;

	float lowFreq = log((float)frequencyInHz);
	float lowFreqFac = (lowFreq-3)/((float)harmMax);
	if(lowFreqFac > 1)
	{
		harmonicFactors.harmFactor = 0;

	}
	else
	{
		harmonicFactors.harmFactor = 2* (1-lowFreqFac);

	}

	harmonicFactors.decayFactor = 2/lowFreq;

	float t = (float)((lowFreq-3) /(8.5-3.0));

	harmonicFactors.volumeFactor = (float)(1.0 + .8 * t * cos((3.1415926/5.3) * (lowFreq-3)));
	return harmonicFactors;
}


struct AudioCharacteristics
{
	float baseFactor;
	float firstAttackLimit;
	float secondAttackLimit;
	float decayLengthInSeconds;





};


float pianoKeys[]= {
	   27.50f,
	   29.14f,
	   30.87f,
	   32.70f,
	   34.65f,
	   36.71f,
	   38.89f,
	   41.20f,
	   43.65f,
	   46.25f,
	   49.00f,
	   51.91f,
	   55.00f,
	   58.27f,
	   61.74f,
	   65.41f,
	   69.30f,
	   73.42f,
	   77.78f,
	   82.41f,
	   87.31f,
	   92.50f,
	   98.00f,
	  103.83f,
	  110.00f,
	  116.54f,
	  123.47f,
	  130.81f,
	  138.59f,
	  146.83f,
	  155.56f,
	  164.81f,
	  174.61f,
	  185.00f,
	  196.00f,
	  207.65f,
	  220.00f,
	  233.08f,
	  246.94f,
	  261.63f,
	  277.18f,
	  293.66f,
	  311.13f,
	  329.63f,
	  349.23f,
	  369.99f,
	  392.00f,
	  415.30f,
	  440.00f,
	  466.16f,
	  493.88f,
	  523.25f,
	  554.37f,
	  587.33f,
	  622.25f,
	  659.26f,
	  698.46f,
	  739.99f,
	  783.99f,
	  830.61f,
	  880.00f,
	  932.33f,
	  987.77f,
	 1046.50f,
	 1108.73f,
	 1174.66f,
	 1244.51f,
	 1318.51f,
	 1396.91f,
	 1479.98f,
	 1567.98f,
	 1661.22f,
	 1760.00f,
	 1864.66f,
	 1975.53f,
	 2093.00f,
	 2217.46f,
	 2349.32f,
	 2489.02f,
	 2637.02f,
	 2793.83f,
	 2959.96f,
	 3135.96f,
	 3322.44f,
	 3520.00f,
	 3729.31f,
	 3951.07f,
	 4186.01f
};





signed short int * generateSumOfSineWavesPianoStyle(int lengthInMilliseconds, int numberOfFrequencies,int * freqs, int sampleFreq, signed short int * scaleFactors,float * omegaBuffer,int & bufferLength)
{
	int numberOfEntries = (lengthInMilliseconds*sampleFreq)/1000 +1;
	const float piVal =  3.14159265358979323846f;
	bufferLength = numberOfEntries * sizeof(signed short int);
	signed short int * sineWaveBuffer = (signed short int *)malloc(bufferLength);




	float * omega = omegaBuffer;
	if(omegaBuffer ==NULL)
	{
		omega = (float*) malloc(sizeof(float) * numberOfFrequencies);
	}
	HarmonicFactors	* harmonicFactors = (HarmonicFactors	*)calloc(numberOfFrequencies,sizeof(HarmonicFactors));

	for(int i = 0; i < numberOfFrequencies; i++)
	{
		omega[i] = 2 * piVal * freqs[i];
		harmonicFactors[i] = getHarmonicFactors(freqs[i]);
	}





	float timeStep = 1.0f/(float)sampleFreq;
	float currentTime = -timeStep;
	float lengthInSecondsFloat = (float)lengthInMilliseconds/1000.0f;
	AudioCharacteristics audioChars = {1.0f, .0045f, .0136f,.01814f};

	for(int i = 0; i < numberOfEntries; i++)
	{
		currentTime +=timeStep;
		float attackDecayFactor = audioChars.baseFactor;
		/*


		if(i < 100)
		{
			attackDecayFactor = (float)i/80;//(float)(currentTime/.003628);

		}
		else if(100 <= i && i < 300  )
		{
			attackDecayFactor = (float)(1.25-(i-100)/800);
			//attackDecayFactor = (float)(1.25-(currentTime-audioChars.firstAttackLimit)/.036281);

		}
		else if(currentTime > lengthInSecondsFloat-audioChars.decayLengthInSeconds)
		{

			//attackDecayFactor = (float)(1.0-(currentTime-lengthInSecondsFloat +audioChars.decayLengthInSeconds)/audioChars.decayLengthInSeconds);
			attackDecayFactor = (float)(1.0-(i-numberOfEntries +400)/400);

		}
		float amountComplete = ((float)i)/ (float)numberOfEntries;

*/

		float firstFactorDenom = (audioChars.firstAttackLimit*.8); //.0036281
		float secondFactorDenom = firstFactorDenom*10;//.036281
		if(currentTime < audioChars.firstAttackLimit)
		{
			attackDecayFactor = (float)(currentTime/firstFactorDenom);

		}
		else if(audioChars.firstAttackLimit <= currentTime && currentTime < audioChars.secondAttackLimit  )
		{

			attackDecayFactor = (float)(1.25-(currentTime-audioChars.firstAttackLimit)/(secondFactorDenom));

		}
		else if(currentTime > lengthInSecondsFloat-audioChars.decayLengthInSeconds)
		{

			attackDecayFactor = (float)(1.0-(currentTime-lengthInSecondsFloat +audioChars.decayLengthInSeconds)/audioChars.decayLengthInSeconds);

		}

		float amountComplete = currentTime/lengthInSecondsFloat;


		sineWaveBuffer[i] = 0;
		for(int j = 0; j< numberOfFrequencies; j++)
		{


			float currentDecay = (float)(1.0- amountComplete + amountComplete*harmonicFactors[j].decayFactor);
				//ow=ow+sixteenbit((asin(float(x)/l[0])+harm*asin(float(x)/(l[0]/2.))+.5*harm*asin(float(x)/(l[0]/4.)))/4.*fac*vol*dfac*volfac)


			//sineWaveBuffer[i]+= (signed short int)(sin(currentTime* omega[j])* (float)scaleFactors[j]);
			sineWaveBuffer[i] += (signed short int)(((sin(currentTime* omega[j])
													 + harmonicFactors[j].harmFactor*sin(currentTime* omega[j]*2.)
													 + harmonicFactors[j].harmFactor*.5*sin(currentTime* omega[j]*4.))/4.)
													 * attackDecayFactor*(float)scaleFactors[j]*harmonicFactors[j].volumeFactor*currentDecay);
			//sineWaveBuffer[i] += (signed short int)(((sin(2 * 3.1415926* i/100.0)
			//										 + harmonicFactors[j].harmFactor*sin((2 * 3.1415926* i/100.0)/2)
			//										 + harmonicFactors[j].harmFactor*.5*sin((2 * 3.1415926* i/100.0)/4))/4)
			//										 * attackDecayFactor*(float)scaleFactors[j]*harmonicFactors[j].volumeFactor*currentDecay);


		}

	}

	if(omegaBuffer ==NULL)
	{
		free(omega);
	}

	free(harmonicFactors);


	return sineWaveBuffer;




}



ALuint createSumOfSineWaveBuffer(int lengthInMilliseconds, int attackLength,int decayLength,int numberOfSineWaves,int * sineWaveFreqs, int sampleFreq, float* phases, signed short int * scaleFactors,float * omegaBuffer)
{
	ALuint alBuffer;
	int bufferLength;
	signed short int * sumOfSineWavesBuffer = generateSumOfSinWaves(lengthInMilliseconds,attackLength ,decayLength,numberOfSineWaves,sineWaveFreqs, sampleFreq, phases, scaleFactors,NULL,bufferLength);
	alBuffer =  createSoundWaveBuffer(sumOfSineWavesBuffer , bufferLength, sampleFreq);
	free(sumOfSineWavesBuffer);
	return alBuffer;
}

ALuint createSineWaveBuffer(int lengthInMilliseconds, int attackLength,int decayLength,int sineWaveFreq, int sampleFreq, float phase, signed short int  scaleFactor)
{
	ALuint alBuffer;
	int bufferLength;
	signed short int * sineWaveBuffer = generateSineWave(lengthInMilliseconds,attackLength ,decayLength,sineWaveFreq, sampleFreq, phase, scaleFactor,bufferLength);
	alBuffer =  createSoundWaveBuffer(sineWaveBuffer , bufferLength, sampleFreq);
	free(sineWaveBuffer);
	return alBuffer;
}

ALuint createSineWaveWithTransitionBuffer(int lengthInMilliseconds, int transitionLength, int sineWaveFreq, int sampleFreq, float phase, signed short int  scaleFactor, int prevFreq)
{
	ALuint alBuffer;
	int bufferLength;
	signed short int * sineWaveBuffer = generateSineWaveWithTransition(lengthInMilliseconds,transitionLength, sineWaveFreq, sampleFreq, phase, scaleFactor, prevFreq,bufferLength);//generateSineWave(lengthInMilliseconds,attackLength ,decayLength,sineWaveFreq, sampleFreq, phase, scaleFactor,bufferLength);
	alBuffer =  createSoundWaveBuffer(sineWaveBuffer , bufferLength, sampleFreq);
	free(sineWaveBuffer);
	return alBuffer;
}


ALuint createFMModulatedSineWaveBuffer(int lengthInMilliseconds, float * signal, int signalCount,int sineWaveFreq, int sampleFreq,signed short int  scaleFactor)
{
	ALuint alBuffer;
	int bufferLength;

	signed short int * sineWaveBuffer = generateFMModulatedSineWave(lengthInMilliseconds, signal, signalCount, sineWaveFreq, sampleFreq, scaleFactor,bufferLength);
	alBuffer =  createSoundWaveBuffer(sineWaveBuffer , bufferLength, sampleFreq);
	free(sineWaveBuffer);
	return alBuffer;
}



ALuint createSumOfSineWavesPianoStyleBuffer(int lengthInMilliseconds, int numberOfFrequencies,int * freqs, int sampleFreq, signed short int * scaleFactors,float * omegaBuffer)
{
	ALuint alBuffer;
	int bufferLength;
	signed short int * wavesBuffer = generateSumOfSineWavesPianoStyle(lengthInMilliseconds, numberOfFrequencies,freqs, sampleFreq, scaleFactors,NULL,bufferLength);
	alBuffer =  createSoundWaveBuffer(wavesBuffer , bufferLength, sampleFreq);
	free(wavesBuffer);
	return alBuffer;
}



void saveSineWaveToFile( char * filename,signed short int * dataBuffer,int bufferLength, int bufferEntrySize)
{
	FILE * outputFile;


	outputFile = fopen(filename,"w");

	int arrayLength = bufferLength/bufferEntrySize;

	for(int i = 0; i< arrayLength; i++)
	{
		fprintf(outputFile,"%d\n",dataBuffer[i]);

	}
	fclose(outputFile);
}

void testSineWave()
{
	int bufferLength;
	signed short int * sineWaveBuffer = generateSineWave(250,0,0,10, 44100, 0, 32767,bufferLength);
	saveSineWaveToFile("sineWave.txt",sineWaveBuffer, bufferLength, sizeof(signed short int));
	int numberOfSineWaves = 1;//5;
	int sineWaveFreqs[] = {600, 1200, 542, 1084, 1400};//262,523,785,1046,1318,1568,1865,2093};//, 440, 660, 880, 1100};
	int sampleFreq = 44100;
	float phases[] = {0,0,0,0,0,0,0,0};//,0,0,0,0};
	signed short int scaleFactors[] = {32767/5,32767/5,32767/5,32767/5,32767/5};//32767/2,32767/4,32767/8,32767/16,32767/32,32767/64,32767/128,32767/256}; ///5,32767/5,32767/5,32767/5,32767/5};
	signed short int * sumOfSineWavesBuffer = generateSumOfSinWaves(250, 0,0,numberOfSineWaves,sineWaveFreqs, sampleFreq, phases, scaleFactors,NULL,bufferLength);
	saveSineWaveToFile("sumSineWave.txt",sumOfSineWavesBuffer, bufferLength, sizeof(signed short int));

	ALuint sineWaveAlBuffer, sineWaveSource;
	alGenSources (1, &sineWaveSource);

	sineWaveAlBuffer =  createSumOfSineWaveBuffer(500, 100,100,numberOfSineWaves,sineWaveFreqs, sampleFreq, phases, scaleFactors,NULL);
	alSourcei (sineWaveSource, AL_BUFFER, sineWaveAlBuffer);
	alSourcePlay (sineWaveSource);
	Sleep (2000);	
	alDeleteBuffers(1,&sineWaveAlBuffer);

	return;

}
void testToneWave()
{
	int bufferLength;

	int numberOfSineWaves = 1;//5;
	int sineWaveFreqs[] = {500,(int)pianoKeys[47],(int)pianoKeys[48],(int)pianoKeys[49]};//262,523,785,1046,1318,1568,1865,2093};//, 440, 660, 880, 1100};
	int sampleFreq = 44100;

	signed short int scaleFactors[] = {32000/numberOfSineWaves,32000/numberOfSineWaves,32000/numberOfSineWaves,32000/numberOfSineWaves,32000/numberOfSineWaves};//32767/2,32767/4,32767/8,32767/16,32767/32,32767/64,32767/128,32767/256}; ///5,32767/5,32767/5,32767/5,32767/5};

	signed short int * sumOfSineWavesBuffer =generateSumOfSineWavesPianoStyle(100,numberOfSineWaves,sineWaveFreqs,sampleFreq, scaleFactors,NULL,bufferLength);
	saveSineWaveToFile("ToneWave.txt",sumOfSineWavesBuffer, bufferLength, sizeof(signed short int));

	ALuint sineWaveAlBuffer, sineWaveSource;
	alGenSources (1, &sineWaveSource);
	sineWaveAlBuffer =  createSumOfSineWavesPianoStyleBuffer(100,numberOfSineWaves,sineWaveFreqs,sampleFreq, scaleFactors,NULL);

	alSourcei (sineWaveSource, AL_BUFFER, sineWaveAlBuffer);
	alSourcePlay (sineWaveSource);
	Sleep (90);
	alDeleteBuffers(1,&sineWaveAlBuffer);

	return;

}

void testToneWave(int i)
{
	int bufferLength;

	int numberOfSineWaves = 1;//5;
	int sineWaveFreqs[] = {(int)pianoKeys[i],(int)pianoKeys[47],(int)pianoKeys[48],(int)pianoKeys[49]};//262,523,785,1046,1318,1568,1865,2093};//, 440, 660, 880, 1100};
	int sampleFreq = 44100;

	signed short int scaleFactors[] = {32000/numberOfSineWaves,32000/numberOfSineWaves,32000/numberOfSineWaves,32000/numberOfSineWaves,32000/numberOfSineWaves};//32767/2,32767/4,32767/8,32767/16,32767/32,32767/64,32767/128,32767/256}; ///5,32767/5,32767/5,32767/5,32767/5};

	signed short int * sumOfSineWavesBuffer =generateSumOfSineWavesPianoStyle(100,numberOfSineWaves,sineWaveFreqs,sampleFreq, scaleFactors,NULL,bufferLength);
	//saveSineWaveToFile("ToneWave.txt",sumOfSineWavesBuffer, bufferLength, sizeof(signed short int));

	ALuint sineWaveAlBuffer, sineWaveSource;
	alGenSources (1, &sineWaveSource);
	sineWaveAlBuffer =  createSumOfSineWavesPianoStyleBuffer(100,numberOfSineWaves,sineWaveFreqs,sampleFreq, scaleFactors,NULL);

	alSourcei (sineWaveSource, AL_BUFFER, sineWaveAlBuffer);
	alSourcePlay (sineWaveSource);
	Sleep (90);
	alDeleteBuffers(1,&sineWaveAlBuffer);
	alDeleteSources(1,&sineWaveSource);
	return;

}



/*
 * Note: alutContext contains something valid only when initialisationState
 * contains ALUTDeviceAndContext.
 */
//static ALCcontext *alutContext;

ALboolean
_alutSanityCheck (void)
{
  ALCcontext *context;

  if (initialisationState == Unintialized)
    {
      //_alutSetError (ALUT_ERROR_INVALID_OPERATION);
		DisplayALError("Sanity Check Invalid op", 0);
		return AL_FALSE;
    }

  context = alcGetCurrentContext ();
  if (context == NULL)
    {
      //_alutSetError (ALUT_ERROR_NO_CURRENT_CONTEXT);
		DisplayALError("Sanity Check No current context", 0);
		return AL_FALSE;
    }
  ALenum error = alGetError();
  if ( error != AL_NO_ERROR)
    {
      //_alutSetError (ALUT_ERROR_AL_ERROR_ON_ENTRY);
		DisplayALError("Sanity Check AL error", error);
		return AL_FALSE;
    }

  if (alcGetError (alcGetContextsDevice (context)) != ALC_NO_ERROR)
    {
      //_alutSetError (ALUT_ERROR_ALC_ERROR_ON_ENTRY);
		DisplayALError("Sanity Check ALC error", 0);
		return AL_FALSE;
    }

  return AL_TRUE;
}

ALboolean
auInit ()
{
  ALCdevice *device;
  ALCcontext *context;

  if (initialisationState != Unintialized)
    {
      //_alutSetError (ALUT_ERROR_INVALID_OPERATION);
		DisplayALError("Init Invalid op", 0);
		return AL_FALSE;
    }


  device = alcOpenDevice (NULL);
  if (device == NULL)
    {
      DisplayALError("Init device is null", 0);;
      return AL_FALSE;
    }

  context = alcCreateContext (device, NULL);
  if (context == NULL)
    {
		alcCloseDevice (device);
		DisplayALError("Init context is null", 0);
		return AL_FALSE;
    }

  if (!alcMakeContextCurrent (context))
    {
		alcDestroyContext (context);
		alcCloseDevice (device);
		DisplayALError("Init make context failed", 0);;
		return AL_FALSE;
    }

  initialisationState = ALUTDeviceAndContext;
  alutContext = context;
  return AL_TRUE;
}


ALboolean auExit (void)
{
  ALCdevice *device;

  if (initialisationState == Unintialized)
    {
      DisplayALError("Sanity Check Invalid op", 0);
      return AL_FALSE;
    }

  if (initialisationState == ExternalDeviceAndContext)
    {
      initialisationState = Unintialized;
      return AL_TRUE;
    }

  if (!_alutSanityCheck ())
    {
      return AL_FALSE;
    }

  if (!alcMakeContextCurrent (NULL))
    {
      DisplayALError("Exit Make context current failed", 0);;
      return AL_FALSE;
    }

  device = alcGetContextsDevice (alutContext);
  alcDestroyContext (alutContext);
  if (alcGetError (device) != ALC_NO_ERROR)
    {
      DisplayALError("Exit Destroy conext failed", 0);
      return AL_FALSE;
    }

  if (!alcCloseDevice (device))
    {
      DisplayALError("Exit Close device failed", 0);
      return AL_FALSE;
    }

  initialisationState = Unintialized;
  return AL_TRUE;
}
