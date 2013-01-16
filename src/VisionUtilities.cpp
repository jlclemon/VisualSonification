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

#include "VisionUtilities.h"





/*
void vuErode(MatrixData * dataIn, MatrixData * dataOut, MatrixSize windowSize)
{
	int y=0,x=0,dx=0,dy=0;
	int baseRowLocationInBytes = 0;		
	int rowStepInBytes = 0;
	int rowIncrement =0;
	int dxStart = -windowSize.width/2;
	int dyStart = -windowSize.width/2;
	int dxStop = windowSize.width/2;
	int dyStop = windowSize.width/2;

	unsigned char * dataInRowBytePtr;
	unsigned char * dataOutRowBytePtr;

	float * dataInRowPtr;
	float * dataOutRowPtr;
	float currentMin;
	

	MatrixPointInt currentLocation;

	rowStepInBytes = dataIn->step[0];
	for(y = 0; y< dataIn->rows; y++)
	{
		for(x = 0; x< dataIn->cols; x++)
		{

			dataInRowBytePtr = dataIn->data + baseRowLocationInBytes;
			dataOutRowBytePtr = dataOut->data + baseRowLocationInBytes;

			rowIncrement= -rowStepInBytes;
			currentMin = FLT_MAX;
			for(dy = dyStart; dy <=dyStop; dy++)
			{
				currentLocation.y = dy+y;
				if((currentLocation.y>=0 ) && (currentLocation.y< dataIn->rows))
				{
				
					for(dx = -dxStart; dx <=dxStop; dx++)
					{
						currentLocation.x = dx+x;

						if((currentLocation.x>=0 ) && (currentLocation.x< dataIn->cols))
						{
							dataInRowPtr = (float *)(dataInRowBytePtr+ rowIncrement);
							currentMin = (currentMin<dataInRowPtr[x]?currentMin:dataInRowPtr[x]);
						}
					}
				}
				rowIncrement+=rowStepInBytes;
			}
		
			dataOutRowPtr = (float*)dataOutRowBytePtr;
			dataOutRowPtr[x] = currentMin;
		}
		baseRowLocationInBytes += rowStepInBytes;
	}
}
*/

TimingElement timingArray[NUMBER_OF_TIMING_EVENTS];
const string timingElementNames[] = {
	"Init",//TIMING_EVENT_INIT,
	"Capture Images",//TIMING_EVENT_CAPTURE_IMAGE,
	"Calc Disparity",//TIMING_EVENT_CALC_DISPARITY,
	"Calc Depth",//TIMING_EVENT_CALC_DEPTH,
	"Segment Image",//TIMING_EVENT_SEGMENT,
	"Feature Extraction",//TIMING_EVENT_FEATURE_EXTRACTION,
	"Histogram Generation",//TIMING_EVENT_HISTOGRAM_GENERATION,
	"Signtature Generation",//TIMING_EVENT_SIGNATURE_GENERATION,
	"Audio Generation",//TIMING_EVENT_AUDIO_GENERATION,
	"General Timing 0",//TIMING_EVENT_GENERAL_0,
	"General Timing 1",//TIMING_EVENT_GENERAL_1,
	"General Timing 2",//TIMING_EVENT_GENERAL_2,
	"General Timing 3",//TIMING_EVENT_GENERAL_3,
	"All Processing",	//TIMING_EVENT_ALL,
	//NUMBER_OF_TIMING_EVENTS
};

void vuPrintTimingInformationToConsole()
{
	int i =0;
	clock_t totalExecutionTime = 0; 
	clock_t currentExecutionTime = 0; 
	printf("Timing Information for run through:\n");
	printf("Clocks Per second: %d \n", CLOCKS_PER_SEC);

	for(i = 0; i < NUMBER_OF_TIMING_EVENTS; i++)
	{
		currentExecutionTime = timingArray[i].end- timingArray[i].start;
		printf("%s : %d Clocks\n", timingElementNames[i].c_str(),currentExecutionTime);
		totalExecutionTime += currentExecutionTime;
	}
	printf("\nTotal Work Time: %d\n", totalExecutionTime);
}

void vuPrintTimingInformationToFile()
{
	int i =0;
	clock_t totalExecutionTime = 0; 
	clock_t currentExecutionTime = 0; 
	FILE * outputFile;

	outputFile = fopen(TIMING_FILE_NAME,"a");
	if(outputFile ==NULL)
	{
		outputFile = fopen(TIMING_FILE_NAME,"w");
		
	}
	if(outputFile ==NULL)
	{
		return;
	}
	fprintf(outputFile,"Timing Information for run through:\n");
	fprintf(outputFile,"Clocks Per second:, %d \n", CLOCKS_PER_SEC);

	for(i = 0; i < NUMBER_OF_TIMING_EVENTS; i++)
	{
		currentExecutionTime = timingArray[i].end- timingArray[i].start;
		fprintf(outputFile,"%s,%d, Clocks\n", timingElementNames[i].c_str(),currentExecutionTime);
		totalExecutionTime += currentExecutionTime;
	}
	fprintf(outputFile,"\nTotal Work Time,%d\n", totalExecutionTime);
	fclose(outputFile);


}


void vuInitTimingInformation()
{
	int i = 0;
	for(i = 0; i < NUMBER_OF_TIMING_EVENTS; i++)
	{
		timingArray[i].end = 0;
		timingArray[i].start = 0;
	}



}

