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

#ifndef VISION_UTILITIES_H
#define VISION_UTILITIES_H
#include <iostream>
#include <time.h>
#include <stdio.h>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/ml/ml.hpp>
#include <opencv2/objdetect/objdetect.hpp>

using namespace std;
using namespace cv;
#ifdef ANDROID

#define TIMING_FILE_NAME "/sdcard/Timing.csv"


#else
#define TIMING_FILE_NAME "Timing.csv"

#endif

typedef Mat JasonType;
typedef Mat MatrixData;
typedef Size MatrixSize;
typedef Point MatrixPointInt;
typedef Point2f MatrixPointFloat;
typedef Point3i MatrixPoint3dInt;
typedef Point3f MatrixPoint3dFloat;
typedef float HistogramEntryFloat;
typedef HistogramEntryFloat * HistogramFloatPtr;
typedef float MatrixElemFloat;
typedef struct {
	
int queryIdx;
int trainIdx;
int imgIdx;
float distance;
bool valid;	
	
} FeatureMatch;


typedef struct
{
	clock_t start;
	clock_t end;
	

} TimingElement;

//#define NUMBER_OF_TIMING_EVENTS 40
typedef enum {
	TIMING_EVENT_INIT,
	TIMING_EVENT_CAPTURE_IMAGE,
	TIMING_EVENT_CALC_DISPARITY,
	TIMING_EVENT_CALC_DEPTH,
	TIMING_EVENT_SEGMENT,
	TIMING_EVENT_FEATURE_EXTRACTION,
	TIMING_EVENT_HISTOGRAM_GENERATION,
	TIMING_EVENT_SIGNATURE_GENERATION,
	TIMING_EVENT_AUDIO_GENERATION,
	TIMING_EVENT_GENERAL_0,
	TIMING_EVENT_GENERAL_1,
	TIMING_EVENT_GENERAL_2,
	TIMING_EVENT_GENERAL_3,
	TIMING_EVENT_ALL,
	NUMBER_OF_TIMING_EVENTS
}TimingEvents;

#define DISPARITY_FIXED_POINT_FRACTIONAL_BITS 4
#define LOCATION_3D_POINT_FRACTIONAL_BITS 4

#define getStartClockTime(timingId) { timingArray[timingId].start = clock();  }
#define getEndClockTime(timingId) { timingArray[timingId].end = clock();  }
extern TimingElement timingArray[NUMBER_OF_TIMING_EVENTS];


void vuErodeFloat(MatrixData * dataIn,MatrixSize windowSize, MatrixData *buffer, MatrixData * dataOut);
void vuErodeInt(MatrixData * dataIn,MatrixSize windowSize, MatrixData *buffer, MatrixData * dataOut);
void vuErodeShortInt(MatrixData * dataIn,MatrixSize windowSize, MatrixData *buffer, MatrixData * dataOut);
void vuDilateFloat(MatrixData * dataIn,MatrixSize windowSize, MatrixData *buffer, MatrixData * dataOut);
void vuDilateInt(MatrixData * dataIn,MatrixSize windowSize, MatrixData *buffer, MatrixData * dataOut);
void vuDilateShortInt(MatrixData * dataIn,MatrixSize windowSize, MatrixData *buffer, MatrixData * dataOut);

void vuConvertUnsignedCharToFloatMatrix(MatrixData * dataIn, MatrixData * dataOut);
void vuConvertUnsignedCharToFloatMatrixAndScale(MatrixData * dataIn, MatrixData * dataOut, float scaleFactor);

void vuExtractChannelFloat(MatrixData * dataIn,int dataInNumberOfChannels ,int channelToExtract, MatrixData * dataOut);
void vuExtractChannelShortInt(MatrixData * dataIn,int dataInNumberOfChannels ,int channelToExtract, MatrixData * dataOut);


void vuAbsFloatSingleChannel(MatrixData * dataIn, MatrixData * dataOut);
void vuAbsShortIntSingleChannel(MatrixData * dataIn, MatrixData * dataOut);


void vuProjectPointsTo3D(MatrixData * disparityDataIn, MatrixData * QIn, MatrixData * depthDataOut, int xOffset=0, int yOffset=0);
void vuProjectPointsTo3DFixedPoint(MatrixData * disparityDataIn, MatrixData * QIn, MatrixData * depthDataOut, int xOffset=0, int yOffset=0);

void vuPrintHistogram(HistogramFloatPtr histogram, int histogramSize);
void vuPrintFeatureVectorToConsole(MatrixData * featuresData, int index);
void vuPrintFeatureMatchToConsole(FeatureMatch * matchToPrint, MatrixData * queryData, MatrixData * trainData);


void vuTopNMatchesFloat(MatrixData * queryDataIn, MatrixData * trainDataIn,int numberOfMatches, FeatureMatch* matchResults);
void converFeatureMatchToDMatch(FeatureMatch * featureMatches, int queryCount, int matchCount, vector<vector<DMatch > > & dmatches);

void vuPrintTimingInformationToFile();
void vuPrintTimingInformationToConsole();
void vuInitTimingInformation();

void vuColorDisparityMap(MatrixData * dataIn, MatrixData * colorDisparity, float numberOfDisparities, float subpixelDisparities);

void testVuFunctions();






#endif
