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

#include "CodeWordDictionaryUtilities.h"





void codeWordGenLoadFeatureFiles(string filename, Mat & descriptors, vector<KeyPoint> & keypoints)
{
		FileStorage fs;
	
		fs.open(filename,FileStorage::READ);

		//Clean out the data
		keypoints.clear();
		descriptors.release();

		if(fs.isOpened())
		{

			fs["Descriptors"] >> descriptors;

			FileNode keyPointsNode = fs["Keypoints"];
			read((const FileNode)keyPointsNode, keypoints);

			fs.release();
		}
		else
		{

			cout << "ERROR: Unable to load feature file" << endl;
		}

	




}


void codeWordGenLoadDictionary(string filename, Mat & dictionary, int & numberOfDictionaryEntries)
{
		FileStorage fs;
	
		fs.open(filename,FileStorage::READ);

		//Clean out the data
		dictionary.release();

		if(fs.isOpened())
		{

			fs["Dictionary"] >> dictionary;

			fs["DictionarySize"] >> numberOfDictionaryEntries;

			fs.release();
#ifdef VERBOSE_OUTPUT
			for(int i =0; i < dictionary.rows; i++)
			{
				cout <<"Row " <<i << ": [" << endl;
				for(int j = 0; j < dictionary.cols; j++)
				{
					cout << " " << dictionary.at<float>(i,j) << ",";


				}
				cout << "] \n" << endl;

			}
#endif
		}
		else
		{

			cout << "ERROR: Unable to load dictionary file" << endl;
		}






}


void codeWordGenLoadDictionaryPlus(string filename, Mat & dictionary, int & numberOfDictionaryEntries,Mat & labels, Mat & descriptors, vector<KeyPoint> keypoints)
{
		FileStorage fs;
	
		fs.open(filename,FileStorage::READ);

		//Clean out the data
		dictionary.release();

		if(fs.isOpened())
		{

			fs["Dictionary"] >> dictionary;

			fs["DictionarySize"] >> numberOfDictionaryEntries;

			fs["Descriptors"] >> descriptors;

			FileNode keyPointsNode = fs["Keypoints"];
			read((const FileNode)keyPointsNode, keypoints);

			fs["Labels"] >> labels;

			fs.release();
		}
		else
		{

			cout << "ERROR: Unable to load dictionary file" << endl;
		}






}
