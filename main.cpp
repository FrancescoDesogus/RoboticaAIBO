#include <QApplication>
#include <iostream>
#include "mapmanager.h"


int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    MapManager mapManager;

    mapManager.init();

    return 0;
}


