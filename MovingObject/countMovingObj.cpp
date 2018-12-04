/*

Segmentation of frame is done using "Mean-shift Segmentation Method"
An advanced and versatile technique for clustering based segmentation
Motion fields are detected using optical flow farneback method.

*/

#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include "opencv2/opencv.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/imgproc.hpp"
#include <iostream>
#include <string>

using namespace cv;
using namespace std;

int taskOne(const string);
int taskTwo(const string);

int main(int argc, char **argv)
{
	if (argc != 3) {
		cerr << "Enter proper arguments" << endl;
		return EXIT_FAILURE;
	}

	if (string(argv[1]) == "-b") {
		taskOne(string(argv[2]));
		destroyAllWindows();
	}
	else if (string(argv[1]) == "-s") {
		taskTwo(string(argv[2]));
		destroyAllWindows();
	}
	else {
		cerr << "Enter proper arguments n" << endl;
		return EXIT_FAILURE;
	}
	
	return EXIT_SUCCESS;
}


int taskOne(const string fileName)
{
	Mat frame , frame1; 
	Mat fgMask , fgMask3 , fgMask3Filter; 
	Mat bgImage , bgImageGray , bgImageGray3;
	Mat finalImg;
	Mat taskOneImag;
	Mat ImageH[2] , ImageV[2] , ImageCon[2];
	int erosionSize = 3;

	Ptr<BackgroundSubtractor> pMOG2; 
	char keyboard;


	VideoCapture capture(fileName);
	if (!capture.isOpened()) {
		//error in opening the video input
		cerr << "Unable to open video file: " << fileName << endl;
		exit(EXIT_FAILURE);
	}
	namedWindow("TaskOne" , CV_WINDOW_AUTOSIZE);
	pMOG2 = createBackgroundSubtractorMOG2();
	keyboard = 0;

	while (keyboard != 'q' && keyboard != 27) {
		if (!capture.read(frame1)) {
			cerr << "Unable to read next frame." << endl;
			cerr << "Exiting..." << endl;
			return 0;
		}
		
		resize(frame1, frame, Size(), 0.75, 0.75);
		pMOG2->apply(frame, fgMask);

		pMOG2->getBackgroundImage(bgImage);
		cvtColor(bgImage, bgImageGray, COLOR_BGR2GRAY);
		cvtColor(bgImageGray, bgImageGray3, CV_GRAY2BGR);

		Mat element = getStructuringElement(MORPH_OPEN,
			Size(2 * erosionSize + 1, 2 * erosionSize + 1),
			Point(erosionSize, erosionSize));
		morphologyEx(fgMask, fgMask3 , MORPH_OPEN , element);

		cvtColor(fgMask3, fgMask3Filter, CV_GRAY2BGR);
		bitwise_and(frame, fgMask3Filter, finalImg);
		
		Mat labelImage(fgMask3.size(), CV_32S);
		Mat stats, centroids;
		int nLabels = connectedComponentsWithStats(fgMask3, labelImage, stats, centroids, 8, CV_32S);
		std::vector<Vec3b> colors(nLabels);
		std::vector<int> labels_human;
		std::vector<int> labels_car;
		std::vector<int> labels_other;
		colors[0] = Vec3b(0, 0, 0);

		for (int label = 1; label < nLabels; ++label) { 
			if ((stats.at<int>(label, CC_STAT_AREA)) > 300 && (stats.at<int>(label, CC_STAT_AREA)) < 1500) {
				labels_human.push_back(label);

			}
			if ((stats.at<int>(label, CC_STAT_AREA)) > 1800 && (stats.at<int>(label, CC_STAT_AREA)) < 5500) {
				labels_car.push_back(label);
			}
			if ((stats.at<int>(label, CC_STAT_AREA)) > 5600) {
				labels_other.push_back(label);
			}
		}
		
		labels_car.empty();
		labels_human.empty();
		cvtColor(fgMask, fgMask3, CV_GRAY2BGR);
		putText(frame, "Original Frame", cv::Point(15, 15),
			FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));
		putText(bgImageGray3, "BackGround Frame", cv::Point(15, 15),
			FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(0, 0, 0));
		putText(fgMask3, "Moving Pixel Frame", cv::Point(15, 15),
			FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255));
		putText(finalImg, "Detected Objects Color", cv::Point(15, 15),
			FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255));

		ImageH[0] = frame.clone();
		ImageH[1] = bgImageGray3.clone();
		
		
		ImageV[0] = fgMask3.clone();
		ImageV[1] = finalImg.clone();

		hconcat(ImageH, 2, ImageCon[0]);
		hconcat(ImageV, 2, ImageCon[1]);
		vconcat(ImageCon, 2, taskOneImag);

		//get the frame number 
		stringstream ss;
		rectangle(frame, cv::Point(10, 2), cv::Point(100, 20),
			cv::Scalar(255, 255, 255), -1);
		ss << capture.get(CAP_PROP_POS_FRAMES);
		string frameNumberString = ss.str();
		
		int totalObjects = labels_human.size() + labels_car.size() + labels_other.size();

		cout <<"Frame "<<frameNumberString <<": " << totalObjects << " Objects ( "  <<
			labels_human.size() <<" persons , " << labels_car.size() << " cars ";
		if (labels_other.size() > 0) {
			cout << "and " << labels_other.size() << " others )" << endl;
		}
		else {
			cout << " )" << endl;
		}

		
		imshow("TaskOne", taskOneImag);
		
		keyboard = (char)waitKey(24);
	}
	
	capture.release();
	
	return 0;
}


