#ifndef ROBOTMANAGER_H
#define ROBOTMANAGER_H

#include <imagemanager.h>
#include <QObject>
#include <QVariant>
#include <lib/uclient.h>


class RobotManager : public QObject
{
    Q_OBJECT

public:
    RobotManager(QObject *parent = 0);


public slots:
    void init();

private:
    // Valore threshold per la media calcolata sui sensori della distanza per generare meglio la mappa
    static const int AVERAGE_THRESHOLD;

    UCallbackAction onImageCallback(const UMessage &msg);
    UCallbackAction onHeadPanCallback( const UMessage &msg);
    UCallbackAction onDistanceSensorCallback( const UMessage &msg);
    UCallbackAction onGenerateStepCallback( const UMessage &msg);
    UCallbackAction onRobotTurnCallback( const UMessage &msg);
    UCallbackAction onVictimFoundCallback( const UMessage &msg);
    UCallbackAction onPrinterCallback(const UMessage &msg);


    ImageManager imageManager;
    std::vector<double> distanceVec;
    std::vector<double> headPanVec;
    int averageCounter;
    int victimFoundCounter;

signals:
    void pointFound(QVariant distance, QVariant angle);
    void newCameraImage(cv::Mat cvImage);
    void victimImageFound(cv::Mat cvImage);
    void victimFound(QVariant distance);
    void robotTurn(QVariant angle);
    void generateStep();
};


#endif // ROBOTMANAGER_H
