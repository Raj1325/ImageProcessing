/*
GRAYSCALE CONVERSION:
1.Where-erver possible I have used a flag COLOR_BGR2GRAY
2.If I am using single channel image I have used saturate_cast<uchar>(<data>)
to display the components in gray-scale images whose pixel values are within 0 to 255.
*/

#include <iostream>
#include <cstdlib>
#include <string>
#include <opencv2\core\core.hpp>
#include <opencv2\highgui\highgui.hpp>
#include <opencv2\imgproc\imgproc.hpp>

using namespace std;
using namespace cv;

bool performTaskOne(const string, const string);
void getSeperateChannel(Mat *, int);

bool performTaskTwo(const string);
void generateInterpolatedImage(const Mat, Mat &);
void generateColorCorrectedImage(const Mat, Mat &);
void generateGammaCorrectedImage(const Mat, Mat &, float gamma);

const uchar	 ARGC_FOR_TASKONE = 3;
const uchar	 ARGC_FOR_TASKTWO = 2;

const uchar CHANNELS = 3;
const uchar BCHANNEL = 0;
const uchar GCHANNEL = 1;
const uchar RCHANNEL = 2;

const uchar RAW_IMAGE = 0;
const uchar INTERPOLATED_IMAGE = 1;
const uchar COLOR_CORRECTED_IMAGE = 2;
const uchar GAMMA_CORRECTED_IMAGE = 3;

const float GAMMA_VALUE = 0.5;

int main(int argc, char **argv)
{
	int returnSignal = EXIT_FAILURE;


	if (argc <= 1)
	{
		cout << "No Sufficient Arguments ...." << endl;
		return EXIT_FAILURE;
	}
	else if (argc > 3)
	{
		cout << "Too Many Arguments ...." << endl;
		return EXIT_FAILURE;
	}


	if (argc == ARGC_FOR_TASKONE)
	{
		/*
		argv[2] : filename
		argv[1] : colorSpace code
		*/
		returnSignal = performTaskOne(argv[2], argv[1]);
	}
	else if (argc == ARGC_FOR_TASKTWO)
	{
		//argv[1] : fileName
		returnSignal = performTaskTwo(argv[1]);
	}


	return returnSignal;
}


bool performTaskOne(const string fileName, const string colorSpace)
{
	bool taskOneSucceded = false;


	/*
	Images array contains the images that are going to be displayed.
	Images[0] : contains original image i.e image from .bmp file
	now suppose we want display -YCrCb color space than
	Image[1] : contains Y channel
	Image[2] : contains Cr channel
	Image[3] : contains Cb channel
	*/
	Mat Images[4];


	Images[RAW_IMAGE] = imread(fileName, CV_LOAD_IMAGE_COLOR);
	if (!Images[RAW_IMAGE].data)
	{
		return taskOneSucceded;
	}

	int width = Images[RAW_IMAGE].cols / 2;
	int height = Images[RAW_IMAGE].rows / 2;
	string c1, c2, c3;

	if (colorSpace == "-XYZ")
	{
		getSeperateChannel(Images, CV_BGR2XYZ);
		c1 = "X";
		c2 = "Y";
		c3 = "Z";

	}
	else if (colorSpace == "-Lab")
	{
		getSeperateChannel(Images, CV_BGR2Lab);
		c1 = "L";
		c2 = "a";
		c3 = "b";
	}
	else if (colorSpace == "-YCrCb")
	{
		getSeperateChannel(Images, CV_BGR2YCrCb);
		c1 = "YCrCb_Y";
		c2 = "Cr";
		c3 = "Cb";
	}
	else if (colorSpace == "-HSB")
	{
		getSeperateChannel(Images, CV_BGR2HSV);
		c1 = "H";
		c2 = "S";
		c3 = "B";
	}
	else {
		return taskOneSucceded;
	}
	putText(Images[RAW_IMAGE], "Raw Image ", Point(0, 50), FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 1, LINE_AA);
	putText(Images[1], c1, Point(0, 50), FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 1, LINE_AA);
	putText(Images[2], c2, Point(0, 50), FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 1, LINE_AA);
	putText(Images[3], c3, Point(0, 50), FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 1, LINE_AA);

	resize(Images[RAW_IMAGE], Images[RAW_IMAGE], Size(width, height));
	resize(Images[1], Images[1], Size(width, height));
	resize(Images[2], Images[2], Size(width, height));
	resize(Images[3], Images[3], Size(width, height));

	Mat horizontal[2], finalOutput;
	hconcat(Images, 2, horizontal[0]);
	hconcat(&Images[2], 2, horizontal[1]);
	vconcat(horizontal, 2, finalOutput);

	string window = "Task One";
	namedWindow(window, WINDOW_AUTOSIZE);
	imshow(window, finalOutput);
	waitKey(0);
	return taskOneSucceded;
}


