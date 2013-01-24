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

#include "VisualSonification.hpp"
#define DATA_GENERATION
#ifdef USE_DVP
#include <dvp_kgm_sonification.h>
#endif

#ifndef ANDROID
#define USE_EXTERNAL_PARSE
#endif
#ifdef USE_EXTERNAL_PARSE

#include "Parse.hpp"
#endif
#ifndef ANDROID
VisualSonification visualSonificationObject;
#endif
#ifdef ANDROID
#include <android/log.h>
#if !defined(LOG_TAG)
#define LOG_TAG "FCAM_Capture"
#define LOGV(...) __android_log_print(ANDROID_LOG_VERBOSE, LOG_TAG, __VA_ARGS__)
#define LOGD(...) __android_log_print(ANDROID_LOG_DEBUG  , LOG_TAG, __VA_ARGS__)
#define LOGI(...) __android_log_print(ANDROID_LOG_INFO   , LOG_TAG, __VA_ARGS__)
#define LOGW(...) __android_log_print(ANDROID_LOG_WARN   , LOG_TAG, __VA_ARGS__)
#define LOGE(...) __android_log_print(ANDROID_LOG_ERROR  , LOG_TAG, __VA_ARGS__)
#endif

#endif


static VisualSonification * visualSonificationObjectPtr;


//Local function Defintiions
void localizeFeaturePoints(VisualSonificationInfo & info, Mat & inputImage,  vector<KeyPoint> & keyPoints );
void buildFeatureDescriptors(VisualSonificationInfo & info, Mat & inputImage, vector<KeyPoint> & keyPoints, Mat & descriptors);





//This assumes the depth maps are float, and the mask is unsigned char
//Desc: This thresholds the depth map, setting values to upper or lower limit and also computes a mask of valid depth regions
//  The depth maps are float, the mask is unsigned char.  They must be allocated outside this function
//Return: None
void thresholdDepthMapAndProduceMask(float lowerThreshold, float upperThreshold, float newValForOverVals,Mat & depthMapIn, Mat & depthMapOut, Mat & maskOut)
{

	//Loop through rows
	for(int y = 0; y < depthMapIn.rows; y++)
	{

		//Pointer to depth data and mask row data
		float * depthMapInRow = depthMapIn.ptr<float>(y);
		float * depthMapOutRow = depthMapOut.ptr<float>(y);
		unsigned char * maskRow = maskOut.ptr<unsigned char>(y);

		//Loop through columns
		for(int x =0; x < depthMapIn.cols; x++)
		{

			//Create mask and apply threashold
			maskRow[x] = ((depthMapInRow[x] >= upperThreshold)||(depthMapInRow[x]<lowerThreshold)?(unsigned char)0:(unsigned char)255);

			depthMapOutRow[x] = ((depthMapInRow[x] >= upperThreshold )||(depthMapInRow[x]<lowerThreshold)?newValForOverVals:depthMapInRow[x]);

		}
	}
}


//This assumes the depth maps are float, and the mask is unsigned char
//Desc: This thresholds the depth map, setting values to upper or lower limit and also computes a mask of valid depth regions
//  The depth maps are float, the mask is unsigned char.  They must be allocated outside this function
//Return: None
void thresholdDepthMapAndProduceMaskFixedPoint(int lowerThreshold, int upperThreshold, int newValForOverVals,Mat & depthMapIn, Mat & depthMapOut, Mat & maskOut)
{

	//Loop through rows
	for(int y = 0; y < depthMapIn.rows; y++)
	{

		//Pointer to depth data and mask row data
		short int * depthMapInRow = depthMapIn.ptr<short int>(y);
		short int * depthMapOutRow = depthMapOut.ptr<short int>(y);
		unsigned char * maskRow = maskOut.ptr<unsigned char>(y);

		//Loop through columns
		for(int x =0; x < depthMapIn.cols; x++)
		{

			//Create mask and apply threashold
			maskRow[x] = ((depthMapInRow[x] >= upperThreshold<<LOCATION_3D_POINT_FRACTIONAL_BITS)||(depthMapInRow[x]<lowerThreshold<<LOCATION_3D_POINT_FRACTIONAL_BITS)?(unsigned char)0:(unsigned char)255);

			depthMapOutRow[x] = ((depthMapInRow[x] >= upperThreshold<<LOCATION_3D_POINT_FRACTIONAL_BITS )||(depthMapInRow[x]<lowerThreshold<<LOCATION_3D_POINT_FRACTIONAL_BITS)?newValForOverVals<<LOCATION_3D_POINT_FRACTIONAL_BITS:depthMapInRow[x]);

		}
	}
}

string visualSonificationStripFileExtension(string filename)
{
	string baseFilename;
	size_t found = filename.find_last_of('.');
	if(found == string::npos)
	{
		found = filename.length();

	}
	baseFilename = filename.substr(0,found);

	return baseFilename;



}

string visualSonificationStripFileExtensionAndPath(string filename)
{
	string baseFilename;

	size_t foundExt = filename.find_last_of('.');
	if(foundExt == string::npos)
	{
		foundExt = filename.length();

	}

	size_t foundDir = filename.find_last_of('/');
	if(foundDir == string::npos)
	{
		foundDir = filename.length();

	}
	size_t foundDirBack = filename.find_last_of('\\');
	if(foundDirBack == string::npos)
	{
		foundDirBack = filename.length();

	}

	if(foundDir == filename.length() && foundDirBack!= filename.length())
	{
		foundDir = foundDirBack;


	}


	baseFilename = filename.substr(foundDir,foundExt);





	return baseFilename;

}




int visualSonificationSegmentInputsWithExternalMask(VisualSonificationInfo & info)
{
	int numberOfRegions = 0;
	#ifdef USE_EXTERNAL_PARSE


		string dataFilename = visualSonificationStripFileExtension(info.data.imageList[VISUAL_SONIFICATION_LEFT_CAMERA][(info.data.currentInputFrames>0?info.data.currentInputFrames-1:info.data.currentInputFrames)]);
		string dataFilePath = info.config.imageListBaseDir[VISUAL_SONIFICATION_LEFT_CAMERA] + "Data1/";
		dataFilename = dataFilePath+ dataFilename;
		dataFilename = dataFilename + ".txt";
		Mat tmpImage;

		AnnotatedDatasetInfo dataFromSegmentationFiles = getAnnotatedImageData(tmpImage, dataFilename);


		info.data.regionsByPoints[VISUAL_SONIFICATION_LEFT_CAMERA] = dataFromSegmentationFiles.Points;
		info.data.regionLabelsImages[VISUAL_SONIFICATION_LEFT_CAMERA] = dataFromSegmentationFiles.Labels;
		info.data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA] = dataFromSegmentationFiles.Depths;
		numberOfRegions = (int)info.data.regionsByPoints[VISUAL_SONIFICATION_LEFT_CAMERA].size();
		if(info.config.showEdgeImage)
		{
			displayComponentResults(info.data.regionLabelsImages[VISUAL_SONIFICATION_LEFT_CAMERA]);
		}




	#endif
	return numberOfRegions;
}


//Desc: This function performs the segmentations and places the results in the proper location in the passed
//  in parameter info.  In this case info.data.regionLabelsImages[VISUAL_SONIFICATION_LEFT_CAMERA] has a Matrix structure
//  with each pixels label, and info.data.regionsByPoints[VISUAL_SONIFICATION_LEFT_CAMERA] contains the vector of components 
//  where each component is a vector of points
//Return: Number of components or regions

int visualSonificationSegmentInputs(VisualSonificationInfo & info)
{

	int numberOfRegions = 0;

#ifdef FIXED_POINT

	//Used to hold an extracted depth channel
	Mat depth(info.data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].size(),CV_16SC1);

	//A buffer 
	Mat buffer(info.data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].size(),CV_16SC1);

	//The depth mask
	Mat validDepthMask(info.data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].size(),CV_8UC1);

	//The results of filtering or thresholding
	Mat filteredDepth(info.data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].size(),CV_16SC1);


//	int fromToArray[2] = {2,0};


	//Get the z channel of the 3D points as the depth
//	clock_t before = clock();
//	mixChannels(&info.data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA],1,&depth,1,fromToArray,1);
//	clock_t after = clock();
//	cout << "OpenCV Before: " << before << "  After: " << after << endl;

//	before = clock();

	//Extraction the depth data from the depth camera information which is 3 channel (x,yz)
	vuExtractChannelShortInt(&info.data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA], 3,2, &depth);
	
//	after = clock();
//	cout << "Custom Before: " << before << "  After: " << after << endl;



	//cout << "depth:" << depth << endl;



	//FileStorage fs1("Depth.yml",FileStorage::WRITE);
	//fs1 << "Depth" << depth;
	//fs1.release();

	//Make sure nothing it is all wihtin our range, if it is not make it zero to make finding items easier
	//threshold(depth,filteredDepth,info.config.maxDepth,info.config.maxDepth,THRESH_TRUNC);
	//filteredDepth = abs(depth);

	//Since we are looking down the -z axis, make this easier and just take abs value
	vuAbsShortIntSingleChannel(&depth, &filteredDepth);

	//imshow("Filtered",filteredDepth);


	//	threshold(filteredDepth,filteredDepth,info.config.maxDepth,info.config.maxDepth,THRESH_TOZERO_INV);
	//FileStorage fs1("Depth.yml",FileStorage::WRITE);
	//fs1 << "Depth2" << filteredDepth;
	//fs1.release();

	//Threshold and compute valid mask
	//thresholdDepthMapAndProduceMask(info.config.minDepth,info.config.maxDepth, 0,filteredDepth,filteredDepth, validDepthMask);
	thresholdDepthMapAndProduceMaskFixedPoint((int)info.config.minDepth,(int)info.config.maxDepth, (int)0,filteredDepth,filteredDepth, validDepthMask);
	//Show the depth mask
	//imshow("DepthMask",validDepthMask);
	//waitKey(0);
	//filteredDepth = depth.clone();

	//Now we filter the depth image a little
	//GaussianBlur(filteredDepth,filteredDepth,Size(3,3),1.5,1.5);





	//FileStorage fs2("FilteredDepth.yml",FileStorage::WRITE);
	//fs2 << "FilteredDepth" << filteredDepth;
	//fs2.release();


	//Next we want to dilate and erode just a little
	//dilate(filteredDepth,filteredDepth,Mat(),Point(-1,-1),2);
//	vuDilateFloat(&filteredDepth,Size(3,3),&buffer,&filteredDepth);
//	vuDilateFloat(&filteredDepth,Size(3,3),&buffer,&filteredDepth);
//	vuDilateFloat(&filteredDepth,Size(3,3),&buffer,&filteredDepth);

	//FileStorage fs3("FilteredDepthDilate.yml",FileStorage::WRITE);
	//fs3 << "FilteredDepthDilate" << filteredDepth;
	//fs3.release();



	//Next we want to dilate and erode just a little
	//erode(filteredDepth,filteredDepth,Mat(),Point(-1,-1),2);
//	vuErodeFloat(&filteredDepth,Size(3,3),&buffer,&filteredDepth);
//	vuErodeFloat(&filteredDepth,Size(3,3),&buffer,&filteredDepth);
//	vuErodeFloat(&filteredDepth,Size(3,3),&buffer,&filteredDepth);
	//FileStorage fs4("FilteredDepthErode.yml",FileStorage::WRITE);
	//fs4 << "FilteredDepthErode" << filteredDepth;
	//fs4.release();


	//FileStorage fs1("testCompon.yml",FileStorage::WRITE);
	//fs1 << "Depth" << filteredDepth;
	//fs1.release();
/*
	Mat rawDepthDisplay;
	info.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].convertTo(rawDepthDisplay,CV_32F,255.0/((double)(info.data.stereoBlockMatcher.state->numberOfDisparities* 16)));

	GaussianBlur(rawDepthDisplay,rawDepthDisplay,Size(3,3),1.5,1.5);
	dilate(rawDepthDisplay,rawDepthDisplay,Mat(),Point(-1,-1),2);
	erode(rawDepthDisplay,rawDepthDisplay,Mat(),Point(-1,-1),2);
	info.data.regionsByPoints[VISUAL_SONIFICATION_LEFT_CAMERA] = findConnectedComponents(rawDepthDisplay, info.data.regionLabelsImages[VISUAL_SONIFICATION_LEFT_CAMERA], info.config.connectedComponentConfig);
*/
#else


	//Used to hold an extracted depth channel
	Mat depth(info.data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].size(),CV_32FC1);

	//A buffer
	Mat buffer(info.data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].size(),CV_32FC1);

	//The depth mask
	Mat validDepthMask(info.data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].size(),CV_8UC1);

	//The results of filtering or thresholding
	Mat filteredDepth(info.data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].size(),CV_32FC1);


	//Extraction the depth data from the depth camera information which is 3 channel (x,yz)
	vuExtractChannelFloat(&info.data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA], 3,2, &depth);



	//Since we are looking down the -z axis, make this easier and just take abs value
	vuAbsFloatSingleChannel(&depth, &filteredDepth);


	//Threshold and compute valid mask
	thresholdDepthMapAndProduceMask(info.config.minDepth,info.config.maxDepth, 0,filteredDepth,filteredDepth, validDepthMask);


#endif

#ifdef USE_DEPTH_CONNECTED_COMPONENT


#ifdef FIXED_POINT
	//Get the connected components
	info.data.regionsByPoints[VISUAL_SONIFICATION_LEFT_CAMERA] = findConnectedComponentsFixedPointWrapper(info.config.connectedComponentConfig,filteredDepth,validDepthMask, info.data.regionLabelsImages[VISUAL_SONIFICATION_LEFT_CAMERA]);//findConnectedComponents(info.config.connectedComponentConfig,filteredDepth,validDepthMask, info.data.regionLabelsImages[VISUAL_SONIFICATION_LEFT_CAMERA]);
	//info.data.regionsByPoints[VISUAL_SONIFICATION_LEFT_CAMERA] = findConnectedComponents(info.config.connectedComponentConfig,filteredDepth,validDepthMask, info.data.regionLabelsImages[VISUAL_SONIFICATION_LEFT_CAMERA]);//findConnectedComponents(info.config.connectedComponentConfig,filteredDepth,validDepthMask, info.data.regionLabelsImages[VISUAL_SONIFICATION_LEFT_CAMERA]);
#else


	//Get the connected components
	info.data.regionsByPoints[VISUAL_SONIFICATION_LEFT_CAMERA] = findConnectedComponents(info.config.connectedComponentConfig,filteredDepth,validDepthMask, info.data.regionLabelsImages[VISUAL_SONIFICATION_LEFT_CAMERA]);//findConnectedComponents(info.config.connectedComponentConfig,filteredDepth,validDepthMask, info.data.regionLabelsImages[VISUAL_SONIFICATION_LEFT_CAMERA]);
	//info.data.regionsByPoints[VISUAL_SONIFICATION_LEFT_CAMERA] = findConnectedComponentsFixedPointWrapper(info.config.connectedComponentConfig,filteredDepth,validDepthMask, info.data.regionLabelsImages[VISUAL_SONIFICATION_LEFT_CAMERA]);//findConnectedComponents(info.config.connectedComponentConfig,filteredDepth,validDepthMask, info.data.regionLabelsImages[VISUAL_SONIFICATION_LEFT_CAMERA]);
 

#endif
	//Show results if set
	if(info.config.showEdgeImage)
	{
		displayComponentResults(info.data.regionLabelsImages[VISUAL_SONIFICATION_LEFT_CAMERA]);
	}
	//Set how many components we found
	numberOfRegions = (int)info.data.regionsByPoints[VISUAL_SONIFICATION_LEFT_CAMERA].size();
#else


	

	Mat edgeImage(filteredDepth.size(),CV_8UC1);// = filteredDepth.clone();


	//Now we have cleaned it up a little bit so now we go ahead and calculate the gradient in x and y or we can use canny
	if(info.config.useCannyEdgeDetector)	
	{

		//We will need to normalize to such that it fits in 255 to 0 for the canny function so create a mat for that
		Mat normalizedFilteredDepth(filteredDepth.size(),CV_8UC1);

		
		//normalize so we can call the canny function
		normalize(filteredDepth,normalizedFilteredDepth,0,255,NORM_MINMAX,CV_8UC1);
		//FileStorage fs5("NormalFilteredDepth.yml",FileStorage::WRITE);
		//fs5 << "NormalFilteredDepth" << filteredDepth;
		//fs5.release();

		//Use the canny edge detector
		Canny(normalizedFilteredDepth,edgeImage,.20,5.0,3,false);
		//imshow("Normalized Filtered Depth", normalizedFilteredDepth);


		//Next we want to dilate a little to close off some regions
		//This also helps us get thick borders for contour tracing
		dilate(edgeImage,edgeImage,Mat(),Point(-1,-1),1);

		erode(edgeImage,edgeImage,Mat(),Point(-1,-1),1);

		//FileStorage fs("NormalizedDepth.yml",FileStorage::WRITE);
		//fs << "Norm Depth" << normalizedFilteredDepth;
		//fs.release();
	}
	else
	{

		//We can use the raw gradient also if we don't want to use canny
		Mat sobelX, sobelY;
		Mat sobelDir;
		Mat sobelMagnitude;

		//Use the sobel to compute the x and y gradients
		Sobel(filteredDepth,sobelX,CV_32F,1,0,3);
		Sobel(filteredDepth,sobelY,CV_32F,0,1,3);

		//Store the direction of the gradient
		sobelDir.create(sobelX.size(),CV_32FC1);


		//Calculate all the directions for the gradients
		for(int y = 0; y < sobelX.rows;y++)
		{
			float * sobelXRow = sobelX.ptr<float>(y);
			float * sobelYRow = sobelY.ptr<float>(y);
			float * sobelDirRow = sobelDir.ptr<float>(y);
			for(int x = 0; x < sobelX.cols;x++)
			{
				sobelDirRow[x] = fastAtan2(sobelYRow[x],sobelXRow[x]);
			}
		}



		//Get the L2 for evaluating the magnitude
		sobelX = sobelX.mul(sobelX);
		sobelY = sobelY.mul(sobelY);
		sobelMagnitude= sobelX+sobelY; 


		//Debug code to show sobel results
		//imshow("Sobel X", sobelX);
		//imshow("Sobel Y", sobelY);
		
		//Use the L2 to generate an edge image
		for(int y = 0; y < sobelX.rows;y++)
		{
			float * sobelMagRow = sobelMagnitude.ptr<float>(y);
			unsigned char * edgeImageRow = edgeImage.ptr<unsigned char>(y);

			for(int x = 0; x < sobelX.cols;x++)
			{
				if(sobelMagRow[x] >= info.config.depthDeltaThreshold)
				{
					edgeImageRow[x] = 255;
				}
				else
				{
					edgeImageRow[x] = 0;

				}
			}
		}


	}

	//If asked, show the edge image
	if(info.config.showEdgeImage)
	{
		imshow("Edge Image", edgeImage);
	}

	Mat edgeImageFloat;

	edgeImage.convertTo(edgeImageFloat,CV_32F);
	//Now we need to get the regions using contour tracing.
	info.data.regionsByPoints[VISUAL_SONIFICATION_LEFT_CAMERA] = findConnectedComponents(edgeImageFloat, info.data.regionLabelsImages[VISUAL_SONIFICATION_LEFT_CAMERA], info.config.connectedComponentConfig);
	numberOfRegions = info.data.regionsByPoints[VISUAL_SONIFICATION_LEFT_CAMERA].size();
	//waitKey(0);

#endif
	return numberOfRegions;

}

//Desc: This function performs the feature extraction based on the data in info
//Return: Number of descriptors created

int visualSonificationExtractFeatures(VisualSonificationInfo & info)
{

	getStartClockTime(TIMING_EVENT_GENERAL_0);
	//First step extract all the features in the image
	localizeFeaturePoints( info, info.data.currentCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA],  info.data.currentKeypoints[VISUAL_SONIFICATION_LEFT_CAMERA] );
	getEndClockTime(TIMING_EVENT_GENERAL_0);



		
	getStartClockTime(TIMING_EVENT_GENERAL_1);
	//Get descriptors
	buildFeatureDescriptors(info, info.data.currentCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA], info.data.currentKeypoints[VISUAL_SONIFICATION_LEFT_CAMERA], info.data.currentDescriptors[VISUAL_SONIFICATION_LEFT_CAMERA]);

	getEndClockTime(TIMING_EVENT_GENERAL_1);





	return info.data.currentDescriptors[VISUAL_SONIFICATION_LEFT_CAMERA].rows;
}



//Desc: This function gets the next set of image data including depth.  In the case of stereo cameras, it computes the depth
//The depth camera is actually returned as a 3D point cloud in a three channel image
//Return: Number of images remaining

int getNextImages(VisualSonificationInfo & info)
{

	int imageCountRemaining = INT_MAX;
	Mat currentDepthFloat;
	Mat currentDepthShifted;
	MatrixData * QPtr;
	if(info.data.inputMode == VISUAL_SONIFICATION_STEREO)
	{
		switch(info.data.inputType)
		{

			case VISUAL_SONIFICATION_STREAM:

				info.data.currentInputFrames++;
				//Get the new data frames
				info.data.capture[VISUAL_SONIFICATION_LEFT_CAMERA] >> info.data.currentCameraBuffers[VISUAL_SONIFICATION_LEFT_CAMERA];
				info.data.capture[VISUAL_SONIFICATION_RIGHT_CAMERA] >> info.data.currentCameraBuffers[VISUAL_SONIFICATION_RIGHT_CAMERA];

				//Copy the the current frames to the Raw holder
				info.data.currentCameraBuffers[VISUAL_SONIFICATION_LEFT_CAMERA].copyTo(info.data.currentRawCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA]);
				info.data.currentCameraBuffers[VISUAL_SONIFICATION_RIGHT_CAMERA].copyTo(info.data.currentRawCameraFrames[VISUAL_SONIFICATION_RIGHT_CAMERA]);

				//Remap the frames and save them to the currentFrame buffer
				remap(info.data.currentCameraBuffers[VISUAL_SONIFICATION_LEFT_CAMERA], info.data.currentCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA], info.data.rmap[VISUAL_SONIFICATION_LEFT_CAMERA][VISUAL_SONIFICATION_CAMERA_REMAP_AXIS_X], info.data.rmap[VISUAL_SONIFICATION_LEFT_CAMERA][VISUAL_SONIFICATION_CAMERA_REMAP_AXIS_Y], CV_INTER_LINEAR);
				remap(info.data.currentCameraBuffers[VISUAL_SONIFICATION_RIGHT_CAMERA], info.data.currentCameraFrames[VISUAL_SONIFICATION_RIGHT_CAMERA], info.data.rmap[VISUAL_SONIFICATION_RIGHT_CAMERA][VISUAL_SONIFICATION_CAMERA_REMAP_AXIS_X], info.data.rmap[VISUAL_SONIFICATION_RIGHT_CAMERA][VISUAL_SONIFICATION_CAMERA_REMAP_AXIS_Y], CV_INTER_LINEAR);

				//Get the grayscale versions
				cvtColor(info.data.currentCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA],info.data.currentGrayCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA],CV_BGR2GRAY);
				cvtColor(info.data.currentCameraFrames[VISUAL_SONIFICATION_RIGHT_CAMERA],info.data.currentGrayCameraFrames[VISUAL_SONIFICATION_RIGHT_CAMERA],CV_BGR2GRAY);

				getStartClockTime(TIMING_EVENT_CALC_DISPARITY);
				//Now we need the depth data and since this is a stereo camera configuration lets compute the disparity
				if(!info.config.useGlobalBlockMatching)
				{
					info.data.stereoBlockMatcher(info.data.currentGrayCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA],info.data.currentGrayCameraFrames[VISUAL_SONIFICATION_RIGHT_CAMERA],info.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA],CV_16S);
				}
				else
				{
					info.data.stereoGlobalBlockMatcher(info.data.currentCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA],info.data.currentCameraFrames[VISUAL_SONIFICATION_RIGHT_CAMERA],info.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA]);
				}
				getEndClockTime(TIMING_EVENT_CALC_DISPARITY);






				getStartClockTime(TIMING_EVENT_CALC_DEPTH);


#ifndef FIXED_POINT
				//The integer disparity is shift by 4 bits for sub pixel accuracy.  Lets get rid of that for the depth calculation
				//info.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].convertTo(currentDepthFloat,CV_32F);
				//divide((16),info.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA],currentDepthShifted);
				//currentDepthShifted.create(info.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].size(),info.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].type());
				currentDepthShifted.create(info.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].size(),CV_32FC1);
				for(int y=0; y<currentDepthShifted.rows; y++)
				{

					//short int * shiftedDepthRow = currentDepthShifted.ptr<signed short int>(y);
					float * shiftedDepthRow = currentDepthShifted.ptr<float>(y);
					short int * rawDepthRow = info.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].ptr<signed short int>(y);

					for(int x=0; x<currentDepthShifted.cols; x++)
					{
						//shiftedDepthRow[x]  = rawDepthRow[x]/16;
						shiftedDepthRow[x]  = (float)rawDepthRow[x]/16.0f;
					}
				}


#endif

				//Now we need to convert the disparity to 3D  
				//reprojectImageTo3D(currentDepthShifted, info.data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA], info.data.stereoCalibration.getQ(), true);
				QPtr = info.data.stereoCalibration.getQPtr();


#ifdef FIXED_POINT

				info.data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].create(info.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].rows,info.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].cols,CV_16SC3);
				//Mat fixedPointDepth(info.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].size(),CV_16SC3);
				vuProjectPointsTo3DFixedPoint(&info.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA], QPtr, &info.data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA]);

#else
				info.data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].create(currentDepthShifted.rows,currentDepthShifted.cols,CV_32FC3);
				vuProjectPointsTo3D(&currentDepthShifted, QPtr, &info.data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA]);
				//reprojectImageTo3D(currentDepthShifted, info.data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA], info.data.stereoCalibration.getQ(), true);



