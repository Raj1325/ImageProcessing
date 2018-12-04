

/*

	how to resize image maintaining aspect ratio?

	rescaleKeepingAspect( sourceImage , requiredScale )
	{
		requiredAspect[2] = {requiredScale.width/sourceImage.width , requiredScale.height/sourceImage.height} 

		if( requiredAspect[0] < requiredAspect[1] )
		{	
			return { sourceImage.width * requiredAspect[0] , sourceImage.height * requiredAspect[0] }
		}
		else{
			return { sourceImage.width * requiredAspect[1] , sourceImage.height * requiredAspect[1] }
		}
	}

*/
#include<opencv2/core/core.hpp>
#include<opencv2/highgui/highgui.hpp>
#include<opencv2/imgproc/imgproc.hpp>
#include<opencv2/xfeatures2d.hpp>
#include<iostream>

using namespace cv;
using namespace std;

void	performTaskOne(vector<String>);
void	performTaskTwo(vector<String>);

void	rescalePreservingAspectRatio(const Mat &, Mat&, Size);
void	extractYComponent(const Mat &, Mat &);
void	drawCircles(Mat &, const vector<KeyPoint>&);

// image file names are passed as comma seperated, this function is used to get the individual file names.
vector<String>	getFileNames(const char * , int&); 
String	split(String, char); //split the string using delimiter


int main(int argc, char **argv)
{
	int totalImages;
	vector<String> fileNames = getFileNames(argv[1], totalImages);

	if (argc >= 2)
	{
		if (fileNames.size() == 1)
		{
			performTaskOne(fileNames);

		}
		else if (fileNames.size() > 1)
		{
			performTaskTwo(fileNames);
		}
	}
	else {
		cout << "Invalid arguments..." << endl;
		return EXIT_FAILURE;
	}

	waitKey(0);
	destroyAllWindows();
	return EXIT_SUCCESS;

}

void	performTaskOne(vector<String> imageNames)
{
	Mat srcImage = imread(imageNames[0]);
	Mat rescaledImage , yimage1D , plottedImage;
	String windowName = "TaskOne";
	Ptr<Feature2D> shift = xfeatures2d::SIFT::create();
	vector<KeyPoint> keypoints;

	rescalePreservingAspectRatio(srcImage, rescaledImage, Size(600, 480));
	extractYComponent(rescaledImage, yimage1D);
	shift->detect(yimage1D, keypoints);
	cout << "# of keypoints in " << imageNames[0] <<" is " << keypoints.size() << endl;
	yimage1D.copyTo(plottedImage);
	drawCircles(plottedImage, keypoints);

	//get images ready to display, as the source image is 3-channel image
	//and its ycomponent is single channel image, so convert ycomponent to 3-channel image
	vector<Mat> yimage3D(3);
	yimage3D[0] = plottedImage;
	yimage3D[1] = plottedImage;
	yimage3D[2] = plottedImage;

	Mat yimage , images[2] , taskOneImage;
	merge(yimage3D, yimage);

	images[0] = rescaledImage.clone();
	images[1] = yimage.clone();
	hconcat(images,2, taskOneImage);

	namedWindow(windowName, WINDOW_NORMAL);
	imshow(windowName, taskOneImage);
}


