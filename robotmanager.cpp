#include <QtGui/QGuiApplication>
#include <QApplication>
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
#include "imagemanager.h"
#include "robotmanager.h"
#include <QThread>
#include <chrono>
#include <thread>
#include <QVariant>


using namespace cv;

RobotManager::RobotManager(QObject *parent) :
    QObject(parent), averageCounter(0), distanceValue(0), headPanValue(0), average(12), imageFoundCounter(0)
{

}

// Classe ausiliaria temporanea che implementa un'interfaccia per poter mostrare gli errori del codice del robot su C++
class MyCallback : public UCallbackWrapper {
public:
    UCallbackAction operator ()(const UMessage& message) override {
        message.client.printf("got an error: %s\n", message.message);
    }
};

void RobotManager::init()
{
    // Creo il client, che si connetterà al robot
    USyncClient* client = new USyncClient("192.168.0.111", 54000);


    // Setto il callback per visualizzare le immagini del robot. Ogni volta che il robot eseguirà "onImage: camera.val;" verrà chiamato il callback
    client->setCallback(*this, &RobotManager::onImageCallback, "onImage");

    // Callback per il valore del pan della testa
    client->setCallback(*this, &RobotManager::onHeadPanCallback, "headAngle");


    // Callback per i sensori di distanza posti nella testa; entrambi i sensori condividono il callback
    client->setCallback(*this, &RobotManager::onDistanceSensorCallback, "distance");

    // Callback lo stato del robot (camminata, giramento, ricerca) e per l'orientamento
    client->setCallback(*this, &RobotManager::onRobotStateChanged, "robotState");
    client->setCallback(*this, &RobotManager::onOrientationChanged, "robotOrientation");


client->setCallback(*this, &RobotManager::onProva, "prova");



    URBI((
         motors on;

         playSound = 1;


         // Quando si avvia lo script, se il robot non è stato riavviato alcune definizioni sono ancora presenti e alcuni comportamenti possono essere stati
         // disattivati al momento del blocco dell'esecuzione precedente. Quindi per assicurarmi che tutti i comportamenti siano inizialmente attivi in questa
         // esecuzione, faccio l'unfreeze dei behaviour che verranno definiti dopo nel codice. Se non erano stato definiti o erano già unfreezati, non succede niente
         unfreeze walkingBehaviour;
         unfreeze turningBehaviour;

         // Variabile che indica la durata della fase di ricerca (7 secondi)
         walkSteps = 1;
         turnSteps = 2;
         searchTime = 4s;
         walkTime = 3s;
         turnTime = 8s;
         defaultNeck = -5;
         defaultHeadPan = 0;
         defaultHeadTilt = 14;
//         timeToTurn90degrees = 2390ms;

         rotation = 0;
         isTurningClockwise = 0;

         // Funzione di convenienza che stampa i valori dell'headPan e della distanza in modo che vengano chiamati i rispettivi callback C++.
         // La dichiaro prima di tutto il resto perchè viene usata quasi subito e deve essere visibile nello scope
         function printHeadAngleAndDistance() {
            headAngle: headPan;

            // Dato che quando il robot cerca gira la testa ed il campo visivo varia molto (potrebbe guardare in un momento una parete molto vicina e un attimo
            // dopo una parete molto lontana), passo a C++ il valore della distanza giusto, ovvero se il sensore distanceNear è >= 20 (che è il lower bound per
            // il distanceFar), stampo il distanceFar direttamente che avrà il valore più accurato; se è minore di quel valore, il distanceFar non va bene
            // perchè sotto i 20cm ha il punto morto, quindi passo il valore distanceNear
            if(distanceNear >= 20)
                distance: distanceFar
            else
                distance: distanceNear;
         };

         function turnOffLeds() {
             ledF1 = 0;
             ledF2 = 0;
             ledF3 = 0;
             ledF4 = 0;
             ledF5 = 0;
             ledF6 = 0;
             ledF7 = 0;
             ledF8 = 0;
             ledF9= 0;
             ledF10 = 0;
             ledF11 = 0;
             ledF12 = 0;
             ledF13 = 0;
             ledF14 = 0;
         };

         // Booleani per indicare l'attività che si sta eseguendo, in modo che si possano inviare a C++ i valori giusti in base alla situazione. In un dato
         // momento uno solo di questi booleani è a 1, e li altri a 0 (se son tutti a 0 invece nessuna attività è in corso)
         walkingBool = 0;
         turningBool = 0;
         searchingBool = 0;


         // Variabili che indicano lo stato del robot (camminata, girata, ricerca) e l'orientamento
         state = 0;
         orientation = 0;

          // All'avvio passo a C++ i valori dello stato e dell'orientamento, per inizializzare la GUI
         robotState: state;
         robotOrientation: orientation;

         robot.stand();


         // Porto la testa nella posizione di default e aspetto un minimo di tempo, in modo che il robot abbia il tempo di spostare la testa prima
         // di eseguire il resto del codice
         headPan = defaultHeadPan | headTilt = defaultHeadTilt | neck = defaultNeck | wait(500ms);


//         if(!isdef(alreadyStarted))
//         {
//            robot.lay2stand();
//            alreadyStarted = 1;
//         };


         if(playSound == 1)
         {
             if(random(2) == 1)
                speaker.play("activated.wav")
             else
                 speaker.play("activated.wav");
         };



//         robot.swalk(4);

         /* Passo anche il valore della testa e della distanza in modo che la GUI possa segnarsi la posizione
          * della parete più lontana e basare i futuri movimenti su questo. Dato che la parte C++ fa una media sui valori ottenuti
          * prima di passarli a QML (per migliorare l'accuratezza, visto che i sensori di distanza danno delle misure a tratti random),
          * e questa media richiede 12 elementi di default, passo i valori 12 volte a distanza di qualche millisecondo,
          * in modo che i valori non siano tutti uguali e quindi in modo che la media sia fatta su valori più realistici */
         for(counter = 0; counter < 12; counter++)
            printHeadAngleAndDistance() | wait(10ms);


         turnOffLeds();


         whenever (walkingBool == 1) {
             printHeadAngleAndDistance();
         };

         whenever (searchingBool == 1) {
//             // Passo anche immagini dalla videocamera
//             sendingImages();
//             printHeadAngleAndDistance() & sendingImages();
             sendingImages();
//             wait(100ms);

             // Passo anche immagini dalla videocamera

         };

         whenever (turningBool == 1) {
             rotation += 1;

//            state = rotation;
//            robotState: state;

             // Per ottenere il tempo di attesa per ogni incremento ho fatto la proporzione 2390 : 90 = 26 : x -> x = (26 * 90)/2390 ~ 1 secondo.
             // Ho fatto la proporzione in modo tale che la rotazione aumenti di 1 prima di ogni attesa
             wait(26ms);
         };


         function walking() {
             // All'inizio della camminata blocco la girata, qualora fosse in esecuzione, e sblocco il behaviour della girata, qualora fosse stato bloccato (in modo
             // tale che se ora mentre cammina trova un ostacolo riprenda a girare)
             stop turn;
             unfreeze turningBehaviour;


             /* Se la rotazione è pari a 0, vuol dire che non c'è stata una rotazione di recente, quindi stabilisco che lo stato attuale è quello della
              * camminata. In pratica il problema è che quando il robot ruota per assestarsi mentre cammina su un corridoio o quando deve svoltare,
              * può fare delle piccole camminate ogni tanto; in questi casi però non deve informare il C++ che sta camminando, altrimenti sfaserà di brutto
              * i dati nella mappa. Quindi se la rotazione è != 0 vuol dire che ha ruotato di recente e non dobbiamo dire a C++ che sta camminando; lo dobbiamo
              * dire solamente quando siamo certi che abbia smesso di ruotare, ovvero una volta che ha finito un ciclo di camminata; in quel caso infatti
              * verrà riportato rotation a 0, e nella camminata successiva si rimandano i dati a C++ (i dati di questa prima camminata però vengono persi) */
             if(rotation == 0)
             {
                 // Setto lo stato pari a 2 per indicare che il robot sta camminando
                 state = 2;

                 // Passo lo stato e l'orientamento (che non è stato modificato in questo contesto) a C++ (in modo che scatti il callback), così che
                 // la GUI possa aggiornare la mappa correttamente
                 robotState: state;
                 robotOrientation: orientation;
             };

             headPan = defaultHeadPan | headTilt = defaultHeadTilt | neck = defaultNeck | wait(100ms);


             // Stabilisco che ora il robot sta camminando ed è l'unica attività in esecuzione
             walkingBool = 1;
             turningBool = 0;
             searchingBool = 0;

             turnOffLeds();
             ledF11 = 1;
             ledF12 = 1;


             if(playSound == 1)
                speaker.play("isanyonethere.wav");

             // Avvio la camminata
//             robot.walk(walkTime);
             robot.swalk(walkSteps);

             // Se il robot è arrivato fin qua vuol dire che ha effettuato una camminata completa; adesso controllo se ha effettuato delle rotazioni poco fa, e
             // questo è vero se rotation > 0. In tal caso, se la rotazione è > 45 vuol dire che il robot sta eseguendo una rotazione "grossa",
             // abbastanza grossa da dover modificare l'orientazione del robot in quanto sta' girando di 90 o 180 gradi. Altrimenti sono piccole rotazioni di correzione
             // della rotta lungo un corridoio, e quindi non le considero
             if(rotation > 65)
             {
                 // Se la rotazione è <= 100 considero come se abbia girato 90 gradi
                 if(rotation <= 100)
                 {
                     // In base al senso della rotazione aggiorno l'orientazione
                     if(isTurningClockwise)
                         orientation += 90
                     else
                         orientation += -90;
                 }
                 // Altrimenti ha girato 180°
                 else
                 {
                    if(isTurningClockwise)
                        orientation += 180
                    else
                        orientation += -180;
                 };

                 // Setto lo stato pari a 0 per indicare che il robot si è appena girato
                 state = 0;

                 // Passo lo stato e l'orientamento (che non è stato modificato in questo contesto) a C++ (in modo che scatti il callback), così che
                 // la GUI possa aggiornare la mappa correttamente con il nuovo orientamento
                 robotState: state;
                 robotOrientation: orientation;

                 // Passo i valori di testa e distanza a C++ per mettere nella GUI il pallino del muro più lontano
                 for(counter = 0; counter < 12; counter++)
                    printHeadAngleAndDistance() | wait(10ms);
             };

             // In ogni caso, riporto a 0 la rotazione in modo da ignorare anche qualsiasi piccola rotazione (< 45) effettuata
             rotation = 0;
         };


         // Searching state
         function searching() {
             // Blocco il behaviour per girarsi, in modo tale che se girando la testa in fase di ricerca il robot trovi un ostacolo non inizi a girarsi e
             // continui a cercare. Bloco anche il behaviour per la camminata, perchè sennò altrimenti mentre il robot cerca se vede uno spazio che soddisfa
             // il behaviour della camminata lui metterebbe quell'esecuzione tipo in coda e una volta finita la ricerca rieseguerebbe la camminata
             freeze turningBehaviour;
             freeze walkingBehaviour;

             // Stabilisco che ora il robot sta cercando
             walkingBool = 0;
             turningBool = 0;
             searchingBool = 1;

             // Setto lo stato pari a 1 per indicare che il robot è entrato in fase di ricerca
             state = 1;

             // Mando l'informazione sullo stato e l'orientamento (che non cambia in fase di ricerca) a C++
             robotState: state;
             robotOrientation: orientation;

//             period = 10s;

//             send: sendingImages() &
//             search: timeout(searchTime)
//             {
//                 {
//                     headPan'n  = 0.5 smooth:1s &
//                     headTilt'n = 1 smooth:1s
//                 } |
//                 {
//                     headPan'n  = 0.5 sin:period ampli:0.5 &
//                     headTilt'n = 0.5 cos:period ampli:0.5
//                 };
//             };

             turnOffLeds();
             ledF13 = 1;
             ledF14 = 1;

             if(playSound == 1)
             {
                 if(random(2) == 1)
                    speaker.play("turret_search_4.wav")
                 else
                    speaker.play("turret_active_8.wav");
             };

             headTilt = defaultHeadTilt | neck = defaultNeck | wait(100ms);

             // Sposto la testa dall'estremo sinistro a destra nel tempo indicato
             headPan = 85 | wait(500ms);
             headPan = -85 time: searchTime;


             // Terminata la ricerca, riporto la testa dritta in modo da poter controllare con il sensore sulla testa se va a sbattere
             headPan = defaultHeadPan | headTilt = defaultHeadTilt | neck = defaultNeck | wait(100ms);

             // Finita la ricerca ripristino i behaviour
             unfreeze turningBehaviour;
             unfreeze walkingBehaviour;


             // Blocco l'invio di immagini (just in case)
//             stop send;

             // Stabilisco che il robot non sta facendo niente ora; se dovrà camminare ancora, verrà richiamata questa funzione che setterà walking su 1 all'inizio
             walkingBool = 0;
             turningBool = 0;
             searchingBool = 0;
         };


         // Funzione per effettuare la girata
         function turning() {
             /* Per prima cosa al momento di girare, il robot deve capire da quale parte girare (senso orario o antiorario); per capirlo gira la testa
              * a destra e controlla quanto spazio ha per voltarsi la. Il problema è che se il behaviour della camminata è attivo, se quando gira la testa
              * a destra c'è spazio per camminare, scatta il behaviour ed il robot si muove in avanti (ma contemporaneamente continuerà a girare, facendo un casino).
              * Di conseguenza mentre il robot gira la testa a destra bisogna sospendere il behaviour della camminata.
              * Per lo stesso motivo bisogna poi sospendere il behaviour della girata stessa, in quanto se quando gira la testa a destra c'è spazio per voltarsi,
              * poi quando rimetterà la testa dritta per iniziare la girata la distanza che vedrà sarà troppo corta ed inizierà di nuovo la girata, andando
              * in un loop eterno. Bisogna quindi anche bloccare il behaviour della girata temporaneamente, in modo tale che non ci sia il rischio che questo
              * blocco di codice possa essere eseguito più volte di fila */
             freeze walkingBehaviour;
             freeze turningBehaviour;

             // Nel caso ci fosse una camminata attiva, la blocco; fermo quindi il pattern con il loop infinito della camminata, e blocco anche le singole
             // componenti del pattern (infatti sennò prima di effettuare la girata si aspetterebbe quella che finisca quella attiva tra "walk" e "search")
             stop pattern;
             stop walk;
             stop search;

             // Stabilisco che di default si gira in senso antiorario
             clockwise = 0;


             // Giro la testa a destra e aspetto un piccolo tot di tempo in modo che il robot abbia il tempo di girarla prima di eseguire il resto
             headPan = -89 | neck = defaultNeck | wait(800ms);


             // Controllo se c'è spazio per andare a destra, e nel caso segnalo che bisogna girare in senso orario
             if(distanceNear >= 40)
             {
                 clockwise = 1;
             };

             // Riporto la testa nella posizione dei default e faccio aspettare un tot di tempo in modo che la giri
             headPan = defaultHeadPan | headTilt = defaultHeadTilt | neck = defaultNeck | wait(500ms);


             // Terminata questa fase, sblocco il behaviour della camminata (in modo che se ci sia via libera mentre giri smetta di girarsi e
             // vada avanti) e quello della girata
             unfreeze walkingBehaviour;
//             unfreeze turningBehaviour;


             isTurningClockwise = clockwise;

//             robot.StandUp();

             // Stablisco che il robot si sta girando
             walkingBool = 0;
             turningBool = 1;
             searchingBool = 0;

             turnOffLeds();

             if(clockwise)
             {
                 ledF1 = 1;
                 ledF3 = 1;
             }
             else
             {
                 ledF2 = 1;
                 ledF4 = 1;
             };

             loop {
                 if(!clockwise)
                 {
//                    robot.turn(-turnTime)
                     robot.sturn(-turnSteps);
                     robot.swalk(1);
                     search: searching();
                 }
                 else
                 {
//                    robot.turn(turnTime);
                     robot.sturn(turnSteps);
                     robot.swalk(1);
                     search: searching();
                 };
             };

             // Stablisco che il robot non sta eseguendo alcuna attività al momento
             walkingBool = 0;
             turningBool = 0;
             searchingBool = 0;
         };

        function sendingImages() {
//            timeout(searchTime)
//            {
//                // Per non inviare troppe immagini insieme, le si inviano ogni 100 millisecondi
//                loop { onImage: camera.val; | wait(100ms); };
//            }
            onImage: camera.val;
            wait(100ms);
        };


         // Quando non c'è un ostacolo per un tot di millisecondi (questo per evitare che quando il robot gira ad un angolo passi da un behaviour all'altro
         // in quanto mentre gira alternerà il vedere ostacoli a non vederne), il robot cammina in avanti, iniziando una fase di ricerca ogni tot
         walkingBehaviour: at (distanceNear >= 20 ~ 1500ms) {

            // Per sicurezza, blocco tutte le possibili esecuzioni che ci potrebbero essere state; prima di tutto blocco la girata, qualora fosse presente
            stop turn;

            // Blocco poi il pattern della camminata definito qua sotto, altrimenti se c'era già il pattern in esecuzione quando si entra in questo blocco
            // di istruzioni continuerebbe ad essere eseguito in loop
            stop pattern;

            // Se c'era un pattern attivo, vuol dire che o il robot stava camminando o stava cercando; in ogni caso blocco l'attività che stava eseguendo,
            // altrimenti finirebbe di eseguirla prima di riniziare il pattern come invece voglio
            stop walk;
            stop search;

            // Pattern della camminata: si cammina un tot di secondi e poi si esegue la ricerca per un altro tot di secondi; questo all'infinito (o meglio fino
            // a quando non trova un ostacolo e si attiva il corrispondente behaviour)
            pattern: loop {
                walk: walking();
                search: searching();
            };
         };

        /* Behaviour per girare. Se c'è un ostacolo il robot deve reagire subito e girare, ma bisogna capire da che parte girare.
         * Ci sono vari casi possibili infatti:
         * 1) Il robot sta camminando in un corridoio e sbanda verso la parete sinistra -> deve girare un po' in senso orario per poi tornare a camminare dritto
         * 2) Come sopra, ma verso la parete destra -> la soluzione è l'opposto di sopra
         * 3) Il robot è arrivato alla fine del corridoio e c'è solo una svolta a sinistra -> il robot deve girare in senso antiorario
         * 4) Come sopra ma la svolta è a destra -> si svolta in senso orario
         * 5) Come sopra, ma c'è una svolta sia a sinistra che a destra -> per poter visitare tutto il labirinto il robot deve seguire la parete destra, quindi orario
         * 6) Il corridoio è un vicolo cieco -> il robot deve girare in senso antiorario fino a ruotare di fatto di 180° e tornare da dove è venuto.
         *
         * Tutti questi casi si possono risolvere in questo modo: il robot gira la testa a destra, e se vede che c'è abbastanza spazio da quella parte si gira
         * in senso orario (dopo aver riportato la testa dritta). Altrimenti se non c'è spazio a destra, gira a sinistra */
        turningBehaviour: at (distanceNear < 20 ~ 300ms) {
            // Se c'era una girata attiva precedentemente, la blocco per sicurezza e la rieseguo. Può capitare infatti che mentre giri ad un certo punto veda spazio
            // per camminare e subito dopo non più, ed in tal caso scatterebbe di nuovo questo behaviour; occorre quindi bloccare la girata precedente e rieseguirla
//            stop turn;

            // Eseguo la girata vera e propria
            turn: turning();
        };



        at (backSensorM > 0) {
            stop search;
        };

        victimFound = 0;
        playedRecently = 0;

        at (victimFound == 1) {
            if(playedRecently == 0)
            {
                speaker.play("bark.wav");
                playedRecently = 1;
                victimFound = 0;
                start = time();
                at (time() - start > 800ms) {
                    playedRecently = 0;
                    victimFound = 0;
                };
            };
        };


//        pickup: at (accelX < -5) {
//            if(playSound == 1)
//            {
//                startTime = time();

//                if(random(2) == 1)
//                    speaker.play("turret_pickup_3.wav")
//                else
//                    speaker.play("turret_pickup_3.wav");


//                waituntil(time() - startTime > 5s) {
//                    unfreeze pickup;
//                };

//                freeze pickup;
//            };
//        };

        // Durante la fase di ricerca, il robot gira la testa da sinistra a 70. Quando guarda a destra (cioè l'angolo della testa è maggiore di un dato valore)
        // se i sensori notano che non c'è una parete (cioè distanceFar è >= 20) allora vuol dire che c'è una strada alla destra del robot. Se questo è vero
        // per un tot di tempo, prendiamo l'informazione per buona e facciamo girare il robot di 90 gradi in senso orario
//        whenever ((distanceFar >= 70 && headPan < -45 && searchingBool == 1) ~ 1000ms) {
//            stop pattern;
//            stop walk;
//            stop search;

//            // Giriamo di 90 gradi negativi per far girare il robot in senzo orario
//            turn: turning();
//        };
    ));


    // Setto il callback degli errori
    MyCallback prova;
    client->setErrorCallback(prova);


    // Questo comando deve essere eseguito prima della chiusura del main. Si occupa di non far terminare il programma,
    // in modo da poter mandare/ricevere messaggi dal robot fino alla chiusura manuale
    urbi::execute();
}