void getSeperateChannel(Mat *Images, int colorSpaceCode)
{
	/*
	This function is called by performTaskOne(String , Sting) function.
	Images : It is array of 4 Mat objects,
	at 0th index we have Original Image.
	at 1st index we will store 1st channel of required colorspace in GrayScale
	at 2nd index we will store 2nd channel of required colorspace in GrayScale
	at 3rd index we will store 3rd channel of required colorspace in GrayScale

	*/

	Mat ClrSpaceImage;
	//convert original color to required colorSpace
	cvtColor(Images[RAW_IMAGE], ClrSpaceImage, colorSpaceCode);

	Mat seperateChannels[CHANNELS];
	//split the colorSpace into individual channels
	split(ClrSpaceImage, seperateChannels);

	//Now convert each single channel to 3 channel equivalent.
	//so that we can display them along with original image which has 3 channels(BGR)
	for (int itr = 1; itr <= CHANNELS; itr++)
	{
		cvtColor(seperateChannels[itr - 1], Images[itr], CV_GRAY2BGR);
	}

}



bool performTaskTwo(const string fileName)
{
	bool taskTwoSucceded = false;

	/*
	Images array contains the images that are going to be displayed.
	Images[0] : contains original image i.e image from .bmp file
	Image[1] : contains interpolated image
	Image[2] : contains color corrected image
	Image[3] : contains gamma corrected image
	*/

	Mat Images[4];

	Images[RAW_IMAGE] = imread(fileName, CV_LOAD_IMAGE_UNCHANGED);

	int width = Images[RAW_IMAGE].cols / 2;
	int height = Images[RAW_IMAGE].rows / 2;


	if (!Images[RAW_IMAGE].data)
	{
		cerr << "Error reading data from file." << endl;
		return taskTwoSucceded;
	}

	//perform bilinear interpolation, on single channel .bmp image.
	generateInterpolatedImage(Images[RAW_IMAGE], Images[INTERPOLATED_IMAGE]);
	//perform color correction on interpolated image.
	generateColorCorrectedImage(Images[INTERPOLATED_IMAGE], Images[COLOR_CORRECTED_IMAGE]);
	//perform gammaCorrection on colorcorrected image.
	generateGammaCorrectedImage(Images[COLOR_CORRECTED_IMAGE], Images[GAMMA_CORRECTED_IMAGE], GAMMA_VALUE);
	//Now convert original image which has single channel, to its 3 channel equivalent
	//so that we can display it along with other images in single window.
	cvtColor(Images[RAW_IMAGE], Images[RAW_IMAGE], COLOR_GRAY2BGR);

	ostringstream os;
	os << GAMMA_VALUE;

	putText(Images[RAW_IMAGE], "Raw Image ", Point(0, 50), FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 1, LINE_AA);
	putText(Images[INTERPOLATED_IMAGE], "Interpolated Image :grbg", Point(0, 50), FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 1, LINE_AA);
	putText(Images[COLOR_CORRECTED_IMAGE], "Color Corrected Image", Point(0, 50), FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 1, LINE_AA);
	putText(Images[GAMMA_CORRECTED_IMAGE], "Gamma Corrected : " + os.str(), Point(0, 50), FONT_HERSHEY_SIMPLEX, 1, (255, 255, 255), 1, LINE_AA);

	resize(Images[RAW_IMAGE], Images[RAW_IMAGE], Size(width, height));
	resize(Images[INTERPOLATED_IMAGE], Images[INTERPOLATED_IMAGE], Size(width, height));
	resize(Images[COLOR_CORRECTED_IMAGE], Images[COLOR_CORRECTED_IMAGE], Size(width, height));
	resize(Images[GAMMA_CORRECTED_IMAGE], Images[GAMMA_CORRECTED_IMAGE], Size(width, height));

	Mat horizontal[2], finalOutput;
	hconcat(Images, 2, horizontal[0]);
	hconcat(&Images[2], 2, horizontal[1]);
	vconcat(horizontal, 2, finalOutput);

	string window = "Task Two";
	namedWindow(window, WINDOW_AUTOSIZE);
	imshow(window, finalOutput);

	waitKey(0);

	return taskTwoSucceded;
}

