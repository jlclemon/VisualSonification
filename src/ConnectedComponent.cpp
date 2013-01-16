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

#include "ConnectedComponent.h"



static signed int connectedComponentsInternalLabelsBuffer[640*2*480*2];


//void createComponent(struct ConnectedComponentConfig & config, struct ConnectedComponentData & data, vector<Point> & currentComponent);
void createComponent(struct ConnectedComponentConfig & config, Mat & mask,struct ConnectedComponentData & data, vector<Point> & currentComponent);
void createComponentFixedPoint(struct ConnectedComponentConfig * config, Mat * mask,struct ConnectedComponentData * data, ConnectedComponentList * componentList, int currentComponent);
//bool initQueue(struct ConnectedComponentConfig * config, struct ConnectedComponentData * data)
bool initQueue(struct ConnectedComponentQueue * queue)
{

	queue->frontOfQueue = &(queue->queueOfPoints[0]);
	queue->backOfQueue = queue->frontOfQueue;
	queue->size = 0;
	return true;
}


//bool addPointToBackOfQueue(struct ConnectedComponentConfig * config, struct ConnectedComponentData * data, Point * newPoint )
bool addPointToBackOfQueue(struct ConnectedComponentQueue * queue, Point * newPoint )
{
	bool returnVal = false;
	if(queue->size < CONNECTED_COMPONENT_MAX_QUEUE_SIZE)
	{
		queue->backOfQueue->x = newPoint->x;
		queue->backOfQueue->y = newPoint->y;
		queue->backOfQueue++;
		queue->size++;
		returnVal = true;
	}

	return returnVal;
}

//bool removePointFromBackOfQueue(struct ConnectedComponentConfig * config, struct ConnectedComponentData * data)
bool removePointFromBackOfQueue(struct ConnectedComponentQueue * queue)
{
	bool returnVal = false;
	if(queue->size > 0)
	{
		queue->backOfQueue--;
		queue->size--;
		returnVal = true;
	}


	return returnVal;







}
//Point getPointFromBackOfQueue(struct ConnectedComponentConfig * config, struct ConnectedComponentData * data)
Point getPointFromBackOfQueue(struct ConnectedComponentQueue * queue)
{

	if(queue->size <=0)
	{
		Point empty;
		empty.x = -1;
		empty.y=-1;
		return empty;

	}
	return *(queue->backOfQueue-1);

}





void initConnectedComponentsList(struct ConnectedComponentList * list)
{

	list->numberOfComponents = 0;


}

void initConnectedComponentIndividualList(struct ConnectedComponentList * list, int componentId)
{

	list->numberOfPointsInComponent[componentId] = 0;
	if(componentId >0)
	{
		list->componentStartIndex[componentId] =list->componentStartIndex[componentId-1] +  list->numberOfPointsInComponent[componentId-1];
	}
	else if(componentId ==0)
	{

		list->componentStartIndex[componentId] = 0;


	}
	else
	{

		printf("Error: Connected component invalid region id");

	}
}

void clearConnectedComponentIndividualList(struct ConnectedComponentList * list, int componentId)
{
	initConnectedComponentIndividualList(list,componentId);
}

int addNewComponentToComponentsList(struct ConnectedComponentList * list)
{
	initConnectedComponentIndividualList(list, list->numberOfComponents);
	list->numberOfComponents++;
	return (list->numberOfComponents-1);

}
int removeLastComponentFromComponentsList(struct ConnectedComponentList * list)
{
	if(list->numberOfComponents >0)
	{
		list->numberOfComponents--;
	}
	return (list->numberOfComponents-1);

}



bool addNewPointToConnectedComponentIndividualList(struct ConnectedComponentList * list, int componentId, Point * newPoint)
{
	bool returnVal = false;

	if(list->numberOfPointsInComponent[componentId] + list->componentStartIndex[componentId] < CONNECTED_COMPONENT_MAX_NUMBER_OF_COMPONENTS)
	{
		int currentIndexInList = list->numberOfPointsInComponent[componentId] + list->componentStartIndex[componentId];



		list->listOfPoints[currentIndexInList].x  = newPoint->x;
		list->listOfPoints[currentIndexInList].y  = newPoint->y;
		list->numberOfPointsInComponent[componentId]++;
		returnVal = true;

	}


	return returnVal;
}

