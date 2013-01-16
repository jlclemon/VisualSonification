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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "JNIHelp.h"
#include "jni.h"
#include "utils/Log.h"
#include "utils/misc.h"
#include "android_runtime/AndroidRuntime.h"
#include <android/bitmap.h>
#include <opencv2/core/core.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <vector>
#include <math.h>
#include <AL/al.h>
#include <AL/alc.h>
#include "VisualSonification.hpp"
 
using namespace std;
using namespace cv;

 typedef struct 
 {
	 char  riff[4];//'RIFF'
	 unsigned int riffSize;
	 char  wave[4];//'WAVE'
	 char  fmt[4];//'fmt '
	 unsigned int fmtSize;
	 unsigned short format;
	 unsigned short channels;
	 unsigned int samplesPerSec;
	 unsigned int bytesPerSec;
	 unsigned short blockAlign;
	 unsigned short bitsPerSample;
	 char  data[4];//'data'
	 unsigned int dataSize;
}BasicWAVEHeader;

//WARNING: This Doesn't Check To See If These Pointers Are Valid
char* readWAV(char* filename,BasicWAVEHeader* header)
{
	char* buffer = 0;
	FILE* file = fopen(filename,"rb");
	if (!file) 
	{
		LOGE("Read Wave: f openfailed");
		return 0;
	}

	if (fread(header,sizeof(BasicWAVEHeader),1,file))
	{
//		string dataString;
		char dataToOut[5];
		dataToOut[4] = '\0';
		memcpy(dataToOut,header->riff,4);
//		dataString = dataToOut;
		LOGE("%s",dataToOut);
		memcpy(dataToOut,header->wave,4);
//		dataString = dataToOut;
		LOGE("%s",dataToOut);
		memcpy(dataToOut,header->fmt,4);
//		dataString = dataToOut;
		LOGE("%s",dataToOut);
		memcpy(dataToOut,header->data,4);
//		dataString = dataToOut;
		LOGE("%s",dataToOut);



		if (!(//these things *must* be valid with this basic header
			memcmp("RIFF",header->riff,4) ||
			memcmp("WAVE",header->wave,4) ||
			memcmp("fmt ",header->fmt,4)  ||
			memcmp("data",header->data,4) 
			))
		{
				

			buffer = (char*)malloc(header->dataSize);
			if (buffer)
			{
				if (fread(buffer,header->dataSize,1,file))
				{
					fclose(file);
					return buffer;
				}
				else
				{
					LOGE("Read Wave: fread failed");
					
				}
				free(buffer);
			}
			else
			{
				
						LOGE("Read Wave: malloc failed");
				
			}
		}
		else
		{
			
			LOGE("Read Wave cmp fail");
		}
	}
	else
	{
		LOGE("Read Wave: header failed");
	}
	fclose(file);
	return 0;
}

ALuint createBufferFromWave(char* data,BasicWAVEHeader header)
{

	ALuint buffer = 0;
	ALuint format = 0;
	switch (header.bitsPerSample)
	{
		case 8:
			format = (header.channels == 1) ? AL_FORMAT_MONO8 : AL_FORMAT_STEREO8;
			break;
		case 16:
			format = (header.channels == 1) ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16;
			break;
		default:
		return 0;
	}

	alGenBuffers(1,&buffer);
	alBufferData(buffer,format,data,header.dataSize,header.samplesPerSec);
	return buffer;
}

namespace android 
{

