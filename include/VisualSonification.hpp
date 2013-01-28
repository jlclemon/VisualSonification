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

#ifndef VISUAL_SONIFICATION_H
#define VISUAL_SONIFICATION_H

#include <string>
#include <fstream>
#include <iostream>
#include <time.h>
#include <stdio.h>
#include <opencv2/features2d/features2d.hpp>
#include <opencv2/nonfree/features2d.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/core.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <opencv2/calib3d/calib3d.hpp>
#include <opencv2/ml/ml.hpp>
#include <opencv2/objdetect/objdetect.hpp>
#include "StereoCameraCalibration.h"
#include "ConnectedComponent.h"
#include "CodeWordDictionaryUtilities.h"
#include "HistogramSoundSource.h"
#include "VisionUtilities.h"
#include "sift.hpp"
#include "audioUtilities.h"
#include "ThreadManager.h"


using namespace std;
using namespace cv;


//This turns on the use of the connected component code for segmentation
#define USE_DEPTH_CONNECTED_COMPONENT

#define DEFAULT_CONFIG_FILE_COMMAND "-configFile"
#ifdef ANDROID
	#warning "Building for android"
	#define DEFAULT_CONFIG_FILE_LOCATION "/sdcard/visionData/configFileAndroid.txt"
#else
	#ifdef LINUX
		#define DEFAULT_CONFIG_FILE_LOCATION "configFileLinux.txt"
	#else
		#define DEFAULT_CONFIG_FILE_LOCATION "configFile.txt"
	#endif

#endif

#define DISPLAY_TIMING_WINDOWS
//#define SAVE_TIMING

//#define TEST_CLASS


//#define VISUAL_SONIFICATION_IMAGE_NUMBER_OF_PROCESS_IMAGES 6  //(2+1+1+1+1)

#define VISUAL_SONIFICATION_DEFAULT_REGION_OUTPUT_TIME 1.0
#define VISUAL_SONIFICATION_DEFAULT_MIN_FREQ 500
#define VISUAL_SONIFICATION_DEFAULT_MAX_FREQ 2000;
#define VISUAL_SONIFICATION_DEFAULT_SONIFICATION_TECH HistogramSoundSource::SOUND_GENERATION_PURE_SINEWAVE_WITH_TRANSITION
#define VISUAL_SONIFICATION_DEFAULT_AUDIO_SAMPLE_FREQ 44100

#define SONIFICATION_MAX_NUMBER_AUDIO_BUFFERS 20
#define SONIFICATION_MAX_AUDIO_SAMPLE_RATE_BUFFERS_HZ 44100
#define SONIFICATION_MAX_AUDIO_BUFFER_TIME_IN_SEC 1
#define SONIFICATION_MAX_AUDIO_SAMPLES_PER_BUFFER (SONIFICATION_MAX_AUDIO_BUFFER_TIME_IN_SEC*SONIFICATION_MAX_AUDIO_SAMPLE_RATE_BUFFERS_HZ)



enum VisualSonificationProcessImages
{
	VISUAL_SONIFICATION_PROCESS_IMAGE_LEFT_RAW=0,
	VISUAL_SONIFICATION_PROCESS_IMAGE_RIGHT_RAW,
	VISUAL_SONIFICATION_PROCESS_IMAGE_LEFT_GRAY,
	VISUAL_SONIFICATION_PROCESS_IMAGE_RIGHT_GRAY,
	VISUAL_SONIFICATION_PROCESS_IMAGE_DEPTH,
	VISUAL_SONIFICATION_PROCESS_IMAGE_FEATURE,
	VISUAL_SONIFICATION_PROCESS_IMAGE_REGION,
	VISUAL_SONIFICATION_IMAGE_NUMBER_OF_PROCESS_IMAGES
};



//This holds enums for the various cameras will will support
enum VisualSonificationCameras
{
	VISUAL_SONIFICATION_LEFT_CAMERA,
	VISUAL_SONIFICATION_RIGHT_CAMERA,
	VISUAL_SONIFICATION_DEPTH_CAMERA,
	VISUAL_SONIFICATION_NUMBER_OF_CAMERAS
};


