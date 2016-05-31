#include <iostream>
#include <lib/uclient.h>
#include <lib/usoftdevice.h>
#include <lib/uabstractclient.h>
#include <lib/usyncclient.h>
#include <pthread.h>
#include <unistd.h>
#include "opencv2/imgproc.hpp"
#include "opencv2/imgcodecs.hpp"
#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>

using namespace cv;


UCallbackAction onImage(const UMessage &msg) {
    if (msg.binaryType != BINARYMESSAGE_IMAGE)
        return URBI_CONTINUE;

    msg.client.printf("something received");

    msg.client.printf("Image of size (%d,%d) received from server at %d\n",msg.image.width, msg.image.height, msg.timestamp);

    unsigned char *image = new unsigned char[msg.image.width * msg.image.height * 3];
    int sz = msg.image.width*msg.image.height * 3;

    if (msg.image.imageFormat == IMAGE_JPEG)
        convertJPEGtoRGB((const byte *) msg.image.data, msg.image.size, (byte *) image, sz); //provided by liburbi
    else
        return URBI_CONTINUE;


    Mat imageMat(Size(msg.image.width, msg.image.height), CV_8UC3, image, Mat::AUTO_STEP);

    cvtColor(imageMat, imageMat, CV_RGB2GRAY);

    cv::imshow("Display Image", imageMat);

    if (waitKey(30) == 27)
    {
        std::cout << "esc key is pressed by user" << std::endl;
    }

    delete image;
    return URBI_CONTINUE;
}




int main() {
    USyncClient* client = new USyncClient("192.168.0.111", 54000);

    client->send("motors on;");
   // client->send("robot.stand();");


    client->send("headTilt = 30;");
    client->send("headPan = 0;");

    double distanceNear;
    client->syncGetDevice("distanceNear", distanceNear);

    std::cout << "beginning distanceNear: " << distanceNear << "\n" << std::flush;


    // Con questo script il robot cammina in avanti e muove la testa prendendo immagini finchè non incontra un ostacolo; appena lo incontra, ferma la testa e si
    // gira fino a quando non trova una via libera. Problema: si controlla solo col sensore di distanza nel petto, quindi non si controlla se il robot sta
    // andando a sbattere sulle pareti laterali.
    // Possibile soluzione: far si che il robot muova la testa prendendo immagini per un po' (tipo 1-2 secondi) e poi si giri normalmente da una parte all'altra
    // controllando il sensore distanceNear per vedere se si sta per sbattere; se si sta per sbattere da una parte si gira verso l'altra, si cammina un poco
    // e poi si rigira di nuovo dall'altra per tornare lungo la direzione (magari ricontrollando con la testa a destra e a sinistra).
    // Mi sa che conviene far si che quando inizi questa fase in cui il robot muove la testa a destra e a sinistra il robot si fermi temporaneamente
    URBI((
         function walking() {
             robot.stopturn();
             a: { robot.walk(2000); searching(); };
         };

         function turning() {
             robot.stopwalk();
             loop { robot.turn(2000); };
         };

        function sendingImages() {
            onImage: camera.val;
        };

         // Searching state
         function searching() {
             freeze turningBehaviour;
             period = 10s;
             send: loop { sendingImages() | wait(100ms)} & // Forse l'immagine non è mandata in parallelo mentre muove la testa; bisogna provare
             search: timeout(7000)
             {
                 {
                     headPan'n  = 0.5 smooth:1s &
                     headTilt'n = 1 smooth:1s
                 } |
                 {
                     headPan'n  = 0.5 sin:period ampli:0.5 &
                     headTilt'n = 0.5 cos:period ampli:0.5
                 };

                 stop send;
             };



             headPan = 0;
             headTilt = 30;

             unfreeze turningBehaviour;
         };

         // Forse la sintassi per far le cose in parallelo è da rivedere
        walkingBehaviour: whenever (distanceNear >= 20 ~ 500ms) {
            stop turn;
            walk: walking();
    //        search: searching();
        };

        // Finchè c'è un ostacolo davanti si gira; appena non ci sono ostacoli smette di girarsi e scatta il whenever di prima
        turningBehaviour: whenever (distanceNear < 20) {
            stop walk;
            stop search;
            stop a;
            turn: turning();
        };
    ));


    client->setCallback(onImage, "onImage");

    while(true)
    {
        double distanceNear;
        client->syncGetDevice("distanceNear", distanceNear);

        std::cout << "distanceNear: " << distanceNear << "\n" << std::flush;
    }


    urbi::execute();


    return 0;
}


