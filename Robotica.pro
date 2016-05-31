TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG -= qt
CONFIG += c++11

SOURCES += main.cpp

INCLUDEPATH += $$PWD\..\..\liburbi
INCLUDEPATH += /usr/local/include/opencv
LIBS += $$PWD//lib/liburbi.a -pthread -L/usr/local/lib -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc
