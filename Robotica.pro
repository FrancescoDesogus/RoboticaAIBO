TEMPLATE = app
CONFIG += console
CONFIG -= app_bundle
CONFIG += c++11

QT += core gui qml quick widgets

# Additional import path used to resolve QML modules in Creator's code model
#QML_IMPORT_PATH =

SOURCES += main.cpp \
    imagemanager.cpp \
    mapmanager.cpp \
    robotmanager.cpp

INCLUDEPATH += $$PWD\..\..\liburbi
INCLUDEPATH += /usr/local/include/opencv
LIBS += $$PWD//lib/liburbi.a -pthread -L/usr/local/lib -lopencv_core -lopencv_imgcodecs -lopencv_highgui -lopencv_imgproc

HEADERS += \
    imagemanager.h \
    mapmanager.h \
    robotmanager.h

OTHER_FILES += \
    main.qml
