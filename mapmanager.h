#ifndef MAPMANAGER_H
#define MAPMANAGER_H

#include <QQuickView>
#include "robotmanager.h"

class MapManager : public QObject
{
    Q_OBJECT
public:
    MapManager(QObject *parent = 0);


    static const int WIDTH;
    static const int HEIGHT;
    static const int WINDOW_ROTATION;

private:
    RobotManager robotManager;
    QThread* robotManagerThread;

    void setupRobotManagerThread();

public slots:
    void init();
    void terminateRobot();

signals:
    void pointFound(QVariant distance, QVariant angle);
    void robotStateChanged(QVariant newState, QVariant newAngle);
};

#endif // MAPMANAGER_H
