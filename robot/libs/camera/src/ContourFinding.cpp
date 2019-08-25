#include "ContourFinding.h"

ContourFinding::ContourFinding(){}

ContourFinding::ContourFinding(Mat frame){
	sourceFrame = frame;
	outputFrame = frame;
	pointToStartCutting = Point(0,outputFrame.rows/3);
	pointToFinishCutting = Point(outputFrame.cols, 2*(outputFrame.rows)/3);
}

void ContourFinding::setFrame(Mat frame){
	sourceFrame = frame;
	outputFrame = frame;
	contour.erase(contour.begin(),contour.end());
	hierarchy.erase(hierarchy.begin(),hierarchy.end());
	pointToStartCutting = Point(0,outputFrame.rows/3);
	pointToFinishCutting = Point(outputFrame.cols, 2*(outputFrame.rows)/3);
}

Mat ContourFinding::getSourceFrame(){
	return sourceFrame;
}

Mat ContourFinding::getOutputFrame(){
	return outputFrame;
}

Point ContourFinding::getStartPointToApproachCutting(){
	return pointToStartCutting;
}

Point ContourFinding::getEndPointToApproachCutting(){
	return pointToFinishCutting;
}

void ContourFinding::setPointsToApproachCutting(Point startPoint, Point endPoint){
	pointToStartCutting = startPoint;
	pointToFinishCutting = endPoint;
}

void ContourFinding::setScaleFactor(double scaleFactor){
	this->scaleFactor = scaleFactor;
}

double ContourFinding::getScaleFactor(){
	return this->scaleFactor;
}

//przeskalowanie kaltki
void ContourFinding::scaleImage(){
	resize(outputFrame, outputFrame, Size(0,0), scaleFactor, scaleFactor, 2); //2->INTER_AREA_INTERPOLATION
	resize(sourceFrame, sourceFrame, Size(0,0), scaleFactor, scaleFactor, 2); //2->INTER_AREA_INTERPOLATION
	pointToStartCutting = Point(0,outputFrame.rows/3);
	pointToFinishCutting = Point(outputFrame.cols, 2*(outputFrame.rows)/3);	
}

void ContourFinding::cutImage(){
	Rect r(pointToStartCutting, pointToFinishCutting);
	outputFrame = outputFrame(r);
}

void ContourFinding::toGrayScale(){
	cvtColor(outputFrame, outputFrame, cv::COLOR_RGB2GRAY);
}
void ContourFinding::useBlur(){
	medianBlur (outputFrame, outputFrame, max_kernel_length);
}

void ContourFinding::erodeFrame(Mat element){
	erode(outputFrame, outputFrame, element );
}

void ContourFinding::dilateFrame(Mat element){
	dilate(outputFrame, outputFrame, element);
}

void ContourFinding::thresholdFrame(){
	threshold(outputFrame, outputFrame, 127, 255, THRESH_BINARY_INV);
}

vector<Point> ContourFinding::findCenters(){
	vector<Point> centers;
	vector<vector<Point>> contours;
    	vector<Vec4i> hierarchy;

	double largest_area;
	int largest_contour_index;

	//znalezienie konturów (najbardziej zewnętrznych)
	findContours(outputFrame, contours, hierarchy, RETR_EXTERNAL, CHAIN_APPROX_NONE);    

	//jeśli znaleziono jakiś kontur
	if(contours.size()>0) {
		//szukanie największego konturu
		for( int i = 0; i< contours.size(); i++ ){
       			double a=contourArea(contours[i],false);
       			if(a>largest_area)
			{
       				largest_area=a;
       				largest_contour_index=i;
       			}
    		}

		vector<Point> contour = contours[largest_contour_index];
		this->contour=contour;
		this->hierarchy = hierarchy;

		//wyliczenie momentów figury
		Moments mu = moments(contours[largest_contour_index], false);

		//wyliczenie i zapamiętanie środka cięzkości figury
		centers.push_back(Point(mu.m10/mu.m00, mu.m01/mu.m00));
	}
	
	//jeśli nie znaleziono konturu
	else{
		printf("No contours \n");
	} 

	return centers;
}

vector<Point> ContourFinding::findLineCenters(){
	scaleImage();

	cutImage();

	toGrayScale();

	//useBlur();//commented, because might be unuseful

	Mat element = getStructuringElement( MORPH_RECT,Size( 2*max_kernel_size + 1, 2*max_kernel_size+1 ),Point( max_kernel_size, max_kernel_size ));

	erodeFrame(element);

	dilateFrame(element);

	thresholdFrame();

	return findCenters();
}

Mat ContourFinding::drawPoints(vector<Point> centers){
	Mat frame = sourceFrame.clone();
	vector<vector<Point>> contours;

	//rysowanie wykrytych środków ciężkosci linii i konturów
	Scalar color(0, 0, 255);

	
	//przesunięcie konturu
	for( int i = 0; i< contour.size(); i++ )
    	{
        	contour[i]+=pointToStartCutting;
    	}
	
	contours.push_back(contour);

	//rysowanie konturu
	drawContours(frame, contours, 0, color, 2, 8);
	

	//rysowanie linii zasięgu
	//line(frame, pointToStartCutting, pointToFinishCutting, Scalar(0, 255, 0));
	rectangle( frame, pointToStartCutting, pointToFinishCutting, Scalar( 0, 255, 0), 0, 8 );


	//rysowanie środka ciężkosci konturu
	for(int i=0;i<centers.size();i++)
	{
    		//rysowanie wykrytego środka ciężkosci figury
    		circle(frame, centers[i]+pointToStartCutting, 5, color, -1, 8);
    	}

	return frame;
}
		