#endif


				getEndClockTime(TIMING_EVENT_CALC_DEPTH);
				
				break;
			case VISUAL_SONIFICATION_VIDEO_FILE:




				break;
			case VISUAL_SONIFICATION_IMAGE_LIST:
				{
					string leftImageNameWithPath = info.config.imageListBaseDir[VISUAL_SONIFICATION_LEFT_CAMERA] + info.data.imageList[VISUAL_SONIFICATION_LEFT_CAMERA][info.data.currentInputFrames];
					string rightImageNameWithPath = info.config.imageListBaseDir[VISUAL_SONIFICATION_RIGHT_CAMERA] + info.data.imageList[VISUAL_SONIFICATION_RIGHT_CAMERA][info.data.currentInputFrames];

					//Get the new data frames
					info.data.currentCameraBuffers[VISUAL_SONIFICATION_LEFT_CAMERA] = imread(leftImageNameWithPath);
					info.data.currentCameraBuffers[VISUAL_SONIFICATION_RIGHT_CAMERA] = imread(rightImageNameWithPath);

					
					info.data.currentInputFrames++;
					#ifdef ENDLESS_LIST
					info.data.currentInputFrames--;
					#endif
					if(!info.data.currentCameraBuffers[VISUAL_SONIFICATION_LEFT_CAMERA].empty()&& !info.data.currentCameraBuffers[VISUAL_SONIFICATION_RIGHT_CAMERA].empty())
					{

						imageCountRemaining = (int)info.data.imageList[VISUAL_SONIFICATION_LEFT_CAMERA].size() - info.data.currentInputFrames;
					}
					else
					{
						imageCountRemaining = -1;
						break;
					}

					//Copy the the current frames to the Raw holder
					info.data.currentCameraBuffers[VISUAL_SONIFICATION_LEFT_CAMERA].copyTo(info.data.currentRawCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA]);
					info.data.currentCameraBuffers[VISUAL_SONIFICATION_RIGHT_CAMERA].copyTo(info.data.currentRawCameraFrames[VISUAL_SONIFICATION_RIGHT_CAMERA]);

					//Remap the frames and save them to the currentFrame buffer
					remap(info.data.currentCameraBuffers[VISUAL_SONIFICATION_LEFT_CAMERA], info.data.currentCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA], info.data.rmap[VISUAL_SONIFICATION_LEFT_CAMERA][VISUAL_SONIFICATION_CAMERA_REMAP_AXIS_X], info.data.rmap[VISUAL_SONIFICATION_LEFT_CAMERA][VISUAL_SONIFICATION_CAMERA_REMAP_AXIS_Y], CV_INTER_LINEAR);
					remap(info.data.currentCameraBuffers[VISUAL_SONIFICATION_RIGHT_CAMERA], info.data.currentCameraFrames[VISUAL_SONIFICATION_RIGHT_CAMERA], info.data.rmap[VISUAL_SONIFICATION_RIGHT_CAMERA][VISUAL_SONIFICATION_CAMERA_REMAP_AXIS_X], info.data.rmap[VISUAL_SONIFICATION_RIGHT_CAMERA][VISUAL_SONIFICATION_CAMERA_REMAP_AXIS_Y], CV_INTER_LINEAR);

					//Get the grayscale versions
					cvtColor(info.data.currentCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA],info.data.currentGrayCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA],CV_BGR2GRAY);
					cvtColor(info.data.currentCameraFrames[VISUAL_SONIFICATION_RIGHT_CAMERA],info.data.currentGrayCameraFrames[VISUAL_SONIFICATION_RIGHT_CAMERA],CV_BGR2GRAY);

					getStartClockTime(TIMING_EVENT_CALC_DISPARITY);
					//Now we need the depth data and since this is a stereo camera configuration lets compute the disparity
					if(!info.config.useGlobalBlockMatching)
					{
						info.data.stereoBlockMatcher(info.data.currentGrayCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA],info.data.currentGrayCameraFrames[VISUAL_SONIFICATION_RIGHT_CAMERA],info.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA],CV_16S);
					}
					else
					{
						info.data.stereoGlobalBlockMatcher(info.data.currentCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA],info.data.currentCameraFrames[VISUAL_SONIFICATION_RIGHT_CAMERA],info.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA]);
					}
					getEndClockTime(TIMING_EVENT_CALC_DISPARITY);


					getStartClockTime(TIMING_EVENT_CALC_DEPTH);


#ifndef FIXED_POINT
					//The integer disparity is shift by 4 bits for sub pixel accuracy.  Lets get rid of that for the depth calculation
					//info.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].convertTo(currentDepthFloat,CV_32F);
					//divide((16),info.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA],currentDepthShifted);
					//currentDepthShifted.create(info.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].size(),info.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].type());
					currentDepthShifted.create(info.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].size(),CV_32FC1);
					for(int y=0; y<currentDepthShifted.rows; y++)
					{

						//short int * shiftedDepthRow = currentDepthShifted.ptr<signed short int>(y);
						float * shiftedDepthRow = currentDepthShifted.ptr<float>(y);
						short int * rawDepthRow = info.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].ptr<signed short int>(y);
					
						for(int x=0; x<currentDepthShifted.cols; x++)
						{
							//shiftedDepthRow[x]  = rawDepthRow[x]/16;
							shiftedDepthRow[x]  = (float)rawDepthRow[x]/16.0f;
						}
					}


#endif

					//Now we need to convert the disparity to 3D  
					//reprojectImageTo3D(currentDepthShifted, info.data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA], info.data.stereoCalibration.getQ(), true);
					QPtr = info.data.stereoCalibration.getQPtr();


#ifdef FIXED_POINT

					info.data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].create(info.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].rows,info.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].cols,CV_16SC3);
					//Mat fixedPointDepth(info.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].size(),CV_16SC3);
					vuProjectPointsTo3DFixedPoint(&info.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA], QPtr, &info.data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA]);

					//FileStorage testing;


					//testing.open("testingDepth.yml",FileStorage::WRITE);

					//testing << "FP"<<fixedPointDepth;
#else
					info.data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].create(currentDepthShifted.rows,currentDepthShifted.cols,CV_32FC3);
					vuProjectPointsTo3D(&currentDepthShifted, QPtr, &info.data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA]);
					//reprojectImageTo3D(currentDepthShifted, info.data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA], info.data.stereoCalibration.getQ(), true);

							//info.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA], info.data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA], info.data.stereoCalibration.getQ(), true);

#endif
					//testing << "Float"<<info.data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA];

					//testing.release();


					getEndClockTime(TIMING_EVENT_CALC_DEPTH);



				}
				break;

			case VISUAL_SONIFICATION_EXTERNAL_STREAM:
				//Get the new data frames
				//info.data.capture[VISUAL_SONIFICATION_LEFT_CAMERA] >> info.data.currentCameraBuffers[VISUAL_SONIFICATION_LEFT_CAMERA];
				//info.data.capture[VISUAL_SONIFICATION_RIGHT_CAMERA] >> info.data.currentCameraBuffers[VISUAL_SONIFICATION_RIGHT_CAMERA];


				info.data.currentInputFrames++;

				//Copy the the current frames to the Raw holder
				info.data.currentCameraBuffers[VISUAL_SONIFICATION_LEFT_CAMERA].copyTo(info.data.currentRawCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA]);
				info.data.currentCameraBuffers[VISUAL_SONIFICATION_RIGHT_CAMERA].copyTo(info.data.currentRawCameraFrames[VISUAL_SONIFICATION_RIGHT_CAMERA]);

				//Remap the frames and save them to the currentFrame buffer
				remap(info.data.currentCameraBuffers[VISUAL_SONIFICATION_LEFT_CAMERA], info.data.currentCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA], info.data.rmap[VISUAL_SONIFICATION_LEFT_CAMERA][VISUAL_SONIFICATION_CAMERA_REMAP_AXIS_X], info.data.rmap[VISUAL_SONIFICATION_LEFT_CAMERA][VISUAL_SONIFICATION_CAMERA_REMAP_AXIS_Y], CV_INTER_LINEAR);
				remap(info.data.currentCameraBuffers[VISUAL_SONIFICATION_RIGHT_CAMERA], info.data.currentCameraFrames[VISUAL_SONIFICATION_RIGHT_CAMERA], info.data.rmap[VISUAL_SONIFICATION_RIGHT_CAMERA][VISUAL_SONIFICATION_CAMERA_REMAP_AXIS_X], info.data.rmap[VISUAL_SONIFICATION_RIGHT_CAMERA][VISUAL_SONIFICATION_CAMERA_REMAP_AXIS_Y], CV_INTER_LINEAR);




				//Get the grayscale versions
				cvtColor(info.data.currentCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA],info.data.currentGrayCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA],CV_BGR2GRAY);
				cvtColor(info.data.currentCameraFrames[VISUAL_SONIFICATION_RIGHT_CAMERA],info.data.currentGrayCameraFrames[VISUAL_SONIFICATION_RIGHT_CAMERA],CV_BGR2GRAY);

				getStartClockTime(TIMING_EVENT_CALC_DISPARITY);
				//Now we need the depth data and since this is a stereo camera configuration lets compute the disparity
				if(!info.config.useGlobalBlockMatching)
				{
					info.data.stereoBlockMatcher(info.data.currentGrayCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA],info.data.currentGrayCameraFrames[VISUAL_SONIFICATION_RIGHT_CAMERA],info.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA],CV_16S);
				}
				else
				{
					info.data.stereoGlobalBlockMatcher(info.data.currentCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA],info.data.currentCameraFrames[VISUAL_SONIFICATION_RIGHT_CAMERA],info.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA]);
				}
				getEndClockTime(TIMING_EVENT_CALC_DISPARITY);






				getStartClockTime(TIMING_EVENT_CALC_DEPTH);


#ifndef FIXED_POINT
				//The integer disparity is shift by 4 bits for sub pixel accuracy.  Lets get rid of that for the depth calculation
				//info.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].convertTo(currentDepthFloat,CV_32F);
				//divide((16),info.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA],currentDepthShifted);
				//currentDepthShifted.create(info.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].size(),info.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].type());
				currentDepthShifted.create(info.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].size(),CV_32FC1);
				for(int y=0; y<currentDepthShifted.rows; y++)
				{

					//short int * shiftedDepthRow = currentDepthShifted.ptr<signed short int>(y);
					float * shiftedDepthRow = currentDepthShifted.ptr<float>(y);
					short int * rawDepthRow = info.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].ptr<signed short int>(y);

					for(int x=0; x<currentDepthShifted.cols; x++)
					{
						//shiftedDepthRow[x]  = rawDepthRow[x]/16;
						shiftedDepthRow[x]  = (float)rawDepthRow[x]/16.0f;
					}
				}


#endif

				//Now we need to convert the disparity to 3D  
				//reprojectImageTo3D(currentDepthShifted, info.data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA], info.data.stereoCalibration.getQ(), true);
				QPtr = info.data.stereoCalibration.getQPtr();


#ifdef FIXED_POINT

				info.data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].create(info.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].rows,info.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].cols,CV_16SC3);
				//Mat fixedPointDepth(info.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].size(),CV_16SC3);
				vuProjectPointsTo3DFixedPoint(&info.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA], QPtr, &info.data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA]);

#else
				info.data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].create(currentDepthShifted.rows,currentDepthShifted.cols,CV_32FC3);
				vuProjectPointsTo3D(&currentDepthShifted, QPtr, &info.data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA]);
				//reprojectImageTo3D(currentDepthShifted, info.data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA], info.data.stereoCalibration.getQ(), true);



#endif


				getEndClockTime(TIMING_EVENT_CALC_DEPTH);


				break;
			default:

				break;
		}

	}
	else if(info.data.inputMode == VISUAL_SONIFICATION_MONO_AND_DEPTH)
	{
		switch(info.data.inputType)
		{
			case VISUAL_SONIFICATION_STREAM:

				break;
			case VISUAL_SONIFICATION_VIDEO_FILE:

				break;
			case VISUAL_SONIFICATION_IMAGE_LIST:
				break;
			default:

				break;
		}


	}else if (info.data.inputMode == VISUAL_SONIFICATION_MONO_AND_SEPARATE_SEGMENTATION)
	{
		switch(info.data.inputType)
		{
			case VISUAL_SONIFICATION_STREAM:

				break;
			case VISUAL_SONIFICATION_VIDEO_FILE:

				break;
			case VISUAL_SONIFICATION_IMAGE_LIST:
			{
				string leftImageNameWithPath = info.config.imageListBaseDir[VISUAL_SONIFICATION_LEFT_CAMERA] + info.data.imageList[VISUAL_SONIFICATION_LEFT_CAMERA][info.data.currentInputFrames];


				//Get the new data frames
				info.data.currentCameraBuffers[VISUAL_SONIFICATION_LEFT_CAMERA] = imread(leftImageNameWithPath);



				info.data.currentInputFrames++;
				#ifdef ENDLESS_LIST
				info.data.currentInputFrames--;
				#endif
				if(!info.data.currentCameraBuffers[VISUAL_SONIFICATION_LEFT_CAMERA].empty())
				{

					imageCountRemaining = (int)info.data.imageList[VISUAL_SONIFICATION_LEFT_CAMERA].size() - info.data.currentInputFrames;
				}
				else
				{
					imageCountRemaining = -1;
					break;
				}

				//Copy the the current frames to the Raw holder
				info.data.currentCameraBuffers[VISUAL_SONIFICATION_LEFT_CAMERA].copyTo(info.data.currentRawCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA]);



				info.data.currentCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA] = info.data.currentCameraBuffers[VISUAL_SONIFICATION_LEFT_CAMERA].clone();


				//Get the grayscale versions
				cvtColor(info.data.currentCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA],info.data.currentGrayCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA],CV_BGR2GRAY);


				getStartClockTime(TIMING_EVENT_CALC_DISPARITY);


				//Now to create a fake disparity map

				info.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].create(info.data.currentGrayCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA].rows,info.data.currentGrayCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA].cols,CV_16SC1);


				Scalar tmpScalar(10);
				info.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].setTo(tmpScalar);

				getEndClockTime(TIMING_EVENT_CALC_DISPARITY);


				getStartClockTime(TIMING_EVENT_CALC_DEPTH);








				tmpScalar = Scalar(0,0,50);
				info.data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].create(currentDepthShifted.rows,currentDepthShifted.cols,CV_32FC3);
				info.data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].setTo(tmpScalar);


				getEndClockTime(TIMING_EVENT_CALC_DEPTH);





			}
				break;
			default:

				break;
		}





	}










return imageCountRemaining;

}

//Desc: This function loads a configuration file and stores the lines in the vector
//Return: true if file loaded, false otherwise

bool loadConfigFile(vector<string> &commandArgs, string filename)
{

	std::ifstream configFile;
//	int i = 3;
	string tmpString;
	bool returnVal = false;

	//open the file
	configFile.open(filename.c_str());
	//cout << "Config Load" << endl;
	string test;
	//If the file is open
	if(configFile.good())
	{
		//cout << "Config Opened" << endl;

		//Get all the data
		while(!configFile.eof())
		{
			getline(configFile,tmpString);

			//cout << "Ends with: ("<<(int)tmpString[tmpString.size()-1] << "," <<(int)tmpString[tmpString.size()-2]<< ")" << endl;
			//cin >> test;
			if(tmpString[tmpString.size()-1] == 13)
			{
				
				tmpString = tmpString.erase(tmpString.size()-1);
			}
			if(tmpString[tmpString.size()-2] == 13)
			{
				tmpString = tmpString.erase(tmpString.size()-2);
			}
			if(tmpString[tmpString.size()-1] == 10)
			{
				tmpString = tmpString.erase(tmpString.size()-1);
			}
			if(tmpString[tmpString.size()-2] == 10)
			{
				tmpString = tmpString.erase(tmpString.size()-2);
			}

			if((tmpString.compare("")!=0) && tmpString[0] != '#')
			{
				//cout << tmpString << endl;
				commandArgs.push_back(tmpString);
			}

			//commandArgs.push_back(tmpString);

		}
		returnVal = true;

	}
	else
	{
		#ifndef ANDROID
		cout << "WARNING: Unable to open visual sonification params config file" << endl;
		#endif
	}
	return returnVal;

}


bool loadTextFileList(vector<string> &fileLines, string filename)
{

	std::ifstream configFile;
	//int i = 3;
	string tmpString;
	bool returnVal = false;


	configFile.open(filename.c_str());


	if(configFile.good())
	{

		while(!configFile.eof())
		{
			getline(configFile,tmpString);



			if(!(tmpString.compare("") == 0) && !(tmpString[0] == '#'))
			{

				if(tmpString[tmpString.size()-1] == 13)
				{
					
					tmpString = tmpString.erase(tmpString.size()-1);
				}
				if(tmpString[tmpString.size()-2] == 13)
				{
					tmpString = tmpString.erase(tmpString.size()-2);
				}
				if(tmpString[tmpString.size()-1] == 10)
				{
					tmpString = tmpString.erase(tmpString.size()-1);
				}
				if(tmpString[tmpString.size()-2] == 10)
				{
					tmpString = tmpString.erase(tmpString.size()-2);
				}

				fileLines.push_back(tmpString);
			}
		}
		returnVal = true;

	}
	else
	{
#ifndef ANDROID
		cout << "WARNING: Unable to open File List file" << endl;
#endif
	}
	return returnVal;

}




void parseAudioGenerationInformationFromParamsVector(vector<string> &commandArgs,struct HistogramSoundSource::AudioGenerationInformation & audioGenInfo)
{
	

	//Buffer
	stringstream stringBuffer("");
	audioGenInfo.currentSonificationTechnique = VISUAL_SONIFICATION_DEFAULT_SONIFICATION_TECH;

	audioGenInfo.maxFrequencyInHz = VISUAL_SONIFICATION_DEFAULT_MAX_FREQ;
	audioGenInfo.minFrequencyInHz = VISUAL_SONIFICATION_DEFAULT_MIN_FREQ;
	audioGenInfo.histogramOutputTotalTime = VISUAL_SONIFICATION_DEFAULT_REGION_OUTPUT_TIME;
	audioGenInfo.sampleFrequency = VISUAL_SONIFICATION_DEFAULT_AUDIO_SAMPLE_FREQ;

	for(int i = 0; i < (int)commandArgs.size(); i++)
	{


		if(commandArgs[i].compare("-sonificationTechnique")==0 ||commandArgs[i].compare("-SonificationTechnique")==0)
		{

			if(commandArgs[i+1].compare("pure_sine")==0)
			{
				audioGenInfo.currentSonificationTechnique = HistogramSoundSource::SOUND_GENERATION_PURE_SINEWAVE;


			}

			if(commandArgs[i+1].compare("pure_sine_linear_transition")==0)
			{
				audioGenInfo.currentSonificationTechnique = HistogramSoundSource::SOUND_GENERATION_PURE_SINEWAVE_WITH_TRANSITION;


			}

			if(commandArgs[i+1].compare("fundamental_with_2_harmonics")==0)
			{
				audioGenInfo.currentSonificationTechnique = HistogramSoundSource::SOUND_GENERATION_HARMONICS;

			}
			if(commandArgs[i+1].compare("convolution")==0)
			{
				audioGenInfo.currentSonificationTechnique = HistogramSoundSource::SOUND_GENERATION_CONVOLUTION;

			}
			if(commandArgs[i+1].compare("harmonics_piano")==0)
			{
				audioGenInfo.currentSonificationTechnique = HistogramSoundSource::SOUND_GENERATION_HARMONICS_PIANO;

			}

		}

		if(commandArgs[i].compare("-maxHistogramFreq")==0 || commandArgs[i].compare("-MaxHistogramFreq")==0)
		{
			stringBuffer.str("");
			stringBuffer.clear();

			stringBuffer << commandArgs[i+1];
			if((stringBuffer >> audioGenInfo.maxFrequencyInHz).fail())
			{
#ifndef ANDROID
				cout << "Max Histogram Freq could not be parsed." << endl;
#endif
			}

		}
		if(commandArgs[i].compare("-minHistogramFreq")==0 || commandArgs[i].compare("-MinHistogramFreq")==0)
		{
			stringBuffer.str("");
			stringBuffer.clear();

			stringBuffer << commandArgs[i+1];
			if((stringBuffer >> audioGenInfo.minFrequencyInHz).fail())
			{
#ifndef ANDROID
				cout << "Min Histogram Freq could not be parsed." << endl;
#endif
			}

		}




		if(commandArgs[i].compare("-regionOutputTime")==0 || commandArgs[i].compare("-RegionOutputTime")==0)
		{
			stringBuffer.str("");
			stringBuffer.clear();

			stringBuffer << commandArgs[i+1];
			if((stringBuffer >> audioGenInfo.histogramOutputTotalTime).fail())
			{
#ifndef ANDROID
				cout << "Region Output time could not be parsed." << endl;
#endif
			}

		}


		if(commandArgs[i].compare("-audioSampleFreq")==0 || commandArgs[i].compare("-AudioSampleFreq")==0)
		{
			stringBuffer.str("");
			stringBuffer.clear();

			stringBuffer << commandArgs[i+1];
			if((stringBuffer >> audioGenInfo.sampleFrequency).fail())
			{
#ifndef ANDROID
				cout << "Audio Sample Freq could not be parsed." << endl;
#endif
			}

		}


	}	
	
	
}

struct HistogramSoundSource::AudioGenerationInformation loadAndParseAudioGenerationParams(string filename)
{
	struct HistogramSoundSource::AudioGenerationInformation audioGenInfo;

	vector<string> commandArgs;

	loadConfigFile(commandArgs, filename);


	parseAudioGenerationInformationFromParamsVector(commandArgs,audioGenInfo);


	return audioGenInfo;
}