void	performTaskTwo(vector<String> imageNames)
{
	
	vector<Mat> srcImages(imageNames.size()), rescaledImages(imageNames.size()) , descriptors(imageNames.size());
	vector<Mat> ycomponent(imageNames.size());
	vector<vector<KeyPoint>> keypoints(imageNames.size());
	Ptr<Feature2D> shift = xfeatures2d::SIFT::create();
	int kvalues[] = {
		5,10,20
	};
	int totKValues = 3;
	int totalKeyPoints = 0;

	//read image file into Mat object rescale them and extract y component.
	for (int itr = 0; itr < imageNames.size(); itr++)
	{
		srcImages[itr] = (imread(imageNames[itr], CV_LOAD_IMAGE_UNCHANGED));
		Mat temp;
		rescalePreservingAspectRatio(srcImages[itr], temp, Size(600, 480));
		rescaledImages[itr] = (temp.clone());
		extractYComponent(rescaledImages[itr], ycomponent[itr]);
		temp.deallocate();
	}

	//calculate keyppoints and descriptors from the y component of all the images 
	//obtained from previous step
	for (int itr = 0; itr < rescaledImages.size(); itr++)
	{
		shift->detectAndCompute(ycomponent[itr], noArray() , keypoints[itr] , descriptors[itr]);
		cout << "# of keypoints in " << imageNames[itr] << " is " << keypoints[ itr].size() << endl;
		totalKeyPoints += keypoints[itr].size();
	}
	cout << "# of keypoints of all the images is " << totalKeyPoints << endl << endl;
	
	//start kmeans, for given values i.e 5% , 10% and 20%....
	for (int itr = 0; itr < totKValues; itr++)
	{
		int percent = (int)(kvalues[itr] * totalKeyPoints / 100);
		cout << "K = " << kvalues[itr] << "%*(" << totalKeyPoints << ") = " << percent << endl;
		//add descriptors to Bag OF Words which implements kmeans
		BOWKMeansTrainer trainer(percent);
		for (int descriptorItr = 0; descriptorItr < descriptors.size(); descriptorItr++)
		{
			trainer.add(descriptors[descriptorItr]);
		}
		//extract Visual Words, and set the Vocabulary
		Mat dictionary = trainer.cluster();
		BOWImgDescriptorExtractor BOWde(shift , new FlannBasedMatcher );
		BOWde.setVocabulary(dictionary);

		//calculate histogram for each image
		vector<Mat> histogram(imageNames.size());
		for (int imageItr = 0; imageItr < histogram.size(); imageItr++)
		{
			BOWde.compute(ycomponent[imageItr], keypoints[imageItr], histogram[imageItr]);
		}

		//calculate dissimilarity matrix...
		cout << "Dissimilarity Matrix" << endl;
		cout << "\t";
		for (int imgNamesItr = 0; imgNamesItr < imageNames.size(); imgNamesItr++)
		{
			cout << split( imageNames[imgNamesItr] , '.') << "\t\t";
		}
		cout << endl;
		for (int outerItr = 0; outerItr < imageNames.size(); outerItr++)
		{
			cout << split(imageNames[outerItr] , '.') << "  ";

			for (int innerItr = 0; innerItr < imageNames.size(); innerItr++)
			{
				cout.precision(3);
				cout <<fixed<< compareHist(histogram[outerItr] , histogram[innerItr], CV_COMP_CHISQR)<<"\t\t";
			}
			cout << endl;
		}

		cout << endl << endl << endl;
		
	}
	system("pause");
}


vector<String>	getFileNames(const char *arguments , int &totalImages)
{
	vector<String> fileNames;
	String temp;
	totalImages = 0;

	for (int itr = 0; arguments[itr] != '\0'; itr++)
	{
		if (arguments[itr] == ',')
		{
			fileNames.push_back(temp);
			temp = "";
			totalImages++;
		}
		else if(arguments[itr] != ' ' || arguments[itr] != '\t')
		{
			temp += arguments[itr];
		}
	}
	fileNames.push_back(temp);
	temp = "";
	totalImages++;
	
	return fileNames;

}


void	rescalePreservingAspectRatio(const Mat &src , Mat &dest , Size requiredSize)
{
	
	float aspectRatio[2] = { (float)requiredSize.width / src.cols , (float)requiredSize.height / src.rows };

	Size newAspectRatio(src.cols, src.rows);

	if (aspectRatio[0] < aspectRatio[1]) {
		newAspectRatio.width = (int)(newAspectRatio.width * aspectRatio[0] + 0.5);
		newAspectRatio.height = (int)(newAspectRatio.height * aspectRatio[0] + 0.5);
	}
	else {
		newAspectRatio.width = (int)(newAspectRatio.width * aspectRatio[1] + 0.5);
		newAspectRatio.height = (int)(newAspectRatio.height * aspectRatio[1] + 0.5);
	}

	resize(src, dest, newAspectRatio, 0, 0, CV_INTER_AREA);

}

void	extractYComponent(const Mat &src, Mat &yimage)
{

	Mat imgYCrCb;
	cvtColor(src, imgYCrCb, CV_BGR2YCrCb);	//Convert RGB to YUV color space

	vector<Mat> imgYCrCbPlanes;	//Use the STL's vector structure to store multiple Mat objects
	split(imgYCrCb, imgYCrCbPlanes);	//split the image into separate color planes (Y U V)

	yimage = imgYCrCbPlanes[0];	//Extract the Y component from the Y U V planes


}

void	drawCircles( Mat &dest, const vector<KeyPoint> &keypoints)
{
	int x1, y1 , x2, y2;
	for (int itr = 0; itr < keypoints.size(); itr++)
	{
		x1 = keypoints[itr].pt.x;
		y1 = keypoints[itr].pt.y + keypoints[itr].size;
		x2 = keypoints[itr].pt.x;
		y2 = keypoints[itr].pt.y - keypoints[itr].size;
		line(dest, Point(x1, y1), Point(x2, y2), Scalar(255, 0, 0));

		x1 = keypoints[itr].pt.x - keypoints[itr].size;
		y1 = keypoints[itr].pt.y;
		x2 = keypoints[itr].pt.x + keypoints[itr].size;
		y2 = keypoints[itr].pt.y;
		line(dest, Point(x1, y1), Point(x2, y2), Scalar(255, 0, 0));

		circle(dest, keypoints[itr].pt, keypoints[itr].size , Scalar(255,0,0));

	}

}

String	split(String src, char delimiter)
{
	String str = "";

	for (int itr = 0; itr < src.length(); itr++)
	{
		if (src[itr] == delimiter)
		{
			break;
		}
		else {
			str += src[itr];
		}
	}

	return str;
}