void generateInterpolatedImage(const Mat src, Mat &dest)
{

	// GRBG interpoltion.

	int srcCols = src.cols;
	int srcRows = src.rows;

	dest = Mat::zeros(Size(srcCols, srcRows), CV_8UC3);

	bool isEvenRow = false;
	bool isEvenCol = false;
	int bUp, bDown, bLeft, bRight, rLeft, rRight, rUp, rDown, gUp, gDown, gLeft, gRight;
	int bleftUp, bleftDown, bRightUp, bRightDown, rleftUp, rleftDown, rRightUp, rRightDown;
	double r, g, b;
	int rAverageFactor, gAverageFactor, bAverageFactor;

	//iterate through the raw image.
	for (int rowItr = 0; rowItr < srcRows; rowItr++)
	{
		isEvenRow = ((rowItr + 1) % 2 == 0) ? true : false;
		for (int colItr = 0; colItr < srcCols; colItr++)
		{
			isEvenCol = ((colItr + 1) % 2 == 0) ? true : false;

			if (isEvenRow)
			{

				//row consist of [B , G] elements , where B value is in odd column location
				//and G value is in even column location.
				if (isEvenCol)
				{

					/*
					r
					b [g] b
					r
					*/
					//Store G value
					gAverageFactor = 1; // keeps track of boundry condition
					dest.at<Vec3b>(rowItr, colItr)[GCHANNEL] = src.at<uchar>(rowItr, colItr);

					//Store R value
					rAverageFactor = 2; // keeps track of boundry condition
					rUp = 0;
					if ((rowItr - 1) >= 0)
						rUp = src.at<uchar>(rowItr - 1, colItr);
					else
						rAverageFactor--;
					rDown = 0;
					if ((rowItr + 1) <= src.rows)
						rDown = src.at<uchar>(rowItr + 1, colItr);
					else
						rAverageFactor--;
					if (rAverageFactor > 0)
					{
						r = (rUp + rDown) / rAverageFactor;
					}
					else {
						r = 0; //no r present nearby..
					}
					dest.at<Vec3b>(rowItr, colItr)[RCHANNEL] = saturate_cast<uchar>(r);

					//store B value
					bAverageFactor = 2; // keeps track of boundry condition
					bLeft = 0;
					if ((colItr - 1) >= 0)
						bLeft = src.at<uchar>(rowItr, colItr - 1);
					else
						bAverageFactor--;
					bRight = 0;
					if ((colItr + 1) <= src.cols)
						bRight = src.at<uchar>(rowItr, colItr + 1);
					else
						bAverageFactor--;
					if (bAverageFactor > 0)
					{
						b = (bLeft + bRight) / bAverageFactor;
					}
					else {
						b = 0; //no b present nearby
					}
					dest.at<Vec3b>(rowItr, colItr)[BCHANNEL] = saturate_cast<uchar>(b);

				}
				else
				{

					/*
					r  g  r
					g [b] g
					r  g  r
					*/

					//store b value
					bAverageFactor = 1; // keeps track of boundry condition
					dest.at<Vec3b>(rowItr, colItr)[BCHANNEL] = src.at<uchar>(rowItr, colItr);

					//store g value
					gAverageFactor = 4; // keeps track of boundry condition
					gUp = 0;
					if ((rowItr - 1) >= 0)
						gUp = src.at<uchar>(rowItr - 1, colItr);
					else
						gAverageFactor--;
					gDown = 0;
					if ((rowItr + 1) <= src.rows)
						gDown = src.at<uchar>(rowItr + 1, colItr);
					else
						gAverageFactor--;
					gLeft = 0;
					if ((colItr - 1) >= 0)
						gLeft = src.at<uchar>(rowItr, colItr - 1);
					else
						gAverageFactor--;
					gRight = 0;
					if ((colItr + 1) <= src.cols)
						gRight = src.at<uchar>(rowItr, colItr + 1);
					else
						gAverageFactor--;
					if (gAverageFactor > 0)
						g = (gUp + gDown + gLeft + gRight) / gAverageFactor;
					else
						g = 0;

					dest.at<Vec3b>(rowItr, colItr)[GCHANNEL] = saturate_cast<uchar>(g);


					//store r value
					rAverageFactor = 4; // keeps track of boundry condition
					rleftUp = 0;
					if ((rowItr - 1) >= 0 && (colItr - 1) >= 0)
						rleftUp = src.at<uchar>(rowItr - 1, colItr - 1);
					else
						rAverageFactor--;
					rleftDown = 0;
					if ((rowItr + 1) <= src.rows && (colItr - 1) >= 0)
						rleftDown = src.at<uchar>(rowItr + 1, colItr - 1);
					else
						rAverageFactor--;
					rRightUp = 0;
					if ((rowItr - 1) >= 0 && (colItr + 1) <= src.cols)
						rRightUp = src.at<uchar>(rowItr - 1, colItr + 1);
					else
						rAverageFactor--;
					rRightDown = 0;
					if ((rowItr + 1) <= src.rows && (colItr + 1) <= src.cols)
						rRightDown = src.at<uchar>(rowItr + 1, colItr + 1);
					else
						rAverageFactor--;
					if (rAverageFactor > 0)
						r = (rleftUp + rleftDown + rRightUp + rRightDown) / rAverageFactor;
					else
						r = 0;
					dest.at<Vec3b>(rowItr, colItr)[RCHANNEL] = saturate_cast<uchar>(r);

				}

			}
			else {

				//row consist of [G , R]
				if (isEvenCol)
				{

					/*	b  g  b
					g [r] g
					b  g  b
					*/

					//store r value
					rAverageFactor = 1;
					dest.at<Vec3b>(rowItr, colItr)[RCHANNEL] = src.at<uchar>(rowItr, colItr);

					//store g value
					gAverageFactor = 4; // keeps track of boundry condition
					gUp = 0;
					if ((rowItr - 1) >= 0)
						gUp = src.at<uchar>(rowItr - 1, colItr);
					else
						gAverageFactor--;
					gDown = 0;
					if ((rowItr + 1) <= src.rows)
						gDown = src.at<uchar>(rowItr + 1, colItr);
					else
						gAverageFactor--;
					gRight = 0;
					if ((colItr + 1) <= src.cols)
						gRight = src.at<uchar>(rowItr, colItr + 1);
					else
						gAverageFactor--;
					gLeft = 0;
					if ((colItr - 1) >= 0)
						gLeft = src.at<uchar>(rowItr, colItr - 1);
					else
						gAverageFactor--;
					if (gAverageFactor > 0)
						g = (gUp + gDown + gLeft + gRight) / gAverageFactor;
					else
						g = 0;
					dest.at<Vec3b>(rowItr, colItr)[GCHANNEL] = saturate_cast<uchar>(g);


					//store b value
					bAverageFactor = 4; // keeps track of boundry condition
					bleftUp = 0;
					if ((colItr - 1) >= 0 && (rowItr - 1) >= 0)
						bleftUp = src.at<uchar>(rowItr - 1, colItr - 1);
					else
						bAverageFactor--;
					bleftDown = 0;
					if ((colItr - 1) >= 0 && (rowItr + 1) <= src.rows)
						bleftDown = src.at<uchar>(rowItr + 1, colItr - 1);
					else
						bAverageFactor--;
					bRightUp = 0;
					if ((colItr + 1) <= src.cols && (rowItr - 1) >= 0)
						bRightUp = src.at<uchar>(rowItr - 1, colItr + 1);
					else
						bAverageFactor--;
					bRightDown = 0;
					if ((colItr + 1) <= src.cols && (rowItr + 1) <= src.rows)
						bRightDown = src.at<uchar>(rowItr + 1, colItr + 1);
					else
						bAverageFactor--;
					if (bAverageFactor > 0)
						b = (bleftUp + bleftDown + bRightUp + bRightDown) / bAverageFactor;
					else
						b = 0;
					dest.at<Vec3b>(rowItr, colItr)[BCHANNEL] = saturate_cast<uchar>(b);

				}
				else
				{

					/*
					b
					r [g] r
					b
					*/

					//store g value
					gAverageFactor = 1;
					dest.at<Vec3b>(rowItr, colItr)[GCHANNEL] = src.at<uchar>(rowItr, colItr);


					//store r value
					rAverageFactor = 2; // keeps track of boundry condition
					rLeft = 0;
					if ((colItr - 1) >= 0)
						rLeft = src.at<uchar>(rowItr, colItr - 1);
					else
						rAverageFactor--;
					rRight = 0;
					if ((colItr + 1) <= src.cols)
						rRight = src.at<uchar>(rowItr, colItr + 1);
					else
						rAverageFactor--;
					if (rAverageFactor > 0)
						r = (rLeft + rRight) / rAverageFactor;
					else
						r = 0;
					dest.at<Vec3b>(rowItr, colItr)[RCHANNEL] = saturate_cast<uchar>(r);


					//store b value
					bAverageFactor = 2; // keeps track of boundry condition
					bUp = 0;
					if ((rowItr - 1) >= 0)
						bUp = src.at<uchar>(rowItr - 1, colItr);
					else
						bAverageFactor--;
					bDown = 0;
					if ((rowItr + 1) <= src.rows)
						bDown = src.at<uchar>(rowItr + 1, colItr);
					else
						bAverageFactor--;
					if (bAverageFactor > 0)
						b = (bDown + bUp) / bAverageFactor;
					else
						b = 0;
					dest.at<Vec3b>(rowItr, colItr)[BCHANNEL] = saturate_cast<uchar>(b);

				}
			}

		}
	}

}


