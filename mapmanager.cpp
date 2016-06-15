#include "mapmanager.h"
#include "robotmanager.h"

#include <iostream>
#include <QQmlContext>
#include <QThread>
#include <QtGui/QGuiApplication>

// Grandezza dello schermo
const int MapManager::WIDTH = 800;
const int MapManager::HEIGHT = 600;

// Rotazione della finestra
const int MapManager::WINDOW_ROTATION = 0;

MapManager::MapManager(QObject *parent) :
    QObject(parent)
{

}

/**
 * @brief MapManager::init si occupa di inizializzare il thread che controlla il robot e di fare il setup della GUI
 */
void MapManager::init()
{
    // Creo il thread del robot
    setupRobotManagerThread();

    // Adesso creo la view QML
    QQuickView view;


    QQmlContext* rootContext = view.rootContext();

    // Espongo al modulo QML questa classe, in modo che possa connettere i suoi signal
    rootContext->setContextProperty("window", this);

    // Inserisco come proprietà QML delle costanti che corrispondono alla grandezza della finestra
    rootContext->setContextProperty("WINDOW_WIDTH", WIDTH);
    rootContext->setContextProperty("WINDOW_HEIGHT", HEIGHT);

    // Espongo anche una costante che indica la rotazione dello schermo; serve se ad esempio il robot ha un'orientazione
    // diversa da quella verso nord e si vuole che la finestra abbia la stessa rotazione
    rootContext->setContextProperty("WINDOW_ROTATION", WINDOW_ROTATION);

    //Carico il file base
    view.setSource(QStringLiteral("main.qml"));

    // Finito con i setaggi, mostro la finestra
    view.show();


//    emit pointFound(30, 0);

    // Avvio il loop della GUI
    QGuiApplication::instance()->exec();
}

/**
 * @brief MapManager::setupRobotManagerThread esegue il setup del thread su cui gira la parte di controllo del robot
 */
void MapManager::setupRobotManagerThread()
{
    //Creo l'istanza del thread secondario
//    QThread* robotManagerThread = new QThread(this);
    robotManagerThread = new QThread(this);


    // Sposto l'oggetto che controlla il robot nell'altro thread
    robotManager.moveToThread(robotManagerThread);

    //Connetto il signal che dichiara l'inizio dell'esecuzione del thread con lo slot che si occupa dell'inizializzazione della classe
    QObject::connect(robotManagerThread, SIGNAL(started()), &robotManager, SLOT(init()));

    // Connetto i signal della classe del robot a quelli della GUI, in modo da poterli usare da QML
    QObject::connect(&robotManager, SIGNAL(pointFound(QVariant,QVariant)), this, SIGNAL(pointFound(QVariant,QVariant)));
    QObject::connect(&robotManager, SIGNAL(robotStateChanged(QVariant,QVariant)), this, SIGNAL(robotStateChanged(QVariant,QVariant)));


    // Segnalo di terminare il thread alla chiusura della GUI (anche se non sembra far nulla)
    QObject::connect(this, SIGNAL(destroyed()), robotManagerThread, SLOT(quit()));

    // Avvio il thread
    robotManagerThread->start();
}

void MapManager::terminateRobot()
{
    std::cout << "aa\n" << std::flush;
    robotManagerThread->terminate();
    delete robotManagerThread;
}



