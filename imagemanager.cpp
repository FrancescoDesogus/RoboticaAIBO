#include "imagemanager.h"

#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"

ImageManager::ImageManager()
{
}

bool ImageManager::findVictim(cv::Mat image)
{

    return fa  VideoCapture cap(0); // open the default camera
    if(!cap.isOpened())  // check if we succeeded
        return -1;

    img_object = imread("human_figure.bmp", IMREAD_GRAYSCALE);
//    cv::blur( img_object, img_object, Size(3,3) );
    cv::Canny(img_object, img_object, lowThreshold, 3*lowThreshold, 3);
    resize(img_object, img_object, Size(), 0.4, 0.4, cv::INTER_LANCZOS4);
    match_method = 4;

    for(;;)
    {
        cap >> img_scene; // get a new img_scene from camera

        cvtColor(img_scene, img_scene, CV_BGR2GRAY);
//        cv::blur( img_scene, img_scene, Size(3,3) );
        cv::Canny(img_scene, img_scene, lowThreshold, 3*lowThreshold, 3);

        Mat img_display;
        img_scene.copyTo( img_display );

        /// Create the result matrix
        int result_cols =  img_scene.cols - img_object.cols + 1;
        int result_rows = img_scene.rows - img_object.rows + 1;

        result.create( result_rows, result_cols, CV_32FC1 );

        /// Do the Matching and Normalize
        matchTemplate( img_scene, img_object, result, match_method );
        //        normalize( result, result, 0, 1, NORM_MINMAX, -1, Mat() );

        /// Localizing the best match with minMaxLoc
        double minVal;
        double maxVal;
        Point minLoc;
        Point maxLoc;
        Point matchLoc;

        minMaxLoc( result, &minVal, &maxVal, &minLoc, &maxLoc, Mat() );
        cout << "minVal: " << float(minVal) << " maxVal: " << float(maxVal)<< "\n" << endl;
        //        printf("%f %f\n", maxVal, minVal);

        /// For SQDIFF and SQDIFF_NORMED, the best matches are lower values. For all the other methods, the higher the better
        if( match_method  == CV_TM_SQDIFF || match_method == CV_TM_SQDIFF_NORMED ){
            matchLoc = minLoc;
        }
        else{
            matchLoc = maxLoc;
        }

        /// Show me what you got
        if(maxVal > 4.10000e+006 && maxVal < 2.30000e+007){
            rectangle( img_display, matchLoc, Point( matchLoc.x + img_object.cols , matchLoc.y + img_object.rows ), Scalar(255,0,0), 2, 8, 0 );
            rectangle( result, matchLoc, Point( matchLoc.x + img_object.cols , matchLoc.y + img_object.rows ), Scalar(255,0,0), 2, 8, 0 );
        }
        imshow( image_window, img_display );
        imshow( result_window, result );


        //        imshow("matches", img_matches);

        imshow("img_scene", img_object);
        if(waitKey(27) >= 0)
            break;
    }
    // the camera will be deinitialized automatically in VideoCapture destructor
    return false;
}