void vuErodeFloat(MatrixData * dataIn,MatrixSize windowSize, MatrixData *buffer, MatrixData * dataOut)
{
	int y=0,x=0,dx=0,dy=0;
	int baseRowLocationInBytes = 0;		
	int rowStepInBytes = 0;
	int rowIncrement =0;
	int dxStart = -windowSize.width/2;
	int dyStart = -windowSize.width/2;
	int dxStop = windowSize.width/2;
	int dyStop = windowSize.width/2;

	unsigned char * dataInRowBytePtr;
	unsigned char * dataOutRowBytePtr;
	unsigned char * bufferRowBytePtr;
	float * dataInRowPtr;
	float * dataOutRowPtr;
	float * bufferRowPtr;
	float currentMin;
	int initialRowIncrement;
		
	
	MatrixPointInt currentLocation;

	rowStepInBytes = dataIn->step[0];
	initialRowIncrement = 	rowStepInBytes *dyStart;	

	for(y = 0; y< dataIn->rows; y++)
	{
		for(x = 0; x< dataIn->cols; x++)
		{

			dataInRowBytePtr = dataIn->data + baseRowLocationInBytes;
			bufferRowBytePtr = buffer->data + baseRowLocationInBytes;

			rowIncrement= initialRowIncrement;
			currentMin = FLT_MAX;
			currentLocation.x = x;
			for(dy = dyStart; dy <=dyStop; dy++)
			{
				currentLocation.y = dy+y;
				if((currentLocation.y>=0 ) && (currentLocation.y< dataIn->rows))
				{

					dataInRowPtr = (float *)(dataInRowBytePtr+ rowIncrement);
					currentMin = (currentMin<dataInRowPtr[x]?currentMin:dataInRowPtr[x]);
				}
				rowIncrement+=rowStepInBytes;
			}
		
			bufferRowPtr = (float*)bufferRowBytePtr;
			bufferRowPtr[x] = currentMin;
		}
		baseRowLocationInBytes += rowStepInBytes;
	}

//	imshow("MidBuffer", *buffer);
//	waitKey(0);
	baseRowLocationInBytes = 0;		
	for(y = 0; y< dataIn->rows; y++)
	{
		currentLocation.y = y;
		for(x = 0; x< dataIn->cols; x++)
		{

			bufferRowBytePtr = buffer->data + baseRowLocationInBytes;
			dataOutRowBytePtr = dataOut->data + baseRowLocationInBytes;

			currentMin = FLT_MAX;

			for(dx = dxStart; dx <=dxStop; dx++)
			{
				currentLocation.x = dx+x;

				if((currentLocation.x>=0 ) && (currentLocation.x< dataIn->cols))
				{
					bufferRowPtr = (float *)(bufferRowBytePtr);
					currentMin = (currentMin<bufferRowPtr[currentLocation.x]?currentMin:bufferRowPtr[currentLocation.x]);
				}
			}											
		
			dataOutRowPtr = (float*)dataOutRowBytePtr;
			dataOutRowPtr[x] = currentMin;
		}
		baseRowLocationInBytes += rowStepInBytes;
	}
}


void vuErodeInt(MatrixData * dataIn,MatrixSize windowSize, MatrixData *buffer, MatrixData * dataOut)
{
	int y=0,x=0,dx=0,dy=0;
	int baseRowLocationInBytes = 0;		
	int rowStepInBytes = 0;
	int rowIncrement =0;
	int dxStart = -windowSize.width/2;
	int dyStart = -windowSize.width/2;
	int dxStop = windowSize.width/2;
	int dyStop = windowSize.width/2;

	unsigned char * dataInRowBytePtr;
	unsigned char * dataOutRowBytePtr;
	unsigned char * bufferRowBytePtr;
	int * dataInRowPtr;
	int * dataOutRowPtr;
	int * bufferRowPtr;
	int currentMin;
	int initialRowIncrement;	
	
	MatrixPointInt currentLocation;

	rowStepInBytes = dataIn->step[0];
	initialRowIncrement = 	rowStepInBytes *dyStart;	
	for(y = 0; y< dataIn->rows; y++)
	{
		for(x = 0; x< dataIn->cols; x++)
		{

			dataInRowBytePtr = dataIn->data + baseRowLocationInBytes;
			bufferRowBytePtr = buffer->data + baseRowLocationInBytes;

			rowIncrement= initialRowIncrement;
			currentMin = INT_MAX;
			currentLocation.x = x;
			for(dy = dyStart; dy <=dyStop; dy++)
			{
				currentLocation.y = dy+y;
				if((currentLocation.y>=0 ) && (currentLocation.y< dataIn->rows))
				{

					dataInRowPtr = (int *)(dataInRowBytePtr+ rowIncrement);
					currentMin = (currentMin<dataInRowPtr[x]?currentMin:dataInRowPtr[x]);
				}
				rowIncrement+=rowStepInBytes;
			}
		
			bufferRowPtr = (int*)bufferRowBytePtr;
			bufferRowPtr[x] = currentMin;
		}
		baseRowLocationInBytes += rowStepInBytes;
	}

	baseRowLocationInBytes = 0;		
	for(y = 0; y< dataIn->rows; y++)
	{
		currentLocation.y = y;
		for(x = 0; x< dataIn->cols; x++)
		{

			bufferRowBytePtr = buffer->data + baseRowLocationInBytes;
			dataOutRowBytePtr = dataOut->data + baseRowLocationInBytes;

			currentMin = INT_MAX;

			for(dx = dxStart; dx <=dxStop; dx++)
			{
				currentLocation.x = dx+x;

				if((currentLocation.x>=0 ) && (currentLocation.x< dataIn->cols))
				{
					bufferRowPtr = (int *)(bufferRowBytePtr);
					currentMin = (currentMin<bufferRowPtr[currentLocation.x]?currentMin:bufferRowPtr[currentLocation.x]);
				}
			}											
		
			dataOutRowPtr = (int*)dataOutRowBytePtr;
			dataOutRowPtr[x] = currentMin;
		}
		baseRowLocationInBytes += rowStepInBytes;
	}
}

void vuErodeShortInt(MatrixData * dataIn,MatrixSize windowSize, MatrixData *buffer, MatrixData * dataOut)
{
	int y=0,x=0,dx=0,dy=0;
	int baseRowLocationInBytes = 0;		
	int rowStepInBytes = 0;
	int rowIncrement =0;
	int dxStart = -windowSize.width/2;
	int dyStart = -windowSize.width/2;
	int dxStop = windowSize.width/2;
	int dyStop = windowSize.width/2;

	unsigned char * dataInRowBytePtr;
	unsigned char * dataOutRowBytePtr;
	unsigned char * bufferRowBytePtr;
	short int * dataInRowPtr;
	short int * dataOutRowPtr;
	short int * bufferRowPtr;
	short int currentMin;
	int initialRowIncrement;	
	
	MatrixPointInt currentLocation;

	rowStepInBytes = dataIn->step[0];

	initialRowIncrement = 	rowStepInBytes *dyStart;	
	for(y = 0; y< dataIn->rows; y++)
	{
		for(x = 0; x< dataIn->cols; x++)
		{

			dataInRowBytePtr = dataIn->data + baseRowLocationInBytes;
			bufferRowBytePtr = buffer->data + baseRowLocationInBytes;

			rowIncrement= initialRowIncrement;
			currentMin = SHRT_MAX;
			currentLocation.x = x;
			for(dy = dyStart; dy <=dyStop; dy++)
			{
				currentLocation.y = dy+y;
				if((currentLocation.y>=0 ) && (currentLocation.y< dataIn->rows))
				{

					dataInRowPtr = (short int *)(dataInRowBytePtr+ rowIncrement);
					currentMin = (currentMin<dataInRowPtr[x]?currentMin:dataInRowPtr[x]);
				}
				rowIncrement+=rowStepInBytes;
			}
		
			bufferRowPtr = (short int*)bufferRowBytePtr;
			bufferRowPtr[x] = currentMin;
		}
		baseRowLocationInBytes += rowStepInBytes;
	}

	baseRowLocationInBytes = 0;		
	for(y = 0; y< dataIn->rows; y++)
	{
		currentLocation.y = y;
		for(x = 0; x< dataIn->cols; x++)
		{

			bufferRowBytePtr = buffer->data + baseRowLocationInBytes;
			dataOutRowBytePtr = dataOut->data + baseRowLocationInBytes;

			currentMin = SHRT_MAX;

			for(dx = dxStart; dx <=dxStop; dx++)
			{
				currentLocation.x = dx+x;

				if((currentLocation.x>=0 ) && (currentLocation.x< dataIn->cols))
				{
					bufferRowPtr = (short int *)(bufferRowBytePtr);
					currentMin = (currentMin<bufferRowPtr[currentLocation.x]?currentMin:bufferRowPtr[currentLocation.x]);
				}
			}											
		
			dataOutRowPtr = (short int*)dataOutRowBytePtr;
			dataOutRowPtr[x] = currentMin;
		}
		baseRowLocationInBytes += rowStepInBytes;
	}
}