//These are enums for identifying images for example in getting a debug image to display
enum VisualSonificationImages 
{
	VISUAL_SONIFICATION_IMAGE_CAMERA_BUFFER,
	VISUAL_SONIFICATION_IMAGE_RAW_IMAGE,
	VISUAL_SONIFICATION_IMAGE_REMAPPED_IMAGE,
	VISUAL_SONIFICATION_IMAGE_GRAY_IMAGE,
	VISUAL_SONIFICATION_IMAGE_COLOR_DEPTH_IMAGE,
	VISUAL_SONIFICATION_IMAGE_FEATURE_IMAGE,
	VISUAL_SONIFICATION_IMAGE_AUDIO_DISPLAY_IMAGE,
	VISUAL_SONIFICATION_IMAGE_CONNECTED_COMPONENT_LABELS_IMAGE,
	VISUAL_SONIFICATION_IMAGE_NUMBER_OF_IMAGES


};

//These enums are for the remap table ids
enum VisualSonifcationCameraRemapAxisTypes
{
	VISUAL_SONIFICATION_CAMERA_REMAP_AXIS_X =0,
	VISUAL_SONIFICATION_CAMERA_REMAP_AXIS_Y=1,
	VISUAL_SONIFICATION_NUMBER_OF_CAMERA_REMAP_AXES


};

//These are for the mode of operation in terms of camera input
enum VisualSonificationCameraInputMode
{
	VISUAL_SONIFICATION_STEREO,
	VISUAL_SONIFICATION_MONO_AND_DEPTH,
	VISUAL_SONIFICATION_MONO_AND_SEPARATE_SEGMENTATION,
	VISUAL_SONIFICATION_NUMBER_OF_CAMERA_INPUT_MODES
};



//These are the inputs themselves
enum VisualSonificationCameraInputType
{
	VISUAL_SONIFICATION_STREAM,
	VISUAL_SONIFICATION_VIDEO_FILE,
	VISUAL_SONIFICATION_IMAGE_LIST,
	VISUAL_SONIFICATION_EXTERNAL_STREAM,
	VISUAL_SONIFICATION_NUMBER_OF_CAMERA_INPUT_TYPES
};


//These define the legal image sizes
enum VisualSonificationCameraInputSize
{
	VISUAL_SONIFICATION_INPUT_VGA, //640x480
	VISUAL_SONIFICATION_INPUT_QVGA, //320x240
	VISUAL_SONIFICATION_NUMBER_OF_CAMERA_INPUT_SIZES
};


//These are the keypoint detection algorithms
enum FeatureLocalizationAlgos
{
	FEATURE_LOCALIZATION_SIFT = 1,
	FEATURE_LOCALIZATION_SURF = 2,
	FEATURE_LOCALIZATION_FAST = 4,
	FEATURE_LOCALIZATION_HOG = 8,
	FEATURE_LOCALIZATION_GRID = 16,
	FEATURE_LOCALIZATION_SIFT_VL = 32,   //Vedaldi SIFT without OpenCV overhead
	NUMBER_OF_FEATURE_LOCALIZATION_ALGOS=6

};


//These are the descriptor building algos
enum FeatureDescriptorAlgos
{
	FEATURE_DESC_SIFT = 1,
	FEATURE_DESC_SURF = 2,
	FEATURE_DESC_HOG = 8,
	FEATURE_DESC_BRIEF = 16,
	FEATURE_DESC_SIFT_VL = 32,//Vedaldi SIFT without OpenCV overhead
	NUMBER_OF_FEATURE_DESC_ALGOS = 5

};

//These are teh feature matching algorithms
enum FeaturesMatchingAlgos
{
	FEATURE_MATCHER_BRUTEFORCE = 1,
	FEATURE_MATCHER_FLANN = 2,
	FEATURE_MATCHER_BRUTEFORCE_C = 4,  //Custom brute force
	NUMBER_OF_FEATURE_MATCHER_ALGOS = 3

};