vector <vector <Point > > convertConnectComponentListsToVectors(struct ConnectedComponentList * list)
{
	vector <vector <Point > > results;
	if(list->numberOfComponents>0)
	{
		results.resize(list->numberOfComponents);
		for(int i = 0; i < list->numberOfComponents; i++)
		{
			results[i].resize(list->numberOfPointsInComponent[i]);
//			int k = 0;
//			int j = 0;
			for(int j=0, k=list->componentStartIndex[i]; j< list->numberOfPointsInComponent[i];j++,k++)
			{
				results[i][j] = list->listOfPoints[k];
			}


		}
	}
	return results;
}



void connectedComponentsLabelsInit(struct ConnectedComponentConfig & config, struct ConnectedComponentData & data, Size & depthImageSize)
{
///////Edit here adding in the fixed point C like code
	//data->labels = Mat(*depthImageSize,CV_16SC1,connectedComponentsInternalLabelsBuffer);
	data.labels.create(depthImageSize,CV_16SC1);//,connectedComponentsInternalLabelsBuffer);



	//data.labels.setTo(Scalar(CONNECTED_COMPONENT_UNLABELED));
	for(int y = 0; y < data.labels.rows; y++)
	{

		signed short int * currentLabelPtr = (signed short int *)(data.labels.data + data.labels.step[0] * y);
		for(int x = 0; x < data.labels.cols; x++)
		{
			currentLabelPtr[x] = CONNECTED_COMPONENT_UNLABELED;


		}
	}


}

void connectedComponentsLabelsInitFixedPoint(struct ConnectedComponentConfig * config, struct ConnectedComponentData * data, Size * depthImageSize)
{
///////Edit here adding in the fixed point C like code
	data->labels = Mat(*depthImageSize,CV_16SC1,connectedComponentsInternalLabelsBuffer);
	//data->labels.create(*depthImageSize,CV_16SC1);//,connectedComponentsInternalLabelsBuffer);



	//data.labels.setTo(Scalar(CONNECTED_COMPONENT_UNLABELED));
	for(int y = 0; y < data->labels.rows; y++)
	{

		signed short int * currentLabelPtr = (signed short int *)(data->labels.data + data->labels.step[0] * y);
		for(int x = 0; x < data->labels.cols; x++)
		{
			currentLabelPtr[x] = CONNECTED_COMPONENT_UNLABELED;


		}
	}


}


void connectedComponentsDataInitFixedPoint(struct ConnectedComponentConfig * config, struct ConnectedComponentData * data)
{
	Size depthImageSize;

	data->currentLabel = 0;


	depthImageSize.height= data->depthImage.rows;
	depthImageSize.width= data->depthImage.cols;


	connectedComponentsLabelsInitFixedPoint(config,data,&depthImageSize);


}
 
void connectedComponentsDataInit(struct ConnectedComponentConfig & config, struct ConnectedComponentData & data)
{
	Size depthImageSize;

	data.currentLabel = 0;
	
	
	depthImageSize.height= data.depthImage.rows;
	depthImageSize.width= data.depthImage.cols;


	connectedComponentsLabelsInit(config,data,depthImageSize);


}





void resetInqueueLabels(Mat & labels)
{

	for(int y =0; y < labels.rows; y++)
	{
		signed short int * labelsRow = labels.ptr<signed short int>(y);
		for(int x =0; x < labels.cols; x++)
		{
			if(labelsRow[x] ==CONNECTED_COMPONENT_IN_QUEUE)
			{
				labelsRow[x] = CONNECTED_COMPONENT_UNLABELED;
			}
		}
	}


}