/**
 * @brief onImageCallback: funzione di callback, che viene chiamata quando la il robot manda un'immagine al pc
 *
 * @param msg il messaggio contenente l'immagine
 *
 * @return una costante che indica se il robot deve continuare a mandare messaggi o fermarsi
 */
UCallbackAction RobotManager::onImageCallback(const UMessage &msg)
{
    // Controllo se il messaggio riguarda un'immagine; se non lo è, termino la funzione
    if (msg.binaryType != BINARYMESSAGE_IMAGE)
        return URBI_CONTINUE;

//    msg.client.printf("Image of size (%d,%d) received from server at %d\n",msg.image.width, msg.image.height, msg.timestamp);

    // Recupero l'immagine e la sua grandezza (l'immagine ha 3 canali (R, G, B), quindi si moltiplicano per 3 i valori)
    unsigned char *imageRaw = new unsigned char[msg.image.width * msg.image.height * 3];
    int size = msg.image.width*msg.image.height * 3;

    // Se il formato è JPEG (come previsto), converto l'immagine con un'apposita funzione fornita da liburbi in un'immagine RGB standard
    if (msg.image.imageFormat == IMAGE_JPEG)
        convertJPEGtoRGB((const byte *) msg.image.data, msg.image.size, (byte *) imageRaw, size);
    else
        return URBI_CONTINUE;


    // Trasformo l'immagine da un blob di byte in un oggetto di OpenCV. La costante CV_8UC3 è molto importante, solo con questa si ottiene l'immagine giusta
    Mat image(Size(msg.image.width, msg.image.height), CV_8UC3, imageRaw, Mat::AUTO_STEP);


    // Mostro l'immagine
//    cv::imshow("Display Window", image);

//    waitKey(1);

    // Controllo se nell'immagine è presente una vittima
    if(imageManager.findVictim(image))
    {
        imageFoundCounter++;

        msg.client.printf("FOUND VICTIM number %d\n", imageFoundCounter);
        msg.client.send("victimFound = 1;");

        cv::imshow("Found it", image);
    }

    // Controllo se si preme il tasto esc; è per evitare che si chiuda la finestra con l'immagine
//    if (waitKey(0) == 27)
//        msg.client.printf("Esc key was pressed by the user");


    // Elimino l'immagine in byte, non serve più
    delete imageRaw;

    return URBI_CONTINUE;
}