void vuDilateFloat(MatrixData * dataIn,MatrixSize windowSize, MatrixData *buffer, MatrixData * dataOut)
{
	int y=0,x=0,dx=0,dy=0;
	int baseRowLocationInBytes = 0;		
	int rowStepInBytes = 0;
	int rowIncrement =0;
	int dxStart = -windowSize.width/2;
	int dyStart = -windowSize.width/2;
	int dxStop = windowSize.width/2;
	int dyStop = windowSize.width/2;

	unsigned char * dataInRowBytePtr;
	unsigned char * dataOutRowBytePtr;
	unsigned char * bufferRowBytePtr;
	float * dataInRowPtr;
	float * dataOutRowPtr;
	float * bufferRowPtr;
	float currentMax;
	int initialRowIncrement;
	
	MatrixPointInt currentLocation;

	rowStepInBytes = dataIn->step[0];
	initialRowIncrement = 	rowStepInBytes *dyStart;	
	for(y = 0; y< dataIn->rows; y++)
	{
		for(x = 0; x< dataIn->cols; x++)
		{

			dataInRowBytePtr = dataIn->data + baseRowLocationInBytes;
			bufferRowBytePtr = buffer->data + baseRowLocationInBytes;

			rowIncrement= initialRowIncrement;
			currentMax = -FLT_MIN;
			currentLocation.x = x;
			for(dy = dyStart; dy <=dyStop; dy++)
			{
				currentLocation.y = dy+y;
				if((currentLocation.y>=0 ) && (currentLocation.y< dataIn->rows))
				{

					dataInRowPtr = (float *)(dataInRowBytePtr+ rowIncrement);
					currentMax = (currentMax>dataInRowPtr[x]?currentMax:dataInRowPtr[x]);
				}
				rowIncrement+=rowStepInBytes;
			}
		
			bufferRowPtr = (float*)bufferRowBytePtr;
			bufferRowPtr[x] = currentMax;
		}
		baseRowLocationInBytes += rowStepInBytes;
	}

	baseRowLocationInBytes = 0;		
	for(y = 0; y< dataIn->rows; y++)
	{
		currentLocation.y = y;
		for(x = 0; x< dataIn->cols; x++)
		{

			bufferRowBytePtr = buffer->data + baseRowLocationInBytes;
			dataOutRowBytePtr = dataOut->data + baseRowLocationInBytes;

			currentMax = -FLT_MIN;

			for(dx = dxStart; dx <=dxStop; dx++)
			{
				currentLocation.x = dx+x;

				if((currentLocation.x>=0 ) && (currentLocation.x< dataIn->cols))
				{
					bufferRowPtr = (float *)(bufferRowBytePtr);
					currentMax = (currentMax>bufferRowPtr[currentLocation.x]?currentMax:bufferRowPtr[currentLocation.x]);
				}
			}											
		
			dataOutRowPtr = (float*)dataOutRowBytePtr;
			dataOutRowPtr[x] = currentMax;
		}
		baseRowLocationInBytes += rowStepInBytes;
	}



}

void vuDilateInt(MatrixData * dataIn,MatrixSize windowSize, MatrixData *buffer, MatrixData * dataOut)
{
	int y=0,x=0,dx=0,dy=0;
	int baseRowLocationInBytes = 0;		
	int rowStepInBytes = 0;
	int rowIncrement =0;
	int dxStart = -windowSize.width/2;
	int dyStart = -windowSize.width/2;
	int dxStop = windowSize.width/2;
	int dyStop = windowSize.width/2;

	unsigned char * dataInRowBytePtr;
	unsigned char * dataOutRowBytePtr;
	unsigned char * bufferRowBytePtr;
	int * dataInRowPtr;
	int * dataOutRowPtr;
	int * bufferRowPtr;
	int currentMax;
	int initialRowIncrement;	
	
	MatrixPointInt currentLocation;

	rowStepInBytes = dataIn->step[0];
	initialRowIncrement = 	rowStepInBytes *dyStart;	
	for(y = 0; y< dataIn->rows; y++)
	{
		for(x = 0; x< dataIn->cols; x++)
		{

			dataInRowBytePtr = dataIn->data + baseRowLocationInBytes;
			bufferRowBytePtr = buffer->data + baseRowLocationInBytes;

			rowIncrement= initialRowIncrement;
			currentMax = INT_MIN;
			currentLocation.x = x;
			for(dy = dyStart; dy <=dyStop; dy++)
			{
				currentLocation.y = dy+y;
				if((currentLocation.y>=0 ) && (currentLocation.y< dataIn->rows))
				{

					dataInRowPtr = (int *)(dataInRowBytePtr+ rowIncrement);
					currentMax = (currentMax>dataInRowPtr[x]?currentMax:dataInRowPtr[x]);
				}
				rowIncrement+=rowStepInBytes;
			}
		
			bufferRowPtr = (int*)bufferRowBytePtr;
			bufferRowPtr[x] = currentMax;
		}
		baseRowLocationInBytes += rowStepInBytes;
	}

	baseRowLocationInBytes = 0;		
	for(y = 0; y< dataIn->rows; y++)
	{
		currentLocation.y = y;
		for(x = 0; x< dataIn->cols; x++)
		{

			bufferRowBytePtr = buffer->data + baseRowLocationInBytes;
			dataOutRowBytePtr = dataOut->data + baseRowLocationInBytes;

			currentMax = INT_MIN;

			for(dx = dxStart; dx <=dxStop; dx++)
			{
				currentLocation.x = dx+x;

				if((currentLocation.x>=0 ) && (currentLocation.x< dataIn->cols))
				{
					bufferRowPtr = (int *)(bufferRowBytePtr);
					currentMax = (currentMax>bufferRowPtr[currentLocation.x]?currentMax:bufferRowPtr[currentLocation.x]);
				}
			}											
		
			dataOutRowPtr = (int*)dataOutRowBytePtr;
			dataOutRowPtr[x] = currentMax;
		}
		baseRowLocationInBytes += rowStepInBytes;
	}



}