//Thses are the states
enum VisualSonificationState
{
	VISUAL_SONIFICATION_STATE_INIT,							//Initialize the system
	VISUAL_SONIFICATION_STATE_DATA_CAPTURE,					//Get the next set of image data including depth
	VISUAL_SONIFICATION_STATE_SEGMENTATION,					//Segment the image into regions
	VISUAL_SONIFICATION_STATE_FEATURE_EXTRACTION,			//Perform feature extraction
	VISUAL_SONIFICATION_STATE_HISTOGRAM_GENERATION,			//Compute the histograms from bag of words
	VISUAL_SONIFICATION_STATE_SIGNTATURE_GENERATION,		//Turn the histograms into audio
	VISUAL_SONIFICATION_STATE_AUDIO_GENERATION,				//Output the audio to the user
	VISUAL_SONIFICATION_NUMBER_OF_STATES					
};







struct SonificationAudioBuffer
{
	int32_t sampleRateInHz;
	int32_t location[3];
	int32_t numberOfSamplesInUse;
	int16_t data[SONIFICATION_MAX_AUDIO_SAMPLES_PER_BUFFER]; 		
};


struct SonificationAudioBufferList
{
	int16_t numberOfBuffersInUse;
	int16_t maxNumberOfBuffers;
	SonificationAudioBuffer buffers[SONIFICATION_MAX_NUMBER_AUDIO_BUFFERS];		
};



#define SONIFICATION_MAX_HISOTGRAM_SIZE 200
#define SONIFICATION_MAX_NUMBER_HISTOGRAM_BUFFERS 20
struct SonificationHistogramBuffer
{
	int16_t histogramEntriesInUse;
	int32_t location[3];
	int16_t locationInImage[2];
	int32_t numberOfFeaturesUsedToMakeHistogram;
	float data[SONIFICATION_MAX_HISOTGRAM_SIZE]; 		
};

struct SonificationHistogramBufferList
{
	int16_t numberOfBuffersInUse;
	int16_t maxNumberOfBuffers;
	SonificationHistogramBuffer buffers[SONIFICATION_MAX_NUMBER_HISTOGRAM_BUFFERS];		
};



//This is the configuration struct
struct VisualSonificationStateConfig : BaseConfig
{
	//These are the ids used to setup a camera capture stream
	int leftCameraId;
	int rightCameraId;
	int depthCameraId;

	//This holds the size of an image frame
	Size frameSize;

	//These filenames include path
	//Filename for the camera calibration 
	string calibrationFilename;

	//Filename for the configu file used to initialize this run
	string configFilename;

	//File lists for the images
	string imageListFilenames[VISUAL_SONIFICATION_NUMBER_OF_CAMERAS];


	string imageListBaseDir[VISUAL_SONIFICATION_NUMBER_OF_CAMERAS];



	//The dictionary filename
	string dictionaryFilename;

	//How many of the top neighbors to use for histogram generation
	int numberOfTopNeighborsForHistogram;

	//The max depth we are looking for objects
	float maxDepth;

	//The min depth we are looking for objects
	float minDepth;

	//The threshold where a new object is considered
	float depthDeltaThreshold;

	//The connected component configuration data
	struct ConnectedComponentConfig connectedComponentConfig;

	//The minimum area or pixel count in a component
	int minimumRegionPixelArea;

	//Flags to show debug images
	bool showRawDepthImage;
	bool showRawInputImage;
	bool showRemappedInputImage;
	bool showGrayscaleInputImage;
	bool showEdgeImage;
	bool showFeaturesImage;
	bool showComponentRegionsAtSignatureGen;
	bool showComponentRegionsAtAudioOutput;

	//Flags to save debug data
	bool saveRawDepthImage;
	bool save3dPoints;
	bool saveRawInputImage;
	bool saveRemappedInputImage;
	bool saveGrayscaleInputImage;
	bool saveRegionAudioFiles;
	bool saveRegionHistogramFiles;



