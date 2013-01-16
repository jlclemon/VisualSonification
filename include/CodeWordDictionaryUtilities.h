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

#ifndef CODE_WORD_DICTIONARY_UNITILITIES_H
#define CODE_WORD_DICTIONARY_UNITILITIES_H


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



void codeWordGenLoadFeatureFiles(string filename, Mat & descriptors, vector<KeyPoint> & keypoints);
void codeWordGenLoadDictionaryPlus(string filename, Mat & dictionary, int & numberOfDictionaryEntries,Mat & labels, Mat & descriptors, vector<KeyPoint> keypoints);
void codeWordGenLoadDictionary(string filename, Mat & dictionary, int & numberOfDictionaryEntries);

#endif