int taskTwo(const string fileName)
{
	Mat frame, frame1 , fGray; 
	Mat fgMask, fgMask3, fgMask3Filter;
	Mat bgEstimated, bgEstimatedGray , secImage;
	Mat finalImg , flowImg;
	Mat taskOneImag;
	Mat ImageH[2], ImageV[2], ImageCon[2];
	UMat flow;
	int erosionSize = 3;
	int spatialRad = 2;	
	int colorRad = 2;
	int maxPyrLevel = 2;

	Ptr<BackgroundSubtractor> KNNSeg; 
	char keyboard; 


	VideoCapture capture(fileName);
	if (!capture.isOpened()) {
		//error in opening the video input
		cerr << "Unable to open video file: " << fileName << endl;
		exit(EXIT_FAILURE);
	}
	namedWindow("TaskTwo", CV_WINDOW_AUTOSIZE);
	KNNSeg = createBackgroundSubtractorKNN();
	keyboard = 0;

	while (keyboard != 'q' && keyboard != 27) {
		//read the current frame
		if (!capture.read(frame1)) {
			cerr << "Unable to read next frame." << endl;
			cerr << "Exiting..." << endl;
			return 0;
		}
		
		resize(frame1, frame, Size(), 0.75, 0.75);
		frame.copyTo(secImage);
		KNNSeg->apply(frame, fgMask);
		KNNSeg->getBackgroundImage(bgEstimated);
		cvtColor(bgEstimated, bgEstimatedGray, COLOR_BGR2GRAY);

		cvtColor(frame, fGray, COLOR_BGR2GRAY);
		
		calcOpticalFlowFarneback(bgEstimatedGray, fGray , flow , 0.4, 1, 12, 2, 8, 1.2, 0);
		flow.copyTo(flowImg);
		for (int y = 0; y < frame.rows; y += 5) {
			for (int x = 0; x < frame.cols; x += 5)
			{
				// get the flow from y, x position * 10 for better visibility
				const Point2f flowatxy = flowImg.at<Point2f>(y, x) * 10;
				// draw line at flow direction
				line(secImage, Point(x, y), Point(cvRound(x + flowatxy.x), cvRound(y + flowatxy.y)), Scalar(255, 0, 0));
				// draw initial point
				circle(secImage, Point(x, y), 1, Scalar(0, 0, 0), -1);


			}

		}

		Mat element = getStructuringElement(MORPH_OPEN,
			Size(2 * erosionSize + 1, 2 * erosionSize + 1),
			Point(erosionSize, erosionSize));

		morphologyEx(fgMask, fgMask3, MORPH_OPEN, element);

		cvtColor(fgMask3, fgMask3Filter, CV_GRAY2BGR);
		bitwise_and(frame, fgMask3Filter, finalImg);

		Mat labelImage(fgMask3.size(), CV_32S);
		Mat stats, centroids;
		int nLabels = connectedComponentsWithStats(fgMask3, labelImage, stats, centroids, 8, CV_32S);
		std::vector<Vec3b> colors(nLabels);
		std::vector<int> labels_human;
		std::vector<int> labels_car;
		std::vector<int> labels_other;
		colors[0] = Vec3b(0, 0, 0);

		for (int label = 1; label < nLabels; ++label) { //label  0 is the background
			if ((stats.at<int>(label, CC_STAT_AREA)) > 300 && (stats.at<int>(label, CC_STAT_AREA)) < 1500) {
				labels_human.push_back(label);

			}
			if ((stats.at<int>(label, CC_STAT_AREA)) > 1800 && (stats.at<int>(label, CC_STAT_AREA)) < 5500) {
				labels_car.push_back(label);
			}
			if ((stats.at<int>(label, CC_STAT_AREA)) > 5600) {
				labels_other.push_back(label);
			}
		}

		labels_car.empty();
		labels_human.empty();
		cvtColor(fgMask, fgMask3, CV_GRAY2BGR);
		putText(frame, "Original Frame", cv::Point(15, 15),
			FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 50, 255));
		putText(secImage, "Estimated Motion Field", cv::Point(15, 15),
			FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 50, 255));
		putText(fgMask3, "Moving Pixel Frame", cv::Point(15, 15),
			FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255));
		putText(finalImg, "Detected Objects Color", cv::Point(15, 15),
			FONT_HERSHEY_SIMPLEX, 0.5, cv::Scalar(255, 255, 255));

		ImageH[0] = frame.clone();
		ImageH[1] = secImage.clone();


		ImageV[0] = fgMask3.clone();
		ImageV[1] = finalImg.clone();

		hconcat(ImageH, 2, ImageCon[0]);
		hconcat(ImageV, 2, ImageCon[1]);
		vconcat(ImageCon, 2, taskOneImag);

		stringstream ss;
		rectangle(frame, cv::Point(10, 2), cv::Point(100, 20),
			cv::Scalar(255, 255, 255), -1);
		ss << capture.get(CAP_PROP_POS_FRAMES);
		string frameNumberString = ss.str();

		int totalObjects = labels_human.size() + labels_car.size() + labels_other.size();

		cout << "Frame " << frameNumberString << ": " << totalObjects << " Objects ( " <<
			labels_human.size() << " persons , " << labels_car.size() << " cars ";
		if (labels_other.size() > 0) {
			cout << "and " << labels_other.size() << " others )" << endl;
		}
		else {
			cout << " )" << endl;
		}


		imshow("TaskTwo", taskOneImag);
		
		keyboard = (char)waitKey(24);
	}
	
	capture.release();
	
	return 0;
}
