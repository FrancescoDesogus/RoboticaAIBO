#ifndef MAPMANAGER_H
#define MAPMANAGER_H

#include <QQuickView>
#include <opencv2/imgcodecs.hpp>
#include "robotmanager.h"
#include <QLabel>
#include <QScrollArea>
#include <QVBoxLayout>

class MapManager : public QObject
{
    Q_OBJECT
public:
    MapManager(QObject *parent = 0);


    static const int WIDTH;
    static const int HEIGHT;

private:
    RobotManager robotManager;
    QThread* robotManagerThread;

    // Label per le immagini della camera
    QLabel cameraLabel;

    // Layout su cui inserire le immagini delle vittime trovate
    QVBoxLayout *layout;

    QImage Mat2QImage(const cv::Mat3b &src);
    void setupRobotManagerThread();

public slots:
    void init();
    void printImage(cv::Mat cvImage);
    void printVictimImage(cv::Mat cvImage);

signals:
    void pointFound(QVariant distance, QVariant angle);
    void victimFound(QVariant distance);
    void robotTurn(QVariant angle);
    void generateStep();
};

#endif // MAPMANAGER_H
