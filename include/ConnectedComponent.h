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

/*
 * ConnectedComponent.h
 *
 *  Created on: Jul 10, 2011
 *      Author: jlclemon
 */

#ifndef CONNECTEDCOMPONENTE_H_
#define CONNECTEDCOMPONENTE_H_


#include <string>
#include <fstream>
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
#define CONNECTED_COMPONENT_AREA_TOO_SMALL_LABEL  -4
#define CONNECTED_COMPONENT_MASKED_LABEL  -3
#define CONNECTED_COMPONENT_IN_QUEUE  -2
#define CONNECTED_COMPONENT_UNLABELED  -1
#define CONNECTED_COMPONENT_MAX_QUEUE_SIZE 640*480
#define CONNECTED_COMPONENT_MAX_NUMBER_OF_POINTS_IN_LIST 640*480
#define CONNECTED_COMPONENT_MAX_NUMBER_OF_COMPONENTS 640*480
struct ConnectedComponentConfig
{
	float depthDifferenceThreshold; //If depth difference with neighbors < depthDifferenceThreshold then new component
	short int depthDifferenceThresholdFixedPoint; //(4 fractional bits resolution)
	short int depthFractionalBits; //(4 fractional bits resolution)
	int minPixelArea;
	bool eightWayNeighborhood;


};

struct ConnectedComponentQueue
{
	Point queueOfPoints[CONNECTED_COMPONENT_MAX_QUEUE_SIZE];
	Point * frontOfQueue;
	Point * backOfQueue;
	int size;
};

struct ConnectedComponentList
{
	Point listOfPoints[CONNECTED_COMPONENT_MAX_NUMBER_OF_POINTS_IN_LIST];
	int numberOfPointsInComponent[CONNECTED_COMPONENT_MAX_NUMBER_OF_COMPONENTS];
	int componentStartIndex[CONNECTED_COMPONENT_MAX_NUMBER_OF_COMPONENTS];
	int numberOfComponents;
};




struct ConnectedComponentData
{
	Mat depthImage;
	Mat labels;
	ConnectedComponentQueue queue;
	ConnectedComponentList * list;
	vector<Point> currentQueue;
	vector <vector <Point> > listOfConnectedComponents;
	int currentLabel;

};



vector <vector <Point > > findConnectedComponentsFixedPointWrapper(struct ConnectedComponentConfig & config,Mat & depthImage, Mat & mask,Mat & labeledComponents);
void findConnectedComponentsFixedPoint(struct ConnectedComponentConfig * config,ConnectedComponentList * list,Mat * depthImage, Mat * mask,Mat * labeledComponents);


Mat convertToDisplayComponentResults(Mat componentMat);
void displayComponentResults(Mat componentMat);
void displayComponentResults(vector<vector <Point> > &componentLists, Size frameSize);
void displayComponent(vector<vector <Point> > & componentLists, int componentId, Size frameSize);
//vector<vector <Point> > findConnectedComponents(Mat & depthImage, Mat & labeledComponents, struct ConnectedComponentConfig & config);
vector<vector <Point> > findConnectedComponents(struct ConnectedComponentConfig & config,Mat & depthImage, Mat & mask,Mat & labeledComponents);
void testConnectedComponent();
#endif