void displayComponentResults(Mat componentMat)
{
	unsigned char colors[11][3] = {	{255,0,0},
								{0,255,0},
								{0,0,255},
								{255,255,0},
								{255,0,255},
								{0,255,255},
								{255,255,255},
								{128,0,0},
								{0,128,0},
								{0,0,128},
								{0,0,0}
								};
	int numberOfColors =10;
	Mat componentDisplay(componentMat.size(),CV_8UC3);
	
	componentDisplay.setTo(Scalar(0,0,0));

	for(int y = 0; y < componentDisplay.rows; y++) 
	{
		for(int x = 0; x < componentDisplay.cols; x++)
		{

			for(int c = 0; c < 3; c++)
			{
				signed short int currentComponent = componentMat.at<signed short int>(Point(x,y));
				if(currentComponent <0)
				{
					//currentComponent = numberOfColors;
					componentDisplay.at<Vec3b>(y,x)[c] = colors[numberOfColors][c];
				}
				else
				{
					componentDisplay.at<Vec3b>(y,x)[c] = colors[currentComponent%numberOfColors][c];
				}
			}
		}
	}
	
	//namedWindow("ComponentDisplay");


	imshow("ComponentDisplay",componentDisplay);
	imwrite("ccdisp.png",componentDisplay);
	waitKey(10);
	
}

Mat convertToDisplayComponentResults(Mat componentMat)
{
	unsigned char colors[21][3] = {	{255,0,0},
								{0,255,0},
								{0,0,255},
								{255,255,0},
								{255,0,255},
								{0,255,255},
								{255,255,255},
								{128,0,0},
								{0,128,0},
								{0,0,128},
								{128,128,0},
								{0,128,128},
								{128,0,128},
								{128,255,0},
								{0,128,255},
								{128,0,255},
								{255,128,0},
								{0,255,128},
								{255,0,128},
								{128,255,128},
								{0,0,0}
								};
	int numberOfColors =20;
	Mat componentDisplay(componentMat.size(),CV_8UC3);
	
	componentDisplay.setTo(Scalar(0,0,0));

	for(int y = 0; y < componentDisplay.rows; y++) 
	{
		for(int x = 0; x < componentDisplay.cols; x++)
		{

			for(int c = 0; c < 3; c++)
			{
				signed short int currentComponent = componentMat.at<signed short int>(Point(x,y));
				if(currentComponent <0)
				{
					//currentComponent = numberOfColors;
					componentDisplay.at<Vec3b>(y,x)[c] = colors[numberOfColors][c];
				}
				else
				{
					componentDisplay.at<Vec3b>(y,x)[c] = colors[currentComponent%numberOfColors][c];
				}
			}
		}
	}
	
	//namedWindow("ComponentDisplay");


	//imshow("ComponentDisplay",componentDisplay);
	//imwrite("ccdisp.png",componentDisplay);
	//waitKey(10);
	return componentDisplay;
}



void displayComponent(vector<vector <Point> > &componentLists, int componentId, Size frameSize)
{
	unsigned char colors[11][3] = {	{255,0,0},
								{0,255,0},
								{0,0,255},
								{255,255,0},
								{255,0,255},
								{0,255,255},
								{255,255,255},
								{128,0,0},
								{0,128,0},
								{0,0,128},
								{0,0,0}
								};
	int numberOfColors =10;
	Mat componentDisplay(frameSize,CV_8UC3);
	
	componentDisplay.setTo(Scalar(0,0,0));

	for(int i=0; i< (int)componentLists.size(); i++)
	{
		for(int j=0; j< (int)componentLists[i].size(); j++)
		{

			for(int c = 0; c < 3; c++)
			{
				unsigned char currentComponent = i;

				if(i != componentId)
				{
					componentDisplay.at<Vec3b>(componentLists[i][j])[c] = colors[6][c];
				}
				else
				{
					componentDisplay.at<Vec3b>(componentLists[i][j])[c] = colors[2][c];
				}
			}

		}



	}

	
	namedWindow("SingleComponentDisplay");


	imshow("SingleComponentDisplay",componentDisplay);
	imwrite("ccdisp2.png",componentDisplay);
	waitKey(10);



}

