#include "Parse.hpp"


#ifdef PARSE_MAIN_MODULE
int main(int argc, char** argv){
	if(argc != 3){
		cout << "USAGE :: ./Parse <image_path> <datafile_path>";
		exit(-1);
	}
	string imagefile(argv[1]);
	string datafile(argv[2]);
	bool DEBUG = false;
	Mat Image = imread(imagefile, 1);

	if(DEBUG){
		// show image
		namedWindow("image", CV_WINDOW_AUTOSIZE);
		imshow("image", Image);
	}

	// get image info in a struct
	AnnotatedDatasetInfo info = getImageData(Image, datafile);

	if(DEBUG){
		// printing data
		int rows = 240;
		int cols = 320;
		cout << "Mat\n";
		for(int i = 0; i < rows; i++)
		{
			for(int j = 0; j < cols; j++)
				cout << info.Labels.at<int>(i, j) << " ";
			cout << endl;
		}
		cout << "\nDepth\n";
		for(int i = 0; i < rows; i++)
		{
			for(int j = 0; j < cols; j++)
				cout << info.Depths.at<Vec3f>(i, j)[0] << ", " << info.Depths.at<Vec3f>(i, j)[1] << ", " << info.Depths.at<Vec3f>(i, j)[2] << " :: ";
			cout << endl;
		}	
		cout << "\nPoints\n";
		for(int i = 0; i <= 1; i++)
		{
			for(int j = 0; j < info.Points[i].size(); j++)
				cout << "(" << info.Points[i][j].x << ", " << info.Points[i][j].y << "), ";
			cout << endl;
		}	

		waitKey(0);
	}
}
#endif


AnnotatedDatasetInfo getAnnotatedImageData(Mat Image, string datafile){
	AnnotatedDatasetInfo info;	
	int rows = 240;
	int cols = 320;
	float depth = 50.00;

	Mat_<signed short int> Labels(rows, cols, CV_16SC1);
	vector< vector<Point> > Points(1);
	Mat_<Vec3f> Depths(rows, cols, CV_32FC3);

	// datafile is binary image that represent object mask
	// 1 represent object, 0 means non object
	ifstream ImgData(datafile.c_str());
	int row = 0;

	if (ImgData.is_open()){
		while (ImgData.good())	{
			string line;
			getline(ImgData, line);
			if(line.length() == 0) continue;

			int col = 0;
			for(unsigned int i = 0 ; i < line.length() ; i++){
				char ch = line[i];

				// add data into "Labels"
				if(ch != ' '){
					if(ch == '1') {
						Labels(row, col) = 0;
						// add Point (row, col) in points for class 2
						Point p(col, row);
						Points[0].push_back(p);
					}
					else { 
						Labels(row, col) = -3;
						// add Point (row, col) in points for class 1
						//Point p(col, row);
						//Points[0].push_back(p);
					}
					// add depth into "Depths"
					// Depths is a R X C X 3 matrix where each pixel has (X,Y,Z) location in world coords
					Depths(row, col) = Vec3f(float(col), float(row), depth); 
					col++;
				}
			}
			row++;
		}
		ImgData.close();
	}
	else cout << "Unable to open file"; 
	info.Labels = Labels;
	info.Points = Points;
	info.Depths = Depths;
	return info;
}