	ALCdevice* device;
	ALCcontext* context;
	ALuint source;
	ALuint buffer;
	const ALint context_attribs[] = { ALC_FREQUENCY, 22050, 0 };
	static jboolean Initialize(JNIEnv *env, jobject obj)
	{
		 
		 //device = alcOpenDevice(0);
		 //context = alcCreateContext(device, context_attribs);
		 //alcMakeContextCurrent(context);

			//auInit();
		 
		 char fnameArray[] = "/sdcard/test.wav";//{'t','e','s','t','.','w','a','v','\0'};
		 const char* fnameptr = fnameArray;
		 BasicWAVEHeader header;
		 char* data = readWAV(fnameArray,&header);
		 if (data)
		 {

			LOGE("Buffer Sux");
			 //Now We've Got A Wave In Memory, Time To Turn It Into A Usable Buffer
			 buffer = createBufferFromWave(data,header);
			 if (!buffer)
			 {
				LOGE("Buffer Failed");
				free(data);
				return false;
			 }

		 } 
		 else 
		 {

			LOGE("Data not loaded");
			return false;
		 }

		 
		// Create source from buffer and play it
		 source = 0;
		 alGenSources(1, &source );
		 alSourcei(source, AL_BUFFER, buffer);

		 // Play source
		 alSourcePlay(source);
//
//		 int        sourceState = AL_PLAYING;
//		 do {
//		 alGetSourcei(source, AL_SOURCE_STATE, &sourceState);
//		 } while(sourceState == AL_PLAYING);


		return false;
	}

	
	static void Finalize(JNIEnv *env, jobject obj)
	{
		auInit();;
	}
	static void PausePlaying(JNIEnv *env, jobject obj)
	{

		 int        sourceState = AL_PLAYING;
		 
		 alGetSourcei(source, AL_SOURCE_STATE, &sourceState);
		 
		 
		 if(sourceState == AL_PLAYING)
		 {
			alSourcePause(source);			 
			 
		 }


		
	}
	static void StopPlaying(JNIEnv *env, jobject obj)
	{
		 int        sourceState = AL_PLAYING;
		 alGetSourcei(source, AL_SOURCE_STATE, &sourceState);
		 if(sourceState == AL_PLAYING)
		 {

			alSourceStop(source);
		 }	
		 // Release source
		 alDeleteSources(1, &source);

		 // Release audio buffer
		 alDeleteBuffers(1, &buffer);
		 // Cleaning up
		 alcMakeContextCurrent(0);
		 alcDestroyContext(context);
		 alcCloseDevice(device);


		
	}
	static void StartPlaying(JNIEnv *env, jobject obj)
	{
		 // Play source
		 alSourcePlay(source);


		
	}


	static jstring Run(JNIEnv *env, jobject obj)
	{
		Mat mrgba(240, 320, CV_8UC4);
		vector<int> test;
		// Global Variables
		 //device = 0;
		 //context = 0;
	

		 // Initialization

		// Create audio buffer






		return (env)->NewStringUTF("Visual SOnfication !");
	}
  
	static jboolean GetLowerImage(JNIEnv *env, jobject obj, jint width, jint height,  jobject bitmap)//jintArray rgba)
	{
		static int locationX=0, locationY=0;
		float step = 3.1415f/50.0f;
		static float currentAngle = 0;

		float radius = 20;
		static float t = 0;
		bool paint = true;
		static int paintToggleCount=0;
		AndroidBitmapInfo  info;		
		void*              pixels;		
		

//		jint*  _rgba = env->GetIntArrayElements(rgba, 0);
		Mat mrgba(height, width, CV_8UC4);

		mrgba.setTo(Scalar(0,0,0,0));
		//circle(mrgba, Point(locationX, locationY), 12, Scalar(0,0,0,0), CV_FILLED);
		Mat imageFromSdCardTmp= imread("/sdcard/test.jpg");
		Mat imageFromSdCard(height,width,CV_8UC3);
		Mat imageFromSdCardTmp2(height,width,CV_8UC3);
		LOGE("SonificationDemo REsize");
		resize(imageFromSdCardTmp,imageFromSdCardTmp2,Size(width,height));
		currentAngle +=5;
		if(currentAngle >=360)
		{
			
			currentAngle = 0;
		}


		Point2f src_center(imageFromSdCard.cols/2.0F, imageFromSdCard.rows/2.0F);
		Mat rot_mat = getRotationMatrix2D(src_center, currentAngle, 1.0);
		Mat dst;

		imageFromSdCard.setTo(Scalar(0,0,0));
		warpAffine(imageFromSdCardTmp2, imageFromSdCard, rot_mat, Size(width,height));


		if ( AndroidBitmap_getInfo(env, bitmap, &info) < 0 )
			return false; // can't get info
 
		if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888)
			return false; // incompatible format

		if ( AndroidBitmap_lockPixels(env, bitmap, &pixels) < 0 )
			return false; // can't get pixels
		int * pixelData = (int *) pixels;

