#ifndef IMAGEMANAGER_H
#define IMAGEMANAGER_H

#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"

class ImageManager
{
public:
    ImageManager();
    bool findVictim(cv::Mat img_scene);

private:
    static const int MATCH_METHOD;
    static const int LOW_THRESHOLD;

    cv::Mat templateImage;
    cv::Mat resultImage;
};

#endif // IMAGEMANAGER_H
