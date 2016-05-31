#ifndef IMAGEMANAGER_H
#define IMAGEMANAGER_H

#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"

class ImageManager
{
public:
    ImageManager();
    bool findVictim(cv::Mat image);
};

#endif // IMAGEMANAGER_H
