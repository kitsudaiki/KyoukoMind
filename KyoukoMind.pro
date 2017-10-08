QT += core
QT += sql
QT += xml
QT -= gui

CONFIG += c++11

TARGET = KyoukoMind
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

LIBS += -L../libKyoChanNetwork -lKyoChanNetwork
LIBS += -L../libKyoChanNetwork/debug -lKyoChanNetwork
LIBS += -L../libKyoChanNetwork/release -lKyoChanNetwork
INCLUDEPATH += ../libKyoChanNetwork/include/libKyoChanNetwork

LIBS += -L../libKyoChanMQ -lKyoChanMQ
LIBS += -L../libKyoChanMQe/debug -lKyoChanMQ
LIBS += -L../libKyoChanMQ/release -lKyoChanMQ
INCLUDEPATH += ../libKyoChanMQ/include/libKyoChanMQ

LIBS += -L../libKyoChanPersistence -lKyoChanPersistence
LIBS += -L../libKyoChanPersistence/debug -lKyoChanPersistence
LIBS += -L../libKyoChanPersistence/release -lKyoChanPersistence
INCLUDEPATH += ../libKyoChanPersistence/include/libKyoChanPersistence

LIBS += -L../libPersistence -lPersistence
LIBS += -L../libPersistence/debug -lPersistence
LIBS += -L../libPersistence/release -lPersistence
INCLUDEPATH += ../libPersistence/include/libPersistence

INCLUDEPATH += $$PWD

SOURCES += src/main.cpp

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