//Desc: This function parses a vector of strings to configure the sonfication config struct
//Return: None
void parseParamsVector(vector<string> & commandArgs,VisualSonificationInfo & visualSonificationInfo)
{

	//Buffer
	stringstream stringBuffer("");

	//Set the defaults, I make no promise that the default config works without file
	//I am not going to waste time verrifying and fixing
	visualSonificationInfo.config.calibrationFilename = "stereoCalibration.yml";
	visualSonificationInfo.config.leftCameraId = 0;
	visualSonificationInfo.config.rightCameraId = 0;
	visualSonificationInfo.data.inputMode = VISUAL_SONIFICATION_STEREO;
	visualSonificationInfo.data.inputType = VISUAL_SONIFICATION_STREAM;
	visualSonificationInfo.data.featureDescAlgo = FEATURE_DESC_SIFT;
	visualSonificationInfo.data.featureLocalAlgo = FEATURE_LOCALIZATION_SIFT;

	visualSonificationInfo.config.imageListFilenames[VISUAL_SONIFICATION_LEFT_CAMERA] = "";
	visualSonificationInfo.config.imageListFilenames[VISUAL_SONIFICATION_RIGHT_CAMERA] = "";

	visualSonificationInfo.config.connectedComponentConfig.depthDifferenceThreshold = 10.0f;//18.0f;
	visualSonificationInfo.config.connectedComponentConfig.depthDifferenceThresholdFixedPoint =(int)(visualSonificationInfo.config.connectedComponentConfig.depthDifferenceThreshold+5)<<(LOCATION_3D_POINT_FRACTIONAL_BITS);
	visualSonificationInfo.config.connectedComponentConfig.eightWayNeighborhood = true;
	visualSonificationInfo.config.connectedComponentConfig.minPixelArea = 15;
	visualSonificationInfo.config.minimumRegionPixelArea = 15;
	visualSonificationInfo.config.numberOfTopNeighborsForHistogram = -1;
#ifdef ANDROID
	visualSonificationInfo.config.showGrayscaleInputImage = false;//true;
	visualSonificationInfo.config.showRawDepthImage = false;//true;
	visualSonificationInfo.config.showRawInputImage = false;
	visualSonificationInfo.config.showRemappedInputImage = false;
	visualSonificationInfo.config.showEdgeImage = false;//true;
	visualSonificationInfo.config.showFeaturesImage = false;//true;
	visualSonificationInfo.config.showComponentRegionsAtSignatureGen = false;//true;
	visualSonificationInfo.config.showComponentRegionsAtAudioOutput = false;//true;
#else
	#ifdef DATA_GENERATION
		visualSonificationInfo.config.showGrayscaleInputImage = false;//true;
		visualSonificationInfo.config.showRawDepthImage = false;//true;
		visualSonificationInfo.config.showRawInputImage = false;
		visualSonificationInfo.config.showRemappedInputImage = false;
		visualSonificationInfo.config.showEdgeImage = false;//true;
		visualSonificationInfo.config.showFeaturesImage = false;//true;
		visualSonificationInfo.config.showComponentRegionsAtSignatureGen = false;//true;
		visualSonificationInfo.config.showComponentRegionsAtAudioOutput = false;//true;
	#else
		visualSonificationInfo.config.showGrayscaleInputImage = true;
		visualSonificationInfo.config.showRawDepthImage = true;
		visualSonificationInfo.config.showRawInputImage = true;
		visualSonificationInfo.config.showRemappedInputImage = true;
		visualSonificationInfo.config.showEdgeImage = true;
		visualSonificationInfo.config.showFeaturesImage = true;
		visualSonificationInfo.config.showComponentRegionsAtSignatureGen = false;//true;
		visualSonificationInfo.config.showComponentRegionsAtAudioOutput = false;//true;
	#endif


#endif

	visualSonificationInfo.config.maxDepth = 300;
	visualSonificationInfo.config.minDepth = 30;

	visualSonificationInfo.config.useGlobalBlockMatching = false;
	visualSonificationInfo.config.useCannyEdgeDetector = true;


	visualSonificationInfo.config.save3dPoints = false;
	visualSonificationInfo.config.saveGrayscaleInputImage = false;
	visualSonificationInfo.config.saveRawDepthImage = false;
	visualSonificationInfo.config.saveRawInputImage = false;
	visualSonificationInfo.config.saveRemappedInputImage = false;

	visualSonificationInfo.config.saveRegionAudioFiles = false;
	visualSonificationInfo.config.saveRegionHistogramFiles = false;


	visualSonificationInfo.config.useOpenAlToPlay = false;
	visualSonificationInfo.config.placeAudioInBuffers = false;	
	visualSonificationInfo.config.placeHistogramInBuffers = true;

	visualSonificationInfo.config.pauseAfterEachStateLoop = true;
	visualSonificationInfo.config.pauseForSetupAtStart = false;


	visualSonificationInfo.config.inputSizeSetting = VISUAL_SONIFICATION_INPUT_VGA;

	//This sets the minimum features for audio signatures
	visualSonificationInfo.data.minimumFeaturesForAudioSignature = 10;


	visualSonificationInfo.data.audioGenerationConfigData.currentSonificationTechnique = VISUAL_SONIFICATION_DEFAULT_SONIFICATION_TECH;

	visualSonificationInfo.data.audioGenerationConfigData.maxFrequencyInHz = VISUAL_SONIFICATION_DEFAULT_MAX_FREQ;
	visualSonificationInfo.data.audioGenerationConfigData.minFrequencyInHz = VISUAL_SONIFICATION_DEFAULT_MIN_FREQ;
	visualSonificationInfo.data.audioGenerationConfigData.histogramOutputTotalTime = VISUAL_SONIFICATION_DEFAULT_REGION_OUTPUT_TIME;
	visualSonificationInfo.data.audioGenerationConfigData.sampleFrequency = VISUAL_SONIFICATION_DEFAULT_AUDIO_SAMPLE_FREQ;


	visualSonificationInfo.config.baseRegionAudioFilename = "Image_";
	visualSonificationInfo.config.baseRegionHistogramFilename = "Image_";
//	string testStringJay = "-leftCamera";

//	cout << "Parsing COmmand: " <<testStringJay << endl;
//	for(int k = 0; k < testStringJay.size(); k++)
//	{
//		cout << (int)testStringJay[k] << ",";
// 
//	}
//	cout << endl;

	//Process the command line arguments
	for(int i = 0; i < (int)commandArgs.size(); i++)
	{
//		cout << "Parsing Command: " <<commandArgs[i] << endl;
//		for(int k = 0; k < commandArgs[i].size(); k++)
//		{
//			cout << (int)commandArgs[i][k] << ",";
//
//		}
//		cout << endl;

		if(commandArgs[i].compare("-configFile")==0 || commandArgs[i].compare("-ConfigFile")==0)
		{
			visualSonificationInfo.config.configFilename = commandArgs[i+1];
#ifndef ANDROID
			cout << "Loading config file: " << visualSonificationInfo.config.configFilename << endl;
#endif
			loadConfigFile(commandArgs, visualSonificationInfo.config.configFilename);
		}

		if(commandArgs[i].compare("-calibrationFile")==0 || commandArgs[i].compare("-CalibrationFile")==0)
		{
			visualSonificationInfo.config.calibrationFilename = commandArgs[i+1];

		}

		if(commandArgs[i].compare("-dictionaryFile")==0 || commandArgs[i].compare("-DictionaryFile")==0)
		{
			visualSonificationInfo.config.dictionaryFilename = commandArgs[i+1];

		}
		



		if(commandArgs[i].compare("-leftImageList")==0 || commandArgs[i].compare("-LeftImageList")==0)
		{
			visualSonificationInfo.config.imageListFilenames[VISUAL_SONIFICATION_LEFT_CAMERA] = commandArgs[i+1];


		}

		if(commandArgs[i].compare("-rightImageList")==0 || commandArgs[i].compare("-RightImageList")==0)
		{

			visualSonificationInfo.config.imageListFilenames[VISUAL_SONIFICATION_RIGHT_CAMERA] = commandArgs[i+1];

		}




		if(commandArgs[i].compare("-leftImageListBaseDir")==0 || commandArgs[i].compare("-LeftImageListBaseDir")==0)
		{

			visualSonificationInfo.config.imageListBaseDir[VISUAL_SONIFICATION_LEFT_CAMERA] = commandArgs[i+1];

		}

		if(commandArgs[i].compare("-rightImageListBaseDir")==0 || commandArgs[i].compare("-RightImageListBaseDir")==0)
		{
			visualSonificationInfo.config.imageListBaseDir[VISUAL_SONIFICATION_RIGHT_CAMERA] = commandArgs[i+1];

		}



		if(commandArgs[i].compare("-videoFile")==0 || commandArgs[i].compare("-VideoFile")==0)
		{
			

		}

		//cout << "Camera Compare: " << commandArgs[i] << " and " << commandArgs[i].compare("-leftCamera") << endl;
		if(commandArgs[i].compare("-leftCamera")==0 || commandArgs[i].compare("-LeftCamera")==0)
		{
#ifndef ANDROID
			cout << "Left Camera Set" << endl;
#endif
			stringBuffer.str("");
			stringBuffer.clear();
			stringBuffer << commandArgs[i+1];
			if((stringBuffer >> visualSonificationInfo.config.leftCameraId).fail())
			{
#ifndef ANDROID
				cout << "Left Camera Id could not be parsed." << endl;
#endif
			}

		}
		if(commandArgs[i].compare("-rightCamera")==0 || commandArgs[i].compare("-RightCamera")==0)
		{
#ifndef ANDROID
			cout << "Right Camera Set" << endl;
#endif
			stringBuffer.str("");
			stringBuffer.clear();
			stringBuffer << commandArgs[i+1];
			if((stringBuffer >> visualSonificationInfo.config.rightCameraId).fail())
			{
#ifndef ANDROID
				cout << "Right Camera Id could not be parsed." << endl;
#endif
			}

		}
		if(commandArgs[i].compare("-depthCamera")==0 || commandArgs[i].compare("-DepthCamera")==0)
		{
			stringBuffer.str("");
			stringBuffer << commandArgs[i+1];
			if((stringBuffer >> visualSonificationInfo.config.depthCameraId).fail())
			{
#ifndef ANDROID
				cout << "Depth Camera Id could not be parsed." << endl;
#endif
			}

		}


		if(commandArgs[i].compare("-maxDepth")==0 || commandArgs[i].compare("-MaxDepth")==0)
		{
			stringBuffer.str("");
			stringBuffer.clear();

			stringBuffer << commandArgs[i+1];
			if((stringBuffer >> visualSonificationInfo.config.maxDepth).fail())
			{
#ifndef ANDROID
				cout << "Max depth could not be parsed." << endl;
#endif
			}

		}
		if(commandArgs[i].compare("-depthDiffThreshold")==0 || commandArgs[i].compare("-DepthDiffThreshold")==0)
		{
			stringBuffer.str("");
			stringBuffer.clear();

			stringBuffer << commandArgs[i+1];
			if((stringBuffer >> visualSonificationInfo.config.depthDeltaThreshold).fail())
			{
#ifndef ANDROID
				cout << "Max depth could not be parsed." << endl;
#endif
			}

		}

		if(commandArgs[i].compare("-numberNeighborsForDictionaryMatching")==0 || commandArgs[i].compare("-NumberNeighborsForDictionaryMatching")==0)
		{
			stringBuffer.str("");
			stringBuffer.clear();

			stringBuffer << commandArgs[i+1];
			if((stringBuffer >> visualSonificationInfo.config.numberOfTopNeighborsForHistogram).fail())
			{
#ifndef ANDROID
				cout << "Number of Top Neighbors for Histogram Generation could not be parsed" << endl;
#endif
			}

		}




		if(commandArgs[i].compare("-local")==0 || commandArgs[i].compare("-Local")==0)
		{
			if(commandArgs[i+1].compare("sift")==0)
			{
				visualSonificationInfo.data.featureLocalAlgo = FEATURE_LOCALIZATION_SIFT;

				//featureExtractionConfig.currentLocalizationAlgo = FEATURE_LOCALIZATION_SIFT;
			}
			if(commandArgs[i+1].compare("surf")==0)
			{
				visualSonificationInfo.data.featureLocalAlgo = FEATURE_LOCALIZATION_SURF;
				//featureExtractionConfig.currentLocalizationAlgo = FEATURE_LOCALIZATION_SURF;
				
			}
			if(commandArgs[i+1].compare("fast")==0)
			{
				visualSonificationInfo.data.featureLocalAlgo = FEATURE_LOCALIZATION_FAST;
				//featureExtractionConfig.currentLocalizationAlgo = FEATURE_LOCALIZATION_FAST;
			}
			if(commandArgs[i+1].compare("hog")==0)
			{
				visualSonificationInfo.data.featureLocalAlgo = FEATURE_LOCALIZATION_HOG;
				//featureExtractionConfig.currentLocalizationAlgo = FEATURE_LOCALIZATION_HoG;
			}
			if(commandArgs[i+1].compare("grid")==0)
			{
				visualSonificationInfo.data.featureLocalAlgo = FEATURE_LOCALIZATION_GRID;
				//featureExtractionConfig.currentLocalizationAlgo = FEATURE_LOCALIZATION_SLIDING_WINDOW;
			}

			if(commandArgs[i+1].compare("sift_vl")==0)
			{
				visualSonificationInfo.data.featureLocalAlgo = FEATURE_LOCALIZATION_SIFT_VL;

				//featureExtractionConfig.currentLocalizationAlgo = FEATURE_LOCALIZATION_SIFT;
			}

		}


		if(commandArgs[i].compare("-desc")==0 || commandArgs[i].compare("-Desc")==0)
		{
			if(commandArgs[i+1].compare("sift")==0)
			{

				visualSonificationInfo.data.featureDescAlgo = FEATURE_DESC_SIFT;

				//featureExtractionConfig.currentDescriptorAlgo = FEATURE_DESC_SIFT;

			}
			if(commandArgs[i+1].compare("surf")==0)
			{

				visualSonificationInfo.data.featureDescAlgo = FEATURE_DESC_SURF;				
				//featureExtractionConfig.currentDescriptorAlgo = FEATURE_DESC_SURF;
			}
			if(commandArgs[i+1].compare("brief")==0)
			{
				visualSonificationInfo.data.featureDescAlgo = FEATURE_DESC_BRIEF;
				
				//featureExtractionConfig.currentDescriptorAlgo = FEATURE_DESC_BRIEF;
			}
			if(commandArgs[i+1].compare("hog")==0)
			{
				visualSonificationInfo.data.featureDescAlgo = FEATURE_DESC_HOG;


				//featureExtractionConfig.currentDescriptorAlgo = FEATURE_DESC_HoG;
			}
			if(commandArgs[i+1].compare("sift_vl")==0)
			{

				visualSonificationInfo.data.featureDescAlgo = FEATURE_DESC_SIFT_VL;

				//featureExtractionConfig.currentDescriptorAlgo = FEATURE_DESC_SIFT;

			}



		}
		if(commandArgs[i].compare("-inputMode")==0 || commandArgs[i].compare("-InputMode")==0)
		{
			if(commandArgs[i+1].compare("stereo")==0)
			{
				visualSonificationInfo.data.inputMode = VISUAL_SONIFICATION_STEREO;
				//featureExtractionConfig.currentDescriptorAlgo = FEATURE_DESC_SIFT;

			}
			if(commandArgs[i+1].compare("depth")==0)
			{
				visualSonificationInfo.data.inputMode = VISUAL_SONIFICATION_MONO_AND_DEPTH;
				//featureExtractionConfig.currentDescriptorAlgo = FEATURE_DESC_SURF;
			}
			if(commandArgs[i+1].compare("segment")==0)
			{
				visualSonificationInfo.data.inputMode = VISUAL_SONIFICATION_MONO_AND_SEPARATE_SEGMENTATION;
			}

		}
		if(commandArgs[i].compare("-inputSize")==0 || commandArgs[i].compare("-InputSize")==0)
		{
			if(commandArgs[i+1].compare("vga")==0)
			{
				visualSonificationInfo.config.inputSizeSetting = VISUAL_SONIFICATION_INPUT_VGA;
				//featureExtractionConfig.currentDescriptorAlgo = FEATURE_DESC_SIFT;

			}
			if(commandArgs[i+1].compare("qvga")==0)
			{
				visualSonificationInfo.config.inputSizeSetting = VISUAL_SONIFICATION_INPUT_QVGA;
				//featureExtractionConfig.currentDescriptorAlgo = FEATURE_DESC_SURF;
			}

		}



		if(commandArgs[i].compare("-inputType")==0 || commandArgs[i].compare("-InputType")==0)
		{
			if(commandArgs[i+1].compare("stream")==0)
			{
				visualSonificationInfo.data.inputType = VISUAL_SONIFICATION_STREAM;
				

			}

			if(commandArgs[i+1].compare("streamExt")==0)
			{
				visualSonificationInfo.data.inputType = VISUAL_SONIFICATION_EXTERNAL_STREAM;
				

			}

			if(commandArgs[i+1].compare("videoFile")==0)
			{
				visualSonificationInfo.data.inputType = VISUAL_SONIFICATION_VIDEO_FILE;
				
			}
			if(commandArgs[i+1].compare("imageList")==0)
			{
				visualSonificationInfo.data.inputType = VISUAL_SONIFICATION_IMAGE_LIST;
				
			}


		}
		if(commandArgs[i].compare("-pauseMode")==0)
		{
			visualSonificationInfo.config.pauseAfterEachStateLoop = true;
				
		}
		if(commandArgs[i].compare("-no-pauseMode")==0)
		{
			visualSonificationInfo.config.pauseAfterEachStateLoop = false;
				
		}
		if(commandArgs[i].compare("-pauseAtStart")==0)
		{

			visualSonificationInfo.config.pauseForSetupAtStart = true;
		}
		if(commandArgs[i].compare("-no-pauseAtStart")==0)
		{

			visualSonificationInfo.config.pauseForSetupAtStart = false;
		}
		if(commandArgs[i].compare("-regionAudioFileOutputDir")==0 ||commandArgs[i].compare("-RegionAudioFileOutputDir")==0)
		{

			visualSonificationInfo.config.regionAudioFilesOutputDir = commandArgs[i+1];
		}
		if(commandArgs[i].compare("-regionHistogramFileOutputDir")==0 ||commandArgs[i].compare("-RegionHistogramFileOutputDir")==0)
		{

			visualSonificationInfo.config.regionHistogramFilesOutputDir = commandArgs[i+1];
		}




		if(commandArgs[i].compare("-saveRegionAudioFiles")==0 || commandArgs[i].compare("-SaveRegionAudioFiles")==0)
		{

			visualSonificationInfo.config.saveRegionAudioFiles = true;
		}
		if(commandArgs[i].compare("-no-saveRegionAudioFiles")==0 || commandArgs[i].compare("-no-SaveRegionAudioFiles")==0)
		{

			visualSonificationInfo.config.saveRegionAudioFiles = false;
		}

		if(commandArgs[i].compare("-saveRegionHistogramFiles")==0 || commandArgs[i].compare("-SaveRegionAudioFiles")==0)
		{

			visualSonificationInfo.config.saveRegionHistogramFiles = true;
		}
		if(commandArgs[i].compare("-no-saveRegionHistogramFiles")==0 || commandArgs[i].compare("-no-SaveRegionAudioFiles")==0)
		{

			visualSonificationInfo.config.saveRegionHistogramFiles = false;
		}




		if(commandArgs[i].compare("-useOpenAlToPlay")==0 ||commandArgs[i].compare("-UseOpenAlToPlay")==0)
		{

			visualSonificationInfo.config.useOpenAlToPlay = true;
		}

		if(commandArgs[i].compare("-no-useOpenAlToPlay")==0 ||commandArgs[i].compare("-no-UseOpenAlToPlay")==0)
		{

			visualSonificationInfo.config.useOpenAlToPlay = false;
		}

		if(commandArgs[i].compare("-placeAudioInBuffers")==0 ||commandArgs[i].compare("-PlaceAudioInBuffers")==0)
		{

			visualSonificationInfo.config.placeAudioInBuffers = true;
		}
		if(commandArgs[i].compare("-no-placeAudioInBuffers")==0 ||commandArgs[i].compare("-no-PlaceAudioInBuffers")==0)
		{

			visualSonificationInfo.config.placeAudioInBuffers = false;
		}


		if(commandArgs[i].compare("-placeHistogramInBuffers")==0 ||commandArgs[i].compare("-PlaceHistogramInBuffers")==0)
		{

			visualSonificationInfo.config.placeHistogramInBuffers = true;
		}

		if(commandArgs[i].compare("-no-placeHistogramInBuffers")==0 ||commandArgs[i].compare("-no-PlaceHistogramInBuffers")==0)
		{

			visualSonificationInfo.config.placeHistogramInBuffers = false;
		}
		if(commandArgs[i].compare("-showComponentRegionsAtAudioOutput")==0 ||commandArgs[i].compare("-ShowComponentRegionsAtAudioOutput")==0)
		{

			//Flag for using segmented Image
			visualSonificationInfo.config.showComponentRegionsAtAudioOutput=true;

		}


		if(commandArgs[i].compare("-no-showComponentRegionsAtAudioOutput")==0 ||commandArgs[i].compare("-no-ShowComponentRegionsAtAudioOutput")==0)
		{

			//Flag for using segmented Image
			visualSonificationInfo.config.showComponentRegionsAtAudioOutput=false;

		}


		if(commandArgs[i].compare("-segmentUsingExternalMask")==0 ||commandArgs[i].compare("-SegmentUsingExternalMask")==0)
		{

			//Flag for using segmented Image
			visualSonificationInfo.config.segmentUsingExternalMask=true;

		}


		if(commandArgs[i].compare("-no-segmentUsingExternalMask")==0 ||commandArgs[i].compare("-no-SegmentUsingExternalMask")==0)
		{

			//Flag for using segmented Image
			visualSonificationInfo.config.segmentUsingExternalMask=false;

		}

		if(commandArgs[i].compare("-sonificationTechnique")==0 ||commandArgs[i].compare("-SonificationTechnique")==0)
		{

			if(commandArgs[i+1].compare("pure_sine")==0)
			{
				visualSonificationInfo.data.audioGenerationConfigData.currentSonificationTechnique = HistogramSoundSource::SOUND_GENERATION_PURE_SINEWAVE;


			}

			if(commandArgs[i+1].compare("pure_sine_linear_transition")==0)
			{
				visualSonificationInfo.data.audioGenerationConfigData.currentSonificationTechnique = HistogramSoundSource::SOUND_GENERATION_PURE_SINEWAVE_WITH_TRANSITION;


			}

			if(commandArgs[i+1].compare("fundamental_with_2_harmonics")==0)
			{
				visualSonificationInfo.data.audioGenerationConfigData.currentSonificationTechnique = HistogramSoundSource::SOUND_GENERATION_HARMONICS;

			}
			if(commandArgs[i+1].compare("convolution")==0)
			{
				visualSonificationInfo.data.audioGenerationConfigData.currentSonificationTechnique = HistogramSoundSource::SOUND_GENERATION_CONVOLUTION;

			}
			if(commandArgs[i+1].compare("harmonics_piano")==0)
			{
				visualSonificationInfo.data.audioGenerationConfigData.currentSonificationTechnique = HistogramSoundSource::SOUND_GENERATION_HARMONICS_PIANO;

			}

		}

		if(commandArgs[i].compare("-maxHistogramFreq")==0 || commandArgs[i].compare("-MaxHistogramFreq")==0)
		{
			stringBuffer.str("");
			stringBuffer.clear();

			stringBuffer << commandArgs[i+1];
			if((stringBuffer >> visualSonificationInfo.data.audioGenerationConfigData.maxFrequencyInHz).fail())
			{
#ifndef ANDROID
				cout << "Max Histogram Freq could not be parsed." << endl;
#endif
			}

		}
		if(commandArgs[i].compare("-minHistogramFreq")==0 || commandArgs[i].compare("-MinHistogramFreq")==0)
		{
			stringBuffer.str("");
			stringBuffer.clear();

			stringBuffer << commandArgs[i+1];
			if((stringBuffer >> visualSonificationInfo.data.audioGenerationConfigData.minFrequencyInHz).fail())
			{
#ifndef ANDROID
				cout << "Min Histogram Freq could not be parsed." << endl;
#endif
			}

		}




		if(commandArgs[i].compare("-regionOutputTime")==0 || commandArgs[i].compare("-RegionOutputTime")==0)
		{
			stringBuffer.str("");
			stringBuffer.clear();

			stringBuffer << commandArgs[i+1];
			if((stringBuffer >> visualSonificationInfo.data.audioGenerationConfigData.histogramOutputTotalTime).fail())
			{
#ifndef ANDROID
				cout << "Region Output time could not be parsed." << endl;
#endif
			}

		}

		if(commandArgs[i].compare("-audioSampleFreq")==0 || commandArgs[i].compare("-AudioSampleFreq")==0)
		{
			stringBuffer.str("");
			stringBuffer.clear();

			stringBuffer << commandArgs[i+1];
			if((stringBuffer >> visualSonificationInfo.data.audioGenerationConfigData.sampleFrequency).fail())
			{
#ifndef ANDROID
				cout << "Audio Sample Freq could not be parsed." << endl;
#endif
			}

		}




	}









}

//Desc: This function converts the typical main parameters to a vector of strings to make manipulation easier
//Return: The vector of strings containing the original program arguments

vector<string> getCommandParams(int argc, char * argv[])
{
	vector<string> commandParams;
	for(int i = 0; i<argc; i++)
	{
		string newArg = argv[i];
		commandParams.push_back(newArg);

	}

	return commandParams;
}


//Desc: This function takes the program parameters and configures the visualSonification info struct for the application
//Return: None

void handleParams(int argc, char * argv[],VisualSonificationInfo & visualSonificationInfo)
{
	//Get the vector of configuration params
	vector<string> commandArgs = getCommandParams(argc, argv);

	//Parse the params to configure the system
	parseParamsVector(commandArgs,visualSonificationInfo);

}

//Desc: This function sets up the feature localization based on the config in info
//Return: A pointer to the new Localization class if OpenCV localization else it is null
//If it is not openCV based, the info is populated with the information for that operation
Ptr<FeatureDetector> setupLocalization(VisualSonificationInfo & info)
{
    FeatureDetector* featureDetector = NULL;

	switch(info.data.featureLocalAlgo)
	{
		case FEATURE_LOCALIZATION_SIFT:
			{
				//SIFT::DetectorParams detectorParams;
				//SIFT::CommonParams commonParams;
				//featureDetector = new SiftFeatureDetector(detectorParams,commonParams);
				//SIFT::DescriptorParams descriptorParams;
				featureDetector = new SiftFeatureDetector( 0, 3,0.04,10,1.6);
			
				break;
			}
		case FEATURE_LOCALIZATION_SIFT_VL:
			{
//				SIFT::DetectorParams detectorParams;
//				SIFT::CommonParams commonParams;


				const int defaultNumOctaves = 4;
				const int defaultNumOctaveLayers = 3;
				const int defaultFirstOctave = -1;
				const double sigman = .5 ;
				const double sigma0 = 1.6 * powf(2.0f, 1.0f / 3.0f) ;


				info.data.siftConfigData.sigman = (float)sigman;
				info.data.siftConfigData.sigma0 = (float)sigma0; 
				info.data.siftConfigData.O=  defaultNumOctaves;
				info.data.siftConfigData.S =  defaultNumOctaveLayers;
				info.data.siftConfigData.omin = defaultFirstOctave;
				info.data.siftConfigData.smin = -1;
				info.data.siftConfigData.smax = defaultNumOctaveLayers+1;


				info.data.siftConfigData.edgeThreshold = 10.0;
				info.data.siftConfigData.threshold =0.04 / info.data.siftConfigData.S / 2.0;
				info.data.siftConfigData.angleMode = 0;

				info.data.siftConfigData.magnfication = 3.0;

				info.data.siftConfigData.unnormalized = false;


				Ptr<VL::Sift> SiftPtr = new VL::Sift((VL::float_t)info.data.siftConfigData.sigman, (VL::float_t)info.data.siftConfigData.sigma0, info.data.siftConfigData.O, info.data.siftConfigData.S,
					info.data.siftConfigData.omin, info.data.siftConfigData.smin, info.data.siftConfigData.smax); 

				info.data.siftPtr = SiftPtr;




				break;
			}

		case FEATURE_LOCALIZATION_SURF:
			{
				//SURF::CvSURFParams detectorParams;
				//detectorParams.hessianThreshold = 400.0;
				//featureDetector = new SurfFeatureDetector (detectorParams.hessianThreshold);
				featureDetector = new SurfFeatureDetector(400.0);
			
				break;
			}
		case FEATURE_LOCALIZATION_FAST:
			{

				int fastThreshold = 10;
				featureDetector = new FastFeatureDetector(fastThreshold,true);
				


			
				break;
			}
		case FEATURE_LOCALIZATION_HOG:
			{

				info.data.localizationHogPtr = new HOGDescriptor();
			
				break;
			}
		case FEATURE_LOCALIZATION_GRID:
			{
				info.data.gridSize.height = info.config.gridSizeY;
				info.data.gridSize.width = info.config.gridSizeX;			
				break;
			}

		default:

			break;
	}

	return featureDetector;

}



//Desc: This function Setups the descriptr extractor based on info
//Return: The pointer to the new descirptor extractor if it is openCV based, null otherwise
//If it is not openCV based, the info is populated with the information for that operation
Ptr<DescriptorExtractor> setupDescriptor(VisualSonificationInfo & info)
{
   Ptr<DescriptorExtractor> descriptorExtractor = NULL;

   switch(info.data.featureDescAlgo)
	{
		case FEATURE_DESC_SIFT:
			{
				//SIFT::DescriptorParams descriptorParams;
				//SIFT::CommonParams commonParams;
				//descriptorExtractor = new SiftDescriptorExtractor(descriptorParams,commonParams);
				descriptorExtractor = new SiftDescriptorExtractor( 0, 3,0.04,10,1.6);
			}
			break;

		case FEATURE_DESC_SURF:
			{
				//SURF::CvSURFParams detectorParams;
								
				//detectorParams.hessianThreshold = 400.0;
				//detectorParams.nOctaves = 4;
				//detectorParams.nOctaveLayers = 2;
				//descriptorExtractor = new SurfDescriptorExtractor( detectorParams.nOctaves,detectorParams.nOctaveLayers);
				descriptorExtractor = new SurfDescriptorExtractor(400.0);


			}
			break;
		case FEATURE_DESC_HOG:
			{


				info.data.descHogPtr = new HOGDescriptor();

			}
			break;
		case FEATURE_DESC_BRIEF:
			{
				int briefDescriptorSize = 32;
				descriptorExtractor = new BriefDescriptorExtractor(briefDescriptorSize);
			}
			break;

		case FEATURE_DESC_SIFT_VL:
			{
				if(info.data.siftPtr.empty())
				{	
					const int defaultNumOctaves = 4;
					const int defaultNumOctaveLayers = 3;
					const int defaultFirstOctave = -1;
					const double sigman = .5 ;
					const double sigma0 = 1.6 * powf(2.0f, 1.0f / 3.0f) ;


					info.data.siftConfigData.sigman = (float)sigman;
					info.data.siftConfigData.sigma0 = (float)sigma0; 
					info.data.siftConfigData.O=  defaultNumOctaves;
					info.data.siftConfigData.S =  defaultNumOctaveLayers;
					info.data.siftConfigData.omin = defaultFirstOctave;
					info.data.siftConfigData.smin = -1;
					info.data.siftConfigData.smax = defaultNumOctaveLayers+1;


					info.data.siftConfigData.edgeThreshold = 10.0;
					info.data.siftConfigData.threshold =0.04 / info.data.siftConfigData.S / 2.0;
					info.data.siftConfigData.angleMode = 0;

					info.data.siftConfigData.magnfication = 3.0;

					info.data.siftConfigData.unnormalized = false;


					Ptr<VL::Sift> SiftPtr = new VL::Sift((VL::float_t)info.data.siftConfigData.sigman, (VL::float_t)info.data.siftConfigData.sigma0, info.data.siftConfigData.O, info.data.siftConfigData.S,
						info.data.siftConfigData.omin, info.data.siftConfigData.smin, info.data.siftConfigData.smax); 
					info.data.siftPtr = SiftPtr;

				}
					break;
			}
		default:

			break;
	}

	return descriptorExtractor;



}

//Desc: This function initializes the feature extraction for the application
//Return: None
void visualSonificationInitFeatureExtraction(VisualSonificationInfo & info)
{
	info.data.featureDetector = setupLocalization(info);
	info.data.descriptorExtractor = setupDescriptor(info);



}


//Desc: This function performs keypoint localization, it takes the visual sonification info struct, an input image and a keypoint vector
//Return: None

