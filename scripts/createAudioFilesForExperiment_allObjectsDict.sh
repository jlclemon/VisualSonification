#!/bin/bash


export LD_LIBRARY_PATH=$OpenCV_DIR/lib:$LD_LIBRARY_PATH

VISUAL_SONIFICATION_EXEC=/home/jlclemon/workspace/VisualSonificationInAndroidDir/Release/VisualSonificationInAndroidDir

VISUAL_SONIFICATION_BASE_CONFIG_FILE=/home/jlclemon/Downloads/BlazeBuild/mydroid/external/sonification/source/jni/configFileLinuxSegmentBase.txt

VISUAL_SONIFICATION_BASE_CONFIG_COMMAND="-configFile $VISUAL_SONIFICATION_BASE_CONFIG_FILE"

DICTIONARY_FILE_BASE_PATH=/home/jlclemon/Documents/visionData/visionData/Dictionaries/

AUDIO_FILE_OUTPUT_DIR_APPEND=audioFiles/

IMAGE_LIST_BASE_DIR=/media/Windows/GSRC_Dataset/3Ddataset/
#/media/0459867d-39ad-440d-86e0-2c0596eed1e8/home/jlclemon/GSRC_Dataset/3Ddataset/

IMAGE_LIST_BASE_FILE="imageList.txt"

ObjectClassList=('bicycle' 'car' 'cellphone' 'head' 'iron' 'monitor' 'mouse' 'shoe' 'stapler' 'toaster')
let ObjectClassCount=${#ObjectClassList[@]}

ObjectIdList=('1' '2' '3' '4' '5' '6' '7' '8' '9' '10')
let ObjectIdCount=${#ObjectIdList[@]}


DictionaryFileList=('dictionaryFile_AllObjects_10.yml' 'dictionaryFile_AllObjects_15.yml' 'dictionaryFile_AllObjects_20.yml' 'dictionaryFile_AllObjects_25.yml' 'dictionaryFile_AllObjects_30.yml' 'dictionaryFile_AllObjects_40.yml' 'dictionaryFile_AllObjects_50.yml' 'dictionaryFile_AllObjects_75.yml' 'dictionaryFile_AllObjects_100.yml')
let DictionaryFileCount=${#DictionaryFileList[@]}

DictionaryFileDirList=('10' '15' '20' '25' '30' '40' '50' '75' '100')



SonificationTechniqueList=('pure_sine' 'pure_sine_linear_transition' 'fundamental_with_2_harmonics' 'harmonics_piano')

SonificationTechniqueDirList=('sine' 'transition' 'harmonic'  'piano')

let SonificationTechniqueCount=${#SonificationTechniqueList[@]}


#if [ -d "$AUDIO_FILE_OUTPUT_DIR" ]; then 
#    if [ -L "$AUDIO_FILE_OUTPUT_DIR" ]; then
        # It is a symlink!
        # Symbolic link specific commands go here
#        rm -rf "$AUDIO_FILE_OUTPUT_DIR"
#    else
        # It's a directory!
        # Directory command goes here
#        rmdir -r "$AUDIO_FILE_OUTPUT_DIR"
#    fi
#   mkdir "$AUDIO_FILE_OUTPUT_DIR"
#
#fi




for (( currentDictionary=0;currentDictionary<$DictionaryFileCount;currentDictionary++)); do
	for (( currentClass=0;currentClass<$ObjectClassCount;currentClass++)); do
		for (( currentObject=0;currentObject<$ObjectIdCount;currentObject++)); do

			for (( currentSonTech=0;currentSonTech<$SonificationTechniqueCount;currentSonTech++)); do


				CURRENT_BASE_DIR="$IMAGE_LIST_BASE_DIR${ObjectClassList[${currentClass}]}/${ObjectClassList[${currentClass}]}_${ObjectIdList[${currentObject}]}/"
			
				AUDIO_FILE_OUTPUT_DIR="$CURRENT_BASE_DIR$AUDIO_FILE_OUTPUT_DIR_APPEND${SonificationTechniqueDirList[${currentSonTech}]}/${DictionaryFileDirList[${currentDictionary}]}/"
			
				CURRENT_AUDIO_OUTPUT_PARAM="-regionAudioFileOutputDir $AUDIO_FILE_OUTPUT_DIR"
				CURRENT_HISTOGRAM_OUTPUT_PARAM="-regionHistogramFileOutputDir $AUDIO_FILE_OUTPUT_DIR"


				CURRENT_SONIFICATION_TECHNIQUE_PARAM="-sonificationTechnique ${SonificationTechniqueList[${currentSonTech}]}"

				DICTIONARY_FILE="$DICTIONARY_FILE_BASE_PATH${DictionaryFileList[${currentDictionary}]}"
			
				CURRENT_DICTIONARY_PARAM="-dictionaryFile $DICTIONARY_FILE"

				CURRENT_IMAGE_LIST="$CURRENT_BASE_DIR$IMAGE_LIST_BASE_FILE"
			
				CURRENT_IMAGE_LIST_PARAM="-leftImageList $CURRENT_IMAGE_LIST"

				CURRENT_IMAGE_LIST_BASE_DIR_PARAM="-leftImageListBaseDir $CURRENT_BASE_DIR"



				if [ ! -d "$AUDIO_FILE_OUTPUT_DIR" ]; then 
				    echo "Making Directory for audio"
				    mkdir -p "$AUDIO_FILE_OUTPUT_DIR"
				fi

				CURRENT_COMMAND="$VISUAL_SONIFICATION_EXEC $VISUAL_SONIFICATION_BASE_CONFIG_COMMAND $CURRENT_AUDIO_OUTPUT_PARAM $CURRENT_HISTOGRAM_OUTPUT_PARAM $CURRENT_DICTIONARY_PARAM $CURRENT_IMAGE_LIST_PARAM $CURRENT_IMAGE_LIST_BASE_DIR_PARAM $CURRENT_SONIFICATION_TECHNIQUE_PARAM"

				echo "Running command once for test"
				$CURRENT_COMMAND


				#echo "Current Command: $CURRENT_COMMAND"
				#echo "Current Audio Output Dir: $AUDIO_FILE_OUTPUT_DIR"
				#echo "Dictionary File : $DICTIONARY_FILE"
				#echo "Current Image List: $CURRENT_IMAGE_LIST"
			done
		done
	done
done 



echo "Done diggity."
#$CURRENT_COMMAND