	bool placeHistogramInBuffers;
	bool placeAudioInBuffers;
	bool useOpenAlToPlay;


	//Flag for using segmented Image
	bool segmentUsingExternalMask;

	//Flag to pause after each step
	bool pauseAfterEachStateLoop;
	

	//Flag to pause after each step
	bool pauseForSetupAtStart;


	//Turns on semiglobal blockmatching
	bool useGlobalBlockMatching;

	//Canny detector
	bool useCannyEdgeDetector;

	//If a grid based detector is used, these are the settings
	int gridSizeX;
	int gridSizeY;

	VisualSonificationCameraInputSize inputSizeSetting;

	string baseRegionAudioFilename;
	string regionAudioFilesOutputDir;

	string baseRegionHistogramFilename;
	string regionHistogramFilesOutputDir;


	//Mulithreaded configuration from config file


	bool multithreaded;
	int numberOfHorizontalThreads;
	int numberOfVerticalThreads;	


};


//Sift configuration data
struct SiftConfigData
{

	//Base smoothing level
	float sigman;

	//Initial blur amount
	float sigma0;

	//Number of Octaves
	int O;

	//Scalse per ocrave
	int S; 

	//First octave
	int omin;

	//First level each octave
	int smin;

	//last Level each octave
	int smax;

	//Detector threshold
	double threshold;

	//Edge detector threshold
	double edgeThreshold;

	//Use first angle or average angle
	int angleMode;

	//What is the initial magnification
	float magnfication;

	//Normalize the descriptors
	bool unnormalized;

};


//This contains the data used by the application
struct VisualSonificationStateData : BaseData
{
	int checkStackStart;
	//Current State
	VisualSonificationState currentState;

	//Stereo Calibration Data
	StereoCameraCalibration stereoCalibration;



	//These are the capture streams for live feed
	VideoCapture capture[VISUAL_SONIFICATION_NUMBER_OF_CAMERAS];

	//Buffers used to reference camera stream data
	Mat currentCameraBuffers[VISUAL_SONIFICATION_NUMBER_OF_CAMERAS];

	//Copies of the camera data so that we can actually modify it
	Mat currentCameraFrames[VISUAL_SONIFICATION_NUMBER_OF_CAMERAS];

	//Gray scale images
	Mat currentGrayCameraFrames[VISUAL_SONIFICATION_NUMBER_OF_CAMERAS];

	//Saved versions of the raw image data
	Mat currentRawCameraFrames[VISUAL_SONIFICATION_NUMBER_OF_CAMERAS];

	//Images for frames with features drawn on them
	Mat currentFeatureCameraFrames[VISUAL_SONIFICATION_NUMBER_OF_CAMERAS];


	//Saved versions of the raw image data
	Mat currentColorDepthFrame;


	//Algorithm settings
	FeatureLocalizationAlgos featureLocalAlgo;
	FeatureDescriptorAlgos featureDescAlgo;
	FeaturesMatchingAlgos featureMatchAlgo;


	//Pointers to OpenCV feature detector, extractor, and matcher
	Ptr<FeatureDetector> featureDetector;
	Ptr<DescriptorExtractor> descriptorExtractor;
	Ptr<DescriptorMatcher> featureMatcher;

	//Pointer to Vedaldi SIFT
	Ptr<VL::Sift> siftPtr;





	//Sift configuration
	SiftConfigData siftConfigData;

	//Vector of feature angles
	vector<float> anglesInRads;

	//Numbner of keypoints found 
	int numberOfKeypoints;


	//HoG localization and descriptor pointers
	//Seperate from others in Open CV
	HOGDescriptor * localizationHogPtr;
	HOGDescriptor * descHogPtr;


	//Input mode and type
	VisualSonificationCameraInputMode inputMode;
	VisualSonificationCameraInputType inputType;


	//Current Image frame size
	Size frameSize;

	//Remap tables for recitifcation and distortion removal
	Mat rmap[VISUAL_SONIFICATION_NUMBER_OF_CAMERAS][VISUAL_SONIFICATION_NUMBER_OF_CAMERA_REMAP_AXES];