void localizeFeaturePoints(VisualSonificationInfo & info, Mat & inputImage,  vector<KeyPoint> & keyPoints )
{
	//A buffer that is used in case the image needs to be converted
	static Mat floatBuffer(inputImage.rows,inputImage.cols,CV_32FC1);

	//Scale factor to prep for sift
	const float siftScaleFactor = 1.0f/255.0f;

	//deg to rad conversion factor
	const double a_180divPI = 180.0/CV_PI;

	//Clear out the keypoints
	keyPoints.clear();

	//Call the correct feature localization routine
	switch(info.data.featureLocalAlgo)
	{
		case FEATURE_LOCALIZATION_SIFT:
		case FEATURE_LOCALIZATION_SURF:
		case FEATURE_LOCALIZATION_FAST:
		{
			//Standard OpenCV keypoint detect
			info.data.featureDetector->detect(inputImage,keyPoints);
			break;
		}
		case FEATURE_LOCALIZATION_HOG:
		{

			//Get bounding boxes for Hog
			vector<Rect> bboxes;
			
			//Run HoG multiscale
			info.data.localizationHogPtr->detectMultiScale(inputImage, bboxes, 0, Size(8,8), Size(32,32), 1.05, 2);

			//Convert the boxes to keypoints
			for(unsigned int i = 0; i< bboxes.size(); i++)
			{
				KeyPoint currentPoint;
				currentPoint.pt.x = (float)bboxes[i].x;
				currentPoint.pt.y = (float)bboxes[i].y;
				currentPoint.size = (float)bboxes[i].height/(float)info.data.localizationHogPtr->winSize.height;
				keyPoints.push_back(currentPoint);
			}
			break;

		}
		case FEATURE_LOCALIZATION_GRID:
			{

				//convert a grid to keypoints
				for(int y = 0; y < inputImage.rows; y+= info.data.gridSize.height)
				{
					
					for(int x = 0; x < inputImage.cols; x+= info.data.gridSize.width)
					{
						KeyPoint newLocation((float)x,(float)y,1);
						keyPoints.push_back(newLocation);
					}

				
				}


				break;
			}
		case FEATURE_LOCALIZATION_SIFT_VL:
		{
			//For Vedaldi sift we need an array for angles
			VL::float_t angles[4];

			//Convert the image to a float
			vuConvertUnsignedCharToFloatMatrixAndScale(&info.data.currentGrayCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA], &floatBuffer, siftScaleFactor);

			//Build the scale space
			info.data.siftPtr->process((VL::pixel_t *)floatBuffer.data,floatBuffer.cols,floatBuffer.rows);

			//Detect the keypoints in the scale space
			info.data.siftPtr->detectKeypoints((VL::float_t)info.data.siftConfigData.threshold, (VL::float_t)info.data.siftConfigData.edgeThreshold);

			//Compute how many keypoints we have
			int numberOfKeypoints = std::abs(int(info.data.siftPtr->keypointsBegin()-info.data.siftPtr->keypointsEnd()));

			//Get ready to convert the openCV keypoints
			KeyPoint currentKeypoint;
			keyPoints.reserve(numberOfKeypoints);

			//Need to set the normilzation and magnification before we get keypoint angles
			info.data.siftPtr->setNormalizeDescriptor( ! info.data.siftConfigData.unnormalized ) ;
			info.data.siftPtr->setMagnification( info.data.siftConfigData.magnfication ) ;

			//If we need to reallocate, to decrease future time, allocate a lot
			if(info.data.anglesInRads.capacity() <= (unsigned int)numberOfKeypoints)
			{
				info.data.anglesInRads.reserve(numberOfKeypoints*2);
			}

			//Adjust the size of the keypoints
			info.data.anglesInRads.resize(numberOfKeypoints);

			int currentAngle = 0;
			info.data.numberOfKeypoints = 0;
			getStartClockTime(TIMING_EVENT_GENERAL_2);
			//Got through the Vedaldi Sift keypoints and create OpenCV keypoints
			for( VL::Sift::KeypointsConstIter iter = info.data.siftPtr->keypointsBegin(); iter != info.data.siftPtr->keypointsEnd(); ++iter )
			{
				//info.data.anglesInRads[currentAngle] = 0.0f;

				angles[0] = 0.0f;

				//Get the keypoint orienation
				int angleCount = info.data.siftPtr->computeKeypointOrientations(angles, *iter);

				info.data.anglesInRads[currentAngle] = -1.0;
				if( angleCount > 0 )
				{
				

					info.data.anglesInRads[currentAngle] = angles[0];
					float size = iter->sigma*3.0f*4;// 4==NBP
					currentKeypoint.pt.x = iter->x;
					currentKeypoint.pt.y = iter->y;
					currentKeypoint.angle = info.data.anglesInRads[currentAngle]* (float)a_180divPI;
					currentKeypoint.size = size;
					currentKeypoint.response = 0;
					currentKeypoint.octave = iter->o;
					currentKeypoint.class_id = -1;

					keyPoints.push_back( currentKeypoint );
					info.data.numberOfKeypoints++;

				}

				currentAngle++;
			}
			//info.data.anglesInRads.resize(currentAngle);
			getEndClockTime(TIMING_EVENT_GENERAL_2);

			break;
		}
		default:

			break;



	}

		
	

}

//Desc: This function builds the descriptors for a set of keypoints,  The parameters are the info, the image from which the descroptors will be pulled,
//the keypoints and a Mat for the descriptors.  The descriptor mat is floating point
//Return: None

void buildFeatureDescriptors(VisualSonificationInfo & info, Mat & inputImage, vector<KeyPoint> & keyPoints, Mat & descriptors)
{
	//Buffer
	static Mat floatBuffer(inputImage.rows,inputImage.cols,CV_32FC1);

	//SIFT scale factor
	const float siftScaleFactor = 1.0f/255.0f;

	//Radian to deg constant
	//const double a_180divPI = 180.0/CV_PI;

	//Perform the correct descriptor building
	switch(info.data.featureDescAlgo)
	{
		case FEATURE_DESC_SIFT:
		{
			//Typical OpenCV descriptor Extraction
			info.data.descriptorExtractor->compute( inputImage, keyPoints, descriptors );			
			descriptors = descriptors * (1/512.0);
			break;
		}


		case FEATURE_DESC_SURF:
		{
			//Typical OpenCV descriptor Extraction
			info.data.descriptorExtractor->compute( inputImage, keyPoints, descriptors );			
			break;
		}
		case FEATURE_DESC_SIFT_VL:
		{

			//If we used Vedaldi for Localization we can save time
			if(info.data.featureLocalAlgo ==FEATURE_LOCALIZATION_SIFT_VL)
			{

				//Setup the descriptors based on the keypoints
				int numberOfDescriptors = info.data.numberOfKeypoints;
				descriptors.create(numberOfDescriptors,128,CV_32FC1);

				int currentKeypointNumber = 0;

				//For looping through descriptors
				int descRowStepInBytes = descriptors.step[0];
				int descRelativeLocationInBytes = 0;

				//For each keypoint
				for( VL::Sift::KeypointsConstIter iter = info.data.siftPtr->keypointsBegin(); iter != info.data.siftPtr->keypointsEnd(); ++iter,currentKeypointNumber++ )
				{
					if(info.data.anglesInRads[currentKeypointNumber] >=0)
					{
						//Computer the descriptor and store it int he descriptor Mat
						info.data.siftPtr->computeKeypointDescriptor((VL::float_t*)(descriptors.data + descRelativeLocationInBytes), *iter, info.data.anglesInRads[currentKeypointNumber]);
				
						descRelativeLocationInBytes += descRowStepInBytes;



					}



				}
                

			}
			else
			{
				//We did not use same localization as descirptor and need to setup some things
				const double a_PIdiv180 = CV_PI/180.;

				int numberOfDescriptors = info.data.numberOfKeypoints;

				//Create scale space after scaling image
				vuConvertUnsignedCharToFloatMatrixAndScale(&inputImage, &floatBuffer, siftScaleFactor);
				info.data.siftPtr->process((VL::pixel_t *)floatBuffer.data,floatBuffer.cols,floatBuffer.rows);

				//Set the normilzation and magnification
				info.data.siftPtr->setNormalizeDescriptor( ! info.data.siftConfigData.unnormalized ) ;
				info.data.siftPtr->setMagnification( info.data.siftConfigData.magnfication ) ;


				//Create the descriptor holders
				descriptors.create(numberOfDescriptors,128,CV_32FC1);
				int descRowStepInBytes = descriptors.step[0];
				int descRelativeLocationInBytes = 0;


				//Loop through the keypoints
				for(unsigned int i = 0; i < keyPoints.size(); i++)
				{
					float currentAngle;

					//Calculate the current scale
					float sigma = keyPoints[i].size/(info.data.siftConfigData.magnfication  *4);// 4==NBP

					//Create Vedaldi Keypoints
					VL::Sift::Keypoint currentVlKeyPoint = info.data.siftPtr->getKeypoint(keyPoints[i].pt.x,keyPoints[i].pt.y,sigma);

					//Set the keypoint angle
					currentAngle = (float)(keyPoints[i].angle * a_PIdiv180);

					//Compute the descriptor
					info.data.siftPtr->computeKeypointDescriptor((VL::float_t*)(descriptors.data + descRelativeLocationInBytes), currentVlKeyPoint, currentAngle);
					//int angleCount = info.data.siftPtr->computeKeypointOrientations(angles, currentVlKeyPoint);


					//Step to next one
					descRelativeLocationInBytes += descRowStepInBytes;
				}


			}





			break;
		}
		case FEATURE_DESC_HOG:
		{
			vector<float> descriptorsVec;
			//If we are not using HoG localization, they convert the keypoints to points and
			// run descriptor building
			if(keyPoints.size() != 0 && info.data.featureLocalAlgo != FEATURE_LOCALIZATION_HOG)
			{
				
				vector<Point> points;
				for(unsigned int i = 0 ; i< keyPoints.size(); i++)
				{
					Point currentPoint;
					currentPoint.x =(int) keyPoints[i].pt.x;
					currentPoint.y = (int)keyPoints[i].pt.y;
					points.push_back(currentPoint);
				}

				info.data.descHogPtr->compute( inputImage,descriptorsVec,Size(),Size(),points);
			}
			else
			{

				//Run the HoG on the while image
				info.data.descHogPtr->compute( inputImage,descriptorsVec,Size(8,8),Size(32,32));
			}
			unsigned int numberOfFeatures =descriptorsVec.size()/info.data.descHogPtr->getDescriptorSize();


			//Take the single long vector and create a Mat with one descriptor per row
			descriptors.create(numberOfFeatures,info.data.descHogPtr->getDescriptorSize(),CV_32FC1);
			for(unsigned int row = 0; row <numberOfFeatures; row++)
			{
				for(unsigned int col = 0; col< info.data.descHogPtr->getDescriptorSize(); col++)
				{
					descriptors.at<float>(row,col) = descriptorsVec[col + row*info.data.descHogPtr->getDescriptorSize()];

				}
			}
			
			
			break;
		}
		case FEATURE_DESC_BRIEF:
		{

			//Run brief but be warned the output is not float
			info.data.descriptorExtractor->compute( inputImage, keyPoints, descriptors );			
			
			break;
		}
		default:
		{
#ifndef ANDROID
			cout << "WARNING: No descriptors made." << endl;
#endif
			break;
		}
	}





}


//Desc: This function checks if a feature point is located in a valid region or component.  the feature point and label mat is passed in
//and the number of pixels in each region is passed in as a vector
//Return: true if the point is in a valid region, false otherwise

bool featureInValidRegion(KeyPoint featurePoint, Mat &labels, vector<int> componentValidMetrics, int threshold)
{
	//Assume it is invalde
	bool returnVal = false;
	
	//Get the label
	short int currentLabel = ((signed short int *)(labels.data + labels.step[0] * (int)featurePoint.pt.y))[(int)featurePoint.pt.x];//labels.at<signed short int>(featurePoint.pt);//



	//If the label is not negative
	if(currentLabel >=0)
	{
		//And the region size is big enough
		if(componentValidMetrics[currentLabel] >= threshold)
		{

			//return in valid region
			returnVal = true;
		}


	}


	return returnVal;
}



//Desc: This function sets up the matcher for the bag of words.  It takes the info and training descriptors or dictionary
//
//Return: a pointer to the matcher
//If the matcher is openCV based, it returns the matcher, else it return NULL and sets the matcher up in info
Ptr<DescriptorMatcher> setupMatcher(VisualSonificationInfo & info, 	vector<Mat> & trainedDescriptors)
{

	Ptr<DescriptorMatcher> matcher;
	switch(info.data.featureMatchAlgo)
	{
	
		case FEATURE_MATCHER_FLANN:
		{

			//Setup the openCV matcher
			//Yeah a memory leak here right now
			Ptr<flann::IndexParams> indexParams = new flann::KDTreeIndexParams(1);
			Ptr<flann::SearchParams> searchParams = new flann::SearchParams(4);//200
			matcher = new FlannBasedMatcher(indexParams, searchParams);
			
			matcher->add(trainedDescriptors);
			matcher->train();

		}
			break;

		case FEATURE_MATCHER_BRUTEFORCE_C:
		{
		

		}
			break;


		case FEATURE_MATCHER_BRUTEFORCE:
		default:
		{

		//matcher = new BruteForceMatcher<L2<float> >;
		matcher = new BFMatcher(NORM_L2);
		matcher->add(trainedDescriptors);
		matcher->train();

		}


			break;

	}


	return matcher;
}


//Desc: This function performs the feature matching.  It takes a descriptorMAtcher Pointer, the query descriptors (float), the keypoints just in case 
// and the dictionary or trained vectors.
//Return: a vector of matches

vector<DMatch> performFeatureMatching(Ptr<DescriptorMatcher> descriptorMatcher,int numberOfNeighborsToFind ,Mat & descriptors, vector<KeyPoint> & keyPoints, Mat & trainedVectors)
{
	//Static data buffers for computation so that
	//they are not constantly reallocated
	static vector<DMatch> featureMatches;
	static vector<vector<DMatch> > topNMatches;
	static vector<FeatureMatch> vuFeatureMatches;
	//int numberOfNeighborsToFind = trainedVectors.rows;//10;


	//float nearestNeighborDistanceRatio = .40f;//.49f; //.8

	if(numberOfNeighborsToFind <= 0 || numberOfNeighborsToFind >trainedVectors.rows)
	{
		numberOfNeighborsToFind = trainedVectors.rows;
	}
#ifdef VERBOSE_OUTPUT
	cout << "Number Of Neighbors To Find: " << numberOfNeighborsToFind << endl;
#endif
	//Compute how much memory to reserve
	int totalNumberOfNeighborMatches = numberOfNeighborsToFind * descriptors.rows;	




	//Reserve more than enough memory if we have to
	if(vuFeatureMatches.capacity() <(unsigned int )totalNumberOfNeighborMatches)
	{
		vuFeatureMatches.reserve(totalNumberOfNeighborMatches*2);

	}

	//Resize the matches data
	vuFeatureMatches.resize(totalNumberOfNeighborMatches);
	

	//descriptorMatcher->knnMatch(descriptors,topNMatches,numberOfNeighborsToFind);

	//vuTopNMatchesFloat(&trainedVectors, &trainedVectors,numberOfNeighborsToFind, &vuFeatureMatches[0]);
	//converFeatureMatchToDMatch(&vuFeatureMatches[0], descriptors.rows, numberOfNeighborsToFind, topNMatches);



	//Compute the matches and convert to classical OpenCV match structure
	vuTopNMatchesFloat(&descriptors, &trainedVectors,numberOfNeighborsToFind, &vuFeatureMatches[0]);
	converFeatureMatchToDMatch(&vuFeatureMatches[0], descriptors.rows, numberOfNeighborsToFind, topNMatches);



	//Clear the feature matches
	featureMatches.clear();

	//Make sure we allocate rarely
	if(featureMatches.capacity() <(unsigned int )totalNumberOfNeighborMatches)
	{
		featureMatches.reserve(totalNumberOfNeighborMatches*2);

	}

	//Take the top matches and place them together in single vector
	for(unsigned int i = 0; i< topNMatches.size(); i++)
	{
		//if(topNMatches[i].size() >=1)
				
		for(int j = 0; j< (int)topNMatches[i].size(); j++)
		{
			featureMatches.push_back(topNMatches[i][j]);
		}
	}


	//Return the top matches
	return featureMatches;

}

//Desc: This function creates the histogram based on the feature matches within a component or region
//
//Return: None
void visualSonificationCreateHistograms(VisualSonificationInfo &info)
{

	//Resize the featureCount per region vector based on number of regions
	info.data.featureCountPerRegion.resize(info.data.regionsByPoints[VISUAL_SONIFICATION_LEFT_CAMERA].size());

	//Clean out the feature count per region
	for(int i = 0; i < (int)info.data.featureCountPerRegion.size(); i++)
	{

		info.data.featureCountPerRegion[i] = 0;

	}



	//Allocate the component sizes
	vector<int> componentSizes(info.data.regionsByPoints[VISUAL_SONIFICATION_LEFT_CAMERA].size());



	//Get the component sizes
	for(int i = 0; i < (int)info.data.regionsByPoints[VISUAL_SONIFICATION_LEFT_CAMERA].size(); i++)
	{
		componentSizes[i] = info.data.regionsByPoints[VISUAL_SONIFICATION_LEFT_CAMERA][i].size();
	}



	//Resuze the histogram vector based on how many regions we have here
	info.data.regionHistograms.resize(info.data.regionsByPoints[VISUAL_SONIFICATION_LEFT_CAMERA].size());

	//Clean the histograms
	for(int i = 0; i < (int)info.data.regionHistograms.size(); i++)
	{
		info.data.regionHistograms[i].resize(info.data.bagOfWordsDictionary.rows);
		for(int j = 0; j < info.data.bagOfWordsDictionary.rows; j++)
		{
			info.data.regionHistograms[i][j] = 0;	
		
		}
	}


	//Compute the matches with the dictionary
	info.data.featureMatches[VISUAL_SONIFICATION_LEFT_CAMERA]=  performFeatureMatching(info.data.featureMatcher, info.config.numberOfTopNeighborsForHistogram,info.data.currentDescriptors[VISUAL_SONIFICATION_LEFT_CAMERA], info.data.currentKeypoints[VISUAL_SONIFICATION_LEFT_CAMERA], info.data.bagOfWordsDictionary);

	/*
	for(int i = 0; i < (int)info.data.currentKeypoints[VISUAL_SONIFICATION_LEFT_CAMERA].size(); i++)
	{
		short int currentLabel = info.data.regionLabelsImages[VISUAL_SONIFICATION_LEFT_CAMERA].at<signed short int>(   info.data.currentKeypoints[VISUAL_SONIFICATION_LEFT_CAMERA][i].pt);//featurePoint.pt);
		if(featureInValidRegion(info.data.currentKeypoints[VISUAL_SONIFICATION_LEFT_CAMERA][i], info.data.regionLabelsImages[VISUAL_SONIFICATION_LEFT_CAMERA], componentSizes, info.config.minimumRegionPixelArea))
		{
			float currentFeatureWeight = info.data.
		}

	}
	*/


	int currentFeatureIndex = 0;
	short int currentLabel = 0;
	int currentFeatureHistogramBin = 0;
	float currentFeatureWeight = 0.0f;
	//For each match that we have
	for(int i = 0; i < (int)info.data.featureMatches[VISUAL_SONIFICATION_LEFT_CAMERA].size(); i++)
	{

		//Get the the current features index
		currentFeatureIndex = info.data.featureMatches[VISUAL_SONIFICATION_LEFT_CAMERA][i].queryIdx;

		//cout << "Feature Match: "<< i << " Train Index:" <<info.data.featureMatches[VISUAL_SONIFICATION_LEFT_CAMERA][i].trainIdx << " Query Index:" <<info.data.featureMatches[VISUAL_SONIFICATION_LEFT_CAMERA][i].queryIdx << endl;

		//Get the current points label
		currentLabel = ((short int*)(info.data.regionLabelsImages[VISUAL_SONIFICATION_LEFT_CAMERA].data + info.data.regionLabelsImages[VISUAL_SONIFICATION_LEFT_CAMERA].step[0]* (int)info.data.currentKeypoints[VISUAL_SONIFICATION_LEFT_CAMERA][currentFeatureIndex].pt.y))[(int)info.data.currentKeypoints[VISUAL_SONIFICATION_LEFT_CAMERA][currentFeatureIndex].pt.x];//at<signed short int>(   info.data.currentKeypoints[VISUAL_SONIFICATION_LEFT_CAMERA][currentFeatureIndex].pt);//featurePoint.pt);


		//Check if the point is in a valid region
		if(featureInValidRegion(info.data.currentKeypoints[VISUAL_SONIFICATION_LEFT_CAMERA][currentFeatureIndex], info.data.regionLabelsImages[VISUAL_SONIFICATION_LEFT_CAMERA], componentSizes, info.config.minimumRegionPixelArea))
		{
			//If the region is valid we need to compute the histogram allocation 




			//Make sure the feature vectors are normalied such that the max is 1
			if(info.data.featureMatches[VISUAL_SONIFICATION_LEFT_CAMERA][i].distance >1 || info.data.featureMatches[VISUAL_SONIFICATION_LEFT_CAMERA][i].distance <0)
			{
#ifndef ANDROID
				cout << "Distance Out of Range: "<< info.data.featureMatches[VISUAL_SONIFICATION_LEFT_CAMERA][i].distance << endl;
#endif
			}
#ifdef VERBOSE_OUTPUT
			cout << "Distance:" <<info.data.featureMatches[VISUAL_SONIFICATION_LEFT_CAMERA][i].distance << endl;
#endif

			//Calculate the histogram amount to add
			currentFeatureWeight = 1-info.data.featureMatches[VISUAL_SONIFICATION_LEFT_CAMERA][i].distance;

			//Since floating point is imprecise we can end up with some slightly greater than 1 distances.  Make them 0.
			if(currentFeatureWeight < 0)
			{
				currentFeatureWeight = 0;

			}

			//Get the correct bin
			currentFeatureHistogramBin = info.data.featureMatches[VISUAL_SONIFICATION_LEFT_CAMERA][i].trainIdx;

			//increment the histogram amount
			info.data.regionHistograms[currentLabel][currentFeatureHistogramBin] +=currentFeatureWeight;

			//Increment the feature count in this region
			info.data.featureCountPerRegion[currentLabel]++;


			//cout <<"Query Index: " <<currentFeatureIndex<< " Train Index: "<< currentFeatureHistogramBin << " Current Feature Weight: " << currentFeatureWeight<< endl;

		}

	}

	//Output debug infomation on feature counts
#ifdef VERBOSE_OUTPUT
	cout << "Region Feature Counts:" << endl;
	for(int i = 0; i < (int)info.data.featureCountPerRegion.size(); i++)
	{
		if(info.data.featureCountPerRegion[i] != 0)
		{
			cout << "Region "<< i <<": "<<info.data.featureCountPerRegion[i] << " Features Found" << endl;
		}
	}
#endif





	return;
}



//Desc: This function computes the centroid of a region in 3D space, the region id, the info struct and an output variable centroid as passed in
//
//Return: None
void visualSonificationComputeCentroidOfRegion(int region, VisualSonificationInfo & info, MatrixPoint3dFloat & centroid)
{
	MatrixPointInt * pointsInRegion = &info.data.regionsByPoints[VISUAL_SONIFICATION_LEFT_CAMERA][region][0];
	int numberOfPointsInRegion = info.data.regionsByPoints[VISUAL_SONIFICATION_LEFT_CAMERA][region].size();
	centroid.x = 0;
	centroid.y = 0;
	centroid.z = 0;
	MatrixData * currentDepthImage = &info.data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA];

	//Calculate the centroid of the 3D region, not the most efficient but it works
	for(int i = 0; i < numberOfPointsInRegion; i++)
	{
		MatrixPoint3dFloat * current3dPoint = (MatrixPoint3dFloat *)(currentDepthImage->data + pointsInRegion[i].y * currentDepthImage->step[0] +pointsInRegion[i].x* currentDepthImage->elemSize());
		centroid.x += current3dPoint->x;
		centroid.y += current3dPoint->y;
		centroid.z += current3dPoint->z;
	}
		centroid.x = centroid.x/numberOfPointsInRegion;
		centroid.y = centroid.y/numberOfPointsInRegion;
		centroid.z = centroid.z/numberOfPointsInRegion;
}

//Desc: This function computes the centroid of a region in image space, the region id, the info struct and an output variable centroid as passed in
//
//Return: None
void visualSonificationComputeCentroidOfRegionInImage(int region, VisualSonificationInfo & info, MatrixPointInt & centroid)
{
	MatrixPointInt * pointsInRegion = &info.data.regionsByPoints[VISUAL_SONIFICATION_LEFT_CAMERA][region][0];
	int numberOfPointsInRegion = info.data.regionsByPoints[VISUAL_SONIFICATION_LEFT_CAMERA][region].size();
	centroid.x = 0;
	centroid.y = 0;


	//Calculate the centroid of the 3D region, not the most efficient but it works
	for(int i = 0; i < numberOfPointsInRegion; i++)
	{
		
		

		centroid.x += pointsInRegion[i].x;
		centroid.y += pointsInRegion[i].y;

	}
		centroid.x = centroid.x/numberOfPointsInRegion;
		centroid.y = centroid.y/numberOfPointsInRegion;

}



void visualSonificationComputeCentroidOfRegionFixedPoint(int region, VisualSonificationInfo & info, MatrixPoint3dInt & centroid)
{
	MatrixPointInt * pointsInRegion = &info.data.regionsByPoints[VISUAL_SONIFICATION_LEFT_CAMERA][region][0];
	int numberOfPointsInRegion = info.data.regionsByPoints[VISUAL_SONIFICATION_LEFT_CAMERA][region].size();
	centroid.x = 0;
	centroid.y = 0;
	centroid.z = 0;
	MatrixData * currentDepthImage = &info.data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA];

	//Calculate the centroid of the 3D region, not the most efficient but it works
	for(int i = 0; i < numberOfPointsInRegion; i++)
	{
		//MatrixPoint3dFloat * current3dPoint = (MatrixPoint3dFloat *)(currentDepthImage->data + pointsInRegion[i].y * currentDepthImage->step[0] +pointsInRegion[i].x* currentDepthImage->elemSize());
		short int * current3dPoint = (short int *)(currentDepthImage->data + pointsInRegion[i].y * currentDepthImage->step[0] +pointsInRegion[i].x* 3* sizeof(short int));
		centroid.x += current3dPoint[0];
		centroid.y += current3dPoint[1];
		centroid.z += current3dPoint[2];
	}
		centroid.x = (centroid.x/numberOfPointsInRegion )>>LOCATION_3D_POINT_FRACTIONAL_BITS;
		centroid.y = (centroid.y/numberOfPointsInRegion )>>LOCATION_3D_POINT_FRACTIONAL_BITS;
		centroid.z = (centroid.z/numberOfPointsInRegion )>>LOCATION_3D_POINT_FRACTIONAL_BITS;
}