void vuDilateShortInt(MatrixData * dataIn,MatrixSize windowSize, MatrixData *buffer, MatrixData * dataOut)
{
	int y=0,x=0,dx=0,dy=0;
	int baseRowLocationInBytes = 0;		
	int rowStepInBytes = 0;
	int rowIncrement =0;
	int dxStart = -windowSize.width/2;
	int dyStart = -windowSize.width/2;
	int dxStop = windowSize.width/2;
	int dyStop = windowSize.width/2;

	unsigned char * dataInRowBytePtr;
	unsigned char * dataOutRowBytePtr;
	unsigned char * bufferRowBytePtr;
	short int * dataInRowPtr;
	short int * dataOutRowPtr;
	short int * bufferRowPtr;
	short int currentMax;
	int initialRowIncrement;
	
	MatrixPointInt currentLocation;

	rowStepInBytes = dataIn->step[0];

	initialRowIncrement = 	rowStepInBytes *dyStart;	
	for(y = 0; y< dataIn->rows; y++)
	{
		for(x = 0; x< dataIn->cols; x++)
		{

			dataInRowBytePtr = dataIn->data + baseRowLocationInBytes;
			bufferRowBytePtr = buffer->data + baseRowLocationInBytes;

			rowIncrement= initialRowIncrement;
			currentMax = SHRT_MIN;
			currentLocation.x = x;
			for(dy = dyStart; dy <=dyStop; dy++)
			{
				currentLocation.y = dy+y;
				if((currentLocation.y>=0 ) && (currentLocation.y< dataIn->rows))
				{

					dataInRowPtr = (short int *)(dataInRowBytePtr+ rowIncrement);
					currentMax = (currentMax>dataInRowPtr[x]?currentMax:dataInRowPtr[x]);
				}
				rowIncrement+=rowStepInBytes;
			}
		
			bufferRowPtr = (short int*)bufferRowBytePtr;
			bufferRowPtr[x] = currentMax;
		}
		baseRowLocationInBytes += rowStepInBytes;
	}

	baseRowLocationInBytes = 0;		
	for(y = 0; y< dataIn->rows; y++)
	{
		currentLocation.y = y;
		for(x = 0; x< dataIn->cols; x++)
		{

			bufferRowBytePtr = buffer->data + baseRowLocationInBytes;
			dataOutRowBytePtr = dataOut->data + baseRowLocationInBytes;

			currentMax = SHRT_MIN;

			for(dx = dxStart; dx <=dxStop; dx++)
			{
				currentLocation.x = dx+x;

				if((currentLocation.x>=0 ) && (currentLocation.x< dataIn->cols))
				{
					bufferRowPtr = (short int *)(bufferRowBytePtr);
					currentMax = (currentMax>bufferRowPtr[currentLocation.x]?currentMax:bufferRowPtr[currentLocation.x]);
				}
			}											
		
			dataOutRowPtr = (short int*)dataOutRowBytePtr;
			dataOutRowPtr[x] = currentMax;
		}
		baseRowLocationInBytes += rowStepInBytes;
	}



}


void printFloatMatrixToConsole(MatrixData * dataIn)
{
	int x,y;
	int rowStepInBytes = dataIn->step[0];
	unsigned char * dataInRowBytePtr;
	float * dataInRowPtr;

	for(y = 0; y< dataIn->rows; y++)
	{

		dataInRowBytePtr = dataIn->data+y*rowStepInBytes;
		dataInRowPtr = (float *)dataInRowBytePtr;
		printf("[");
		printf("%g ", dataInRowPtr[0]);
		for(x = 1; x< dataIn->cols; x++)
		{
			
			printf(", %g ", dataInRowPtr[x]);
		}
		printf("] \n");

	}


}



void vuExtractChannelFloat(MatrixData * dataIn,int dataInNumberOfChannels ,int channelToExtract, MatrixData * dataOut)
{
	int x,y;

	float * dataInRowPtr;
	float * dataOutRowPtr;
	int dataInBaseRowLocationInBytes = 0;		
	int dataOutBaseRowLocationInBytes = 0;
	int dataInRowStepInBytes = 0;
	int dataOutRowStepInBytes = 0;

	int channelLocation = channelToExtract;
	dataInRowStepInBytes = dataIn->step[0];
	dataOutRowStepInBytes = dataOut->step[0];
	dataInBaseRowLocationInBytes = 0;		
	dataOutBaseRowLocationInBytes = 0;		
	for(y = 0; y< dataIn->rows; y++)
	{

		dataInRowPtr = (float *)(dataIn->data + dataInBaseRowLocationInBytes);
		dataOutRowPtr = (float *)(dataOut->data + dataOutBaseRowLocationInBytes);
		channelLocation = channelToExtract;
		for(x = 0; x< dataIn->cols; x++)
		{
			dataOutRowPtr[x] = dataInRowPtr[channelLocation];

			channelLocation += dataInNumberOfChannels;
		}
		dataInBaseRowLocationInBytes += dataInRowStepInBytes;
		dataOutBaseRowLocationInBytes += dataOutRowStepInBytes;
	}
}


void vuExtractChannelShortInt(MatrixData * dataIn,int dataInNumberOfChannels ,int channelToExtract, MatrixData * dataOut)
{
	int x,y;

	short int * dataInRowPtr;
	short int * dataOutRowPtr;
	int dataInBaseRowLocationInBytes = 0;
	int dataOutBaseRowLocationInBytes = 0;
	int dataInRowStepInBytes = 0;
	int dataOutRowStepInBytes = 0;

	int channelLocation = channelToExtract;
	dataInRowStepInBytes = dataIn->step[0];
	dataOutRowStepInBytes = dataOut->step[0];
	dataInBaseRowLocationInBytes = 0;
	dataOutBaseRowLocationInBytes = 0;
	for(y = 0; y< dataIn->rows; y++)
	{

		dataInRowPtr = (short int *)(dataIn->data + dataInBaseRowLocationInBytes);
		dataOutRowPtr = (short int *)(dataOut->data + dataOutBaseRowLocationInBytes);
		channelLocation = channelToExtract;
		for(x = 0; x< dataIn->cols; x++)
		{
			dataOutRowPtr[x] = dataInRowPtr[channelLocation];

			channelLocation += dataInNumberOfChannels;
		}
		dataInBaseRowLocationInBytes += dataInRowStepInBytes;
		dataOutBaseRowLocationInBytes += dataOutRowStepInBytes;
	}
}