	//Stereo matching structures
	StereoBM stereoBlockMatcher;
	StereoSGBM stereoGlobalBlockMatcher;


	//File lists for the images
	vector<string> imageList[VISUAL_SONIFICATION_NUMBER_OF_CAMERAS];



	//Region Filename
	vector<string> regionWaveFilenameBases;

	//Region Filename
	vector<string> regionHistogramFilenameBases;



	//Current index into input frames
	int currentInputFrames;

	//Matrix structure to hold pixel component lables
	Mat regionLabelsImages[VISUAL_SONIFICATION_NUMBER_OF_CAMERAS];

	//Vector of component.  Each component is a vector of points within the component
	vector< vector<Point> > regionsByPoints[VISUAL_SONIFICATION_NUMBER_OF_CAMERAS];

	//Computation of region centroid in 3D for audio output
	vector<MatrixPoint3dFloat> regionCentroids;

	//Number of features found in the given component/region
	vector<int> featureCountPerRegion;

	//The actual dictionary
	Mat bagOfWordsDictionary;

	//The current extracted descriptors
	Mat currentDescriptors[VISUAL_SONIFICATION_NUMBER_OF_CAMERAS];

	//The current set of keypoints
	vector<KeyPoint> currentKeypoints[VISUAL_SONIFICATION_NUMBER_OF_CAMERAS];

	//The keypoints organized by their component
	vector <vector<KeyPoint> > keyPointsGroupedByRegion[VISUAL_SONIFICATION_NUMBER_OF_CAMERAS];

	//keypoints In order by their groups but in single vector
	vector<KeyPoint> keyPointInGroupOrder[VISUAL_SONIFICATION_NUMBER_OF_CAMERAS];

	//Descriptors organized by component
	vector<Mat> descriptorsGroupedByRegion[VISUAL_SONIFICATION_NUMBER_OF_CAMERAS];	

	//The feature matches from comparing found features to dictionary
	vector<DMatch> featureMatches[VISUAL_SONIFICATION_NUMBER_OF_CAMERAS];

	//The histograms for the various regions
	vector< vector<float> > regionHistograms;

	//Audio sources to generate sound
	vector<HistogramSoundSource> audioSignatures;

	//Current capaciry of the signatures
	int audioSignatureCapacity;

	//Number of audioSignatures being used
	int audioSignaturesInUse;

	//Minimum features requred before we generate an output
	int minimumFeaturesForAudioSignature;

	//Grid size for feature extraction if needed
	Size gridSize;

	//List of regions that have enough features to be played out
	vector<int> regionsToBePlayed;


	void * visualSonificationObjectSelfPtr;


	SonificationAudioBufferList audioBuffers;
	SonificationHistogramBufferList histogramBuffers;


	struct HistogramSoundSource::AudioGenerationInformation audioGenerationConfigData;

	int checkStackEnd;

	//Shared Sift structures (they would be private except I want to balance the load and thus other cores need access to other core's sift object)
	//Pointers to OpenCV feature detector, extractor, and matcher
	//Thus these are pointers to arrays that will be configured based on the number of cores
	Ptr<FeatureDetector> * sharedFeatureDetectors;
	Ptr<DescriptorExtractor> * sharedDescriptorExtractors;
	Ptr<DescriptorMatcher>  * sharedFeatureMatchers;

	//Pointer to Vedaldi SIFT
	Ptr<VL::Sift> * sharedSiftPtrs;



	virtual void clearData()
	{
		
	}

};


//This is the configuration struct
struct VisualSonificationStatePrivateConfig : BaseConfig
{
	Rect regionOfInterest[VISUAL_SONIFICATION_NUMBER_OF_STATES];
	Rect bufferedRegionOfInterest[VISUAL_SONIFICATION_NUMBER_OF_STATES];
	Point2i regionBufferSize[VISUAL_SONIFICATION_NUMBER_OF_STATES];
	int horizontalThreadId;
	int verticalThreadId;



};