//Desc: This function creates the audio signatures based on the histograms info.data.regionHistograms[...]
//
//Return: None
void visualSonificationCreateAudioSignatures(VisualSonificationInfo & info)
{


	info.data.audioSignaturesInUse = 0;

#ifdef FIXED_POINT
	MatrixPoint3dInt centroid;

#else
	MatrixPoint3dFloat centroid;
#endif
	MatrixPointInt imageCentroid;


	info.data.regionsToBePlayed.clear();
	info.data.audioBuffers.numberOfBuffersInUse = 0;
	info.data.histogramBuffers.numberOfBuffersInUse = 0;
	//For each region, check if we need to create an audio signature
	for(int currentLabel = 0; currentLabel < (int)info.data.featureCountPerRegion.size(); currentLabel++)
	{
		if(info.data.featureCountPerRegion[currentLabel] >= info.data.minimumFeaturesForAudioSignature)
		{




			info.data.regionsToBePlayed.push_back(currentLabel);


			if(info.config.useOpenAlToPlay)
			{

				//First create the audio buffers for this one etc...
				info.data.audioSignatures[info.data.audioSignaturesInUse].createHistogramOpenALBuffers(&(info.data.regionHistograms[currentLabel][0]),info.data.bagOfWordsDictionary.rows);

			}

			if(info.config.showComponentRegionsAtSignatureGen)
			{
				displayComponent(info.data.regionsByPoints[VISUAL_SONIFICATION_LEFT_CAMERA], currentLabel, info.data.frameSize);
				waitKey(0);
			}
#ifdef VERBOSE_OUTPUT			
			cout << "Signature Generation for Region "<< currentLabel <<": "<<info.data.featureCountPerRegion[currentLabel] << "Features Binned"<< endl;

			//Show me the histogram
			vuPrintHistogram(&info.data.regionHistograms[currentLabel][0], info.data.bagOfWordsDictionary.rows);
#endif


#ifdef FIXED_POINT

			visualSonificationComputeCentroidOfRegionFixedPoint(currentLabel, info, centroid);
#else
			//Now we want the centroid of the 
			visualSonificationComputeCentroidOfRegion(currentLabel, info, centroid);

#endif
			visualSonificationComputeCentroidOfRegionInImage(currentLabel, info, imageCentroid);

			if(info.config.useOpenAlToPlay)
			{

				//Now to set our sounds location
				info.data.audioSignatures[info.data.audioSignaturesInUse].setLocation((int)centroid.x,(int)centroid.y,(int)centroid.z);

				//Set the direction toward the listener
				info.data.audioSignatures[info.data.audioSignaturesInUse].setDirectionTowardListner();
			}
			info.data.audioSignaturesInUse++;

			if(info.config.placeAudioInBuffers)
			{

				int currentAudioBuffer = info.data.audioBuffers.numberOfBuffersInUse;
				unsigned int waveBufferLength = SONIFICATION_MAX_AUDIO_SAMPLES_PER_BUFFER;
				HistogramSoundSource::generateWaveBufferDataFromHistogram(info.data.bagOfWordsDictionary.rows, &(info.data.regionHistograms[currentLabel][0]),1.0f,44100,8000, 2000, 10.0f,true,false,info.data.audioBuffers.buffers[currentAudioBuffer].data, waveBufferLength);
				info.data.audioBuffers.buffers[currentAudioBuffer].numberOfSamplesInUse = waveBufferLength;
				info.data.audioBuffers.buffers[currentAudioBuffer].location[0] = (int)centroid.x;
				info.data.audioBuffers.buffers[currentAudioBuffer].location[1] = (int)centroid.y;
				info.data.audioBuffers.buffers[currentAudioBuffer].location[2] = (int)centroid.z;


				info.data.audioBuffers.numberOfBuffersInUse++;



			}

			if(info.config.placeHistogramInBuffers)
			{

				int currentHistogramBuffer = info.data.histogramBuffers.numberOfBuffersInUse;
				info.data.histogramBuffers.buffers[currentHistogramBuffer].histogramEntriesInUse = ((info.data.bagOfWordsDictionary.rows < SONIFICATION_MAX_HISOTGRAM_SIZE)?info.data.bagOfWordsDictionary.rows:SONIFICATION_MAX_HISOTGRAM_SIZE);
				for(int i = 0; i < info.data.histogramBuffers.buffers[currentHistogramBuffer].histogramEntriesInUse; i++)
				{					
					info.data.histogramBuffers.buffers[currentHistogramBuffer].data[i] = info.data.regionHistograms[currentLabel][i];					
				}

				info.data.histogramBuffers.buffers[currentHistogramBuffer].numberOfFeaturesUsedToMakeHistogram = info.data.featureCountPerRegion[currentLabel];
				info.data.histogramBuffers.buffers[currentHistogramBuffer].location[0] = (int)centroid.x;
				info.data.histogramBuffers.buffers[currentHistogramBuffer].location[1] = (int)centroid.y;
				info.data.histogramBuffers.buffers[currentHistogramBuffer].location[2] = (int)centroid.z;
				info.data.histogramBuffers.buffers[currentHistogramBuffer].locationInImage[0] = (int)imageCentroid.x;
				info.data.histogramBuffers.buffers[currentHistogramBuffer].locationInImage[1] = (int)imageCentroid.y;

				info.data.histogramBuffers.numberOfBuffersInUse++;
				if(info.config.saveRegionHistogramFiles)
				{


					string currentRegionHistogramFileOutputName = info.config.baseRegionHistogramFilename;
					stringstream tmpBuffer;

					if(info.data.inputType ==VISUAL_SONIFICATION_IMAGE_LIST)
					{

						size_t found = info.data.imageList[VISUAL_SONIFICATION_LEFT_CAMERA][(info.data.currentInputFrames>0?info.data.currentInputFrames-1:info.data.currentInputFrames)].find_last_of('.');
						if(found == string::npos)
						{
							found = info.data.imageList[VISUAL_SONIFICATION_LEFT_CAMERA][(info.data.currentInputFrames>0?info.data.currentInputFrames-1:info.data.currentInputFrames)].length();

						}
						string baseImageListName = info.data.imageList[VISUAL_SONIFICATION_LEFT_CAMERA][(info.data.currentInputFrames>0?info.data.currentInputFrames-1:info.data.currentInputFrames)].substr(0,found);
						currentRegionHistogramFileOutputName = baseImageListName;



						//regionWaveFilenameBases
					}
					else
					{
						tmpBuffer.str("");
						tmpBuffer.clear();
						tmpBuffer << (info.data.currentInputFrames>0?info.data.currentInputFrames-1:info.data.currentInputFrames);
						currentRegionHistogramFileOutputName += tmpBuffer.str();


					}

					currentRegionHistogramFileOutputName =currentRegionHistogramFileOutputName + "_Region_";
					tmpBuffer.str("");
					tmpBuffer.clear();
					tmpBuffer << currentHistogramBuffer;
					currentRegionHistogramFileOutputName += tmpBuffer.str() + ".txt";
					currentRegionHistogramFileOutputName = info.config.regionHistogramFilesOutputDir +currentRegionHistogramFileOutputName;


					VisualSonification::saveHistogramToFile(currentRegionHistogramFileOutputName, info.data.histogramBuffers.buffers[currentHistogramBuffer]);



				}



				#ifdef SAVE_WAVE_FILES_CAPABLE
					if(info.config.saveRegionAudioFiles)
					{


						string currentRegionAudioFileOutputName = info.config.baseRegionAudioFilename;
						stringstream tmpBuffer;



						if(info.data.inputType ==VISUAL_SONIFICATION_IMAGE_LIST)
						{

							size_t found = info.data.imageList[VISUAL_SONIFICATION_LEFT_CAMERA][(info.data.currentInputFrames>0?info.data.currentInputFrames-1:info.data.currentInputFrames)].find_last_of('.');
							if(found == string::npos)
							{
								found = info.data.imageList[VISUAL_SONIFICATION_LEFT_CAMERA][(info.data.currentInputFrames>0?info.data.currentInputFrames-1:info.data.currentInputFrames)].length();

							}
							string baseImageListName = info.data.imageList[VISUAL_SONIFICATION_LEFT_CAMERA][(info.data.currentInputFrames>0?info.data.currentInputFrames-1:info.data.currentInputFrames)].substr(0,found);
							currentRegionAudioFileOutputName = baseImageListName;



							//regionWaveFilenameBases
						}
						else
						{
							tmpBuffer.str("");
							tmpBuffer.clear();
							tmpBuffer << (info.data.currentInputFrames>0?info.data.currentInputFrames-1:info.data.currentInputFrames);
							currentRegionAudioFileOutputName += tmpBuffer.str();


						}

						currentRegionAudioFileOutputName =currentRegionAudioFileOutputName + "_Region_";
						tmpBuffer.str("");
						tmpBuffer.clear();
						tmpBuffer << currentHistogramBuffer;
						currentRegionAudioFileOutputName += tmpBuffer.str() + ".wav";
						currentRegionAudioFileOutputName = info.config.regionAudioFilesOutputDir +currentRegionAudioFileOutputName;

						enum HistogramSoundSource::SoundGenerationTechniques technique;
						technique = info.data.audioGenerationConfigData.currentSonificationTechnique;//HistogramSoundSource::SOUND_GENERATION_PURE_SINEWAVE_WITH_TRANSITION;

						VisualSonification::saveHistogramBufferAudioFile(currentRegionAudioFileOutputName, info.data.histogramBuffers.buffers[currentHistogramBuffer],technique, info.data.audioGenerationConfigData);

					}


				#endif


			}




			if(info.data.audioSignaturesInUse == info.data.audioSignatureCapacity)
			{
				break;
			}

		}

	
	}






}

//Desc: This function plays the audio signatures in info.data.audioSignatures[i]
//
//Return: None
void visualSonificationPlayAudioSignatures(VisualSonificationInfo & info)
{

	if(!info.config.useOpenAlToPlay)
	{
		return;
	}

	for(int i = 0; i < info.data.audioSignaturesInUse; i++)
	{

#ifdef VERBOSE_OUTPUT
		cout << "Playing Region: " << info.data.regionsToBePlayed[i]<<endl;
#endif

		if(info.config.showComponentRegionsAtAudioOutput)
		{
			displayComponent(info.data.regionsByPoints[VISUAL_SONIFICATION_LEFT_CAMERA], info.data.regionsToBePlayed[i], info.data.frameSize);
			waitKey(10);
		}
		int x,y,z;
		info.data.audioSignatures[i].getLocation(x,y,z);

#ifdef VERBOSE_OUTPUT
		cout << "Centroid: (" << x <<" , " << y<<" , "<<z << ")"<<endl;
#endif
		//Attach the audio wave to the source in the audio player
		info.data.audioSignatures[i].attachBuffersToInternalSource();

		//Play
		info.data.audioSignatures[i].play();

		//Wait for the audio to complete playing
		while(!info.data.audioSignatures[i].checkPlayingComplete())
		{
			;//cout << "Waiting..";
		}
#ifdef VERBOSE_OUTPUT
		cout << "Done Waiting" << endl;
#endif
		//Detach this audio from the source
		info.data.audioSignatures[i].dettachBuffersFromInternalSource();

		if(info.config.showComponentRegionsAtAudioOutput)
		{

			waitKey(0);

		}
		else
		{

			usleep(10000);

		}


	}

}

//Desc: This function initializes the feature matching for bag of words
//
//Return: None
void visualSonificationInitFeatureMatching(VisualSonificationInfo & info)
{
	vector<Mat> dictionary;

	//Setup the matcher
	dictionary.push_back(info.data.bagOfWordsDictionary);
	info.data.featureMatcher = setupMatcher(info,dictionary);
}



//Desc: This function initializes the audio engine
//
//Return: None

void visualSonificationInitAudioGeneration(VisualSonificationInfo &info)
{
	//int argc = 0;
	//char * argv[1] = {"123"};
	//alutInit (&argc, argv);
	if(info.config.useOpenAlToPlay)
	{
		//Initialize the audio context and devices like OpenGL
		auInit();

		

	//	ALuint helloBuffer, helloSource;
	//	helloBuffer = alutCreateBufferHelloWorld ();
	//	alGenSources (1, &helloSource);
	//	alSourcei (helloSource, AL_BUFFER, helloBuffer);
	//	alSourcePlay (helloSource);
	//	alutSleep (.75);


	//	HistogramSoundSource audioSignatures2;

		//Create an initial set of audio objects
		info.data.audioSignatures.resize(20);
		info.data.audioSignatureCapacity = info.data.audioSignatures.size();

		//Do the initial setup
		for(int i=0; i<(int)info.data.audioSignatures.size(); i++)
		{
			info.data.audioSignatures[i].setHistogramSize(info.data.bagOfWordsDictionary.rows);
			info.data.audioSignatures[i].setHistogramDuration(info.data.audioGenerationConfigData.histogramOutputTotalTime);
	//		info.data.audioSignatures[i].setDirection(0,0,-1);
			info.data.audioSignatures[i].setHistogramClippingMax(10.0);
			info.data.audioSignatures[i].setHistogramMaxFreq(info.data.audioGenerationConfigData.maxFrequencyInHz);
			info.data.audioSignatures[i].setHistogramMinFreq(info.data.audioGenerationConfigData.minFrequencyInHz);
	//		info.data.audioSignatures[i].setRelativeToListener(true);
			info.data.audioSignatures[i].setVelocity(0,0,0);
			//info.data.audioSignatures[i].setWaveform(ALUT_WAVEFORM_SINE);
			//info.data.audioSignatures[i].setTrueModulated(true);
			info.data.audioSignatures[i].setNormalizeHistogram(true);
			info.data.audioSignatures[i].setSoundGenerationTechnique(info.data.audioGenerationConfigData.currentSonificationTechnique);
		}
		


		info.data.audioSignaturesInUse = 0;
	}

	if(info.config.placeAudioInBuffers)
	{
		info.data.audioBuffers.maxNumberOfBuffers = SONIFICATION_MAX_NUMBER_AUDIO_BUFFERS;
		info.data.audioBuffers.numberOfBuffersInUse = 0;	
		for(int i =0; i < info.data.audioBuffers.maxNumberOfBuffers; i++)
		{

			info.data.audioBuffers.buffers[i].sampleRateInHz = 44100;
			info.data.audioBuffers.buffers[i].location[0] = info.data.audioBuffers.buffers[i].location[1] = info.data.audioBuffers.buffers[i].location[2] = 0;
			for(int j = 0; j <SONIFICATION_MAX_AUDIO_SAMPLES_PER_BUFFER; j++)
			{ 
				info.data.audioBuffers.buffers[i].data[j] = 0; 		
			}
			
		}
		
		
		
		
	}
	if(info.config.placeHistogramInBuffers)
	{
		info.data.histogramBuffers.maxNumberOfBuffers = SONIFICATION_MAX_NUMBER_HISTOGRAM_BUFFERS;
		info.data.histogramBuffers.numberOfBuffersInUse = 0;	
		for(int i =0; i < info.data.histogramBuffers.maxNumberOfBuffers; i++)
		{


			info.data.histogramBuffers.buffers[i].location[0] = info.data.histogramBuffers.buffers[i].location[1] = info.data.histogramBuffers.buffers[i].location[2] = 0;
			for(int j = 0; j <SONIFICATION_MAX_HISOTGRAM_SIZE; j++)
			{ 
				info.data.histogramBuffers.buffers[i].data[j] = 0; 		
			}
			
		}
		
		
		
		
	}



	if(info.config.saveRegionAudioFiles)
	{
		//if(info.data.inputType ==VISUAL_SONIFICATION_IMAGE_LIST)

			//regionWaveFilenameBases



	}

}


//Desc: This function initializes the visual sonification application from the init state
//
//Return: None

void visualSonificationInit(VisualSonificationInfo &info)
{

	cout << "Input Type" << info.data.inputType << endl;

	//Setup the input mode
	if(info.data.inputMode == VISUAL_SONIFICATION_STEREO)
	{


		switch(info.data.inputType)
		{
			case VISUAL_SONIFICATION_STREAM:
				cout << "Running regular stream" << endl;
#ifndef ANDROID
				cout << "Opening Camera 1 : " <<info.config.leftCameraId << endl;
				cout << "Opening Camera 2 : " <<info.config.rightCameraId << endl;

#endif
				//Setup the video capture stream
				info.data.capture[VISUAL_SONIFICATION_LEFT_CAMERA].open(info.config.leftCameraId);
				info.data.capture[VISUAL_SONIFICATION_RIGHT_CAMERA].open(info.config.rightCameraId);

				//If the stream didn't open then give error
				if(!info.data.capture[VISUAL_SONIFICATION_LEFT_CAMERA].isOpened() ||!info.data.capture[VISUAL_SONIFICATION_RIGHT_CAMERA].isOpened())
				{
#ifndef ANDROID
					cout << "Error: Unable to open up stereo stream.  Exiting...." << endl;
#endif
					string input;
					cin >> input;
					exit(0);
				}
				else
				{

					//Set the frame size based on the config file
					int frameHeight;
					int frameWidth;
					switch(info.config.inputSizeSetting)
					{
						case VISUAL_SONIFICATION_INPUT_QVGA:
						{
							frameHeight = 240;
							frameWidth = 320;
							break;
						}
						case VISUAL_SONIFICATION_INPUT_VGA:
						default:
						{
							frameHeight = 480;
							frameWidth = 640;

							break;
						}
					}
					if(!(info.data.capture[VISUAL_SONIFICATION_LEFT_CAMERA].set(CV_CAP_PROP_FRAME_WIDTH, frameWidth)))//640)))
					{
#ifndef ANDROID
						cout << "Left frame width set failed" << endl;
#endif
					}
					if(!(info.data.capture[VISUAL_SONIFICATION_LEFT_CAMERA].set(CV_CAP_PROP_FRAME_HEIGHT, frameHeight)))//480)))
					{
#ifndef ANDROID
						cout << "Left frame height set failed" << endl;
#endif
					}


					if(!(info.data.capture[VISUAL_SONIFICATION_RIGHT_CAMERA].set(CV_CAP_PROP_FRAME_WIDTH, frameWidth)))//640)))
					{
#ifndef ANDROID
						cout << "Right frame width set failed" << endl;
#endif

					}
					if(!(info.data.capture[VISUAL_SONIFICATION_RIGHT_CAMERA].set(CV_CAP_PROP_FRAME_HEIGHT, frameHeight)))//480)))
					{
#ifndef ANDROID
						cout << "Right frame height set failed" << endl;
#endif

					}



					//Set the actual frame size based on what the stream says
					info.data.frameSize.width = (int)info.data.capture[VISUAL_SONIFICATION_LEFT_CAMERA].get(CV_CAP_PROP_FRAME_WIDTH);
					info.data.frameSize.height = (int)info.data.capture[VISUAL_SONIFICATION_LEFT_CAMERA].get(CV_CAP_PROP_FRAME_HEIGHT);
				}



				break;
			case VISUAL_SONIFICATION_VIDEO_FILE:
				cout << "Running video stream" << endl;
				break;
			case VISUAL_SONIFICATION_IMAGE_LIST:
				{

					cout << "Running image list" << endl;

					info.data.currentInputFrames = 0;

					if(info.config.imageListFilenames[VISUAL_SONIFICATION_LEFT_CAMERA].compare("") != 0 && info.config.imageListFilenames[VISUAL_SONIFICATION_RIGHT_CAMERA].compare("") != 0)
					{

						if(!loadTextFileList(info.data.imageList[VISUAL_SONIFICATION_LEFT_CAMERA], info.config.imageListFilenames[VISUAL_SONIFICATION_LEFT_CAMERA]) || !loadTextFileList(info.data.imageList[VISUAL_SONIFICATION_RIGHT_CAMERA], info.config.imageListFilenames[VISUAL_SONIFICATION_RIGHT_CAMERA]))
						{
#ifndef ANDROID
							cout << "Error: Unable to open up image file lists.  Exiting...." << endl;
#endif
							string input;
							cin >> input;
							exit(0);
							

						}
						string leftImageNameWithPath = info.config.imageListBaseDir[VISUAL_SONIFICATION_LEFT_CAMERA] + info.data.imageList[VISUAL_SONIFICATION_LEFT_CAMERA][0];
						string rightImageNameWithPath = info.config.imageListBaseDir[VISUAL_SONIFICATION_RIGHT_CAMERA] + info.data.imageList[VISUAL_SONIFICATION_RIGHT_CAMERA][0];
						info.data.currentCameraBuffers[VISUAL_SONIFICATION_LEFT_CAMERA] = imread(leftImageNameWithPath);
						info.data.currentCameraBuffers[VISUAL_SONIFICATION_RIGHT_CAMERA] = imread(rightImageNameWithPath);




						//If the stream didn't open then give error
						if(info.data.currentCameraBuffers[VISUAL_SONIFICATION_LEFT_CAMERA].empty() ||info.data.currentCameraBuffers[VISUAL_SONIFICATION_RIGHT_CAMERA].empty())
						{
							cout << "Error: Unable to open up image file.  Exiting...." << endl;
							string input;
							cin >> input;
							exit(0);
						}
						else
						{

							//Set the actual frame size based on what the stream says
							info.data.frameSize.width = ((int)info.data.currentCameraBuffers[VISUAL_SONIFICATION_LEFT_CAMERA].cols + info.data.currentCameraBuffers[VISUAL_SONIFICATION_RIGHT_CAMERA].cols)/2;
							info.data.frameSize.height = ((int)info.data.currentCameraBuffers[VISUAL_SONIFICATION_LEFT_CAMERA].rows+(int)info.data.currentCameraBuffers[VISUAL_SONIFICATION_LEFT_CAMERA].rows)/2;
							


							//Set the frame size based on the config file
							int frameHeight;
							int frameWidth;
							switch(info.config.inputSizeSetting)
							{
								case VISUAL_SONIFICATION_INPUT_QVGA:
								{
									frameHeight = 240;
									frameWidth = 320;
									break;
								}
								case VISUAL_SONIFICATION_INPUT_VGA:
								default:
								{
									frameHeight = 480;
									frameWidth = 640;

									break;
								}
							}


							if(info.data.frameSize.width != frameWidth || info.data.frameSize.height != frameHeight)
							{
								
								cout << "Error: frame size does not match config file.  Exiting...." << endl;
								string input;
								cin >> input;
								exit(0);


							}


						}

					}

				}	
				break;
			case VISUAL_SONIFICATION_EXTERNAL_STREAM:
				{
					cout << "Running external stream" << endl;

					//Set the frame size based on the config file
					int frameHeight;
					int frameWidth;
					switch(info.config.inputSizeSetting)
					{
						case VISUAL_SONIFICATION_INPUT_QVGA:
						{
							frameHeight = 240;
							frameWidth = 320;
							break;
						}
						case VISUAL_SONIFICATION_INPUT_VGA:
						default:
						{
							frameHeight = 480;
							frameWidth = 640;

							break;
						}
					}


					info.data.currentCameraBuffers[VISUAL_SONIFICATION_LEFT_CAMERA].create(frameWidth,frameHeight,CV_8UC3);
					info.data.currentCameraBuffers[VISUAL_SONIFICATION_RIGHT_CAMERA].create(frameWidth,frameHeight,CV_8UC3);
					//Set the actual frame size based on what the stream says
					info.data.frameSize.width = frameWidth;
					info.data.frameSize.height = frameHeight;


#ifdef ANDROID

										
#else
					cout << "ERROR: Android Stream not avialable unless built for android" << endl;
					cout << "Exiting.....";
					string input;
					cin >> input;
					exit(0);
#endif


				}
				break;
			default:

				break;
		}


		//Load the stereo calibration data
		info.data.stereoCalibration.loadCameraParams(info.config.calibrationFilename,true);

		//Setup the image remap tables
		initUndistortRectifyMap(info.data.stereoCalibration.getCameraMatrix(StereoCameraCalibration::CAMERA_LOCATION_LEFT), info.data.stereoCalibration.getDistCoeffs(StereoCameraCalibration::CAMERA_LOCATION_LEFT), info.data.stereoCalibration.getRx(0), info.data.stereoCalibration.getPx(0), info.data.frameSize, CV_16SC2, info.data.rmap[VISUAL_SONIFICATION_LEFT_CAMERA][VISUAL_SONIFICATION_CAMERA_REMAP_AXIS_X], info.data.rmap[VISUAL_SONIFICATION_LEFT_CAMERA][VISUAL_SONIFICATION_CAMERA_REMAP_AXIS_Y]);
		initUndistortRectifyMap(info.data.stereoCalibration.getCameraMatrix(StereoCameraCalibration::CAMERA_LOCATION_RIGHT), info.data.stereoCalibration.getDistCoeffs(StereoCameraCalibration::CAMERA_LOCATION_LEFT), info.data.stereoCalibration.getRx(1), info.data.stereoCalibration.getPx(1), info.data.frameSize, CV_16SC2, info.data.rmap[VISUAL_SONIFICATION_RIGHT_CAMERA][VISUAL_SONIFICATION_CAMERA_REMAP_AXIS_X], info.data.rmap[VISUAL_SONIFICATION_RIGHT_CAMERA][VISUAL_SONIFICATION_CAMERA_REMAP_AXIS_Y]);


		//Set the number of disparities
		int numberOfDisparities = 128;//192 VGA 96 QVGA;


		//Adjust the Disparity computation based on image size
		switch(info.config.inputSizeSetting)
		{
			case VISUAL_SONIFICATION_INPUT_QVGA:
			{
				numberOfDisparities = 96;//192 VGA 96 QVGA;
				//This is the region size for computing the SAD in the images.  The smaller this is, the more noise affects the matching and smaller
				//the more patchy the output.  The bigger this number, the more averaging occurs and more things that match.
				info.data.stereoBlockMatcher.state->SADWindowSize = 15;//25;//45 VGA;  15 QVGA

				//This is the speckle filtering where a blob of max size with a small range is considered too small to be real
				info.data.stereoBlockMatcher.state->speckleWindowSize = 200;//200;
				info.data.stereoBlockMatcher.state->speckleRange = 64;

				info.data.stereoGlobalBlockMatcher.preFilterCap = 63;

				break;
			}
			case VISUAL_SONIFICATION_INPUT_VGA:
			default:
			{


				numberOfDisparities = 128;//192 VGA 96 QVGA;

				//This is the region size for computing the SAD in the images.  The smaller this is, the more noise affects the matching and smaller
				//the more patchy the output.  The bigger this number, the more averaging occurs and more things that match.
				info.data.stereoBlockMatcher.state->SADWindowSize = 45;//25;//45 VGA;  15 QVGA


				//This is the speckle filtering where a blob of max size with a small range is considered too small to be real
				info.data.stereoBlockMatcher.state->speckleWindowSize = 200;//200;
				info.data.stereoBlockMatcher.state->speckleRange = 64;


				info.data.stereoGlobalBlockMatcher.preFilterCap = 63;

				break;
			}
		}


		//Get the valid regions from the images
		info.data.stereoBlockMatcher.state->roi1 = info.data.stereoCalibration.getValidRoi(0);
		info.data.stereoBlockMatcher.state->roi2 = info.data.stereoCalibration.getValidRoi(1);

		//This is the cutoff for the prefilter that takes place.  The prefilter is an averaging across a neighborhood
		//This is the max value allowed out of the averaging, default size is 9
		info.data.stereoBlockMatcher.state->preFilterCap = 9;


		//This is the value for the smallest allowed disparity, 0 means infinity and then the number of disparities determines
		//how close you can get to the camera
		info.data.stereoBlockMatcher.state->minDisparity = 0;

		//This is basically the number of disparity values (integer)
		info.data.stereoBlockMatcher.state->numberOfDisparities = numberOfDisparities;

		//How much texture a region needs before the matching is allowed.  The smaller, the more regions allowed to match
		info.data.stereoBlockMatcher.state->textureThreshold = 0;//10

		//This is how unique the matched disparity has to be compared to the next closest match  this is in percent
		//This a low value means the match does not have to be that much greater than the next closes match
		//While a high number means a high uniqueness is required and thus you get less disparities
		info.data.stereoBlockMatcher.state->uniquenessRatio = 2;//15


		//I think this is the difference in doing from left to right and then right to left.  This sets
		//a threshold
		info.data.stereoBlockMatcher.state->disp12MaxDiff = -1;
		//info.data.stereoBlockMatcher.state->trySmallerWindows = 1;
		//info.data.stereoGlobalBlockMatcher.

		info.data.stereoGlobalBlockMatcher.SADWindowSize = 3;
    
		int cn = 3;
    
		//Setup the semi global matcher
		info.data.stereoGlobalBlockMatcher.P1 = 8*cn*info.data.stereoGlobalBlockMatcher.SADWindowSize*info.data.stereoGlobalBlockMatcher.SADWindowSize;
		info.data.stereoGlobalBlockMatcher.P2 = 32*cn*info.data.stereoGlobalBlockMatcher.SADWindowSize*info.data.stereoGlobalBlockMatcher.SADWindowSize;
		info.data.stereoGlobalBlockMatcher.minDisparity = 0;
		info.data.stereoGlobalBlockMatcher.numberOfDisparities = numberOfDisparities;
		info.data.stereoGlobalBlockMatcher.uniquenessRatio = 5;//10;
		info.data.stereoGlobalBlockMatcher.speckleWindowSize = 16;//50;
		info.data.stereoGlobalBlockMatcher.speckleRange = 16;
		info.data.stereoGlobalBlockMatcher.disp12MaxDiff = 1;
		info.data.stereoGlobalBlockMatcher.fullDP = false;//true;


	}
	else if(info.data.inputMode == VISUAL_SONIFICATION_MONO_AND_DEPTH)
	{
		switch(info.data.inputType)
		{
			case VISUAL_SONIFICATION_STREAM:

				break;
			case VISUAL_SONIFICATION_VIDEO_FILE:

				break;
			case VISUAL_SONIFICATION_IMAGE_LIST:
				break;
			default:

				break;
		}


	}
	else if(info.data.inputMode == VISUAL_SONIFICATION_MONO_AND_SEPARATE_SEGMENTATION)
	{

		switch(info.data.inputType)
		{
			case VISUAL_SONIFICATION_STREAM:

				break;
			case VISUAL_SONIFICATION_VIDEO_FILE:

				break;
			case VISUAL_SONIFICATION_IMAGE_LIST:				{

				cout << "Running image list" << endl;

				info.data.currentInputFrames = 0;

				if(info.config.imageListFilenames[VISUAL_SONIFICATION_LEFT_CAMERA].compare("") != 0 )
				{

					if(!loadTextFileList(info.data.imageList[VISUAL_SONIFICATION_LEFT_CAMERA], info.config.imageListFilenames[VISUAL_SONIFICATION_LEFT_CAMERA]) )
					{
#ifndef ANDROID
						cout << "Error: Unable to open up image file list.  Exiting...." << endl;
#endif
						string input;
						cin >> input;
						exit(0);


					}
					string leftImageNameWithPath = info.config.imageListBaseDir[VISUAL_SONIFICATION_LEFT_CAMERA] + info.data.imageList[VISUAL_SONIFICATION_LEFT_CAMERA][0];

					info.data.currentCameraBuffers[VISUAL_SONIFICATION_LEFT_CAMERA] = imread(leftImageNameWithPath);





					//If the stream didn't open then give error
					if(info.data.currentCameraBuffers[VISUAL_SONIFICATION_LEFT_CAMERA].empty() )
					{
						cout << "Error: Unable to open up image file.  Exiting...." << endl;
						string input;
						cin >> input;
						exit(0);
					}
					else
					{

						//Set the actual frame size based on what the stream says
						info.data.frameSize.width = ((int)info.data.currentCameraBuffers[VISUAL_SONIFICATION_LEFT_CAMERA].cols );
						info.data.frameSize.height = ((int)info.data.currentCameraBuffers[VISUAL_SONIFICATION_LEFT_CAMERA].rows);



						//Set the frame size based on the config file
						int frameHeight;
						int frameWidth;
						switch(info.config.inputSizeSetting)
						{
							case VISUAL_SONIFICATION_INPUT_QVGA:
							{
								frameHeight = 240;
								frameWidth = 320;
								break;
							}
							case VISUAL_SONIFICATION_INPUT_VGA:
							default:
							{
								frameHeight = 480;
								frameWidth = 640;

								break;
							}
						}


						if(info.data.frameSize.width != frameWidth || info.data.frameSize.height != frameHeight)
						{

							cout << "Error: frame size does not match config file.  Exiting...." << endl;
							string input;
							cin >> input;
							exit(0);


						}


					}

				}

			}
			break;






				break;
			default:

				break;
		}



	}

	//Load the dictionary
	int numberOfDictionaryEntries = 0;
	codeWordGenLoadDictionary(info.config.dictionaryFilename, info.data.bagOfWordsDictionary, numberOfDictionaryEntries);
	if(numberOfDictionaryEntries != 0)
	{
		cout << "Loaded " << info.config.dictionaryFilename << " dictionary file with " << numberOfDictionaryEntries << "." <<endl;
	}
	else
	{
		cout << "ERROR: Dictionary file load failed." <<endl;
	}


	//Setup the feature extraction, matching and audio generation
	visualSonificationInitFeatureExtraction(info);
	visualSonificationInitFeatureMatching(info);
	visualSonificationInitAudioGeneration(info);



//	ALuint helloBuffer, helloSource;
//	helloBuffer = alutCreateBufferHelloWorld ();
//	alGenSources (1, &helloSource);
//	alSourcei (helloSource, AL_BUFFER, helloBuffer);
//	alSourcePlay (helloSource);
//	alutSleep (.75);


}


