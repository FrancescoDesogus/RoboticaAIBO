#include "imagemanager.h"

#include "iostream"
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include "opencv2/highgui.hpp"
#include "opencv2/videoio.hpp"
#include "opencv2/video.hpp"

using namespace cv;
using namespace std;

// Metodo di matching da usare
const int ImageManager::MATCH_METHOD = TM_CCOEFF_NORMED;

// Threshold per considerare un'immagine come una vittima
const double ImageManager::THRESHOLD = 0.64;


ImageManager::ImageManager()
{
    // Immagini con grandezza 0.09 dell'originale
    templateImageArray.push_back(imread("/home/francesco/QtCreatorProjects/Robotica/human_figure.bmp", IMREAD_GRAYSCALE));
    templateImageArray.push_back(imread("/home/francesco/QtCreatorProjects/Robotica/human_figure10.bmp", IMREAD_GRAYSCALE));
    templateImageArray.push_back(imread("/home/francesco/QtCreatorProjects/Robotica/human_figure-10.bmp", IMREAD_GRAYSCALE));

    // Immagini con grandezza 0.13 dell'originale
    templateImageArray.push_back(imread("/home/francesco/QtCreatorProjects/Robotica/human_figure.bmp", IMREAD_GRAYSCALE));
    templateImageArray.push_back(imread("/home/francesco/QtCreatorProjects/Robotica/human_figure10.bmp", IMREAD_GRAYSCALE));
    templateImageArray.push_back(imread("/home/francesco/QtCreatorProjects/Robotica/human_figure-10.bmp", IMREAD_GRAYSCALE));

    // Immagini con grandezza 0.15 dell'originale
    templateImageArray.push_back(imread("/home/francesco/QtCreatorProjects/Robotica/human_figure.bmp", IMREAD_GRAYSCALE));
    templateImageArray.push_back(imread("/home/francesco/QtCreatorProjects/Robotica/human_figure10.bmp", IMREAD_GRAYSCALE));
    templateImageArray.push_back(imread("/home/francesco/QtCreatorProjects/Robotica/human_figure-10.bmp", IMREAD_GRAYSCALE));


    // Eseguo il resize delle immagini
    for(int i = 0; i < 3; i++)
        resize(templateImageArray[i], templateImageArray[i], Size(), 0.09, 0.09, cv::INTER_LANCZOS4);

    for(int i = 3; i < 6; i++)
        resize(templateImageArray[i], templateImageArray[i], Size(), 0.13, 0.13, cv::INTER_LANCZOS4);

    for(int i = 6; i < 9; i++)
        resize(templateImageArray[i], templateImageArray[i], Size(), 0.15, 0.15, cv::INTER_LANCZOS4);
}

/**
 * @brief ImageManager::findVictim cerca una vitima nell'immagine
 * @param cameraImage l'immagine da analizzare
 * @return true se c'è una vittima, false altrimenti
 */
bool ImageManager::findVictim(cv::Mat cameraImage)
{
    bool result = false;

    // Abbasso il contrasto dell'immagine, per rendere i neri più neri e i bianchi più bianchi
    cameraImage.convertTo(cameraImage, -1, 0.5, 0);

    // Converto l'immagine in scala di grigi
    cvtColor(cameraImage, cameraImage, CV_BGR2GRAY);

    // Punteggio massimo trovato (inizialmente nessuno); usato per debuggare e inserirlo nelle immagini da visualizzare
    float max = 0;

    // Immagine che conterrà i risultati temporanei del matchTemplate
    Mat resultImage;

    // Scorro tutti i tmplate fino a trovare una vittima
    for(int i = 0; i < templateImageArray.size() && !result; i++)
    {
        // Eseguo il match del template attuale
        matchTemplate(cameraImage, templateImageArray[i], resultImage, MATCH_METHOD);

//        threshold(resultImage, resultImage, 0.1, 1, CV_THRESH_TOZERO);

        // Variabili che verranno riempite con i risultati del matching
        double minVal;
        double maxVal;
        Point minLoc;
        Point maxLoc;
        Point matchLoc;

        // Analizzo il matching
        minMaxLoc(resultImage, &minVal, &maxVal, &minLoc, &maxLoc, Mat());

        // La variabile maxVal conterrà il punteggio che ci interessa; se supera 0.5 lo stampo per debuggare
        if(float(maxVal) > 0.5)
            cout << "minVal: " << float(minVal) << " maxVal: " << float(maxVal)<< "\n" << endl;

        if(maxVal > max)
            max = maxVal;

        // A seconda del metodo di matching usato i valori del punteggo sono i più piccoli o i più grandi
        if(MATCH_METHOD  == CV_TM_SQDIFF || MATCH_METHOD == CV_TM_SQDIFF_NORMED)
            matchLoc = minLoc;
        else
            matchLoc = maxLoc;

        // Se il valore trovato supera il threshold, è stata trovata una vittima
        if(maxVal > THRESHOLD)
        {
            // Disegno il rettangolo bianco che chiude la vittima
            rectangle(cameraImage, matchLoc, Point( matchLoc.x + templateImageArray[i].cols , matchLoc.y + templateImageArray[i].rows ), Scalar(255,0,0), 2, 8, 0 );
            result = true;
        }
    }

   // Inserisco il punteggio massimo trovato nell'immagine che verrà mostrata
   std::string label = std::to_string(max);
   int baseline=0;
   Size textLength = getTextSize(label, FONT_HERSHEY_PLAIN, 1, 1, &baseline);
   putText(cameraImage, label, Point(cameraImage.size().width/2 - textLength.width/2, cameraImage.size().height/2 - textLength.height/2), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(255,255,255), 1.0, LINE_8, false);

   // Copio l'immagine nella variabile che verrà accessa dalla classe RobotManager
   cameraImage.copyTo(processedImage);

    return result;
}

/**
 * @brief ImageManager::getProcessedImage restituisce l'ultima immagine processata
 * @return l'ultima immagine processata
 */
cv::Mat ImageManager::getProcessedImage()
{
    // Converto prima l'immagine a colori se non lo è già, in quanto per essere visualizzata deve essere convertita a QImage e serve che abbia 3 canali
    if(processedImage.channels() == 1)
        cvtColor(processedImage, processedImage, CV_GRAY2BGR);

    return processedImage;
}