//This contains the data used by the application
struct VisualSonificationStatePrivateData : BaseData
{
	int counter;
	virtual void clearData()
	{
		
	}

};


//Main structure to pass data around
struct VisualSonificationInfo : BaseInfo
{
	int checkStackStart;
	VisualSonificationStateConfig config;
	VisualSonificationStateData data;
	int checkStackEnd;

};

//Main structure to pass data around
struct VisualSonificationMultithreadInfo
{
	ThreadManager * threadManagerPtr;
	vector<GeneralWorkerThreadDataHolder> workerThreadDataHolders;
	vector<VisualSonificationStatePrivateConfig> privateConfigs;
	vector<VisualSonificationStatePrivateData> privateDatas;


};


//Wrapper class for visual sonification
class VisualSonification 
{
public:
	VisualSonification();
	~VisualSonification();


	static void flattenSoundBufferList(struct SonificationAudioBufferList *listIn, unsigned char ** bufferOut);
	static void flattenSoundBuffer (struct SonificationAudioBuffer * bufferIn, unsigned char ** bufferOut);
	static uint32_t getflattenedSoundBufferListSize (unsigned char * bufferOut);

	static void unflattenSoundBufferList(unsigned char * bufferIn,struct SonificationAudioBufferList *listOut);
	static void unflattenSoundBuffer ( unsigned char * bufferIn,struct SonificationAudioBuffer * bufferOut);

	static void playSoundBufferList ( SonificationAudioBufferList * bufferList);



	static void unflattenHistogramBufferList(unsigned char * bufferIn,struct SonificationHistogramBufferList *listOut);
	static void playHistogramBufferList ( SonificationHistogramBufferList * bufferList,HistogramSoundSource::AudioGenerationInformation &audioGenerationConfigData);
	static void playHistogramBuffer ( struct SonificationHistogramBuffer & buffer,HistogramSoundSource::AudioGenerationInformation &audioGenerationConfigData);
	static void saveHistogramBufferAudioFile(string filename, struct SonificationHistogramBuffer & buffer, enum HistogramSoundSource::SoundGenerationTechniques technique,HistogramSoundSource::AudioGenerationInformation &audioGenerationConfigData);
	static void saveHistogramToFile(string filename, struct SonificationHistogramBuffer & buffer);


	SonificationAudioBufferList * getAudioBufferList();
	SonificationHistogramBufferList * getHistogramBufferList();


	//Initializr the sonificaiton processing by loading the config file
	void initSonificationProcessing();
	void initSonificationProcessing(vector<string> commandArgs);


	//Run through one iteration or single step state based on setting
	int runSonificationProcessing();

	//Clean up sonification processing and prepare to exit
	bool endSonificationProcessing();

	//Set the next images for processing
	bool setImagesForProcessing(unsigned char * leftImageData,unsigned char * rightImageData, int width, int height, int yStepInBytes, int xStepInBytes,int channels);

	//Set the next images for processing
	bool setImagesForProcessing(Mat leftImageData,Mat rightImageData);


	//Get an image output from sonification
	unsigned char * getSonificationImageData(int cameraId,int imageId, int &width, int &height, int &stepInbytes);

	//Get an image output from sonification
	Mat getSonificationImageData(int cameraId, int imageId);

	//Get an image output from sonification
	void getSonificationImageData(int cameraId, int imageId, Mat& imageData );


	Size getCurrentFrameSize();


	//Get the current sonification State
	int getSonificationState();
private:
	VisualSonificationInfo visualSonificationInfo;
	VisualSonificationMultithreadInfo visualSonificationMultithreadInfo;

	VisualSonificationInfo * infoForMouse;
};




//extern VisualSonification visualSonificationObject;
#ifndef ANDROID
void visualSonificationClassTechniqueOnPC();
#endif

void parseAudioGenerationInformationFromParamsVector(vector<string> &commandArgs,struct HistogramSoundSource::AudioGenerationInformation & audioGenInfo);
struct HistogramSoundSource::AudioGenerationInformation loadAndParseAudioGenerationParams(string filename);
#endif 