//Desc: This function is the mouse callback for handling getting the depth on a mouse click.  The param is a pointer to 
//VisualSonificationInfo to allow the mouse to get depth and disparity
//Return: None

void mouseCallbackGetDepth(int event, int x, int y, int flags, void* param)
{
	VisualSonificationInfo * infoForMouse = (VisualSonificationInfo *)param;
	if(x>=0 && x < infoForMouse->data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].cols && y>=0 &&infoForMouse->data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].rows)
	{
		switch(event)
		{

			case CV_EVENT_LBUTTONDOWN:
				{

#ifdef FIXED_POINT

					Vec3s point = infoForMouse->data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].at<Vec3s>(y, x);
#else
					Vec3f point = infoForMouse->data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].at<Vec3f>(y, x);
#endif

					cout << "Current 3D Point: (" << point[0]<<","<< point[1]<<"," <<point[2] << ")"<< endl;
				}
				break;
			case CV_EVENT_RBUTTONDOWN:

				cout << "Current Disparity Point: " << infoForMouse->data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].at<signed short int>(Point(x,y)) << endl;
				break;
			case CV_EVENT_MBUTTONDOWN:
				break;

		}

	}
}


//Desc: This function is the primary key handler.  It takes a key and the visual sonification state to change.  it also 
//takes the done bool to change if the quit condition is reached.  When bool is changed to true, the key to quit has been
//pressed
//Return: None

void handleKey(int key, bool & done, VisualSonificationInfo & info)
{
	static int lastKey = 0;
	//Except for quit, these are toggles
	
	//Quit using q or esc
	if(key == 'q'  || key ==27 || ((info.data.currentState == VISUAL_SONIFICATION_STATE_INIT)&&(( key ==' ')||(key=='r')||(key==13) )))
	{
		done = true;

	}

	if(key =='p' && lastKey != 'p')
	{
		info.config.pauseAfterEachStateLoop = !info.config.pauseAfterEachStateLoop;
		cout << "Pause After Each State: "<< (info.config.pauseAfterEachStateLoop?"On":"Off") <<endl;

	}


	//Show depth image
	if(key == '1' && lastKey != '1')
	{
		info.config.showRawDepthImage = !info.config.showRawDepthImage;
		if(!info.config.showRawDepthImage)
		{

			destroyWindow("Raw Depth Image");

		}
		else
		{
			namedWindow("Raw Depth Image");
		
			setMouseCallback("Raw Depth Image", mouseCallbackGetDepth, &info);
			namedWindow("ComponentDisplay");
			setMouseCallback("ComponentDisplay", mouseCallbackGetDepth, &info);
		}
	}

	//Show grayscale images
	if(key == '2' && lastKey != '2')
	{
		info.config.showGrayscaleInputImage = !info.config.showGrayscaleInputImage;
		if(!info.config.showGrayscaleInputImage)
		{

			destroyWindow("Grayscale Left Input Image");
			destroyWindow("Grayscale Right Input Image");
		}
		else
		{
			namedWindow("Grayscale Left Input Image");
			setMouseCallback("Grayscale Left Input Image", mouseCallbackGetDepth, &info);

			namedWindow("Grayscale Right Input Image");

		}
	}

	//Show raw inputs
	if(key == '3' && lastKey != '3')
	{
		info.config.showRawInputImage = !info.config.showRawInputImage;
		if(!info.config.showRawDepthImage)
		{

			destroyWindow("Raw Left Input Image");
			destroyWindow("Raw Right Input Image");


		}
		else
		{
			namedWindow("Raw Left Input Image");
			namedWindow("Raw Right Input Image");

		}
	}

	//Show remapped input images
	if(key == '4' && lastKey != '4')
	{
		info.config.showRemappedInputImage = !info.config.showRemappedInputImage;
		if(!info.config.showRawDepthImage)
		{

			destroyWindow("Remapped Left Input Image");
			destroyWindow("Remapped Right Input Image");
		}
		else
		{
			namedWindow("Remapped Left Input Image");
			namedWindow("Remapped Right Input Image");

		}
	}


	//Show the edge image
	if(key == '5' && lastKey != '5')
	{

		info.config.showEdgeImage = !info.config.showEdgeImage;
		if(!info.config.showEdgeImage)
		{

			destroyWindow("Edge Image");

		}
		else
		{
			namedWindow("Edge Image");

		}



	}

	//Use the global matching
	if(key == '6' && lastKey != '6')
	{
		info.config.useGlobalBlockMatching = !info.config.useGlobalBlockMatching;
		cout << "Global Block Matching: " << info.config.useGlobalBlockMatching << endl;

	}
	
	//Use canny
	if(key == '7' && lastKey != '7')
	{
		info.config.useCannyEdgeDetector = !info.config.useCannyEdgeDetector;
		cout << "Use Canny Edge Detector: " << info.config.useCannyEdgeDetector << endl;

		
	}

	//Save corresponding data
	if(key == '!' && lastKey != '!')
	{
		info.config.saveRawDepthImage = !info.config.saveRawDepthImage;
		cout << "Save Raw Depth Image: " << info.config.saveRawDepthImage << endl;
	}
	if(key == '@' && lastKey != '@')
	{
		info.config.saveGrayscaleInputImage = !info.config.saveGrayscaleInputImage;
		cout << "Save Grayscale Image: " << info.config.saveGrayscaleInputImage << endl;

	}
	if(key == '#' && lastKey != '#')
	{
		info.config.saveRawInputImage = !info.config.saveRawInputImage;
		cout << "Save Raw Input Image: " << info.config.saveRawInputImage << endl;
	}
	if(key == '$' && lastKey != '$')
	{
		info.config.saveRemappedInputImage = !info.config.saveRemappedInputImage;
		cout << "Save Remapped Input Image: " << info.config.saveRemappedInputImage << endl;
	}
	if(key == '%' && lastKey != '%')
	{
		info.config.save3dPoints = !info.config.save3dPoints;
		cout << "Save 3D points: " << info.config.save3dPoints << endl;

	}




	lastKey = key;
}

void * visualSonificationMainThread(void *)
{
	
	
	
	
	return NULL;
}


int visualSonificationMainLoop(VisualSonificationInfo &visualSonificationInfo)
{
	//flag for when done and the key variable
	bool done = false;
	bool lastIteration = false;
	int key = -1;
	
	//The number of remaining images initial setup
	int numberOfImagesRemaining = 2;

//	clock_t extractionTimeStart;
//	clock_t extractionTimeStop;

	//Initialize the timing
	vuInitTimingInformation();

	//While done flag is not set
	while(!done)
	{
#ifndef ANDROID
		cout << "Beginning state: " << visualSonificationInfo.data.currentState << endl;
#endif
		//Execute current state
		switch(visualSonificationInfo.data.currentState)
		{
			case VISUAL_SONIFICATION_STATE_INIT:


				//Timing
				getStartClockTime(TIMING_EVENT_INIT);
				
				//Call the init state handler
				visualSonificationInit(visualSonificationInfo);

				//Timing
				getEndClockTime(TIMING_EVENT_INIT);

				visualSonificationInfo.data.currentState = VISUAL_SONIFICATION_STATE_DATA_CAPTURE;
				break;
			case VISUAL_SONIFICATION_STATE_DATA_CAPTURE:
				getStartClockTime(TIMING_EVENT_ALL);
				getStartClockTime(TIMING_EVENT_CAPTURE_IMAGE);

				//Get the current set of images and data
 				numberOfImagesRemaining = getNextImages(visualSonificationInfo);
				if(numberOfImagesRemaining ==0)
				{
					lastIteration = true;
					//done = true;
				}
				if(numberOfImagesRemaining <0)
				{
					done = true;
					break;
				}


				getEndClockTime(TIMING_EVENT_CAPTURE_IMAGE);


				//Save or show any images or windows relaitve to this state that are available
				if(visualSonificationInfo.config.showRawDepthImage)
				{
					//Mat rawDepthDisplay;
					//visualSonificationInfo.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].convertTo(rawDepthDisplay,CV_8U,255.0/((double)(visualSonificationInfo.data.stereoBlockMatcher.state->numberOfDisparities* 16)));					
					


					Mat colorDisparity(visualSonificationInfo.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].rows,visualSonificationInfo.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].cols,CV_8UC3);
					vuColorDisparityMap(&visualSonificationInfo.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA], &colorDisparity,(float)visualSonificationInfo.data.stereoBlockMatcher.state->numberOfDisparities, (float)16);
					imshow("Raw Depth Image", colorDisparity);
				}

				if(visualSonificationInfo.config.saveRawDepthImage)
				{
					//Mat rawDepthDisplay;
					//visualSonificationInfo.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].convertTo(rawDepthDisplay,CV_8U,255.0/((double)(visualSonificationInfo.data.stereoBlockMatcher.state->numberOfDisparities* 16)));					
					Mat colorDisparity(visualSonificationInfo.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].rows,visualSonificationInfo.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].cols,CV_8UC3);
					vuColorDisparityMap(&visualSonificationInfo.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA], &colorDisparity,(float)visualSonificationInfo.data.stereoBlockMatcher.state->numberOfDisparities, (float)16);

					imwrite("lastRawDepthImage.png",colorDisparity);

				}
				if(visualSonificationInfo.config.save3dPoints)
				{
					FileStorage fs("last3dPoints.yml", FileStorage::WRITE);
					fs << "3dPoints" << visualSonificationInfo.data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA];
					fs.release();
					ofstream text3dPoints;
					text3dPoints.open("last3dPoints.txt");
					const double max_z = 1.0e4;

					for(int y = 0; y < visualSonificationInfo.data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].rows; y++)
					{
						for(int x = 0; x < visualSonificationInfo.data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].cols; x++)
						{
							Vec3f point = visualSonificationInfo.data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].at<Vec3f>(y, x);
							if(fabs(point[2] - max_z) < FLT_EPSILON || fabs(point[2]) > max_z)
							{
								continue;
							}
							text3dPoints <<point[0]<<" "<< point[1]<<" " <<point[2] << endl;
						}
					}

					text3dPoints.close();


				}



				if(visualSonificationInfo.config.showRawInputImage)
				{
					Mat rawLeftInputDisplay;
					rawLeftInputDisplay = visualSonificationInfo.data.currentRawCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA];

					if(!rawLeftInputDisplay.empty())
					{
						imshow("Raw Left Input Image", rawLeftInputDisplay);
					}
					Mat rawRightInputDisplay;
					rawRightInputDisplay = visualSonificationInfo.data.currentRawCameraFrames[VISUAL_SONIFICATION_RIGHT_CAMERA];

					if(!rawRightInputDisplay.empty())
					{
						imshow("Raw Right Input Image", rawRightInputDisplay);
					}

				}
				if(visualSonificationInfo.config.saveRawInputImage)
				{
					imwrite("lastLeftRawInput.png",visualSonificationInfo.data.currentRawCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA]); 
					imwrite("lastRightRawInput.png",visualSonificationInfo.data.currentRawCameraFrames[VISUAL_SONIFICATION_RIGHT_CAMERA]); 
				}



				if(visualSonificationInfo.config.showRemappedInputImage)
				{
					Mat remappedLeftInputDisplay;
					remappedLeftInputDisplay = visualSonificationInfo.data.currentCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA];

					if(!remappedLeftInputDisplay.empty())
					{
						imshow("Remapped Left Input Image", remappedLeftInputDisplay);
					}
					Mat remappedRightInputDisplay;
					remappedRightInputDisplay = visualSonificationInfo.data.currentCameraFrames[VISUAL_SONIFICATION_RIGHT_CAMERA];

					if(!remappedRightInputDisplay.empty())
					{
						imshow("Remapped Right Input Image", remappedRightInputDisplay);
					}

				}
				if(visualSonificationInfo.config.saveRemappedInputImage)
				{
					imwrite("lastLeftRemappedInput.png",visualSonificationInfo.data.currentCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA]); 
					imwrite("lastRightRemappedInput.png",visualSonificationInfo.data.currentCameraFrames[VISUAL_SONIFICATION_RIGHT_CAMERA]); 
				}



				if(visualSonificationInfo.config.showGrayscaleInputImage)
				{
					Mat grayLeftInputDisplay;
					grayLeftInputDisplay = visualSonificationInfo.data.currentGrayCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA];

					if(!grayLeftInputDisplay.empty())
					{
						imshow("Grayscale Left Input Image", grayLeftInputDisplay);
					}
					
					Mat grayRightInputDisplay;
					grayRightInputDisplay = visualSonificationInfo.data.currentGrayCameraFrames[VISUAL_SONIFICATION_RIGHT_CAMERA];

					if(!grayRightInputDisplay.empty())
					{
						imshow("Grayscale Right Input Image", grayRightInputDisplay);
					}
					
				}

				if(visualSonificationInfo.config.saveGrayscaleInputImage)
				{
					imwrite("lastGrayscaleLeft.png",visualSonificationInfo.data.currentGrayCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA]); 
					imwrite("lastGrayscaleRight.png",visualSonificationInfo.data.currentGrayCameraFrames[VISUAL_SONIFICATION_RIGHT_CAMERA]); 
				}

		
				visualSonificationInfo.data.currentState = VISUAL_SONIFICATION_STATE_SEGMENTATION;
				break;
			case VISUAL_SONIFICATION_STATE_SEGMENTATION:

				getStartClockTime(TIMING_EVENT_SEGMENT);

				if(!visualSonificationInfo.config.segmentUsingExternalMask)
				{
					//Call the segmentation of the input data
					visualSonificationSegmentInputs(visualSonificationInfo);
				}
				else
				{
					visualSonificationSegmentInputsWithExternalMask(visualSonificationInfo);

				}


				getEndClockTime(TIMING_EVENT_SEGMENT);

				visualSonificationInfo.data.currentState = VISUAL_SONIFICATION_STATE_FEATURE_EXTRACTION;
				break;
			case VISUAL_SONIFICATION_STATE_FEATURE_EXTRACTION:
//				extractionTimeStart = clock();
				getStartClockTime(TIMING_EVENT_FEATURE_EXTRACTION);

				//Perform the feature extraction
				visualSonificationExtractFeatures(visualSonificationInfo);
				getEndClockTime(TIMING_EVENT_FEATURE_EXTRACTION);
//				extractionTimeStop = clock();
//				cout << "Feature Extraction Time: "<< extractionTimeStop - extractionTimeStart << endl;

				//Show the requested information
				if(visualSonificationInfo.config.showFeaturesImage)
				{
					Mat grayLeftInput;
					grayLeftInput = visualSonificationInfo.data.currentGrayCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA];

					if(!grayLeftInput.empty())
					{
						Mat featureLeftInputDisplay;
						if(grayLeftInput.channels() ==3)
						{
							cvtColor(grayLeftInput,featureLeftInputDisplay,CV_BGR2GRAY);
						}
						else
						{
							grayLeftInput.copyTo(featureLeftInputDisplay);
						}
						Mat featureLeftInputDisplayWithFeatures;
						drawKeypoints(featureLeftInputDisplay,visualSonificationInfo.data.currentKeypoints[VISUAL_SONIFICATION_LEFT_CAMERA],featureLeftInputDisplayWithFeatures,Scalar(255.0,0), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);			
						
						imshow("GrayLeft",grayLeftInput);
						imshow("Left Features Image", featureLeftInputDisplayWithFeatures);
		
					}
		
				
				}
				visualSonificationInfo.data.currentState = VISUAL_SONIFICATION_STATE_HISTOGRAM_GENERATION;
				break;
			case VISUAL_SONIFICATION_STATE_HISTOGRAM_GENERATION:
				getStartClockTime(TIMING_EVENT_HISTOGRAM_GENERATION);

				//Create the histograms
				visualSonificationCreateHistograms(visualSonificationInfo);
				getEndClockTime(TIMING_EVENT_HISTOGRAM_GENERATION);
				visualSonificationInfo.data.currentState = VISUAL_SONIFICATION_STATE_SIGNTATURE_GENERATION;
				break;
			case VISUAL_SONIFICATION_STATE_SIGNTATURE_GENERATION:
				getStartClockTime(TIMING_EVENT_SIGNATURE_GENERATION);

				//Create the audio signatures
				visualSonificationCreateAudioSignatures(visualSonificationInfo);
				getEndClockTime(TIMING_EVENT_SIGNATURE_GENERATION);

				visualSonificationInfo.data.currentState = VISUAL_SONIFICATION_STATE_AUDIO_GENERATION;
				break;
			case VISUAL_SONIFICATION_STATE_AUDIO_GENERATION:
				getStartClockTime(TIMING_EVENT_AUDIO_GENERATION);

				//Play the audio signatures
				visualSonificationPlayAudioSignatures(visualSonificationInfo);
				getEndClockTime(TIMING_EVENT_AUDIO_GENERATION);
				visualSonificationInfo.data.currentState = VISUAL_SONIFICATION_STATE_DATA_CAPTURE;
				getEndClockTime(TIMING_EVENT_ALL);
				if( lastIteration)
				{
					done = true;
				}


#ifdef DISPLAY_TIMING_WINDOWS
				vuPrintTimingInformationToConsole();
#endif
#ifdef SAVE_TIMING
				vuPrintTimingInformationToFile();
#endif
				break;
			default:
				break;


		}

		cout << "State Complete.  Next State: " << (int)visualSonificationInfo.data.currentState << endl;


		//Handle pause or continuous run mode
		//Note that a waitKey is required to be called periodically to handle window/screen updates
		if(!visualSonificationInfo.config.pauseAfterEachStateLoop)
		{
			key= waitKey(33) & 0xff;
		}
		else
		{
			key= waitKey(0) & 0xff;
		}


		//Call the key handler
		handleKey(key, done,visualSonificationInfo);
	}
	
	
	
	return 0;
}


//Desc: This function is main if this is a standalone executable.  It takes the typical program arugments
//
//Return: 0