		if(!imageFromSdCard.empty())
		{
			LOGE("SonificationDemo Card load Success");
			for(int y = 0; y < imageFromSdCard.rows && y< mrgba.rows; y++)
			{
				unsigned char * mrgbaPtr = mrgba.data + y * mrgba.step[0];
				unsigned char * imagePtr = imageFromSdCard.data + y * imageFromSdCard.step[0];
				
				for(int x = 0; x < imageFromSdCard.cols && x< mrgba.cols; x++)
				{
					mrgbaPtr[x*4 + 0] = 0xff;
					mrgbaPtr[x*4 + 1] = imagePtr[x*3 + 0];
					mrgbaPtr[x*4 + 2] = imagePtr[x*3 + 1];				
					mrgbaPtr[x*4 + 3] = imagePtr[x*3 + 2];
					int index = y * info.stride/sizeof(int) + x;

 
					 //R,G.B - Red, Green, Blue
					 //to restore the values after RGB modification, use 
					 //next statement
					 //pixelData[index] = 0xff000000 | (imagePtr[x*3 + 2] << 16) | (imagePtr[x*3 + 1] << 8) | imagePtr[x*3 + 0];
					 pixelData[index] = (((int)mrgbaPtr[x*4 + 0]<<24)) | (((int)mrgbaPtr[x*4 + 3]) << 16) | (((int)mrgbaPtr[x*4 + 2]) << 8) | mrgbaPtr[x*4 + 1];


					
				}
						
				
				
				
			}
		}
		else
		{
				LOGE("SonificationDemo Card load failed!");
		}
		
		if(paint)
		{
			
			t += step;
			
			if(step >= 6.28f)
			{
				step = 0;
			}

			locationX = width/2+ radius * cos(t);
			locationY = height/2 + radius * sin(t);

		


		
		
			paintToggleCount++;
			
			if(paintToggleCount >=1)
			{
				paint = !paint;
				paintToggleCount = 0;
			}
	
			circle(mrgba, Point(locationX, locationY), 10, Scalar(255,0,0,255), CV_FILLED);
			
		}
		
		else
		{
			//paintToggleCount = 0;	
			mrgba.setTo(Scalar(0));			
			paintToggleCount++;
			if(paintToggleCount >=1)
			{
				paint = !paint;
				paintToggleCount = 0;
			}



		}



  

//		if(mrgba.data && pixels)
//			memcpy(pixels, mrgba.data, info.height * info.width * 4);



		AndroidBitmap_unlockPixels(env, bitmap);




//		env->ReleaseIntArrayElements(rgba, _rgba, 0);