UCallbackAction  RobotManager::onHeadPanCallback(const UMessage &msg)
{
    if (msg.type != MESSAGE_DOUBLE)
        return URBI_CONTINUE;

//    headPanValue = msg.doubleValue;
//    headPanValue += msg.doubleValue;
    headPanVec.push_back(msg.doubleValue);


//    msg.client.printf("got a message at %d with tag %s, our int is %d\n",msg.timestamp, msg.tag, msg.doubleValue);

    return URBI_CONTINUE;
}

UCallbackAction  RobotManager::onDistanceSensorCallback(const UMessage &msg)
{
    if (msg.type != MESSAGE_DOUBLE)
        return URBI_CONTINUE;

//    distanceValue = msg.doubleValue;

//    msg.client.printf("got a distance message: %f\n", msg.doubleValue);

    // Emetto il signal per notificare QML che deve aggiungere un punto alla view
//    emit pointFound(distanceValue, -headPanValue);

//    distanceValue += msg.doubleValue;
    distanceVec.push_back(msg.doubleValue);
    averageCounter++;

    if(averageCounter == average)
    {
        std::sort (distanceVec.begin(), distanceVec.end());
        std::sort (headPanVec.begin(), headPanVec.end());

        for(int i = 1; i < distanceVec.size()-1; i++)
        {
            distanceValue += distanceVec[i];
            headPanValue += headPanVec[i];
        }

        emit pointFound(distanceValue / (distanceVec.size()-2), -headPanValue / (distanceVec.size()-2));
        distanceVec.clear();
        headPanVec.clear();
        distanceValue = 0;
        headPanValue = 0;
        averageCounter = 0;
    }



    return URBI_CONTINUE;
}


UCallbackAction  RobotManager::onRobotStateChanged(const UMessage &msg)
{
    if (msg.type != MESSAGE_DOUBLE)
        return URBI_CONTINUE;

    robotState = msg.doubleValue;

    msg.client.printf("state changed: %f\n",  msg.doubleValue);


    return URBI_CONTINUE;
}

UCallbackAction  RobotManager::onOrientationChanged(const UMessage &msg)
{
    if (msg.type != MESSAGE_DOUBLE)
        return URBI_CONTINUE;

    robotOrientation = msg.doubleValue;

    msg.client.printf("orientation changed: %f\n",  msg.doubleValue);

    // Emetto il signal per notificare QML che lo stato del robot e possibilmente l'orientazione sono cambiati, in modo che possa interpretare
    // correttamente i successivi "pointFound" signal che trova
    emit robotStateChanged(robotState, robotOrientation);

    return URBI_CONTINUE;
}

UCallbackAction  RobotManager::onProva(const UMessage &msg)
{
    if (msg.type != MESSAGE_DOUBLE)
        return URBI_CONTINUE;


    msg.client.printf("onProva changed: %f\n",  msg.doubleValue);


    return URBI_CONTINUE;
}

