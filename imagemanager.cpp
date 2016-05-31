#include "imagemanager.h"

#include "iostream"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/video.hpp"

using namespace cv;
using namespace std;

const int ImageManager::MATCH_METHOD = TM_CCOEFF;
const int ImageManager::LOW_THRESHOLD = 35;


ImageManager::ImageManager()
{
    templateImage = imread("/home/francesco/QtCreatorProjects/Robotica/human_figure.bmp", IMREAD_GRAYSCALE);

    cv::Canny(templateImage, templateImage, LOW_THRESHOLD, 3*LOW_THRESHOLD, 3);

    resize(templateImage, templateImage, Size(), 0.15, 0.15, cv::INTER_LANCZOS4);

}

bool ImageManager::findVictim(cv::Mat cameraImage)
{
    bool result = false;

    cvtColor(cameraImage, cameraImage, CV_BGR2GRAY);
//        cv::blur( cameraImage, cameraImage, Size(3,3) );
    cv::Canny(cameraImage, cameraImage, LOW_THRESHOLD, 3*LOW_THRESHOLD, 3);

    Mat img_display;
    cameraImage.copyTo( img_display );


    /// Do the Matching and Normalize
    matchTemplate( cameraImage, templateImage, resultImage, MATCH_METHOD );

    /// Localizing the best match with minMaxLoc
    double minVal;
    double maxVal;
    Point minLoc;
    Point maxLoc;
    Point matchLoc;

    minMaxLoc( resultImage, &minVal, &maxVal, &minLoc, &maxLoc, Mat() );
    cout << "minVal: " << float(minVal) << " maxVal: " << float(maxVal)<< "\n" << endl;
    //        printf("%f %f\n", maxVal, minVal);

    /// For SQDIFF and SQDIFF_NORMED, the best matches are lower values. For all the other methods, the higher the better
    if( MATCH_METHOD  == CV_TM_SQDIFF || MATCH_METHOD == CV_TM_SQDIFF_NORMED ){
        matchLoc = minLoc;
    }
    else{
        matchLoc = maxLoc;
    }

    rectangle( img_display, matchLoc, Point( matchLoc.x + templateImage.cols , matchLoc.y + templateImage.rows ), Scalar(255,0,0), 2, 8, 0 );

    /// Show me what you got
    if(maxVal > 4.10000e+006 && maxVal < 2.30000e+007){
        rectangle( img_display, matchLoc, Point( matchLoc.x + templateImage.cols , matchLoc.y + templateImage.rows ), Scalar(255,0,0), 2, 8, 0 );
//            rectangle( resultImage, matchLoc, Point( matchLoc.x + templateImage.cols , matchLoc.y + templateImage.rows ), Scalar(255,0,0), 2, 8, 0 );
        result = true;
    }
    imshow("Resulting image", img_display );


    imshow("cameraImage", templateImage);

    if(waitKey(27) >= 0)
        return false;

    // the camera will be deinitialized automatically in VideoCapture destructor
    return result;
}