void displayComponentResults(vector<vector <Point> > & componentLists, Size frameSize)
{
	unsigned char colors[11][3] = {	{255,0,0},
								{0,255,0},
								{0,0,255},
								{255,255,0},
								{255,0,255},
								{0,255,255},
								{255,255,255},
								{128,0,0},
								{0,128,0},
								{0,0,128},
								{0,0,0}
								};
	int numberOfColors =10;
	Mat componentDisplay(frameSize,CV_8UC3);
	
	componentDisplay.setTo(Scalar(0,0,0));

	for(int i=0; i< (int)componentLists.size(); i++)
	{
		for(int j=0; j< (int)componentLists[i].size(); j++)
		{

			for(int c = 0; c < 3; c++)
			{
				unsigned char currentComponent = i;

				componentDisplay.at<Vec3b>(componentLists[i][j])[c] = colors[currentComponent%numberOfColors][c];

			}

		}



	}

	
	namedWindow("ComponentDisplayPoints");


	imshow("ComponentDisplayPoints",componentDisplay);
	waitKey(10);


}


void findConnectedComponentsFixedPoint(struct ConnectedComponentConfig * config,ConnectedComponentList * list,Mat * depthImage, Mat * mask,Mat * labeledComponents)
{
	struct ConnectedComponentData data;
	int numberOfComponents = 1;

	data.depthImage = *depthImage;
	data.list = list;
	//Initialize the data to prep calculating the connected component
	connectedComponentsDataInitFixedPoint(config,&data);
	initQueue(&data.queue);
	initConnectedComponentsList(data.list);
	//initConnectedComponentIndividualList(struct ConnectedComponentConfig * config, struct ConnectedComponentData * data, int componentId)

	//Loop through pixels
	for(int y =0; y < data.depthImage.rows; y++)
	{

		//Get the pointers to the rows
		signed short int * depthImageRow = (signed short int *)(data.depthImage.data + data.depthImage.step[0] * y);//ptr<signed short int>(y);
		signed short int * labelsRow = (signed short int *)(data.labels.data +data.depthImage.step[0] * y);//ptr<signed short int>(y);

		unsigned char * maskRow = (unsigned char *)(mask->data + mask->step[0] *y);//ptr<unsigned char>(y);

		for(int x =0; x < data.depthImage.cols; x++)
		{

			//Check if the current pixel is valid for labeling
			if(labelsRow[x] ==CONNECTED_COMPONENT_UNLABELED && maskRow[x] !=0)
			{
//				resetInqueueLabels(data.labels);


//				vector <Point> currentComponent;
				//Handle resizing the component vector
				if((int)data.list->numberOfComponents < numberOfComponents)
				{
					int numberOfComponentsNow = addNewComponentToComponentsList(data.list);

				}
				else
				{

					initConnectedComponentIndividualList(data.list, numberOfComponents-1);

				}
				int currentComponent = numberOfComponents-1;



				//Current point struct
				Point currentPoint(x,y);


				//Clear the quue
				initQueue(&data.queue);

				//Add this point to the queue to start a new component
				addPointToBackOfQueue(&data.queue, &currentPoint );

				//Change the current node/pixels class
				((signed short int*)(data.labels.data + data.labels.step[0]*currentPoint.y))[currentPoint.x] = data.currentLabel;//      at<signed short int>(currentPoint) = data.currentLabel;

				//Create the component
				createComponentFixedPoint(config, mask, &data, data.list, currentComponent);

				//Make sure the pixel area size is acceptable, if it isn't then label the component as too small
				if((int)data.list->numberOfPointsInComponent[currentComponent] < config->minPixelArea)
				{
					for(int pointId = 0; pointId < (int)data.list->numberOfPointsInComponent[currentComponent]; pointId++)
					{
						((signed short int*)(data.labels.data + data.labels.step[0]*data.list->listOfPoints[data.list->componentStartIndex[currentComponent]+pointId].y))[data.list->listOfPoints[data.list->componentStartIndex[currentComponent]+pointId].x] = CONNECTED_COMPONENT_AREA_TOO_SMALL_LABEL;//data.labels.at<signed short int>(data.listOfConnectedComponents[currentComponent][pointId]) = CONNECTED_COMPONENT_AREA_TOO_SMALL_LABEL;
					}
				}
				else
				{
					//Increment the currentLabel id
					data.currentLabel++;
					numberOfComponents++;

				}
//				displayComponentResults(data.labels);
//				waitKey(0);
			}
			else if(maskRow[x] ==0)
			{
				labelsRow[x] = CONNECTED_COMPONENT_MASKED_LABEL;


			}




		}
	}

	//Since we may have over allocated the list of connected components vector, lets clear out the empty ones
	while(numberOfComponents < (int)data.list->numberOfComponents)
	{
		removeLastComponentFromComponentsList(data.list);
	}



//	data.listOfConnectedComponents = convertConnectComponentListsToVectors(data.list);
	*labeledComponents = data.labels;
//	return data.listOfConnectedComponents;
	return;





}