		return true;
	}

	static jlong CreateVisualSonificationObj(JNIEnv *env, jobject obj)
	{
			VisualSonification * visualSonificationInstance = new VisualSonification;
			visualSonificationInstance->initSonificationProcessing();
			LOGE("VisualSonification Object created and initialized!");		
			return (jlong)(visualSonificationInstance);
		
		
	}
	static jlong VisualSonificationExecuteState(JNIEnv *env, jobject obj, jlong visualSonificationPtrAsLong)
	{
			VisualSonification * visualSonificationInstance = (VisualSonification *)visualSonificationPtrAsLong;
			LOGE("VisualSonification State Executed!");		
			return visualSonificationInstance->runSonificationProcessing();
		
		
	}
	static jboolean VisualSonificationInit(JNIEnv *env, jobject obj,jlong visualSonificationPtrAsLong)
	{
			VisualSonification * visualSonificationInstance = (VisualSonification *)visualSonificationPtrAsLong;
//			visualSonificationInstance->initSonificationProcessing();
			LOGE("VisualSonification Object Initialized!");		
			return (jboolean)true;
		
		
	}

	static jboolean VisualSonificationCleanUp(JNIEnv *env, jobject obj,jlong visualSonificationPtrAsLong)
	{
			VisualSonification * visualSonificationInstance = (VisualSonification *)visualSonificationPtrAsLong;

			delete visualSonificationInstance;
			LOGE("VisualSonification Object Deleted");		
			return (jboolean)true;
		
		
	}


	static jboolean VisualSonificationGetImage(JNIEnv *env, jobject obj, jlong visualSonificationPtrAsLong, jint cameraId,jint imageId,jint width, jint height,  jobject bitmap)//jintArray rgba)
	{

		
		Mat visualSonificationImageResized(height,width,CV_8UC3);
		unsigned char * imageData;
		int imageWidth, imageHeight, imageStep;

		

		AndroidBitmapInfo  info;		
		void*              pixels;		
		
		VisualSonification * visualSonificationInstance = (VisualSonification *)visualSonificationPtrAsLong;
//		LOGE("SonificationDemo Grabbing Image");
		imageData = visualSonificationInstance->getSonificationImageData( cameraId,imageId, imageWidth, imageHeight, imageStep);

		if(imageData == NULL)
		{
			return false;
		}
		Mat visualSonificationImage(imageHeight,imageWidth,CV_8UC3,imageData);


//		LOGE("SonificationDemo Image Grab Resize");
		resize(visualSonificationImage,visualSonificationImageResized,Size(width,height));

		if ( AndroidBitmap_getInfo(env, bitmap, &info) < 0 )
			return false; // can't get info
 
		if (info.format != ANDROID_BITMAP_FORMAT_RGBA_8888)
			return false; // incompatible format

		if ( AndroidBitmap_lockPixels(env, bitmap, &pixels) < 0 )
			return false; // can't get pixels
		int * pixelData = (int *) pixels;

			LOGE("SonificationDemo Image Grab Success");
			for(int y = 0; y < (int)visualSonificationImageResized.rows && y< (int)info.height; y++)
			{

				unsigned char * imagePtr = visualSonificationImageResized.data + y * visualSonificationImageResized.step[0];
				
				for(int x = 0; x < (int)visualSonificationImageResized.cols && x< (int)info.width; x++)
				{
					int index = y * info.stride/sizeof(int) + x;
					 //R,G.B - Red, Green, Blue
					 //to restore the values after RGB modification, use 
					 //next statement
					 pixelData[index] = 0xff000000 | (imagePtr[x*3 + 0] << 16) | (imagePtr[x*3 + 1] << 8) | imagePtr[x*3 + 2];
					 


					
				}
						
				
				
			}	
			

  




		AndroidBitmap_unlockPixels(env, bitmap);






		return true;
	}


	static JNINativeMethod method_table[] = {
		{"InitNative", 	"()Z",  (void *)Initialize},
		{"CreateVisualSonificationObjNative", 	"()J",  (void *)CreateVisualSonificationObj},
		{"FinalizeNative", "()V", (void *)Finalize},	
		{"StopPlayingNative", "()V", (void *)StopPlaying},	
		{"PausePlayingNative", "()V", (void *)PausePlaying},	
		{"StartPlayingNative", "()V", (void *)StartPlaying},	
		{"RunNative", "()Ljava/lang/String;", (void *)Run},
		{"VisualSonificationExecuteStateNative", "(J)J", (void *)VisualSonificationExecuteState},
		{"VisualSonificationInitNative", "(J)Z", (void *)VisualSonificationInit},
		{"VisualSonificationCleanupNative", "(J)Z", (void *)VisualSonificationCleanUp},										
		{"VisualSonificationGetImageNative", "(JIIIILandroid/graphics/Bitmap;)Z", (void *)VisualSonificationGetImage},//{"GetImageNative", "(II[I)V", (void *)GetLowerImage},
		{"GetImageNative", "(IILandroid/graphics/Bitmap;)Z", (void *)GetLowerImage},//{"GetImageNative", "(II[I)V", (void *)GetLowerImage},
	}; 
	 
//	int runtime_link(JNIEnv *env)
//	{
//		return jniRegisterNativeMethods(env, "edu/umich/acal/sonification/SonificationDemo", method_table, NELEM(method_table));
//	}

	int runtime_link_view(JNIEnv *env)
	{
		return jniRegisterNativeMethods(env, "edu/umich/acal/sonification/BasicSonificationView", method_table, NELEM(method_table));
	}




};
  
using namespace android;

extern "C" jint JNI_OnLoad(JavaVM *vm, void *reserved)
{
	JNIEnv *env = NULL;
	jint result = -1;
	int ret;
	if (vm->GetEnv((void **)&env, JNI_VERSION_1_6) != JNI_OK)
	{
		LOGE("SonificationDemo JNI GetEnv Failure!");
	}
	//ret = runtime_link(env);
	//if (ret == -1)
	//{
	//	LOGE("Sonification Link Failure!");
	//}

	ret = runtime_link_view(env);
	if (ret == -1)
	{
		LOGE("Sonification Link Failure!");
	}

	return JNI_VERSION_1_6;
}