void vuAbsFloatSingleChannel(MatrixData * dataIn, MatrixData * dataOut)
{
	int x,y;

	float * dataInRowPtr;
	float * dataOutRowPtr;
	int dataInBaseRowLocationInBytes = 0;		
	int dataOutBaseRowLocationInBytes = 0;
	int dataInRowStepInBytes = 0;
	int dataOutRowStepInBytes = 0;


	dataInRowStepInBytes = dataIn->step[0];
	dataOutRowStepInBytes = dataOut->step[0];
	dataInBaseRowLocationInBytes = 0;		
	dataOutBaseRowLocationInBytes = 0;		
	for(y = 0; y< dataIn->rows; y++)
	{

		dataInRowPtr = (float *)(dataIn->data + dataInBaseRowLocationInBytes);
		dataOutRowPtr = (float *)(dataOut->data + dataOutBaseRowLocationInBytes);

		for(x = 0; x< dataIn->cols; x++)
		{
			dataOutRowPtr[x] = (dataInRowPtr[x] >0?dataInRowPtr[x]:-dataInRowPtr[x]);


		}
		dataInBaseRowLocationInBytes += dataInRowStepInBytes;
		dataOutBaseRowLocationInBytes += dataOutRowStepInBytes;
	}
}

void vuAbsShortIntSingleChannel(MatrixData * dataIn, MatrixData * dataOut)
{
	int x,y;

	short int * dataInRowPtr;
	short int * dataOutRowPtr;
	int dataInBaseRowLocationInBytes = 0;
	int dataOutBaseRowLocationInBytes = 0;
	int dataInRowStepInBytes = 0;
	int dataOutRowStepInBytes = 0;


	dataInRowStepInBytes = dataIn->step[0];
	dataOutRowStepInBytes = dataOut->step[0];
	dataInBaseRowLocationInBytes = 0;
	dataOutBaseRowLocationInBytes = 0;
	for(y = 0; y< dataIn->rows; y++)
	{

		dataInRowPtr = (short int *)(dataIn->data + dataInBaseRowLocationInBytes);
		dataOutRowPtr = (short int *)(dataOut->data + dataOutBaseRowLocationInBytes);

		for(x = 0; x< dataIn->cols; x++)
		{
			dataOutRowPtr[x] = (dataInRowPtr[x] >0?dataInRowPtr[x]:-dataInRowPtr[x]);


		}
		dataInBaseRowLocationInBytes += dataInRowStepInBytes;
		dataOutBaseRowLocationInBytes += dataOutRowStepInBytes;
	}
}



void vuConvertUnsignedCharToFloatMatrix(MatrixData * dataIn, MatrixData * dataOut)
{
	int x,y;

	unsigned char * dataInRowPtr;
	float * dataOutRowPtr;
	int dataInBaseRowLocationInBytes = 0;		
	int dataOutBaseRowLocationInBytes = 0;
	int dataInRowStepInBytes = 0;
	int dataOutRowStepInBytes = 0;


	dataInRowStepInBytes = dataIn->step[0];
	dataOutRowStepInBytes = dataOut->step[0];
	dataInBaseRowLocationInBytes = 0;		
	dataOutBaseRowLocationInBytes = 0;		
	for(y = 0; y< dataIn->rows; y++)
	{

		dataInRowPtr = (unsigned char *)(dataIn->data + dataInBaseRowLocationInBytes);
		dataOutRowPtr = (float *)(dataOut->data + dataOutBaseRowLocationInBytes);

		for(x = 0; x< dataIn->cols; x++)
		{
			dataOutRowPtr[x] = (float)dataInRowPtr[x] ;


		}
		dataInBaseRowLocationInBytes += dataInRowStepInBytes;
		dataOutBaseRowLocationInBytes += dataOutRowStepInBytes;
	}

}
void vuConvertUnsignedCharToFloatMatrixAndScale(MatrixData * dataIn, MatrixData * dataOut, float scaleFactor)
{
	int x,y;

	unsigned char * dataInRowPtr;
	float * dataOutRowPtr;
	int dataInBaseRowLocationInBytes = 0;		
	int dataOutBaseRowLocationInBytes = 0;
	int dataInRowStepInBytes = 0;
	int dataOutRowStepInBytes = 0;


	dataInRowStepInBytes = dataIn->step[0];
	dataOutRowStepInBytes = dataOut->step[0];
	dataInBaseRowLocationInBytes = 0;		
	dataOutBaseRowLocationInBytes = 0;		
	for(y = 0; y< dataIn->rows; y++)
	{

		dataInRowPtr = (unsigned char *)(dataIn->data + dataInBaseRowLocationInBytes);
		dataOutRowPtr = (float *)(dataOut->data + dataOutBaseRowLocationInBytes);

		for(x = 0; x< dataIn->cols; x++)
		{
			dataOutRowPtr[x] = (float)dataInRowPtr[x]* scaleFactor;


		}
		dataInBaseRowLocationInBytes += dataInRowStepInBytes;
		dataOutBaseRowLocationInBytes += dataOutRowStepInBytes;
	}

}