void createComponentFixedPoint(struct ConnectedComponentConfig * config, Mat * mask,struct ConnectedComponentData * data, ConnectedComponentList * componentList, int currentComponent)
{

	while(data->queue.size != 0)
	{

		//Get the current node from the queue
		Point currentNode= getPointFromBackOfQueue(&data->queue);
		//data.currentQueue.pop_back();
		removePointFromBackOfQueue(&data->queue);


		//Get the current nodes depth
		signed short int currentNodesDepth = ((signed short int*)(data->depthImage.data + currentNode.y * data->depthImage.step[0]))[currentNode.x];//at<unsigned int>(currentNode);

		//To avoid two loops around the point, just save the neighbor points we will add
		//So tha
		Point currentUnlabeledNeighbor;
		//int currentUnlabeledNeighborsCount = 0;


		//Add this node as a part of this component
		//currentComponent.push_back(currentNode);
		addNewPointToConnectedComponentIndividualList(componentList, currentComponent, &currentNode);


		//Look around the point using a diff
		for(int dy = -1; dy<=1; dy++)
		{

			//This is the current neighbor row we are looking at
			int currentNeighborRow =currentNode.y+ dy;

			if(!(currentNeighborRow <0)  && !(currentNeighborRow >= data->depthImage.rows))
			{
				//Get pointers to the rows in the images we will manipulate
				signed short int * depthImageRow = (signed short int *)(data->depthImage.data + data->depthImage.step[0] * currentNeighborRow);//ptr<signed short int>(currentNeighborRow);
				signed short int * labelsRow = (signed short int *)(data->labels.data + data->labels.step[0] * currentNeighborRow);//ptr<signed short int>(currentNeighborRow);

				unsigned char * maskRow = (unsigned char *)(mask->data + mask->step[0] * currentNeighborRow);//ptr<unsigned char>(currentNeighborRow);

				//Look around the col dimension
				for(int dx = -1; dx<=1; dx++)
				{

					//This is the current neighbor col we are looking at
					int currentNeighborCol =currentNode.x+ dx;



					//The pixels we are comparing to are not at the center so only compute if we are not at the center
					//Also make sure we are not outside the image
					if(!((dx==0) && (dy ==0))  && !(currentNeighborCol <0)  && !(currentNeighborCol >= data->depthImage.cols))
					{

						if(abs(depthImageRow[currentNeighborCol] - currentNodesDepth) <= config->depthDifferenceThresholdFixedPoint)
						{

							if(labelsRow[currentNeighborCol] == CONNECTED_COMPONENT_UNLABELED && maskRow[currentNeighborCol] != 0)
							{


								//Setup the point to be added to the queue
								currentUnlabeledNeighbor.x = currentNeighborCol;
								currentUnlabeledNeighbor.y = currentNeighborRow;


								//Add my acceptable neighbors to the queue
								//data.currentQueue.push_back(currentUnlabeledNeighbor);
								//Add this point to the queue to start a new component
								addPointToBackOfQueue(&data->queue, &currentUnlabeledNeighbor );



								//Modify the label for this neighbor
								labelsRow[currentNeighborCol] = data->currentLabel;
							}
						}
					}
				}
			}
		}
	}
}

