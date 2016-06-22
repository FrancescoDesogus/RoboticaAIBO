#ifndef IMAGEMANAGER_H
#define IMAGEMANAGER_H

#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include <vector>

class ImageManager
{
public:
    ImageManager();
    bool findVictim(cv::Mat img_scene);
    cv::Mat getProcessedImage();

private:
    static const int MATCH_METHOD; // Metodo di matching da usare
    static const double THRESHOLD; // Threshold per considerare un'immagine come una vittima

    // Array con le immagini template
    std::vector<cv::Mat> templateImageArray;

    // Ultima immagine processata
    cv::Mat processedImage;
};

#endif // IMAGEMANAGER_H
