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

// Valore threshold per la media calcolata sui sensori della distanza per generare meglio la mappa
const int RobotManager::AVERAGE_THRESHOLD = 5;


RobotManager::RobotManager(QObject *parent) :
    QObject(parent), averageCounter(0), victimFoundCounter(0)
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

    // Callback per informazioni usate per generare la mappa
    client->setCallback(*this, &RobotManager::onHeadPanCallback, "headAngle");
    client->setCallback(*this, &RobotManager::onDistanceSensorCallback, "distance");
    client->setCallback(*this, &RobotManager::onVictimFoundCallback, "victimFound");
    client->setCallback(*this, &RobotManager::onRobotTurnCallback, "robotTurn");
    client->setCallback(*this, &RobotManager::onGenerateStepCallback, "generateStep");

    // Callback per stampe ausiliarie
    client->setCallback(*this, &RobotManager::onPrinterCallback, "printer");


    // Prealloco spazio per due array usati per calcolare la media dei valori dei sensori
    distanceVec.resize(AVERAGE_THRESHOLD);
    headPanVec.resize(AVERAGE_THRESHOLD);


    // Invio lo script al robot
    URBI((
         motors on;

         // Booleano per dire se il robot deve far partire gli audio o no
         playSound = 1;


         // Quando si avvia lo script, se il robot non è stato riavviato alcune definizioni sono ancora presenti e alcuni comportamenti possono essere stati
         // disattivati al momento del blocco dell'esecuzione precedente. Quindi per assicurarmi che tutti i comportamenti siano inizialmente attivi in questa
         // esecuzione, faccio l'unfreeze dei behaviour che verranno definiti dopo nel codice. Se non erano stato definiti o erano già unfreezati, non succede niente
         unfreeze walkingBehaviour;
         unfreeze turningBehaviour;

         // Variabili che indicano durata di alcuni comportamenti
         walkSteps = 1;
         turnSteps = 2;
         searchingSteps = 8;

         // I valori della distanza che sono passati a C++ sono calcolati applicando una media; questo valore costante indica quanti valori della distanza si devono
         // prendere per il calcolo della media, e deve essere uguale alla corrispondente costante C++
         sensorAverageCounter = 5;

         // Valori di default per la testa del robot
         defaultNeck = -5;
         defaultHeadPan = 0;
         defaultHeadTilt = 14;

         // Distanza minima di default per la camminata; se non ci sono ostacoli entro 15 centimetri, il robot cammina in avanti
         defaultWalkingDistanceMin = 15;

         // Valore corrente minimo per la camminata; in certi momenti varia per aggirare un problema del linguaggio URBI
         walkingDistanceMin = defaultWalkingDistanceMin;

         // Tempo di default necessario per iniziare la camminata; questo vuol dire che non ci devono essere ostacoli a distanza walkingDistanceMin
         // per il tot di tempo specificato da questa costante
         defaultTimeNeededToWalk = 1800ms;

         // Tempo corrente necessario per la camminata; varia in certi momenti per far si che il robot giri più a lungo in certe occasioni
         timeNeededToWalk = defaultTimeNeededToWalk;

         // Rotazione del robot e booleano che indica il senso della rotazione; servono per essere passati a C++ per disegnare la mappa, e variano quando il robot gira
         rotation = 0;
         isTurningClockwise = 0;

         // Bool che indica se il robot deve aggiustare la rotta ruotando poco poco; può diventare true se durante la ricerca il robot vede che è troppo vicino a una parete
         adjustRoute = 0;

         // Variabile usata per trovare il valore minimo del sensore di distanza durante la ricerca, usato per capire se bisogna aggiustarsi
         // o no da una parete o dall'altra; di default è il valore massimo di distanza captabile, cioè 150
         adgjustmentMinDistance = 150;

         // Booleano per indicare se è stata trovata una vittima (è settato a true da C++ quando viene trovata)
         victimFoundBool = 0;

         // Booleano per indicare se è stato suonato il suono della vittima trovata di recente, in modo da non suonarlo troppo spesso
         playedSoundRecently = 0;


         // Contatori usati in un behaviour per capire se il robot deve girare a destra
         currentTurningSearchCounter = 0;
         previousTurningSearchCounter = 0;


         // Funzione di convenienza che stampa i valori dell'headPan e della distanza in modo che vengano chiamati i rispettivi callback C++.
         // La dichiaro prima di tutto il resto perchè viene usata quasi subito e deve essere visibile nello scope
         function printHeadAngleAndDistance() {

             // I valori sono mandati più volte per poter calcolare la media da C++
             for(u=0; u<sensorAverageCounter; u++) {
                headAngle: headPan;

                // Dato che quando il robot cerca gira la testa ed il campo visivo varia molto (potrebbe guardare in un momento una parete molto vicina e un attimo
                // dopo una parete molto lontana), passo a C++ il valore della distanza giusto, ovvero se il sensore distanceNear è >= 20 (che è il lower bound per
                // il distanceFar), stampo il distanceFar direttamente che avrà il valore più accurato; se è minore di quel valore, il distanceFar non va bene
                // perchè sotto i 20cm ha il punto morto, quindi passo il valore distanceNear
                if(distanceNear >= 20)
                    distance: distanceFar
                else
                    distance: distanceNear;

                // Attendo un minimo in modo che i valori dei sensori cambino ad ogni iterazione
                wait(2ms);
             }
         };

         // Funzione di convenienza che spegne tutti i led usati dal robot
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
             ledHW = 0;
             ledHC = 0;
             modeB = 1;
             modeG = 1;
             modeR = 0;
         };

         // Booleani per indicare l'attività che si sta eseguendo, in modo che si possano inviare a C++ i valori giusti in base alla situazione. In un dato
         // momento uno solo di questi booleani è a 1, e li altri a 0 (se son tutti a 0 invece nessuna attività è in corso)
         walkingBool = 0;
         turningBool = 0;
         searchingBool = 0;

         // All'avvio spengo eventuali led accesi
         turnOffLeds();

         // Porto il robot in piedi
         robot.stand();

         // Porto la testa nella posizione di default e aspetto un minimo di tempo, in modo che il robot abbia il tempo di spostare la testa prima
         // di eseguire il resto del codice
         headPan = defaultHeadPan | headTilt = defaultHeadTilt | neck = defaultNeck | wait(300ms);


         // Funzione ausiliaria che calcola una media dei valori del sensore di distanza near; usato a volte per ottenere valori più accurati per decidere cosa fare
         function averageDistanceNear(counter) {
             avg = 0;
             accumulator = 0;

             for(n = 0; n < counter; n++)
             {
                 accumulator = accumulator + distanceNear.val;
                 wait(1ms);
             };

             avg = (accumulator/counter);

             return avg;
         };

         // Stessa funzione di sopra, solo che è per il distanceFar
         function averageDistanceFar(counter) {
             avg = 0;
             accumulator = 0;

             for(n = 0; n < counter; n++)
             {
                 accumulator = accumulator + distanceFar.val;
                 wait(1ms);
             };

             avg = (accumulator/counter);

             return avg;
         };


         // Questo behaviour si occupa di notificare C++ che il robot sta' camminando, per generare la mappa
         whenever (walkingBool == 1) {
             // Creo il tag in modo che venga chiamato il callback C++, e genero un'attesa tra una notifica e l'altra. L'attesa
             // è fatta valutando la velocità della camminata del robot ad occhio
             generateStep: 1;
             wait(250ms);
         };

         // Behaviour che viene eseguito quando il robot è in fase di ricerca
         whenever (searchingBool == 1) {
             // Mando a C++ informazioni sulla distanza rilevata dai sensori e sulla rotazione della testa
             printHeadAngleAndDistance();

             // Mando a C++ un'immagine presa dalla camera
             sendingImages();


             // Ora, se la testa è posta molto a sinistra controllo se il robot è troppo vicino al muro, e nel caso faccio si che dopo la ricerca si assesti
             if(headPan > 80)
             {
                 // Prendo la distanza con una media
                 avgDist = averageDistanceNear(2);

                 // Controllo se la distanza è sotto una certa soglia e se è minore del valore minimo trovato
                 if(avgDist < 7 && avgDist < adgjustmentMinDistance)
                 {
                     // Faccio delle stampe pseudo-random su C++ per capire cosa accade
                     ppp = 333;
                     printer: ppp;
                     adgjustmentMinDistance = avgDist;
                     printer: adgjustmentMinDistance;

                     // Segnalo che il robot deve aggiustare la rotta e girare in senso orario
                     adjustRoute = 1;
                     turnClockwise = 1;
                 };
             };

             // Stessa cosa di prima ma guardando a destra
             if(headPan < -80)
             {
                 avgDist = averageDistanceNear(2);

                 // Se il robot guarda a destra il threshold è diverso; questo è dovuto al fatto che a sinistra è più sensibile, in quanto guarda per più
                 // tempo a sinistra durante la ricerca
                 if(avgDist < 10 && avgDist < adgjustmentMinDistance)
                 {
                     aaa = 222;
                     printer: aaa;
                     adgjustmentMinDistance = avgDist;
                     printer: adgjustmentMinDistance;
                     adjustRoute = 1;
                     turnClockwise = 0;
                 };
             };
         };

         // Behaviour che scatta quando i lrobot sta girando
         whenever (turningBool == 1) {
             // A seconda del senso della girata incremento o decremento la rotazione, facendo un'attesa presa ad occhio in base a quanto velocemente gira il robot
             if(isTurningClockwise)
             {
                rotation += 1;
                wait(65ms);
             }
             else
             {
                rotation -= 1;
                wait(45ms);
             };

             // Mando il valore dell'orientamento a C++
             robotTurn: rotation;
         };


         // Funzione che esegue la camminata del robot
         function walking() {
             // All'inizio della camminata blocco la girata, qualora fosse in esecuzione, e sblocco il behaviour della girata, qualora fosse stato bloccato (in modo
             // tale che se ora mentre cammina trova un ostacolo inizi a girare)
             stop turn;
             unfreeze turningBehaviour;

             // Porto la testa nella posizione di default
             headPan = defaultHeadPan | headTilt = defaultHeadTilt | neck = defaultNeck | wait(100ms);

             // Spengo tutti i led e accendo quelli della camminata
             turnOffLeds();
             ledF11 = 1;
             ledF12 = 1;

             // Stabilisco che ora il robot sta camminando ed è l'unica attività in esecuzione
             walkingBool = 1;
             turningBool = 0;
             searchingBool = 0;

             // Avvio la camminata
             robot.swalk(walkSteps);
         };


         // Funzione che esegue la ricerca
         function searching() {
             // Blocco il behaviour per girarsi, in modo tale che se girando la testa in fase di ricerca il robot trovi un ostacolo non inizi a girarsi e
             // continui a cercare. Blocco anche il behaviour per la camminata, perchè sennò altrimenti mentre il robot cerca se vede uno spazio che soddisfa
             // il behaviour della camminata lui metterebbe quell'esecuzione tipo in coda e una volta finita la ricerca rieseguirebbe la camminata
             freeze turningBehaviour;
             freeze walkingBehaviour;


             // Stabilisco che ora il robot sta cercando
             walkingBool = 0;
             turningBool = 0;
             searchingBool = 1;

             // Spengo i led e accendo quelli della ricerca
             turnOffLeds();
             ledF13 = 1;
             ledF14 = 1;

             // Durante la ricerca se il robot guardando a destra trova spazio, lo segna incrementando un contatore; se per più volte di fila il robot vede
             // durante la ricerca spazio a destra fino ad arrivare ad un dato valore (di default 2 volte), il robot gira a destra; altrimenti reseta il counter.
             // Di conseguenza controllo se il counter corrente è uguale a quello precedente; in tal caso durante la ricerca passata non è stato trovato spazio
             // a destra, e di conseguenza il counter deve essere riazzerato
             if(currentTurningSearchCounter == previousTurningSearchCounter)
             {
                 currentTurningSearchCounter = 0;
                 previousTurningSearchCounter = 0;
             }
             else
             {
                 // Altrimenti il precedente counter prende il valore di quello corrente
                 previousTurningSearchCounter = currentTurningSearchCounter;
             };


             // Setto il minimo valore della distanza trovato durante la ricerca come il massimo valore captabile dai sensori; è usato per
             // capire se il robot deve assestarsi a destra o a sinistra perchè troppo vicino ad una parete
             adgjustmentMinDistance = 150;


             // Adesso inizia la ricerca vera e propria; setto il valore iniziale della testa tutto a sinistra
             startingPan = 90;

             // Setto il valore dell'angolo da incrementare per ogni step (il numero di step è 8)
             step = 180/searchingSteps;

             // Eseguo tutti gli step della ricerca
             for(k = 0; k <= searchingSteps; k++)
             {
                 // Ruoto la testa in base alla posizione corrente
                 headPan = startingPan - k*step smooth: 150ms | headTilt = -5 smooth: 150ms | wait(100ms);

                 // Se la rotazione della testa supera 45°, alzo il collo del robot in modo che la testa rimanga dritta mentre guarda ai lati
                 // (così prende immagini dalla camera nel modo migliore); altrimenti lo abbasso
                 if(headPan < -45 || headPan > 45)
                     neck = 2
                 else
                     neck = -10;

                 // Controllo la distanza attuale; se il robot è troppo vicino ad un muro o sta guardando molto lontano, non ha senso fare la ricerca in
                 // quanto è troppo vicino o troppo lontano per trovare una vittima; di conseguenza in quel caso si salta l'iterazione
                 avgDistanceNear = averageDistanceNear(2);

                 // Se la distanza è corretta, porto la testa dal basso verso l'alto in un tot di tempo
                 if(avgDistanceNear > 8 && avgDistanceNear <= 40)
                 {
                    headTilt = 30 time: 1000ms;
                 };
             };


             // Terminata la ricerca, riporto la testa dritta
             headPan = defaultHeadPan | headTilt = defaultHeadTilt | neck = defaultNeck | wait(100ms);

             // Controllo se durante la ricerca si è visto che il robot doveva assestarsi perchè troppo vicino ad un murp
             if(adjustRoute == 1)
             {
                adjustRoute = 0;

                startingTime = time();

                // Dopo un tot di tempo blocco l'aggiustamento, altrimenti gira troppo
                at(time() - startingTime > 2300ms) {
                   stop adjustment;
                };


                // Segnalo che sta girando e segno il senso della girata
                turningBool = 1;
                isTurningClockwise = turnClockwise;

                // Giro di poco il robot
                if(turnClockwise == 1)
                    adjustment: robot.sturn(1)
                else
                    adjustment: robot.sturn(-1);

                // Segnalo che non sta' più girando
                turningBool = 0;
             };

             // Finita la ricerca ripristino i behaviour
             unfreeze turningBehaviour;
             unfreeze walkingBehaviour;

             // Stabilisco che il robot non sta facendo niente ora
             walkingBool = 0;
             turningBool = 0;
             searchingBool = 0;
         };


         // Funzione per effettuare la girata
         function turning() {
             /* Per prima cosa al momento di girare il robot deve capire da quale parte girare (senso orario o antiorario); per capirlo gira la testa
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

             // Stabilisco che ora il robot non sta facendo niente (teoricamente sta girando, ma non ancora)
             walkingBool = 0;
             turningBool = 0;
             searchingBool = 0;

             // Spengo i led e accendo quello relativo a questo comportamento
             turnOffLeds();
             ledHW = 1;

             // Stabilisco che di default si gira in senso antiorario
             clockwise = 0;

             // Per capire se deve girare a destra controllo se mentre il robot guarda a destra trova spazio per un tot di tempo di fila; per farlo sfrutto un booleano
             checkingDistance = 0;

             // Quando il booleano diventa 1, inizio a controllare la distanza
             at(checkingDistance == 1) {

                 // Stampo nel mentre dei valori di debug sulla distanza
                 whenever(checkingDistance == 1){
                     printer: distanceNear.val;
                 };

                 // Se il robot vede che dal sensore far c'è spazio e quello vicino è >= 20 (per evitare casi in cui ci sono buchi nelle pareti e il sensore
                 // distanceFar schizza a valori altissimi mentre distanceNear rimane basso) per un tot di tempo, allora c'è spazio per girare a destra
                 at((distanceFar >= 50 && distanceNear >= 20) ~ 150ms) {
                   clockwise = 1;

                   // Riporto a 0 il booleano in modo che si smetta di controllare
                   checkingDistance = 0;
                 };
             };

             // Faccio partire il behaviour appena creato
             checkingDistance = 1;

             // Giro la testa a destra e aspetto un piccolo tot di tempo in modo che il robot abbia il tempo di girarla prima di eseguire il resto
             neck = defaultNeck | headPan = -89 time: 1s | wait(800ms);

             // Riporto la testa nella posizione dei default e faccio aspettare un tot di tempo in modo che la giri
             headPan = defaultHeadPan | headTilt = defaultHeadTilt | neck = defaultNeck | wait(500ms);


             // Terminata questa fase, sblocco il behaviour della camminata (in modo che se ci sia via libera mentre giri smetta di girarsi e
             // vada avanti) e quello della girata
             unfreeze walkingBehaviour;


             // Riporto il valore minimo per camminare al valore di default, in quanto poteva essere stato cambiato da un altro behaviour
             walkingDistanceMin = defaultWalkingDistanceMin;


             // Segno il senso della girata
             isTurningClockwise = clockwise;

             // Spengo tutti i led
             turnOffLeds();

             // Accendo i led di destra o di sinistra a seconda del verso della rotazione
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


             // Stablisco che il robot si sta girando
             walkingBool = 0;
             turningBool = 1;
             searchingBool = 0;

             // Eseguo in loop infinito la girata; si bloccherà quando troverà spazio per camminare
             loop {
                 if(!clockwise)
                 {
                     robot.sturn(-turnSteps);

                     // Dopo 2 passi di girata faccio un passo in avanti, in quanto mentre gira il robot tende ad indietreggiare un po'
                     turningBool = 0;
                     robot.swalk(1);
                     turningBool = 1;
                 }
                 else
                 {
                     robot.sturn(turnSteps);
                     turningBool = 0;
                     robot.swalk(1);
                     turningBool = 1;
                 };
             };

             // Stablisco che il robot non sta eseguendo alcuna attività al momento
             walkingBool = 0;
             turningBool = 0;
             searchingBool = 0;
         };


         // Funzione per inviare frame a C++; le invia con un'attesa per non bombardare C++ di immagini
         function sendingImages() {
            onImage: camera.val;
            wait(30ms);
         };


         // Behaviour della camminata. Quando non c'è un ostacolo per un tot di millisecondi (questo per evitare che quando il robot gira ad un angolo passi
         // da un behaviour all'altro in quanto mentre gira alternerà il vedere ostacoli a non vederne), il robot cammina in avanti,
         // iniziando una fase di ricerca ogni tot
         walkingBehaviour: at (distanceNear >= walkingDistanceMin ~ timeNeededToWalk) {
            // Setto il tempo di camminata come quello originario, in quanto potrebbe essere stato cambiato da un altro behaviour
            timeNeededToWalk = defaultTimeNeededToWalk;

            // Ci sono casi in cui questo behaviour scatta quando il robot sta già camminando; di conseguenza eseguo la camminata solo
            //  se effettivamente il robot non stava già camminando
            if(walkingBool == 0)
            {
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
            }

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
        turningBehaviour: at (distanceNear < defaultWalkingDistanceMin ~ 300ms) {
            // Eseguo la girata vera e propria
            turn: turning();
        };



        // Durante la fase di ricerca, il robot gira la testa da sinistra a 90. Quando guarda a destra (cioè l'angolo della testa è maggiore di un dato valore)
        // se i sensori notano che non c'è una parete (cioè distanceFar è >= 45) allora vuol dire che c'è una strada alla destra del robot. Se questo è vero
        // per un tot di tempo, prendiamo l'informazione per buona e se è la seconda votla che capita di fila facciamo girare il robot a destra
        at ((distanceFar >= 45 && headPan < -75 && searchingBool == 1) ~ 200ms) {

            // Incremento il counter che indica che è stato trovato spazio a destra
            currentTurningSearchCounter++;

            // Accendo un led per indicare questo incremento
            ledHC = 1;

            // Stampo il valore del contatore in C++ per capire cosa succede
            printer: currentTurningSearchCounter;

            // Se il contatore è arrivato a 2, giro a destra
            if(currentTurningSearchCounter == 2)
            {
                // Riporto i contatori a zero
                currentTurningSearchCounter = 0;
                previousTurningSearchCounter = 0;

                // Accendo alcuni led per indicare il comportamento
                ledHC = 1;
                ledHW = 1;

                // Stabilisco che il robot ora non sta facendo niente ancora
                walkingBool = 0;
                turningBool = 0;
                searchingBool = 0;

                // Blocco eventuali comportamenti attualmente in esecuzione
                stop pattern;
                stop walk;
                stop search;

                // Blocco la girata, per evitare che adesso venga eseguita
                freeze turningBehaviour;


                // Riporto a 0 il booleano che indica se dopo la ricerca il robot deve assestarsi, altrimenti alla ricerca successiva
                // alla girata lo farebbe (anche se non ci fosse bisogno) se alla ricerca corrente era stato messo a 1 il booleano
                adjustRoute = 0;

                // Stabilisco che il robot sta camminando e gli faccio fare un passo in avanti prima di girare, in modo che il robot si metta meglio
                walkingBool = 1;
                robot.swalk(1);
                walkingBool = 0;

                // Sblocco il behaviour della girata, qualora fosse ancora bloccato dalla ricerca
                unfreeze walkingBehaviour;

                // Stabilisco che per un attimo la distanza necessaria per il behaviour della camminata è 151, un valore irraggiungibile; faccio questo perchè
                // voglio che il behaviour della camminata, che è un "at" del linguaggio URBI, diventi falso, in modo che appena torna vero venga eseguito il behaviour
                walkingDistanceMin = 151;
                timeNeededToWalk = 1ms;
                wait(100ms);

                // Blocco il behaviour della camminata, che è appena diventato falso al 100%; verrà riattivato da turning()
                freeze walkingBehaviour;

                // Setto il tempo necessario per l'attivazione del behaviour della camminata; lo metto più alto del solito, perchè voglio che il robot giri di più
                timeNeededToWalk = 3600ms;

                 // Avvio la girata vera e propria
                turn: turning();
            };
        };


        // Behaviour eseguito quando C++ trova una vittima nelle immagini
        at (victimFoundBool == 1) {
            // Eseguo il codice seguente solo se non è stata trovata una vittima troppo recentemente, per evitare troppe cose tutte di fila
            if(playedSoundRecently == 0)
            {
                // Segnalo C++ che è stata trovata la vittima e passo la distanza della vittima dal robot per visualizzarla nella mappa
                victimFound: distanceNear.val;

                // Eseguo il suono
                speaker.play("iseeyou.wav");

                playedSoundRecently = 1;

                start = time();

                // Accendo alcuni led
                modeB = 0;
                modeG = 0;
                modeR = 1;

                // Muovo le orecchie
                earR = 1 smooth: 200ms | earL = 1 smooth: 200ms;

                // Muovo la coda
                tailPan = 50 smooth: 300ms;
                tailPan = -50 smooth: 300ms;
                tailPan = 50 smooth: 300ms;
                tailPan = -50 smooth: 300ms;
                tailPan = 50 smooth: 300ms;
                tailPan = -50 smooth: 300ms;
                tailPan = 0 smooth: 200ms;

                // Se è passato un tot di tempo riporto a 0 i booleani
                at (time() - start > 2000ms) {
                    playedSoundRecently = 0;
                    victimFoundBool = 0;

                    // Riporto i led delle orecchie come prima
                    modeB = 1;
                    modeG = 1;
                    modeR = 0;

                    // Riporto le orecchie come prima
                    earR = 0 smooth: 200ms | earL = 0 smooth: 200ms;
                };
            };
        };


        // Behaviour di convenienza per bloccare la ricerca se si tocca il pulsante sulla schiena del robot
        at (backSensorM > 0) {
            stop search;
        };

    ));

    // Setto il callback degli errori
    MyCallback errorCallback;
    client->setErrorCallback(errorCallback);

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


    // Controllo se nell'immagine è presente una vittima
    if(imageManager.findVictim(image))
    {
        victimFoundCounter++;

        msg.client.printf("FOUND VICTIM number %d\n", victimFoundCounter);

        // Invio al robot la notifica che è stata trovata una vittima
        msg.client.send("victimFoundBool = 1;");


        // Prendo l'immagine trovata
        Mat victimImage = imageManager.getProcessedImage();

        // Aggiungo la scritta che mostra il numero della vittima
        std::string v("#");
        std::string label = v.append(std::to_string(victimFoundCounter));
        int baseline=0;
        Size textLength = getTextSize(label, FONT_HERSHEY_PLAIN, 1, 1, &baseline);
        putText(victimImage, label, Point(10, 20 - textLength.height/2), FONT_HERSHEY_PLAIN, 1.0, CV_RGB(255,255,255), 1.0, LINE_8, false);

        // Passo l'immagine al modulo che la mostrerà graficamente
        emit victimImageFound(victimImage);
    }

    // In ogni caso passo l'immagine processata al modulo che la mostra graficamente
    emit newCameraImage(imageManager.getProcessedImage());


    // Elimino l'immagine in byte, non serve più
    delete imageRaw;

    return URBI_CONTINUE;
}


/**
 * Callback chiamato dal robot; riceve l'headPan del robot
 */
UCallbackAction  RobotManager::onHeadPanCallback(const UMessage &msg)
{
    if (msg.type != MESSAGE_DOUBLE)
        return URBI_CONTINUE;

    // Salvo il valore della testa nell'array apposito
    headPanVec[averageCounter] = msg.doubleValue;

    return URBI_CONTINUE;
}

/**
 * Callback chiamato dal robot; riceve il valore della distanza trovato dal robot
 */
UCallbackAction  RobotManager::onDistanceSensorCallback(const UMessage &msg)
{
    if (msg.type != MESSAGE_DOUBLE)
        return URBI_CONTINUE;

    // Aggiungo il valore ricevuto all'array
    distanceVec[averageCounter] = msg.doubleValue;
    averageCounter++;

    // Se sono arrivato al valore della media stabilito, la eseguo e passo il valore al modulo che genera la mappa
    if(averageCounter == AVERAGE_THRESHOLD)
    {
        // Ordino gli array per togliere il valore più piccolo e il più grande da entrambi prima di calcolare la media
        std::sort (distanceVec.begin(), distanceVec.end());
        std::sort (headPanVec.begin(), headPanVec.end());

        double distanceValue = 0;
        double headPanValue = 0;

        for(int i = 1; i < distanceVec.size()-1; i++)
        {
            distanceValue += distanceVec[i];
            headPanValue += headPanVec[i];
        }

        // Passo la media dei valori trovati
        emit pointFound(distanceValue / (distanceVec.size()-2), -headPanValue / (distanceVec.size()-2));

        averageCounter = 0;
    }

    return URBI_CONTINUE;
}

/**
 * Callback chiamato dal robot; riceve il valore della distanza trovato dal robot e notifica il modulo della mappa che è stata trovata una vittima
 */
UCallbackAction  RobotManager::onVictimFoundCallback(const UMessage &msg)
{
    if (msg.type != MESSAGE_DOUBLE)
        return URBI_CONTINUE;

    emit victimFound(msg.doubleValue);

    return URBI_CONTINUE;
}

/**
 * Callback chiamato dal robot; riceve il valore della rotazione del robot e notifica il modulo della mappa che si sta girando
 */
UCallbackAction  RobotManager::onRobotTurnCallback(const UMessage &msg)
{
    if (msg.type != MESSAGE_DOUBLE)
        return URBI_CONTINUE;

    emit robotTurn(msg.doubleValue);

    return URBI_CONTINUE;
}


/**
 * Callback chiamato dal robot; notifica il modulo della mappa che sta camminando
 */
UCallbackAction  RobotManager::onGenerateStepCallback(const UMessage &msg)
{
    emit generateStep();

    return URBI_CONTINUE;
}


/**
 * Callback ausiliario chiamato dal robot; stampa il valore che riceve
 */
UCallbackAction  RobotManager::onPrinterCallback(const UMessage &msg)
{
    if (msg.type != MESSAGE_DOUBLE)
        return URBI_CONTINUE;

    msg.client.printf("onPrinterCallback - received value: %f\n",  msg.doubleValue);

    return URBI_CONTINUE;
}

