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
    UCallbackAction onImageCallback(const UMessage &msg);
    UCallbackAction onHeadPanCallback( const UMessage &msg);
    UCallbackAction onDistanceSensorCallback( const UMessage &msg);
    UCallbackAction onRobotStateChanged( const UMessage &msg);
    UCallbackAction onOrientationChanged( const UMessage &msg);
    UCallbackAction  onProva(const UMessage &msg);


    ImageManager imageManager;
    double headPanValue;
    double distanceValue;
    std::vector<double> distanceVec;
    std::vector<double> headPanVec;
    double averageCounter;
    int robotState;
    int robotOrientation;
    int average;

    int imageFoundCounter;

signals:
    void pointFound(QVariant distance, QVariant angle);
    void robotStateChanged(QVariant newState, QVariant newAngle);
};


#endif // ROBOTMANAGER_H
