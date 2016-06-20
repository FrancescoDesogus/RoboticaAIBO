#include "imagemanager.h"

#include "iostream"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/video.hpp"

using namespace cv;
using namespace std;

const int ImageManager::MATCH_METHOD = TM_CCOEFF_NORMED;
const int ImageManager::LOW_THRESHOLD = 35;


ImageManager::ImageManager()
{
    templateImageArray.push_back(imread("/home/francesco/QtCreatorProjects/Robotica/human_figure.bmp", IMREAD_GRAYSCALE));
    templateImageArray.push_back(imread("/home/francesco/QtCreatorProjects/Robotica/human_figure10.bmp", IMREAD_GRAYSCALE));
    templateImageArray.push_back(imread("/home/francesco/QtCreatorProjects/Robotica/human_figure-10.bmp", IMREAD_GRAYSCALE));

//    cv::Canny(templateImage, templateImage, LOW_THRESHOLD, 3*LOW_THRESHOLD, 3);


    for(int i = 0; i < templateImageArray.size(); i++)
        resize(templateImageArray[i], templateImageArray[i], Size(), 0.10, 0.10, cv::INTER_LANCZOS4);


    imshow("cameraImage", templateImageArray[0]);

}

bool ImageManager::findVictim(cv::Mat cameraImage)
{
    bool result = false;

    cameraImage.convertTo(cameraImage, -1, 0.5, 0); //decrease the contrast (halve)


    cvtColor(cameraImage, cameraImage, CV_BGR2GRAY);
//        cv::blur( cameraImage, cameraImage, Size(3,3) );
//    cv::Canny(cameraImage, cameraImage, LOW_THRESHOLD, 3*LOW_THRESHOLD, 3);

//    Mat img_display;
//    cameraImage.copyTo( img_display );


    float max = 0;


    for(int i = 0; i < templateImageArray.size() && !result; i++)
    {
        /// Do the Matching and Normalize
        matchTemplate( cameraImage, templateImageArray[i], resultImage, MATCH_METHOD );
        threshold(resultImage, resultImage, 0.1, 1, CV_THRESH_TOZERO);

        /// Localizing the best match with minMaxLoc
        double minVal;
        double maxVal;
        Point minLoc;
        Point maxLoc;
        Point matchLoc;

        minMaxLoc( resultImage, &minVal, &maxVal, &minLoc, &maxLoc, Mat() );

        if(float(maxVal) > 0.5)
            cout << "minVal: " << float(minVal) << " maxVal: " << float(maxVal)<< "\n" << endl;

        if(maxVal > max)
            max = maxVal;

        /// For SQDIFF and SQDIFF_NORMED, the best matches are lower values. For all the other methods, the higher the better
        if( MATCH_METHOD  == CV_TM_SQDIFF || MATCH_METHOD == CV_TM_SQDIFF_NORMED ){
            matchLoc = minLoc;
        }
        else{
            matchLoc = maxLoc;
        }

    //    rectangle( img_display, matchLoc, Point( matchLoc.x + templateImage.cols , matchLoc.y + templateImage.rows ), Scalar(255,0,0), 2, 8, 0 );

        /// Show me what you got
        if(maxVal > 0.63) {
            rectangle( cameraImage, matchLoc, Point( matchLoc.x + templateImageArray[i].cols , matchLoc.y + templateImageArray[i].rows ), Scalar(255,0,0), 2, 8, 0 );
    //            rectangle( resultImage, matchLoc, Point( matchLoc.x + templateImage.cols , matchLoc.y + templateImage.rows ), Scalar(255,0,0), 2, 8, 0 );
            result = true;
        }
    }

    // Recupero la nota da stampare sulla regione e ne calcolo la grandezza
   std::string label = std::to_string(max);
   int baseline=0;
   Size textLength = getTextSize(label, FONT_HERSHEY_PLAIN, 1, 1, &baseline);

   // Inserisco la label al centro
   putText(cameraImage, label, Point(cameraImage.size().width/2 - textLength.width/2, cameraImage.size().height/2 - textLength.height/2), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(255,0,0), 1.0, LINE_8, false);

   imshow("Resulting image", cameraImage );

    // the camera will be deinitialized automatically in VideoCapture destructor
    return result;
}