//Disparity is a float, Q is a double, results are floats
void vuProjectPointsTo3D(MatrixData * disparityDataIn, MatrixData * QIn, MatrixData * depthDataOut)
{
	uchar * QRowPtrBase = QIn->data;
	int QRowPtrStep = QIn->step[0];
	double * QRow0Ptr = (double *)QRowPtrBase;
	double * QRow1Ptr = (double *)(QRowPtrBase+QIn->step[0]);
	double * QRow2Ptr = (double *)(QRowPtrBase+QIn->step[0]+QIn->step[0]);
	double * QRow3Ptr = (double *)(QRowPtrBase+QIn->step[0]+QIn->step[0]+QIn->step[0]);

	float * disparityDataInRowPtr;
	float * depthDataOutRowPtr;
	int disparityDataInBaseRowLocationInBytes = 0;		
	int depthDataOutBaseRowLocationInBytes = 0;
	int disparityDataInRowStepInBytes = 0;
	int depthDataOutRowStepInBytes = 0;

	int x, y;
	disparityDataInRowStepInBytes = disparityDataIn->step[0];
	depthDataOutRowStepInBytes = depthDataOut->step[0];
	disparityDataInBaseRowLocationInBytes = 0;		
	depthDataOutBaseRowLocationInBytes = 0;		

	int i;
	//have three channels, x,y,z
	int depthDataOutChannelStep = 3;
	int currentDepthDataOutChannelStep = 0;

	double tempX,tempY, tempZ,tempW;

	float currentData[4];
	currentData[3] = 1.0f;



	for(y = 0; y< disparityDataIn->rows; y++)
	{

		disparityDataInRowPtr = (float *)(disparityDataIn->data + disparityDataInBaseRowLocationInBytes);
		depthDataOutRowPtr = (float *)(depthDataOut->data + depthDataOutBaseRowLocationInBytes);
		currentDepthDataOutChannelStep = 0;
		for(x = 0; x< disparityDataIn->cols; x++)
		{
			
			currentData[0] = (float)x; 
			currentData[1]= (float)y;
			currentData[2] = disparityDataInRowPtr[x];
			tempX = 0;
			tempY = 0;
			tempZ = 0;
			tempW = 0;
			for(i=0; i< QIn->cols; i++)
			{
				tempX += (currentData[i] * QRow0Ptr[i]); 
				tempY += (currentData[i] * QRow1Ptr[i]); 				
				tempZ += (currentData[i] * QRow2Ptr[i]); 				
				tempW += (currentData[i] * QRow3Ptr[i]); 				
			}
			
			depthDataOutRowPtr[currentDepthDataOutChannelStep] = (float)(tempX/tempW);
			depthDataOutRowPtr[currentDepthDataOutChannelStep+1] = (float)(tempY/tempW);
			if(currentData[2] >=0)
			{
				depthDataOutRowPtr[currentDepthDataOutChannelStep+2] = (float)(tempZ/tempW);
			}
			else
			{
				depthDataOutRowPtr[currentDepthDataOutChannelStep+2] = 10000;
			}


			currentDepthDataOutChannelStep += depthDataOutChannelStep;

		}
		disparityDataInBaseRowLocationInBytes += disparityDataInRowStepInBytes;
		depthDataOutBaseRowLocationInBytes += depthDataOutRowStepInBytes;
	}



}


//Disparity is a short int, Q is a double, results are short int
void vuProjectPointsTo3DFixedPoint(MatrixData * disparityDataIn, MatrixData * QIn, MatrixData * depthDataOut)
{
	uchar * QRowPtrBase = QIn->data;
	int QRowPtrStep = QIn->step[0];
	double * QRow0Ptr = (double *)QRowPtrBase;
	double * QRow1Ptr = (double *)(QRowPtrBase+QIn->step[0]);
	double * QRow2Ptr = (double *)(QRowPtrBase+QIn->step[0]+QIn->step[0]);
	double * QRow3Ptr = (double *)(QRowPtrBase+QIn->step[0]+QIn->step[0]+QIn->step[0]);

	short int * disparityDataInRowPtr;
	short int * depthDataOutRowPtr;
	int disparityDataInBaseRowLocationInBytes = 0;
	int depthDataOutBaseRowLocationInBytes = 0;
	int disparityDataInRowStepInBytes = 0;
	int depthDataOutRowStepInBytes = 0;

	int x, y;
	disparityDataInRowStepInBytes = disparityDataIn->step[0];
	depthDataOutRowStepInBytes = depthDataOut->step[0];
	disparityDataInBaseRowLocationInBytes = 0;
	depthDataOutBaseRowLocationInBytes = 0;

	int i;
	//have three channels, x,y,z
	int depthDataOutChannelStep = 3;
	int currentDepthDataOutChannelStep = 0;

	double tempX,tempY, tempZ,tempW;
	//int tempX,tempY, tempZ,tempW;
	int currentData[4];
	currentData[3] = 1 <<DISPARITY_FIXED_POINT_FRACTIONAL_BITS;



	for(y = 0; y< disparityDataIn->rows; y++)
	{

		disparityDataInRowPtr = (short int *)(disparityDataIn->data + disparityDataInBaseRowLocationInBytes);
		depthDataOutRowPtr = (short int  *)(depthDataOut->data + depthDataOutBaseRowLocationInBytes);
		currentDepthDataOutChannelStep = 0;
		for(x = 0; x< disparityDataIn->cols; x++)
		{

			currentData[0] = ((int)x) << DISPARITY_FIXED_POINT_FRACTIONAL_BITS;
			currentData[1]= ((int)y) << DISPARITY_FIXED_POINT_FRACTIONAL_BITS;
			currentData[2] = (int)disparityDataInRowPtr[x];
			tempX = 0;
			tempY = 0;
			tempZ = 0;
			tempW = 0;
			for(i=0; i< QIn->cols; i++)
			{
				tempX += (currentData[i] * QRow0Ptr[i]);
				tempY += (currentData[i] * QRow1Ptr[i]);
				tempZ += (currentData[i] * QRow2Ptr[i]);
				tempW += (currentData[i] * QRow3Ptr[i]);
				//printf("\n%d %d %g %g %g %g\n", tempW, currentData[i],QRow0Ptr[i],QRow1Ptr[i],QRow2Ptr[i],QRow3Ptr[i]);
			}
			if(tempW !=0)
			{

				//depthDataOutRowPtr[currentDepthDataOutChannelStep] = (short int)(((short int)tempX<<LOCATION_3D_POINT_FRACTIONAL_BITS)/tempW);
				//depthDataOutRowPtr[currentDepthDataOutChannelStep+1] = (short int)(((short int)tempY<< LOCATION_3D_POINT_FRACTIONAL_BITS)/tempW);
				depthDataOutRowPtr[currentDepthDataOutChannelStep] = (short int)((tempX*(1<<LOCATION_3D_POINT_FRACTIONAL_BITS))/tempW);
				depthDataOutRowPtr[currentDepthDataOutChannelStep+1] = (short int)((tempY*(1<< LOCATION_3D_POINT_FRACTIONAL_BITS))/tempW);


				if(currentData[2] >=0)
				{
					int currentZ = ((short int)tempZ<<DISPARITY_FIXED_POINT_FRACTIONAL_BITS)/tempW;
					if(abs(currentZ) < 1023<<DISPARITY_FIXED_POINT_FRACTIONAL_BITS)
					{
						depthDataOutRowPtr[currentDepthDataOutChannelStep+2] = (short int)(currentZ);
					}
					else
					{
						depthDataOutRowPtr[currentDepthDataOutChannelStep+2] = 2047<<DISPARITY_FIXED_POINT_FRACTIONAL_BITS;//10000;

					}
				}

				else
				{
					depthDataOutRowPtr[currentDepthDataOutChannelStep+2] = 2047<<DISPARITY_FIXED_POINT_FRACTIONAL_BITS;//10000;
				}
			}
			else
			{
				//depthDataOutRowPtr[currentDepthDataOutChannelStep] = (short int)((short int)tempX<<DISPARITY_FIXED_POINT_FRACTIONAL_BITS);
				//depthDataOutRowPtr[currentDepthDataOutChannelStep+1] = (short int)((short int)tempY<<DISPARITY_FIXED_POINT_FRACTIONAL_BITS);
				depthDataOutRowPtr[currentDepthDataOutChannelStep] = (short int)(tempX* (1<<DISPARITY_FIXED_POINT_FRACTIONAL_BITS));
				depthDataOutRowPtr[currentDepthDataOutChannelStep+1] = (short int)(tempY* (1<<DISPARITY_FIXED_POINT_FRACTIONAL_BITS));

				depthDataOutRowPtr[currentDepthDataOutChannelStep+2] = 2047<<DISPARITY_FIXED_POINT_FRACTIONAL_BITS;//10000;





			}

			currentDepthDataOutChannelStep += depthDataOutChannelStep;

		}
		disparityDataInBaseRowLocationInBytes += disparityDataInRowStepInBytes;
		depthDataOutBaseRowLocationInBytes += depthDataOutRowStepInBytes;
	}



}




