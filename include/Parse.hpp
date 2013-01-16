#ifndef PARSE_HPP
#define PARSE_HPP


#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <opencv2/core/core.hpp>
#include <opencv2/core/operations.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/core/mat.hpp>
#include <opencv2/imgproc/imgproc.hpp>


using namespace std;
using namespace cv;

struct AnnotatedDatasetInfo{
	Mat Labels;
	vector< vector<Point> >  Points;
	Mat Depths;
};



AnnotatedDatasetInfo getAnnotatedImageData(Mat Image , string datafile);


#endif