void generateColorCorrectedImage(const Mat src, Mat &dest)
{
	double matrix[3][3] = {
		{ 1.18,-0.05,-0.13 },
		{ -0.24,1.29,-0.05 },
		{ -0.18,-0.44,1.62 },
	};

	dest = Mat::zeros(src.size(), src.type());
	double result;

	//first two for loops iterate over image rows and column
	for (int rowItr = 0; rowItr < src.rows; rowItr++)
	{
		for (int colItr = 0; colItr < src.cols; colItr++)
		{

			//below two for loops iterate over color corection matrix
			for (int channel = 0; channel < CHANNELS; channel++)
			{
				result = 0;
				for (int matCol = 0; matCol < 3; matCol++)
				{
					result += (matrix[channel][matCol] * src.at<Vec3b>(rowItr, colItr)[matCol]);
				}

				dest.at<Vec3b>(rowItr, colItr)[channel] = saturate_cast<uchar>(result);
			}
		}
	}

}


void generateGammaCorrectedImage(const Mat src, Mat &dest, float gamma)
{

	/*
	Formula for gamma correction is :
	Output = (Constant) * (Intensity ^ gamma)
	*/

	unsigned char lookUpTable[256];

	for (int i = 0; i < 256; i++)
	{
		lookUpTable[i] = saturate_cast<uchar>(pow((float)(i / 255.0), gamma) * 255.0f);
	}

	dest = src.clone();

	//iterate over the image elements over all the channels,
	//and store the values corresponding to lookup table

	MatIterator_<Vec3b> itr, end;
	for (itr = dest.begin<Vec3b>(), end = dest.end<Vec3b>(); itr != end; itr++)
	{
		(*itr)[0] = lookUpTable[((*itr)[0])];
		(*itr)[1] = lookUpTable[((*itr)[1])];
		(*itr)[2] = lookUpTable[((*itr)[2])];
	}

}