#ifndef VISUAL_SONIFICATION_MODULE
int main(int argc, char * argv[])
{


#ifndef ANDROID
	#ifdef TEST_CLASS
		visualSonificationClassTechniqueOnPC();

	#endif
#endif
	//General State information
	VisualSonificationInfo visualSonificationInfo;

	//Pointer to allow the mouse to get data
	VisualSonificationInfo * infoForMouse;

	//Set the mounse pointer
	infoForMouse = &visualSonificationInfo;

	//Set the initial state
	visualSonificationInfo.data.currentState = VISUAL_SONIFICATION_STATE_INIT;

//	testVuFunctions();
//	testConnectedComponent();
//	return 0;

	//Handle the input parameters and setup the info struct
	handleParams(argc,argv,visualSonificationInfo);


	//flag for when done and the key variable
	bool done = false;
	bool lastIteration = false;
	int key = -1;

	//This loop allows activation of debugging information and windows before running starts
	cout << "Press debug keys to activate various windows" << endl;
	while(!done && visualSonificationInfo.config.pauseForSetupAtStart)
	{
		namedWindow("Setup");

		key= waitKey(33) & 0xff;
		handleKey(key, done,visualSonificationInfo);

	}
#ifndef USE_OLD_MAIN_LOOP
	visualSonificationMainLoop(visualSonificationInfo);

#else
	//Reset the done flag
	done = false;


	//The number of remaining images initial setup
	int numberOfImagesRemaining = 2;

//	clock_t extractionTimeStart;
//	clock_t extractionTimeStop;

	//Initialize the timing
	vuInitTimingInformation();

	//While done flag is not set
	while(!done)
	{
#ifndef ANDROID
		cout << "Beginning state: " << visualSonificationInfo.data.currentState << endl;
#endif
		//Execute current state
		switch(visualSonificationInfo.data.currentState)
		{
			case VISUAL_SONIFICATION_STATE_INIT:


				//Timing
				getStartClockTime(TIMING_EVENT_INIT);
				
				//Call the init state handler
				visualSonificationInit(visualSonificationInfo);

				//Timing
				getEndClockTime(TIMING_EVENT_INIT);

				visualSonificationInfo.data.currentState = VISUAL_SONIFICATION_STATE_DATA_CAPTURE;
				break;
			case VISUAL_SONIFICATION_STATE_DATA_CAPTURE:
				getStartClockTime(TIMING_EVENT_ALL);
				getStartClockTime(TIMING_EVENT_CAPTURE_IMAGE);

				//Get the current set of images and data
 				numberOfImagesRemaining = getNextImages(visualSonificationInfo);
				if(numberOfImagesRemaining ==0)
				{
					lastIteration = true;
					//done = true;
				}
				if(numberOfImagesRemaining <0)
				{
					done = true;
					break;
				}


				getEndClockTime(TIMING_EVENT_CAPTURE_IMAGE);


				//Save or show any images or windows relaitve to this state that are available
				if(visualSonificationInfo.config.showRawDepthImage)
				{
					//Mat rawDepthDisplay;
					//visualSonificationInfo.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].convertTo(rawDepthDisplay,CV_8U,255.0/((double)(visualSonificationInfo.data.stereoBlockMatcher.state->numberOfDisparities* 16)));					
					


					Mat colorDisparity(visualSonificationInfo.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].rows,visualSonificationInfo.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].cols,CV_8UC3);
					vuColorDisparityMap(&visualSonificationInfo.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA], &colorDisparity,(float)visualSonificationInfo.data.stereoBlockMatcher.state->numberOfDisparities, (float)16);
					imshow("Raw Depth Image", colorDisparity);
				}

				if(visualSonificationInfo.config.saveRawDepthImage)
				{
					//Mat rawDepthDisplay;
					//visualSonificationInfo.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].convertTo(rawDepthDisplay,CV_8U,255.0/((double)(visualSonificationInfo.data.stereoBlockMatcher.state->numberOfDisparities* 16)));					
					Mat colorDisparity(visualSonificationInfo.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].rows,visualSonificationInfo.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].cols,CV_8UC3);
					vuColorDisparityMap(&visualSonificationInfo.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA], &colorDisparity,(float)visualSonificationInfo.data.stereoBlockMatcher.state->numberOfDisparities, (float)16);

					imwrite("lastRawDepthImage.png",colorDisparity);

				}
				if(visualSonificationInfo.config.save3dPoints)
				{
					FileStorage fs("last3dPoints.yml", FileStorage::WRITE);
					fs << "3dPoints" << visualSonificationInfo.data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA];
					fs.release();
					ofstream text3dPoints;
					text3dPoints.open("last3dPoints.txt");
					const double max_z = 1.0e4;

					for(int y = 0; y < visualSonificationInfo.data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].rows; y++)
					{
						for(int x = 0; x < visualSonificationInfo.data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].cols; x++)
						{
							Vec3f point = visualSonificationInfo.data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].at<Vec3f>(y, x);
							if(fabs(point[2] - max_z) < FLT_EPSILON || fabs(point[2]) > max_z)
							{
								continue;
							}
							text3dPoints <<point[0]<<" "<< point[1]<<" " <<point[2] << endl;
						}
					}

					text3dPoints.close();


				}



				if(visualSonificationInfo.config.showRawInputImage)
				{
					Mat rawLeftInputDisplay;
					rawLeftInputDisplay = visualSonificationInfo.data.currentRawCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA];

					if(!rawLeftInputDisplay.empty())
					{
						imshow("Raw Left Input Image", rawLeftInputDisplay);
					}
					Mat rawRightInputDisplay;
					rawRightInputDisplay = visualSonificationInfo.data.currentRawCameraFrames[VISUAL_SONIFICATION_RIGHT_CAMERA];

					if(!rawRightInputDisplay.empty())
					{
						imshow("Raw Right Input Image", rawRightInputDisplay);
					}

				}
				if(visualSonificationInfo.config.saveRawInputImage)
				{
					imwrite("lastLeftRawInput.png",visualSonificationInfo.data.currentRawCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA]); 
					imwrite("lastRightRawInput.png",visualSonificationInfo.data.currentRawCameraFrames[VISUAL_SONIFICATION_RIGHT_CAMERA]); 
				}



				if(visualSonificationInfo.config.showRemappedInputImage)
				{
					Mat remappedLeftInputDisplay;
					remappedLeftInputDisplay = visualSonificationInfo.data.currentCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA];

					if(!remappedLeftInputDisplay.empty())
					{
						imshow("Remapped Left Input Image", remappedLeftInputDisplay);
					}
					Mat remappedRightInputDisplay;
					remappedRightInputDisplay = visualSonificationInfo.data.currentCameraFrames[VISUAL_SONIFICATION_RIGHT_CAMERA];

					if(!remappedRightInputDisplay.empty())
					{
						imshow("Remapped Right Input Image", remappedRightInputDisplay);
					}

				}
				if(visualSonificationInfo.config.saveRemappedInputImage)
				{
					imwrite("lastLeftRemappedInput.png",visualSonificationInfo.data.currentCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA]); 
					imwrite("lastRightRemappedInput.png",visualSonificationInfo.data.currentCameraFrames[VISUAL_SONIFICATION_RIGHT_CAMERA]); 
				}



				if(visualSonificationInfo.config.showGrayscaleInputImage)
				{
					Mat grayLeftInputDisplay;
					grayLeftInputDisplay = visualSonificationInfo.data.currentGrayCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA];

					if(!grayLeftInputDisplay.empty())
					{
						imshow("Grayscale Left Input Image", grayLeftInputDisplay);
					}
					
					Mat grayRightInputDisplay;
					grayRightInputDisplay = visualSonificationInfo.data.currentGrayCameraFrames[VISUAL_SONIFICATION_RIGHT_CAMERA];

					if(!grayRightInputDisplay.empty())
					{
						imshow("Grayscale Right Input Image", grayRightInputDisplay);
					}
					
				}

				if(visualSonificationInfo.config.saveGrayscaleInputImage)
				{
					imwrite("lastGrayscaleLeft.png",visualSonificationInfo.data.currentGrayCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA]); 
					imwrite("lastGrayscaleRight.png",visualSonificationInfo.data.currentGrayCameraFrames[VISUAL_SONIFICATION_RIGHT_CAMERA]); 
				}

		
				visualSonificationInfo.data.currentState = VISUAL_SONIFICATION_STATE_SEGMENTATION;
				break;
			case VISUAL_SONIFICATION_STATE_SEGMENTATION:

				getStartClockTime(TIMING_EVENT_SEGMENT);

				if(!visualSonificationInfo.config.segmentUsingExternalMask)
				{
					//Call the segmentation of the input data
					visualSonificationSegmentInputs(visualSonificationInfo);
				}
				else
				{
					visualSonificationSegmentInputsWithExternalMask(visualSonificationInfo);

				}


				getEndClockTime(TIMING_EVENT_SEGMENT);

				visualSonificationInfo.data.currentState = VISUAL_SONIFICATION_STATE_FEATURE_EXTRACTION;
				break;
			case VISUAL_SONIFICATION_STATE_FEATURE_EXTRACTION:
//				extractionTimeStart = clock();
				getStartClockTime(TIMING_EVENT_FEATURE_EXTRACTION);

				//Perform the feature extraction
				visualSonificationExtractFeatures(visualSonificationInfo);
				getEndClockTime(TIMING_EVENT_FEATURE_EXTRACTION);
//				extractionTimeStop = clock();
//				cout << "Feature Extraction Time: "<< extractionTimeStop - extractionTimeStart << endl;

				//Show the requested information
				if(visualSonificationInfo.config.showFeaturesImage)
				{
					Mat grayLeftInput;
					grayLeftInput = visualSonificationInfo.data.currentGrayCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA];

					if(!grayLeftInput.empty())
					{
						Mat featureLeftInputDisplay;
						if(grayLeftInput.channels() ==3)
						{
							cvtColor(grayLeftInput,featureLeftInputDisplay,CV_BGR2GRAY);
						}
						else
						{
							grayLeftInput.copyTo(featureLeftInputDisplay);
						}
						Mat featureLeftInputDisplayWithFeatures;
						drawKeypoints(featureLeftInputDisplay,visualSonificationInfo.data.currentKeypoints[VISUAL_SONIFICATION_LEFT_CAMERA],featureLeftInputDisplayWithFeatures,Scalar(255.0,0), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);			
						
						imshow("GrayLeft",grayLeftInput);
						imshow("Left Features Image", featureLeftInputDisplayWithFeatures);
		
					}
		
				
				}
				visualSonificationInfo.data.currentState = VISUAL_SONIFICATION_STATE_HISTOGRAM_GENERATION;
				break;
			case VISUAL_SONIFICATION_STATE_HISTOGRAM_GENERATION:
				getStartClockTime(TIMING_EVENT_HISTOGRAM_GENERATION);

				//Create the histograms
				visualSonificationCreateHistograms(visualSonificationInfo);
				getEndClockTime(TIMING_EVENT_HISTOGRAM_GENERATION);
				visualSonificationInfo.data.currentState = VISUAL_SONIFICATION_STATE_SIGNTATURE_GENERATION;
				break;
			case VISUAL_SONIFICATION_STATE_SIGNTATURE_GENERATION:
				getStartClockTime(TIMING_EVENT_SIGNATURE_GENERATION);

				//Create the audio signatures
				visualSonificationCreateAudioSignatures(visualSonificationInfo);
				getEndClockTime(TIMING_EVENT_SIGNATURE_GENERATION);

				visualSonificationInfo.data.currentState = VISUAL_SONIFICATION_STATE_AUDIO_GENERATION;
				break;
			case VISUAL_SONIFICATION_STATE_AUDIO_GENERATION:
				getStartClockTime(TIMING_EVENT_AUDIO_GENERATION);

				//Play the audio signatures
				visualSonificationPlayAudioSignatures(visualSonificationInfo);
				getEndClockTime(TIMING_EVENT_AUDIO_GENERATION);
				visualSonificationInfo.data.currentState = VISUAL_SONIFICATION_STATE_DATA_CAPTURE;
				getEndClockTime(TIMING_EVENT_ALL);
				if( lastIteration)
				{
					done = true;
				}


#ifdef DISPLAY_TIMING_WINDOWS
				vuPrintTimingInformationToConsole();
#endif
#ifdef SAVE_TIMING
				vuPrintTimingInformationToFile();
#endif
				break;
			default:
				break;


		}

		cout << "State Complete.  Next State: " << (int)visualSonificationInfo.data.currentState << endl;


		//Handle pause or continuous run mode
		//Note that a waitKey is required to be called periodically to handle window/screen updates
		if(!visualSonificationInfo.config.pauseAfterEachStateLoop)
		{
			key= waitKey(33) & 0xff;
		}
		else
		{
			key= waitKey(0) & 0xff;
		}


		//Call the key handler
		handleKey(key, done,visualSonificationInfo);
	}

#endif


	if(visualSonificationInfo.config.useOpenAlToPlay)
	{
		visualSonificationInfo.data.audioSignatures.resize(0);
		auExit();
	}


	cout << "Visual Sonification Main Complete" << endl;
	return 0;
}
#endif

#ifndef ANDROID
void setupVisSonClass()
{
	visualSonificationObject.initSonificationProcessing();
	Mat testLeft(320,240,CV_8UC1);
	Mat testRight(320,240,CV_8UC1);
	testLeft.setTo(Scalar(128));
	testRight.setTo(Scalar(255));	
	visualSonificationObject.setImagesForProcessing(testLeft.data,testRight.data,320,240,testLeft.step[0],1,1);

	return;
	

}
int callClassSonClassRun()
{
	return visualSonificationObject.runSonificationProcessing();

}

void visualSonificationClassTechniqueOnPC()
{
	namedWindow("Control Window");
	namedWindow("Left Current Frame");
	namedWindow("Right Current Frame");
	namedWindow("Depth Current Frame");
	namedWindow("Feature Current Frame");
	int key = -1;
	bool done = false;
	int delayAmount = 100;
	setupVisSonClass();
	while(key != 'q' && !done)
	{
		cout <<"Starting State: " <<visualSonificationObject.getSonificationState() << endl;
		int imagesRemaining = callClassSonClassRun();
		if(imagesRemaining <0)
		{
			done = true;
			
		}
		if(imagesRemaining <=0 && visualSonificationObject.getSonificationState() ==1)
		{
			done = true;
			
		}

		cout <<"Next State: " <<visualSonificationObject.getSonificationState() << endl;
		if(visualSonificationObject.getSonificationState() == 6)
		{
			namedWindow("TestTime");
			waitKey(0);

			/*struct SonificationAudioBufferList * testList;

			testList = visualSonificationObject.getAudioBufferList();
			waitKey(0);
			VisualSonification::playSoundBufferList(testList);

			unsigned char * tmpBuffer;
			VisualSonification::flattenSoundBufferList(testList,&tmpBuffer);

			struct SonificationAudioBufferList reformedTestList;

			VisualSonification::unflattenSoundBufferList(tmpBuffer,&reformedTestList);

			waitKey(0);
			VisualSonification::playSoundBufferList(&reformedTestList);
			*/


			SonificationHistogramBufferList histBuffer = *visualSonificationObject.getHistogramBufferList();
			string filename = "/home/jlclemon/Downloads/BlazeBuild/mydroid/external/sonification/source/jni/configFileLinux.txt";//configFileAndroid.txt";

			struct HistogramSoundSource::AudioGenerationInformation audioGenerationConfigData =  loadAndParseAudioGenerationParams(filename);
			for(int i = 0; i < histBuffer.numberOfBuffersInUse; i++)
			{
				Mat displayImage, displayImage2;
				displayImage= visualSonificationObject.getSonificationImageData(VISUAL_SONIFICATION_LEFT_CAMERA,VISUAL_SONIFICATION_IMAGE_RAW_IMAGE);
				if(!displayImage.empty())
				{


					Scalar color(255,0,0);
					circle(displayImage,Point(histBuffer.buffers[i].locationInImage[0],histBuffer.buffers[i].locationInImage[1]),10,color,-1,8,0);
					imshow("Left Current Frame", displayImage);

					displayImage2= visualSonificationObject.getSonificationImageData(VISUAL_SONIFICATION_LEFT_CAMERA,VISUAL_SONIFICATION_IMAGE_CONNECTED_COMPONENT_LABELS_IMAGE);
					displayImage2  = convertToDisplayComponentResults(displayImage2);
					circle(displayImage2,Point(histBuffer.buffers[i].locationInImage[0],histBuffer.buffers[i].locationInImage[1]),10,color,-1,8,0);

					imshow("Connected Components Frame", displayImage2);
					waitKey(10);
					VisualSonification::playHistogramBuffer(histBuffer.buffers[i],audioGenerationConfigData);

				}

				waitKey(0);



			}



		}

		Mat displayImage;

		displayImage= visualSonificationObject.getSonificationImageData(VISUAL_SONIFICATION_LEFT_CAMERA,VISUAL_SONIFICATION_IMAGE_RAW_IMAGE);
		if(!displayImage.empty())
		{
			
		
			imshow("Left Current Frame", displayImage);
			cout << "Frames Remaining:" << imagesRemaining << endl; 			
		}

		displayImage= visualSonificationObject.getSonificationImageData(VISUAL_SONIFICATION_RIGHT_CAMERA,VISUAL_SONIFICATION_IMAGE_RAW_IMAGE);
		if(!displayImage.empty())
		{
					
			imshow("Right Current Frame", displayImage);

		}
		displayImage= visualSonificationObject.getSonificationImageData(VISUAL_SONIFICATION_LEFT_CAMERA,VISUAL_SONIFICATION_IMAGE_COLOR_DEPTH_IMAGE);
		if(!displayImage.empty())
		{
			imshow("Depth Current Frame", displayImage);
		}
		displayImage= visualSonificationObject.getSonificationImageData(VISUAL_SONIFICATION_LEFT_CAMERA,VISUAL_SONIFICATION_IMAGE_FEATURE_IMAGE);
		if(!displayImage.empty())
		{
			imshow("Feature Current Frame", displayImage);
		}
		displayImage= visualSonificationObject.getSonificationImageData(VISUAL_SONIFICATION_LEFT_CAMERA,VISUAL_SONIFICATION_IMAGE_GRAY_IMAGE);
		if(!displayImage.empty())
		{
			imshow("Left Current Gray Frame", displayImage);
		}
		displayImage= visualSonificationObject.getSonificationImageData(VISUAL_SONIFICATION_RIGHT_CAMERA,VISUAL_SONIFICATION_IMAGE_GRAY_IMAGE);
		if(!displayImage.empty())
		{
			imshow("Right Current Gray Frame", displayImage);
		}

		displayImage= visualSonificationObject.getSonificationImageData(VISUAL_SONIFICATION_LEFT_CAMERA,VISUAL_SONIFICATION_IMAGE_CONNECTED_COMPONENT_LABELS_IMAGE);
		if(!displayImage.empty())
		{

			displayImage  = convertToDisplayComponentResults(displayImage);

			imshow("Connected Components Frame", displayImage);
		}



/*		int width;
		int height;
		int stepInBytes;
		unsigned char * currentImageData = visualSonificationObject.getSonificationImageData(VISUAL_SONIFICATION_LEFT_CAMERA,VISUAL_SONIFICATION_IMAGE_RAW_IMAGE, width, height, stepInBytes);
		if(currentImageData != NULL)
		{
			Mat displayImage(height,width,CV_8UC3,currentImageData,stepInBytes);
		
			imshow("Left Current Frame", displayImage);
			cout << "Frames Remaining:" << imagesRemaining << endl; 			
		}

		currentImageData = visualSonificationObject.getSonificationImageData(VISUAL_SONIFICATION_RIGHT_CAMERA,VISUAL_SONIFICATION_IMAGE_RAW_IMAGE, width, height, stepInBytes);
		if(currentImageData != NULL)
		{
			Mat displayImage(height,width,CV_8UC3,currentImageData,stepInBytes);
		
			imshow("Right Current Frame", displayImage);

		}
		currentImageData = visualSonificationObject.getSonificationImageData(VISUAL_SONIFICATION_RIGHT_CAMERA,VISUAL_SONIFICATION_IMAGE_COLOR_DEPTH_IMAGE, width, height, stepInBytes);
		if(currentImageData != NULL)
		{
			Mat displayImage(height,width,CV_16SC1,currentImageData,stepInBytes);
			Mat colorDisparity(height,width,CV_8UC3);
			vuColorDisparityMap(&displayImage, &colorDisparity,(float)96, (float)16);


			imshow("Depth Current Frame", colorDisparity);

		}
		currentImageData = visualSonificationObject.getSonificationImageData(VISUAL_SONIFICATION_RIGHT_CAMERA,VISUAL_SONIFICATION_IMAGE_FEATURE_IMAGE, width, height, stepInBytes);
		if(currentImageData != NULL)
		{
			Mat displayImage(height,width,CV_8UC3,currentImageData,stepInBytes);
		
			imshow("Feature Current Frame", displayImage);

		}

*/

		key = 0xFF & waitKey(delayAmount);

		if(key == 'p')
		{

			key = 0xFF & waitKey(0);

		}

	}

	destroyWindow("Control Window");


	exit(0);
	return;
}
#endif
//****************************Below is the class wrapper for visual sonification**********************************//

//Constructor
VisualSonification::VisualSonification()
{

	this->visualSonificationInfo.data.visualSonificationObjectSelfPtr = (void *)this;

	visualSonificationInfo.checkStackStart = 0xA5A5A5A5;
	visualSonificationInfo.checkStackEnd = 0x5A5A5A5A;
	visualSonificationInfo.data.checkStackStart = 0xA5A5A5A5;
	visualSonificationInfo.data.checkStackEnd = 0x5A5A5A5A;
	//cout << "Here Stacks:  " << visualSonificationInfo.checkStackStart<< "," << visualSonificationInfo.checkStackEnd<< "," << visualSonificationInfo.data.checkStackStart << ","<<visualSonificationInfo.checkStackEnd << endl;
   // VisionCam * m_pCam = VisionCamFactory(VISIONCAM_OMX);
//	m_pCam->init(this);
}

//Desctuctor
VisualSonification::~VisualSonification()
{



}

	//Buffer List = Buffer Count  | Buffer Data|
	//Buffer Data = SampleRate|x|y|z|BufferSize(inBytes)|Buffer
	//#define BUFFER_LIST_COUNT_LOCATION 0
	//#define BUFFER_LIST_COUNT_SIZE (sizeof(int16_t))

	//#define BUFFER_DATA_START (BUFFER_LIST_COUNT_LOCATION+BUFFER_LIST_COUNT_SIZE)

	//#define BUFFER_DATA_SAMPLE_RATE_START 0
	//#define BUFFER_DATA_SAMPLE_RATE_

	//#define BUFFR_DATA_LOCATION_START 0
	//#define BUFFR_DATA_LOCATION_STEP (sizeof(int16_t))
	//#define BUFFR_DATA_LOCATION_X (BUFFR_DATA_LOCATION_START)
	//#define BUFFR_DATA_LOCATION_Y (BUFFR_DATA_LOCATION_X+BUFFR_DATA_LOCATION_STEP)
	//#define BUFFR_DATA_LOCATION_Z (BUFFR_DATA_LOCATION_Y+BUFFR_DATA_LOCATION_STEP)
	



void VisualSonification::flattenSoundBufferList(struct SonificationAudioBufferList *listIn, unsigned char ** bufferOut)
{
		*bufferOut = (unsigned char *)calloc(1,sizeof(struct SonificationAudioBufferList));
		memcpy(*bufferOut,listIn,sizeof(struct SonificationAudioBufferList));
}
void VisualSonification::flattenSoundBuffer (struct SonificationAudioBuffer * bufferIn, unsigned char ** bufferOut)
{	
		*bufferOut = (unsigned char *)calloc(1,sizeof(struct SonificationAudioBuffer));
		memcpy(*bufferOut,bufferIn,sizeof(struct SonificationAudioBuffer));						
}
uint32_t VisualSonification::getflattenedSoundBufferListSize (unsigned char * bufferOut)
{
		
		if(bufferOut != NULL)
		{
			return (uint32_t)sizeof(struct SonificationAudioBufferList);
		}
		else
		{
			return 0;
		}
}

void VisualSonification::unflattenSoundBufferList(unsigned char * bufferIn,struct SonificationAudioBufferList *listOut)
{
	
		
		memcpy(listOut,bufferIn,sizeof(struct SonificationAudioBufferList));
	
	
}





void VisualSonification::unflattenSoundBuffer ( unsigned char * bufferIn,struct SonificationAudioBuffer * bufferOut)
{
		memcpy(bufferOut,bufferIn,sizeof(struct SonificationAudioBuffer));				
	
}


void VisualSonification::playSoundBufferList ( SonificationAudioBufferList * bufferList)
{

	int velocity[3] = {0,0,0};


	for(int i = 0; i < bufferList->numberOfBuffersInUse && i < bufferList->maxNumberOfBuffers; i++)
	{

		HistogramSoundSource::playWaveBufferData(bufferList->buffers[i].sampleRateInHz, bufferList->buffers[i].data, bufferList->buffers[i].numberOfSamplesInUse, bufferList->buffers[i].location, velocity, true, true);


	}


}

void VisualSonification::unflattenHistogramBufferList(unsigned char * bufferIn,struct SonificationHistogramBufferList *listOut)
{
	
		
		memcpy(listOut,bufferIn,sizeof(struct SonificationHistogramBufferList));
		
}


void VisualSonification::playHistogramBufferList ( SonificationHistogramBufferList * bufferList,HistogramSoundSource::AudioGenerationInformation & audioGenerationConfigData)
{
	
	SonificationAudioBuffer * tmpAudioBuffer = (SonificationAudioBuffer *)calloc(1,sizeof(SonificationAudioBuffer));
	int velocity[3] = {0,0,0};
	int sampleRateInHz = audioGenerationConfigData.sampleFrequency;//44100;


	for(int i= 0; i < bufferList->numberOfBuffersInUse; i++)
	{

			
		//bufferList->buffers[i].histogramEntriesInUse;
		unsigned int waveBufferLength = SONIFICATION_MAX_AUDIO_SAMPLES_PER_BUFFER;
		HistogramSoundSource::generateWaveBufferDataFromHistogram(bufferList->buffers[i].histogramEntriesInUse, &(bufferList->buffers[i].data[0]),audioGenerationConfigData.histogramOutputTotalTime,audioGenerationConfigData.sampleFrequency,	audioGenerationConfigData.maxFrequencyInHz, audioGenerationConfigData.minFrequencyInHz, 10.0f,true,audioGenerationConfigData.currentSonificationTechnique,tmpAudioBuffer->data, waveBufferLength);
		tmpAudioBuffer->sampleRateInHz =sampleRateInHz;
		tmpAudioBuffer->numberOfSamplesInUse = waveBufferLength;
		tmpAudioBuffer->location[0] = bufferList->buffers[i].location[0];
		tmpAudioBuffer->location[1] = bufferList->buffers[i].location[1];
		tmpAudioBuffer->location[2] = bufferList->buffers[i].location[2];
		HistogramSoundSource::playWaveBufferData(tmpAudioBuffer->sampleRateInHz, tmpAudioBuffer->data, tmpAudioBuffer->numberOfSamplesInUse, tmpAudioBuffer->location, velocity, true, true);
		
		
	}	
	free(tmpAudioBuffer);
}

void VisualSonification::playHistogramBuffer ( struct SonificationHistogramBuffer & buffer,HistogramSoundSource::AudioGenerationInformation &audioGenerationConfigData)
{
	SonificationAudioBuffer * tmpAudioBuffer = (SonificationAudioBuffer *)calloc(1,sizeof(SonificationAudioBuffer));
	int velocity[3] = {0,0,0};
	int sampleRateInHz = audioGenerationConfigData.sampleFrequency;//44100;



	unsigned int waveBufferLength = SONIFICATION_MAX_AUDIO_SAMPLES_PER_BUFFER;



	//HistogramSoundSource::generateWaveBufferDataFromHistogram(buffer.histogramEntriesInUse, &(buffer.data[0]),sampleRateInHz,audioGenerationConfigData.histogramOutputTotalTime,audioGenerationConfigData.maxFrequencyInHz, audioGenerationConfigData.minFrequencyInHz, 10.0f,true,false,tmpAudioBuffer->data, waveBufferLength);
	HistogramSoundSource::generateWaveBufferDataFromHistogram(buffer.histogramEntriesInUse, &(buffer.data[0]),audioGenerationConfigData.histogramOutputTotalTime,audioGenerationConfigData.sampleFrequency,audioGenerationConfigData.maxFrequencyInHz, audioGenerationConfigData.minFrequencyInHz, 10.0f,true,audioGenerationConfigData.currentSonificationTechnique,tmpAudioBuffer->data, waveBufferLength);
	tmpAudioBuffer->sampleRateInHz =sampleRateInHz;
	tmpAudioBuffer->numberOfSamplesInUse = waveBufferLength;
	tmpAudioBuffer->location[0] = buffer.location[0];
	tmpAudioBuffer->location[1] = buffer.location[1];
	tmpAudioBuffer->location[2] = buffer.location[2];
#ifdef ANDROID

	LOGD("Play histogram buffer Sample Rate In Hz: %d, Samples In Use: %d", sampleRateInHz,waveBufferLength);

#endif


	HistogramSoundSource::playWaveBufferData(tmpAudioBuffer->sampleRateInHz, tmpAudioBuffer->data, tmpAudioBuffer->numberOfSamplesInUse, tmpAudioBuffer->location, velocity, true, true);
	free(tmpAudioBuffer);


}

void VisualSonification::saveHistogramBufferAudioFile(string filename, struct SonificationHistogramBuffer & buffer, enum HistogramSoundSource::SoundGenerationTechniques technique, HistogramSoundSource::AudioGenerationInformation &audioGenerationConfigData)
{

	SonificationAudioBuffer * tmpAudioBuffer = (SonificationAudioBuffer *)calloc(1,sizeof(SonificationAudioBuffer));
	int velocity[3] = {0,0,0};
	int sampleRateInHz = audioGenerationConfigData.sampleFrequency;//44100;


	unsigned int waveBufferLength = SONIFICATION_MAX_AUDIO_SAMPLES_PER_BUFFER;
	HistogramSoundSource::generateWaveBufferDataFromHistogram(buffer.histogramEntriesInUse, &(buffer.data[0]),audioGenerationConfigData.histogramOutputTotalTime,audioGenerationConfigData.sampleFrequency,audioGenerationConfigData.maxFrequencyInHz, audioGenerationConfigData.minFrequencyInHz, 10.0f,true,technique,tmpAudioBuffer->data, waveBufferLength);
	tmpAudioBuffer->sampleRateInHz =sampleRateInHz;
	tmpAudioBuffer->numberOfSamplesInUse = waveBufferLength;
	tmpAudioBuffer->location[0] = buffer.location[0];
	tmpAudioBuffer->location[1] = buffer.location[1];
	tmpAudioBuffer->location[2] = buffer.location[2];
	HistogramSoundSource::saveWaveBufferDataToAudioFile(filename,tmpAudioBuffer->sampleRateInHz, tmpAudioBuffer->data, tmpAudioBuffer->numberOfSamplesInUse);
	//HistogramSoundSource::playWaveBufferData(tmpAudioBuffer->sampleRateInHz, tmpAudioBuffer->data, tmpAudioBuffer->numberOfSamplesInUse, tmpAudioBuffer->location, velocity, true, true);
	free(tmpAudioBuffer);



}