vector <vector <Point > > findConnectedComponentsFixedPointWrapper(struct ConnectedComponentConfig & config,Mat & depthImage, Mat & mask,Mat & labeledComponents)
{
	ConnectedComponentList list;





	findConnectedComponentsFixedPoint(&config,&list,&depthImage, &mask,&labeledComponents);





	return convertConnectComponentListsToVectors(&list);
}
vector<vector <Point> > findConnectedComponents(struct ConnectedComponentConfig & config,Mat & depthImage, Mat & mask,Mat & labeledComponents)
{
	static struct ConnectedComponentData data;
	int numberOfComponents = 1;


	
	data.depthImage = depthImage;


	//Initialize the data to prep calculating the connected component
	connectedComponentsDataInit(config,data);
	

	
	//Loop through pixels
	for(int y =0; y < data.depthImage.rows; y++)
	{

		//Get the pointers to the rows
		float * depthImageRow = data.depthImage.ptr<float>(y);
		signed short int * labelsRow = data.labels.ptr<signed short int>(y);
		
		unsigned char * maskRow = mask.ptr<unsigned char>(y);

		for(int x =0; x < data.depthImage.cols; x++)
		{

			//Check if the current pixel is valid for labeling
			if(labelsRow[x] ==CONNECTED_COMPONENT_UNLABELED && maskRow[x] !=0)
			{
//				resetInqueueLabels(data.labels);


//				vector <Point> currentComponent;
				//Handle resizing the component vector
				if((int)data.listOfConnectedComponents.size() < numberOfComponents)
				{
					data.listOfConnectedComponents.resize(numberOfComponents);
				}
				else
				{
					data.listOfConnectedComponents[numberOfComponents-1].clear();
				}
				int currentComponent = numberOfComponents-1;

					

				//Current point struct
				Point currentPoint(x,y);

				//Clear the quue
				data.currentQueue.clear();

				//Add this point to the queue to start a new component
				data.currentQueue.push_back(currentPoint);

				//Change the current node/pixels class
				((signed short int*)(data.labels.data + data.labels.step[0]*currentPoint.y))[currentPoint.x] = data.currentLabel;//      at<signed short int>(currentPoint) = data.currentLabel;

				//Create the component
				createComponent(config, mask, data, data.listOfConnectedComponents[currentComponent]);

				//Make sure the pixel area size is acceptable, if it isn't then label the component as too small
				if((int)data.listOfConnectedComponents[currentComponent].size() < config.minPixelArea)
				{
					for(int pointId = 0; pointId < (int)data.listOfConnectedComponents[currentComponent].size(); pointId++)
					{
						((signed short int*)(data.labels.data + data.labels.step[0]*data.listOfConnectedComponents[currentComponent][pointId].y))[data.listOfConnectedComponents[currentComponent][pointId].x] = CONNECTED_COMPONENT_AREA_TOO_SMALL_LABEL;//data.labels.at<signed short int>(data.listOfConnectedComponents[currentComponent][pointId]) = CONNECTED_COMPONENT_AREA_TOO_SMALL_LABEL;
					}
				}
				else
				{
					//Increment the currentLabel id
					data.currentLabel++;
					numberOfComponents++;

				}
//				displayComponentResults(data.labels);
//				waitKey(0);
			}
			else if(maskRow[x] ==0)
			{
				labelsRow[x] = CONNECTED_COMPONENT_MASKED_LABEL;


			}




		}
	}

	//Since we may have over allocated the list of connected components vector, lets clear out the empty ones
	while(numberOfComponents < (int)data.listOfConnectedComponents.size())
	{
		data.listOfConnectedComponents.pop_back();
	}


	labeledComponents = data.labels;
	return data.listOfConnectedComponents;


}