//Query Data in - 1 row = 1 data sample, each column is a feature vector entry 
//				Thus RowxCol = KxM where K is the number of feature vectors and M is the size
//Train data in - same as query data
//
// Feature Match is a pointer to a data buffer capable of holding numberOfMatches* number Of Data In rows worth of matches
//
// If match results is null, it will be allocated
void vuTopNMatchesFloat(MatrixData * queryDataIn, MatrixData * trainDataIn, int numberOfMatches, FeatureMatch* matchResults)
{

	int queryDataBaseRowLocationInBytes = 0;		
	int trainDataBaseRowLocationInBytes = 0;
	int queryDataRowStepInBytes = 0;
	int trainDataRowStepInBytes = 0;
	MatrixElemFloat * queryVectorPtr;
	MatrixElemFloat * trainVectorPtr;


	queryDataRowStepInBytes = queryDataIn->step[0];
	trainDataRowStepInBytes = trainDataIn->step[0];
	queryDataBaseRowLocationInBytes = 0;		
	trainDataBaseRowLocationInBytes = 0;		


	FeatureMatch* currentMatchResultsSet = matchResults;
	int currentMatchResultIndex;
	int matchResultsStep = numberOfMatches;
	int queryIdx, trainIdx, featureElemIdx,i;
	int numberOfMatchesLimit =(numberOfMatches <trainDataIn->rows?numberOfMatches:trainDataIn->rows);

	for( i = 0; i< numberOfMatches*queryDataIn->rows; i++)
	{
		currentMatchResultsSet[i].imgIdx = -1;
		currentMatchResultsSet[i].distance = FLT_MAX;
		currentMatchResultsSet[i].queryIdx = -1;
		currentMatchResultsSet[i].trainIdx = -1;
		currentMatchResultsSet[i].valid = false;
	}


	for(queryIdx = 0; queryIdx< queryDataIn->rows; queryIdx++)
	{

		queryVectorPtr = (MatrixElemFloat *)(queryDataIn->data + queryDataBaseRowLocationInBytes);



		for(trainIdx = 0; trainIdx< trainDataIn->rows; trainIdx++)
		{

			trainVectorPtr = (MatrixElemFloat *)(trainDataIn->data + trainDataBaseRowLocationInBytes);
			

			float totalDistance = 0.0;
			for(featureElemIdx=0; featureElemIdx< queryDataIn->cols && featureElemIdx < trainDataIn->cols; featureElemIdx++)
			{
				float currentDistance = trainVectorPtr[featureElemIdx] - queryVectorPtr[featureElemIdx];
				totalDistance+= currentDistance*currentDistance;
			}
			totalDistance = sqrt(totalDistance);

			for(currentMatchResultIndex = 0; currentMatchResultIndex < numberOfMatchesLimit; currentMatchResultIndex++)
			{
				if(totalDistance < currentMatchResultsSet[currentMatchResultIndex].distance)
				{					
					for(i = numberOfMatchesLimit-1;  i-1 >= currentMatchResultIndex; i--)
					{
						currentMatchResultsSet[i] = currentMatchResultsSet[i-1];
					}
					currentMatchResultsSet[currentMatchResultIndex].imgIdx = 0;
					currentMatchResultsSet[currentMatchResultIndex].queryIdx = queryIdx;
					currentMatchResultsSet[currentMatchResultIndex].trainIdx = trainIdx;
					currentMatchResultsSet[currentMatchResultIndex].distance = totalDistance;
					currentMatchResultsSet[currentMatchResultIndex].valid = true;
					break;
				}
			}
			trainDataBaseRowLocationInBytes += trainDataRowStepInBytes;
		}

		queryDataBaseRowLocationInBytes += queryDataRowStepInBytes;
		trainDataBaseRowLocationInBytes = 0;
		currentMatchResultsSet +=numberOfMatches;
	}

}

void converFeatureMatchToDMatch(FeatureMatch * featureMatches, int queryCount, int matchCount, vector<vector<DMatch > > & dMatches)
{

	FeatureMatch * currentFeatureMatchesSet = featureMatches;
	dMatches.resize(queryCount);

	for(int i = 0; i < queryCount; i++)
	{
		dMatches[i].resize(matchCount);

		for(int j = 0; j < matchCount; j++)
		{
			dMatches[i][j].imgIdx = currentFeatureMatchesSet[j].imgIdx;
			dMatches[i][j].distance = currentFeatureMatchesSet[j].distance;
			dMatches[i][j].queryIdx = currentFeatureMatchesSet[j].queryIdx;
			dMatches[i][j].trainIdx = currentFeatureMatchesSet[j].trainIdx;
		}
		currentFeatureMatchesSet+= matchCount;
	}
}

void vuPrintHistogram(HistogramFloatPtr histogram, int histogramSize)
{
	cout << "[" << histogram[0];

	for(int i = 1; i <histogramSize; i++)
	{
		cout << " , " <<histogram[i];
	}

	cout << "]" << endl;
}