void VisualSonification::saveHistogramToFile(string filename, struct SonificationHistogramBuffer & buffer)
{
	ofstream outputFile;
	outputFile.open(filename.c_str(),ios_base::out);
	if(outputFile.is_open())
	{
		//outputFile << buffer.location[0] << "," << buffer.location[1] << "," << buffer.location[0]
		outputFile << buffer.numberOfFeaturesUsedToMakeHistogram << "\n";
		outputFile << buffer.histogramEntriesInUse;
		for(int i = 0; i < buffer.histogramEntriesInUse; i++)
		{
			outputFile <<"\n" <<buffer.data[i];

		}
	}
	else
	{
		cout << "WARNING: Unable to open histogram output file: " << filename << endl;

	}

	outputFile.close();



}


SonificationAudioBufferList * VisualSonification::getAudioBufferList()
{

	return &(this->visualSonificationInfo.data.audioBuffers);



}
SonificationHistogramBufferList * VisualSonification::getHistogramBufferList()
{

	return &(this->visualSonificationInfo.data.histogramBuffers);



}



//
//Desc: This function intializes the visual sonification for the class inclduing loading the configuration file
//
//Return: true if more images remain, false otherwise

void VisualSonification::initSonificationProcessing()
{

	infoForMouse = &visualSonificationInfo;
	visualSonificationInfo.data.currentState = VISUAL_SONIFICATION_STATE_INIT;

//	testVuFunctions();
//	testConnectedComponent();
//	return 0;
	vector<string> commandArgs;

	//Setup the command string to go get the config file
	commandArgs.push_back(string(DEFAULT_CONFIG_FILE_COMMAND));
	commandArgs.push_back(string(DEFAULT_CONFIG_FILE_LOCATION));
	parseParamsVector(commandArgs,visualSonificationInfo);

	visualSonificationInit(visualSonificationInfo);
	


}


//
//Desc: This function intializes the visual sonification for the class inclduing loading the configuration file
//
//Return: true if more images remain, false otherwise

void VisualSonification::initSonificationProcessing(vector<string> commandArgs)
{

	infoForMouse = &visualSonificationInfo;
	visualSonificationInfo.data.currentState = VISUAL_SONIFICATION_STATE_INIT;

//	testVuFunctions();
//	testConnectedComponent();
//	return 0;

	//Setup the command string to go get the config file
	parseParamsVector(commandArgs,visualSonificationInfo);

	visualSonificationInit(visualSonificationInfo);
	


}


int singleSteponificationProcessing(VisualSonificationInfo & visualSonificationInfo)
{
	static int numberOfImagesRemaining = 2;
	//bool returnVal = false;

	//cout << "Stacks:  " << visualSonificationInfo.checkStackStart<< "," << visualSonificationInfo.checkStackEnd<< "," << visualSonificationInfo.data.checkStackStart << ","<<visualSonificationInfo.checkStackEnd << endl;
	switch(visualSonificationInfo.data.currentState)
	{
		case VISUAL_SONIFICATION_STATE_INIT:
			vuInitTimingInformation();

			getStartClockTime(TIMING_EVENT_INIT);


			//visualSonificationInit(visualSonificationInfo);


			getEndClockTime(TIMING_EVENT_INIT);
			if(visualSonificationInfo.config.pauseAfterEachStateLoop)
			{
				visualSonificationInfo.data.currentState = VISUAL_SONIFICATION_STATE_DATA_CAPTURE;
			
				break;
			}
		case VISUAL_SONIFICATION_STATE_DATA_CAPTURE:
			getStartClockTime(TIMING_EVENT_ALL);
			getStartClockTime(TIMING_EVENT_CAPTURE_IMAGE);
			numberOfImagesRemaining = getNextImages(visualSonificationInfo);
			getEndClockTime(TIMING_EVENT_CAPTURE_IMAGE);
			if(numberOfImagesRemaining <0)
			{
				break;
			}


			if(visualSonificationInfo.config.saveRawDepthImage)
			{
				Mat rawDepthDisplay;
				visualSonificationInfo.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].convertTo(rawDepthDisplay,CV_8U,255.0/((double)(visualSonificationInfo.data.stereoBlockMatcher.state->numberOfDisparities* 16)));					

#ifdef ANDROID
				imwrite("/sdcard/lastRawDepthImage.png",rawDepthDisplay);

#else
				imwrite("lastRawDepthImage.png",rawDepthDisplay);

#endif


			}
			if(visualSonificationInfo.config.save3dPoints)
			{
#ifdef ANDROID
				FileStorage fs("/sdcard/last3dPoints.yml", FileStorage::WRITE);
#else
				FileStorage fs("last3dPoints.yml", FileStorage::WRITE);

#endif

				fs << "3dPoints" << visualSonificationInfo.data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA];
				fs.release();
				ofstream text3dPoints;

#ifdef ANDROID
				text3dPoints.open("/sdcard/last3dPoints.txt");
#else
				text3dPoints.open("last3dPoints.txt");
#endif

				const double max_z = 1.0e4;

				for(int y = 0; y < visualSonificationInfo.data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].rows; y++)
				{
					for(int x = 0; x < visualSonificationInfo.data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].cols; x++)
					{
						Vec3f point = visualSonificationInfo.data.currentCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].at<Vec3f>(y, x);
						if(fabs(point[2] - max_z) < FLT_EPSILON || fabs(point[2]) > max_z)
						{
							continue;
						}
						text3dPoints <<point[0]<<" "<< point[1]<<" " <<point[2] << endl;
					}
				}

				text3dPoints.close();


			}



			if(visualSonificationInfo.config.saveRawInputImage)
			{
#ifdef ANDROID
				imwrite("/sdcard/lastLeftRawInput.png",visualSonificationInfo.data.currentRawCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA]); 
				imwrite("/sdcard/lastRightRawInput.png",visualSonificationInfo.data.currentRawCameraFrames[VISUAL_SONIFICATION_RIGHT_CAMERA]); 

#else
				imwrite("lastLeftRawInput.png",visualSonificationInfo.data.currentRawCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA]); 
				imwrite("lastRightRawInput.png",visualSonificationInfo.data.currentRawCameraFrames[VISUAL_SONIFICATION_RIGHT_CAMERA]); 

#endif

			}



			if(visualSonificationInfo.config.saveRemappedInputImage)
			{

#ifdef ANDROID
				imwrite("/sdcard/lastLeftRemappedInput.png",visualSonificationInfo.data.currentCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA]); 
				imwrite("/sdcard/lastRightRemappedInput.png",visualSonificationInfo.data.currentCameraFrames[VISUAL_SONIFICATION_RIGHT_CAMERA]); 

#else
				imwrite("lastLeftRemappedInput.png",visualSonificationInfo.data.currentCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA]); 
				imwrite("lastRightRemappedInput.png",visualSonificationInfo.data.currentCameraFrames[VISUAL_SONIFICATION_RIGHT_CAMERA]); 


#endif

			}




			if(visualSonificationInfo.config.saveGrayscaleInputImage)
			{

#ifdef ANDROID
				imwrite("/sdcard/lastGrayscaleLeft.png",visualSonificationInfo.data.currentGrayCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA]); 
				imwrite("/sdcard/lastGrayscaleRight.png",visualSonificationInfo.data.currentGrayCameraFrames[VISUAL_SONIFICATION_RIGHT_CAMERA]); 
#else
				imwrite("lastGrayscaleLeft.png",visualSonificationInfo.data.currentGrayCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA]); 
				imwrite("lastGrayscaleRight.png",visualSonificationInfo.data.currentGrayCameraFrames[VISUAL_SONIFICATION_RIGHT_CAMERA]); 


#endif

			}

			if(visualSonificationInfo.config.pauseAfterEachStateLoop)
			{
				visualSonificationInfo.data.currentState = VISUAL_SONIFICATION_STATE_SEGMENTATION;
			
				break;
			}
		case VISUAL_SONIFICATION_STATE_SEGMENTATION:

			getStartClockTime(TIMING_EVENT_SEGMENT);
			visualSonificationSegmentInputs(visualSonificationInfo);
			getEndClockTime(TIMING_EVENT_SEGMENT);
			if(visualSonificationInfo.config.pauseAfterEachStateLoop)
			{
				visualSonificationInfo.data.currentState = VISUAL_SONIFICATION_STATE_FEATURE_EXTRACTION;
			
				break;
			}
		case VISUAL_SONIFICATION_STATE_FEATURE_EXTRACTION:

			getStartClockTime(TIMING_EVENT_FEATURE_EXTRACTION);
			visualSonificationExtractFeatures(visualSonificationInfo);
			getEndClockTime(TIMING_EVENT_FEATURE_EXTRACTION);
			if(visualSonificationInfo.config.pauseAfterEachStateLoop)
			{
				visualSonificationInfo.data.currentState = VISUAL_SONIFICATION_STATE_HISTOGRAM_GENERATION;
			
				break;
			}
		case VISUAL_SONIFICATION_STATE_HISTOGRAM_GENERATION:
			getStartClockTime(TIMING_EVENT_HISTOGRAM_GENERATION);
			visualSonificationCreateHistograms(visualSonificationInfo);
			getEndClockTime(TIMING_EVENT_HISTOGRAM_GENERATION);
			if(visualSonificationInfo.config.pauseAfterEachStateLoop)
			{
				visualSonificationInfo.data.currentState = VISUAL_SONIFICATION_STATE_SIGNTATURE_GENERATION;
			
				break;
			}
		case VISUAL_SONIFICATION_STATE_SIGNTATURE_GENERATION:
			getStartClockTime(TIMING_EVENT_SIGNATURE_GENERATION);
			visualSonificationCreateAudioSignatures(visualSonificationInfo);
			getEndClockTime(TIMING_EVENT_SIGNATURE_GENERATION);
			if(visualSonificationInfo.config.pauseAfterEachStateLoop)
			{
				visualSonificationInfo.data.currentState = VISUAL_SONIFICATION_STATE_AUDIO_GENERATION;
			
				break;
			}
		case VISUAL_SONIFICATION_STATE_AUDIO_GENERATION:
			getStartClockTime(TIMING_EVENT_AUDIO_GENERATION);

			if(visualSonificationInfo.config.useOpenAlToPlay)
			{

				visualSonificationPlayAudioSignatures(visualSonificationInfo);
			}

			getEndClockTime(TIMING_EVENT_AUDIO_GENERATION);
			visualSonificationInfo.data.currentState = VISUAL_SONIFICATION_STATE_DATA_CAPTURE;
			
			getEndClockTime(TIMING_EVENT_ALL);
			//vuPrintTimingInformationToConsole();
			vuPrintTimingInformationToFile();
			break;
		default:
			break;


	}
#ifndef ANDROID
		cout << "Current State: " << visualSonificationInfo.data.currentState << endl;
#endif
	//cout << "Stacks:  " << visualSonificationInfo.checkStackStart<< "," << visualSonificationInfo.checkStackEnd<< "," << visualSonificationInfo.data.checkStackStart << ","<<visualSonificationInfo.checkStackEnd << endl;
	return numberOfImagesRemaining;
	
	
	
	
	
}





//
//Desc: This function runs the visual sonification through one step or loop from the class
//
//Return: number of images remaining after this loop through (negative means this current loop had no new image)

int VisualSonification::runSonificationProcessing()
{
	return singleSteponificationProcessing(visualSonificationInfo);
}


//Desc: This function cleans up after the class
//
//Return: true if cleaned up, false if error

bool VisualSonification::endSonificationProcessing()
{

	bool returnVal = true;
	if(visualSonificationInfo.config.useOpenAlToPlay)
	{
		//Clear the audio engine
		returnVal = (auExit()==AL_FALSE? false:true);
	}
	
	return returnVal;

}


//Desc: This function returns a pointer to the data of the requested image
//  the image ID is from an enum of images and the cameraId is from the camera type enum
//Return: unsigned char * pointer to iamge data

unsigned char * VisualSonification::getSonificationImageData(int cameraId,int imageId, int& width, int &height, int &stepInBytes)
{
	unsigned char * returnVal = NULL;
	switch(imageId)
	{
		case VISUAL_SONIFICATION_IMAGE_CAMERA_BUFFER:
			returnVal = this->visualSonificationInfo.data.currentCameraBuffers[cameraId].data;	
			height = this->visualSonificationInfo.data.currentCameraBuffers[cameraId].rows;
			width = this->visualSonificationInfo.data.currentCameraBuffers[cameraId].cols;
			stepInBytes = this->visualSonificationInfo.data.currentCameraBuffers[cameraId].step[0];
			break;
		case VISUAL_SONIFICATION_IMAGE_REMAPPED_IMAGE:
			returnVal = visualSonificationInfo.data.currentCameraFrames[cameraId].data;
			height = this->visualSonificationInfo.data.currentCameraFrames[cameraId].rows;
			width = this->visualSonificationInfo.data.currentCameraFrames[cameraId].cols;
			stepInBytes = this->visualSonificationInfo.data.currentCameraFrames[cameraId].step[0];

			break;
		case VISUAL_SONIFICATION_IMAGE_GRAY_IMAGE:
			returnVal = visualSonificationInfo.data.currentGrayCameraFrames[cameraId].data;
			height = this->visualSonificationInfo.data.currentGrayCameraFrames[cameraId].rows;
			width = this->visualSonificationInfo.data.currentGrayCameraFrames[cameraId].cols;
			stepInBytes = this->visualSonificationInfo.data.currentGrayCameraFrames[cameraId].step[0];

 
			break;
		case VISUAL_SONIFICATION_IMAGE_FEATURE_IMAGE:
		{
			Mat grayLeftInput;
			grayLeftInput = visualSonificationInfo.data.currentGrayCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA];

			if(!grayLeftInput.empty())
			{
				Mat featureLeftInputDisplay;
				if(grayLeftInput.channels() ==3)
				{
					cvtColor(grayLeftInput,featureLeftInputDisplay,CV_BGR2GRAY);
				}
				else
				{
					grayLeftInput.copyTo(featureLeftInputDisplay);
				}
				
				drawKeypoints(featureLeftInputDisplay,visualSonificationInfo.data.currentKeypoints[VISUAL_SONIFICATION_LEFT_CAMERA],visualSonificationInfo.data.currentFeatureCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA],Scalar(255.0,0), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);			
						
		
			}
		
				

		}


			returnVal = visualSonificationInfo.data.currentFeatureCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA].data;
			height = this->visualSonificationInfo.data.currentFeatureCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA].rows;
			width = this->visualSonificationInfo.data.currentFeatureCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA].cols;
			stepInBytes = this->visualSonificationInfo.data.currentFeatureCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA].step[0];

			break;
		case VISUAL_SONIFICATION_IMAGE_COLOR_DEPTH_IMAGE:
			{

				Mat colorDisparity(visualSonificationInfo.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].rows,visualSonificationInfo.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].cols,CV_8UC3);
				vuColorDisparityMap(&visualSonificationInfo.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA], &colorDisparity,(float)visualSonificationInfo.data.stereoBlockMatcher.state->numberOfDisparities, (float)16);
				this->visualSonificationInfo.data.currentColorDepthFrame = colorDisparity;			
				returnVal = visualSonificationInfo.data.currentColorDepthFrame.data;
				height = this->visualSonificationInfo.data.currentColorDepthFrame.rows;
				width = this->visualSonificationInfo.data.currentColorDepthFrame.cols;
				stepInBytes = this->visualSonificationInfo.data.currentColorDepthFrame.step[0];
			}

			break;
		case VISUAL_SONIFICATION_IMAGE_AUDIO_DISPLAY_IMAGE:

			break;
		case VISUAL_SONIFICATION_IMAGE_RAW_IMAGE:
		default:

			returnVal = visualSonificationInfo.data.currentRawCameraFrames[cameraId].data;
			height = this->visualSonificationInfo.data.currentRawCameraFrames[cameraId].rows;
			width = this->visualSonificationInfo.data.currentRawCameraFrames[cameraId].cols;
			stepInBytes = this->visualSonificationInfo.data.currentRawCameraFrames[cameraId].step[0];


			break;
	
	};


	return returnVal;

}


//Get an image output from sonification
Mat VisualSonification::getSonificationImageData(int cameraId,int imageId)
{
	
	Mat imageData;

	switch(imageId)
	{
		case VISUAL_SONIFICATION_IMAGE_CAMERA_BUFFER:
			imageData =  this->visualSonificationInfo.data.currentCameraBuffers[cameraId].clone();	

			break;
		case VISUAL_SONIFICATION_IMAGE_REMAPPED_IMAGE:
			imageData =  visualSonificationInfo.data.currentCameraFrames[cameraId].clone();

			break;
		case VISUAL_SONIFICATION_IMAGE_GRAY_IMAGE:
			imageData =  visualSonificationInfo.data.currentGrayCameraFrames[cameraId].clone();

 
			break;
		case VISUAL_SONIFICATION_IMAGE_FEATURE_IMAGE:
		{
			Mat grayLeftInput;
			grayLeftInput = visualSonificationInfo.data.currentGrayCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA];

			if(!grayLeftInput.empty())
			{
				Mat featureLeftInputDisplay;
				if(grayLeftInput.channels() ==3)
				{
					cvtColor(grayLeftInput,featureLeftInputDisplay,CV_BGR2GRAY);
				}
				else
				{
					grayLeftInput.copyTo(featureLeftInputDisplay);
				}
				
				drawKeypoints(featureLeftInputDisplay,visualSonificationInfo.data.currentKeypoints[VISUAL_SONIFICATION_LEFT_CAMERA],visualSonificationInfo.data.currentFeatureCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA],Scalar(255.0,0), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);			
						
		
			}
		
				

		


			imageData =  visualSonificationInfo.data.currentFeatureCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA].clone();
		
			break;
			
		}
		case VISUAL_SONIFICATION_IMAGE_COLOR_DEPTH_IMAGE:
		{

			Mat colorDisparity(visualSonificationInfo.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].rows,visualSonificationInfo.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].cols,CV_8UC3);
			vuColorDisparityMap(&visualSonificationInfo.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA], &colorDisparity,(float)visualSonificationInfo.data.stereoBlockMatcher.state->numberOfDisparities, (float)16);
			this->visualSonificationInfo.data.currentColorDepthFrame = colorDisparity;			
			imageData = visualSonificationInfo.data.currentColorDepthFrame.clone();
			

			break;
		}
		case VISUAL_SONIFICATION_IMAGE_AUDIO_DISPLAY_IMAGE:

			break;

		case VISUAL_SONIFICATION_IMAGE_CONNECTED_COMPONENT_LABELS_IMAGE:

			imageData = visualSonificationInfo.data.regionLabelsImages[VISUAL_SONIFICATION_LEFT_CAMERA].clone();

			break;
		case VISUAL_SONIFICATION_IMAGE_RAW_IMAGE:
			default:

			imageData = visualSonificationInfo.data.currentRawCameraFrames[cameraId].clone();


			break;
	
	};


	return imageData;
	
	

}

//Get an image output from sonification
void VisualSonification::getSonificationImageData(int cameraId,int imageId, Mat &imageData)
{

	//Mat imageData;

	switch(imageId)
	{
		case VISUAL_SONIFICATION_IMAGE_CAMERA_BUFFER:
			imageData =  this->visualSonificationInfo.data.currentCameraBuffers[cameraId].clone();

			break;
		case VISUAL_SONIFICATION_IMAGE_REMAPPED_IMAGE:
			imageData =  visualSonificationInfo.data.currentCameraFrames[cameraId].clone();

			break;
		case VISUAL_SONIFICATION_IMAGE_GRAY_IMAGE:
			imageData =  visualSonificationInfo.data.currentGrayCameraFrames[cameraId].clone();


			break;
		case VISUAL_SONIFICATION_IMAGE_FEATURE_IMAGE:
		{
			Mat grayLeftInput;
			grayLeftInput = visualSonificationInfo.data.currentGrayCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA];

			if(!grayLeftInput.empty())
			{
				Mat featureLeftInputDisplay;
				if(grayLeftInput.channels() ==3)
				{
					cvtColor(grayLeftInput,featureLeftInputDisplay,CV_BGR2GRAY);
				}
				else
				{
					grayLeftInput.copyTo(featureLeftInputDisplay);
				}

				drawKeypoints(featureLeftInputDisplay,visualSonificationInfo.data.currentKeypoints[VISUAL_SONIFICATION_LEFT_CAMERA],visualSonificationInfo.data.currentFeatureCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA],Scalar(255.0,0), DrawMatchesFlags::DRAW_RICH_KEYPOINTS);


			}






			imageData =  visualSonificationInfo.data.currentFeatureCameraFrames[VISUAL_SONIFICATION_LEFT_CAMERA].clone();

			break;

		}
		case VISUAL_SONIFICATION_IMAGE_COLOR_DEPTH_IMAGE:
		{

			Mat colorDisparity(visualSonificationInfo.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].rows,visualSonificationInfo.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA].cols,CV_8UC3);
			vuColorDisparityMap(&visualSonificationInfo.data.currentRawCameraFrames[VISUAL_SONIFICATION_DEPTH_CAMERA], &colorDisparity,(float)visualSonificationInfo.data.stereoBlockMatcher.state->numberOfDisparities, (float)16);
			this->visualSonificationInfo.data.currentColorDepthFrame = colorDisparity;
			imageData = visualSonificationInfo.data.currentColorDepthFrame.clone();


			break;
		}
		case VISUAL_SONIFICATION_IMAGE_AUDIO_DISPLAY_IMAGE:

			break;

		case VISUAL_SONIFICATION_IMAGE_CONNECTED_COMPONENT_LABELS_IMAGE:

			imageData = visualSonificationInfo.data.regionLabelsImages[VISUAL_SONIFICATION_LEFT_CAMERA].clone();

			break;
		case VISUAL_SONIFICATION_IMAGE_RAW_IMAGE:
			default:

			imageData = visualSonificationInfo.data.currentRawCameraFrames[cameraId].clone();


			break;

	};






}




//Desc: This function returns the current state
//
//Return: None

int VisualSonification::getSonificationState()
{
	return this->visualSonificationInfo.data.currentState;
}


bool VisualSonification::setImagesForProcessing(unsigned char * leftImageData,unsigned char * rightImageData, int width, int height, int yStepInBytes,int xStepInBytes, int channels)
{




	this->visualSonificationInfo.data.currentCameraBuffers[VISUAL_SONIFICATION_LEFT_CAMERA].create(height,width,CV_8UC3);
	this->visualSonificationInfo.data.currentCameraBuffers[VISUAL_SONIFICATION_RIGHT_CAMERA].create(height,width,CV_8UC3);



	for(int y =0; y < height; y++)
	{
		unsigned char * leftBufferRowPtr = this->visualSonificationInfo.data.currentCameraBuffers[VISUAL_SONIFICATION_LEFT_CAMERA].data + y* this->visualSonificationInfo.data.currentCameraBuffers[VISUAL_SONIFICATION_LEFT_CAMERA].step[0];
		unsigned char * rightBufferRowPtr = this->visualSonificationInfo.data.currentCameraBuffers[VISUAL_SONIFICATION_RIGHT_CAMERA].data + y* this->visualSonificationInfo.data.currentCameraBuffers[VISUAL_SONIFICATION_RIGHT_CAMERA].step[0];
		unsigned char * leftImageDataRowPtr = leftImageData + y* yStepInBytes;
		unsigned char * rightImageDataRowPtr = rightImageData + y* yStepInBytes;
		for(int x =0; x < width; x++)
		{

			if(channels ==3)
			{
				for(int i = 0; i < channels; i++)
				{

					
					leftBufferRowPtr[x*3 + i] = leftImageDataRowPtr[x*xStepInBytes+i];
					rightBufferRowPtr[x*3 + i] = rightImageDataRowPtr[x*xStepInBytes+i];

				}
			}
			else if (channels ==1)
			{
				
	

					
					leftBufferRowPtr[x*3 + 0] = leftImageDataRowPtr[x*xStepInBytes];
					rightBufferRowPtr[x*3 + 0] = rightImageDataRowPtr[x*xStepInBytes];
					leftBufferRowPtr[x*3 + 1] = leftImageDataRowPtr[x*xStepInBytes];
					rightBufferRowPtr[x*3 + 1] = rightImageDataRowPtr[x*xStepInBytes];
					leftBufferRowPtr[x*3 + 2] = leftImageDataRowPtr[x*xStepInBytes];
					rightBufferRowPtr[x*3 + 2] = rightImageDataRowPtr[x*xStepInBytes];


				
				
			}
		}


	}


	//imshow("DebugBufferLeft",this->visualSonificationInfo.data.currentCameraBuffers[VISUAL_SONIFICATION_LEFT_CAMERA]);
	//imshow("DebugBufferRight",this->visualSonificationInfo.data.currentCameraBuffers[VISUAL_SONIFICATION_RIGHT_CAMERA]);
	//waitKey(0);
	return true;
}


bool VisualSonification::setImagesForProcessing(Mat leftImageData,Mat rightImageData)
{

	Size currentFrameSize = this->visualSonificationInfo.data.frameSize;
	this->visualSonificationInfo.data.currentCameraBuffers[VISUAL_SONIFICATION_LEFT_CAMERA].create(currentFrameSize.height,currentFrameSize.width,CV_8UC3);
	this->visualSonificationInfo.data.currentCameraBuffers[VISUAL_SONIFICATION_RIGHT_CAMERA].create(currentFrameSize.height,currentFrameSize.width,CV_8UC3);

	if(leftImageData.channels() != 3)
	{
		Mat tmp;
		cvtColor(leftImageData,tmp,CV_GRAY2BGR);
		leftImageData = tmp;
	}
	if(rightImageData.channels() != 3)
	{
		Mat tmp;
		cvtColor(rightImageData,tmp,CV_GRAY2BGR);
		rightImageData = tmp;
	}


	resize(leftImageData,this->visualSonificationInfo.data.currentCameraBuffers[VISUAL_SONIFICATION_LEFT_CAMERA], currentFrameSize);
	resize(rightImageData,this->visualSonificationInfo.data.currentCameraBuffers[VISUAL_SONIFICATION_RIGHT_CAMERA], currentFrameSize);



	return true;
}


Size VisualSonification::getCurrentFrameSize()
{
	return this->visualSonificationInfo.data.frameSize;

}




bool runOnce = false;

