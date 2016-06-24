#include "mapmanager.h"
#include "robotmanager.h"

#include <iostream>
#include <QQmlContext>
#include <QThread>
#include <QApplication>
#include <QPixmap>
#include <QLabel>
#include <QVBoxLayout>
#include <QScrollArea>

// Grandezza della schermata
const int MapManager::WIDTH = 800;
const int MapManager::HEIGHT = 600;


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

    //Carico il file base
    view.setSource(QStringLiteral("main.qml"));

    // Finito con i setaggi, mostro la finestra
    view.show();


    // Creo il widget che mostra le immagini della camera del robot durante la rircerca
    QWidget cameraWidget;
    QHBoxLayout *hbox = new QHBoxLayout(&cameraWidget);

    // Aggiungo alla horizontal box la QLabel che conterrà le immagini della camera
    hbox->addWidget(&cameraLabel);

    cameraWidget.show();


    // Creo il widget che mostra le vittime trovate
    QWidget victimsFoundWidget;
    QScrollArea *scrollArea = new QScrollArea();

    layout = new QVBoxLayout(&victimsFoundWidget);

    // setto le proprietà della ScrollArea e la mostro
    scrollArea->setWidget(&victimsFoundWidget);
    scrollArea->setWidgetResizable(true);
    scrollArea->setMinimumWidth(260);
    scrollArea->setMinimumHeight(300);
    scrollArea->show();

    // Avvio il loop della GUI
    QApplication::instance()->exec();
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
    QObject::connect(&robotManager, SIGNAL(victimFound(QVariant)), this, SIGNAL(victimFound(QVariant)));
    QObject::connect(&robotManager, SIGNAL(robotTurn(QVariant)), this, SIGNAL(robotTurn(QVariant)));
    QObject::connect(&robotManager, SIGNAL(generateStep()), this, SIGNAL(generateStep()));

    // Dato che uso signal che passano paremtri Mat di opencv, devo esplori al meta system delle Qt
    qRegisterMetaType< cv::Mat >("cv::Mat");

    // Connetto i signal che inviano immagini da mostrare ai rispettivi slot
    QObject::connect(&robotManager, SIGNAL(newCameraImage(cv::Mat)), this, SLOT(printImage(cv::Mat)));
    QObject::connect(&robotManager, SIGNAL(victimImageFound(cv::Mat)), this, SLOT(printVictimImage(cv::Mat)));

    // Segnalo di terminare il thread alla chiusura della GUI (anche se non sembra far nulla)
    QObject::connect(this, SIGNAL(destroyed()), robotManagerThread, SLOT(quit()));

    // Avvio il thread
    robotManagerThread->start();
}


/**
 * @brief MapManager::printImage slot chiamato quando il robot invia immagini della camera da mostrare
 * @param cvImage l'immagine da mostrare
 */
void MapManager::printImage(cv::Mat cvImage)
{
    // Converto l'immagine da Mat a QImage
    QImage image = Mat2QImage(cvImage);

    // Setto l'immagine e aggiorno la grandezza della finestra
    cameraLabel.setPixmap(QPixmap::fromImage((image)));
    cameraLabel.adjustSize();
}

/**
 * @brief MapManager::printVictimImage slot chiamato quando il robot trova una vittima
 * @param cvImage l'immagine da mostrare
 */
void MapManager::printVictimImage(cv::Mat cvImage)
{
    QImage image = Mat2QImage(cvImage);

    QLabel *victimImage = new QLabel();
    victimImage->setPixmap(QPixmap::fromImage((image)));
    victimImage->adjustSize();

    layout->addWidget(victimImage);
}

/**
 * @brief MapManager::Mat2QImage converte immagini da Mat a QImage (funzione trovata online su stackoverflow)
 * @param src l'immagine
 * @return l'immagine convertita
 */
QImage MapManager::Mat2QImage(const cv::Mat3b &src)
{
    QImage dest(src.cols, src.rows, QImage::Format_ARGB32);

    for (int y = 0; y < src.rows; ++y)
    {
        const cv::Vec3b *srcrow = src[y];
        QRgb *destrow = (QRgb*)dest.scanLine(y);
        for (int x = 0; x < src.cols; ++x) {
            destrow[x] = qRgba(srcrow[x][2], srcrow[x][1], srcrow[x][0], 255);
        }
    }

    return dest;
}