void vuPrintFeatureVectorToConsole(MatrixData * featuresData, int index)
{
	MatrixElemFloat * featureVector = (MatrixElemFloat *)(featuresData->data + featuresData->step[0] * index);

	printf("Feature: [ ");
	for(int i = 0; i < featuresData->cols; i++)
	{
		printf("%g, ", featureVector[i]);
	}
	printf("]");

}

void vuPrintFeatureMatchToConsole(FeatureMatch * matchToPrint, MatrixData * queryData, MatrixData * trainData)
{

	printf("\t Dist: %g,  queryId %d ->  train Id %d\n",matchToPrint->distance,matchToPrint->queryIdx,matchToPrint->trainIdx);
	printf("Query: ");
	if(matchToPrint->queryIdx >= 0)
	{
		vuPrintFeatureVectorToConsole(queryData, matchToPrint->queryIdx);
	}
	else
	{
		printf("Invalid");
	}
	printf("\n");
	printf("Train: ");
	if(matchToPrint->trainIdx>=0)
	{
		vuPrintFeatureVectorToConsole(trainData, matchToPrint->trainIdx);
	}
	else
	{
		printf("Invalid");
	}

	printf("\n");
}



void testVuFunctions()
{
	MatrixData testMat(10,10,CV_32FC1);
	MatrixData testMat2(10,10,CV_32FC1);
	MatrixData buffer(10,10,CV_32FC1);

	MatrixSize windowSize;

	windowSize.height = 5;

	windowSize.width = 5;
	testMat.setTo(Scalar(1));

	testMat.at<float>(5,5) = 0;
	namedWindow("Before");
	imshow("Before",testMat);
	waitKey(0);	
	vuErodeFloat(&testMat,windowSize, &buffer, &testMat2);

	namedWindow("After");
	namedWindow("Results");
	namedWindow("Buffer");
	imshow("Before",testMat);
	imshow("After",testMat);
	imshow("Results",testMat2);
	imshow("Buffer",buffer);
	cout <<"TestMat:\n" << endl;
	printFloatMatrixToConsole(&testMat);
	cout << "\n\n" << endl;
	cout <<"TestMat2\n:" << endl;
	printFloatMatrixToConsole(&testMat2);
	cout << "\n\n" << endl;
	cout << "Matrix Results\n" << testMat2 << endl;
	waitKey(0);


	vuDilateFloat(&testMat2,windowSize, &buffer, &testMat);
	imshow("Before",testMat2);
	imshow("After",testMat2);
	imshow("Results",testMat);
	imshow("Buffer",buffer);
	cout <<"TestMat:\n" << endl;
	printFloatMatrixToConsole(&testMat2);
	cout << "\n\n" << endl;
	cout <<"TestMat2\n:" << endl;
	printFloatMatrixToConsole(&testMat);
	cout << "\n\n" << endl;
	cout << "Matrix Results\n" << testMat << endl;


	waitKey(0);

	testMat.create(10,10,CV_16SC1);
	testMat2.create(10,10,CV_16SC1);
	buffer.create(10,10,CV_16SC1);
	vuErodeInt(&testMat,windowSize, &buffer, &testMat2);
	MatrixData queryDataIn(5,3,CV_32FC1);
	MatrixData trainDataIn(10,3,CV_32FC1);
	MatrixData rows;

	vector<FeatureMatch> matchResults;
	int numberOfMatches = 15;

	matchResults.resize(queryDataIn.rows * numberOfMatches);

	for(int i = 0; i< trainDataIn.rows;i++)
	{
		rows= trainDataIn.row(i);
		rows.setTo(Scalar(i));

	}

	for(int i = 0; i< queryDataIn.rows;i++)
	{
		rows= queryDataIn.row(i);
		rows.setTo(Scalar(i));

	}
	rows = trainDataIn.row(9);
	rows.setTo(Scalar(1));


	vuTopNMatchesFloat(&queryDataIn, &trainDataIn, numberOfMatches,&matchResults[0]);

	for(int i = 0; i < queryDataIn.rows; i ++)
	{
		cout << "Query Feature vector " << i << ":"<<endl;
		vuPrintFeatureVectorToConsole(&queryDataIn, i);
		for(int j = 0; j< numberOfMatches; j++)
		{	
			vuPrintFeatureMatchToConsole(&matchResults[i*numberOfMatches + j], &queryDataIn, &trainDataIn);
		}

	}

	vector< vector<DMatch > > testMatches;

	converFeatureMatchToDMatch(&matchResults[0], queryDataIn.rows, numberOfMatches,testMatches);

	for(int i = 0; i < (int)testMatches.size(); i ++)
	{
		cout << "DMatch Query Feature vector " << i << ":"<<endl;

		for(int j = 0; j< (int)testMatches[i].size(); j++)
		{	
			printf("\t Dist: %g,  queryId %d ->  train Id %d\n",testMatches[i][j].distance,testMatches[i][j].queryIdx,testMatches[i][j].trainIdx);
		}

	}


	waitKey(0);


}


void vuColorDisparityMap(MatrixData * dataIn, MatrixData * colorDisparity, float numberOfDisparities, float subpixelDisparities)
{
	unsigned char colors[2][3] ={{0,0,196},{0,0,0}};

	//MatrixData colorDisparity(dataIn->rows,dataIn->cols,CV_8UC3);
	//cvtColor(*dataIn,colorDisparity,CV_

	for(int y = 0; y< dataIn->rows; y++)
	{
		unsigned char * colorDispRowPtr = 	colorDisparity->data + colorDisparity->step[0] * y;
		signed short int * dataInRowPtr = (signed short int * )( dataIn->data + dataIn->step[0] * y);

		for(int x = 0; x< dataIn->cols; x++)
		{
			double currentDisparityValue = (255.0/(numberOfDisparities* subpixelDisparities)) * dataInRowPtr[x];
			if(currentDisparityValue>0)
			{

				colorDispRowPtr[x*3+0] = colors[0][0];
				colorDispRowPtr[x*3+1] = colors[0][1] + (unsigned char)currentDisparityValue;
				colorDispRowPtr[x*3+2] = colors[0][2];

			}
			else
			{
				colorDispRowPtr[x*3+0] = colors[1][0];
				colorDispRowPtr[x*3+1] = colors[1][1];
				colorDispRowPtr[x*3+2] = colors[1][2];
			}



		}



	}

//	imshow("Test Disp", *colorDisparity);
//	imwrite("LastColorRawDepth.png",*colorDisparity);
//	waitKey(0);

}
