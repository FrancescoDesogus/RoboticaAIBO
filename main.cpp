#include <QtGui/QGuiApplication>
#include <iostream>
#include "mapmanager.h"


int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    MapManager mapManager;

    mapManager.init();

    return 0;
}