void createComponent(struct ConnectedComponentConfig & config, Mat & mask,struct ConnectedComponentData & data, vector<Point> & currentComponent)
{

	while(data.currentQueue.size() != 0)
	{

		//Get the current node from the queue
		Point currentNode= data.currentQueue.back();
		data.currentQueue.pop_back();		

		//Get the current nodes depth
		float currentNodesDepth = data.depthImage.at<float>(currentNode);

		//To avoid two loops around the point, just save the neighbor points we will add
		//So tha
		Point currentUnlabeledNeighbor;
		//int currentUnlabeledNeighborsCount = 0;


		//Add this node as a part of this component
		currentComponent.push_back(currentNode);


		//Look around the point using a diff
		for(int dy = -1; dy<=1; dy++)
		{

			//This is the current neighbor row we are looking at
			int currentNeighborRow =currentNode.y+ dy;

			if(!(currentNeighborRow <0)  && !(currentNeighborRow >= data.depthImage.rows))
			{
				//Get pointers to the rows in the images we will manipulate
				float * depthImageRow = data.depthImage.ptr<float>(currentNeighborRow);
				signed short int * labelsRow = data.labels.ptr<signed short int>(currentNeighborRow);

				unsigned char * maskRow = mask.ptr<unsigned char>(currentNeighborRow);

				//Look around the col dimension
				for(int dx = -1; dx<=1; dx++)
				{

					//This is the current neighbor col we are looking at
					int currentNeighborCol =currentNode.x+ dx;



					//The pixels we are comparing to are not at the center so only compute if we are not at the center
					//Also make sure we are not outside the image
					if(!((dx==0) && (dy ==0))  && !(currentNeighborCol <0)  && !(currentNeighborCol >= data.depthImage.cols))
					{
							
						if(abs(depthImageRow[currentNeighborCol] - currentNodesDepth) <= config.depthDifferenceThreshold)
						{
											
							if(labelsRow[currentNeighborCol] == CONNECTED_COMPONENT_UNLABELED && maskRow[currentNeighborCol] != 0)
							{


								//Setup the point to be added to the queue
								currentUnlabeledNeighbor.x = currentNeighborCol;
								currentUnlabeledNeighbor.y = currentNeighborRow;


								//Add my acceptable neighbors to the queue
								data.currentQueue.push_back(currentUnlabeledNeighbor);

								//Modify the label for this neighbor
								labelsRow[currentNeighborCol] = data.currentLabel;
							}
						}
					}
				}
			}
		}
	}
}






void testConnectedComponent()
{
	Mat testMat(480,640,CV_32FC1);
	Mat mask(480,640,CV_32FC1);

	mask.setTo(Scalar(1.0));
	Mat firstGroup = testMat(Range(0,120),Range(0,200));
	Mat secondGroup = testMat(Range(120,240),Range(200,400));
	Mat thirdGroup = testMat(Range(240,480),Range(400,640));

	firstGroup.setTo(Scalar(15));
	secondGroup.setTo(Scalar(21));
	thirdGroup.setTo(Scalar(15));

	FileStorage fs1("testCompon.yml",FileStorage::READ);
	fs1["Depth"] >> testMat;
	fs1.release();

	//Next we want to dilate and erode just a little
	dilate(testMat,testMat,Mat(),Point(-1,-1),2);

	//FileStorage fs3("FilteredDepthDilate.yml",FileStorage::WRITE);
	//fs3 << "FilteredDepthDilate" << filteredDepth;
	//fs3.release();



	//Next we want to dilate and erode just a little
	erode(testMat,testMat,Mat(),Point(-1,-1),2);
	//We will need to normalize to such that it fits in 255 to 0 for the canny function so create a mat for that
	Mat normalizedFilteredDepth(testMat.size(),CV_8UC1);

	normalize(testMat,normalizedFilteredDepth,0,255,NORM_MINMAX,CV_32FC1);
	testMat = normalizedFilteredDepth;

	imshow("TestMat",testMat);

	Mat testComponentMat;
	struct ConnectedComponentConfig config;
	config.eightWayNeighborhood = true;
	config.depthDifferenceThreshold = .05f;
	config.minPixelArea = 10;
	vector<vector <Point> > testComponents = findConnectedComponents(config, testMat, mask, testComponentMat);


	displayComponentResults(testComponentMat);
	displayComponentResults(testComponents, testMat.size());

}
