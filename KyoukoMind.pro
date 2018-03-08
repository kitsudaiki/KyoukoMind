QT += core
QT += sql
QT += xml
QT += testlib
QT += network
QT -= gui

CONFIG += c++11

TARGET = KyoukoMind
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

LIBS += -L../libPersistence -lPersistence
LIBS += -L../libPersistence/debug -lPersistence
LIBS += -L../libPersistence/release -lPersistence
INCLUDEPATH += ../libPersistence/include/libPersistence

LIBS += -L../libNetworkConnection -lNetworkConnection
LIBS += -L../libNetworkConnection/debug -lNetworkConnection
LIBS += -L../libNetworkConnection/release -lNetworkConnection
INCLUDEPATH += ../libNetworkConnection/include/libNetworkConnection

INCLUDEPATH += $$PWD \
            src

SOURCES += main.cpp \
            src/KyoChanNetwork.cpp \
            src/core/cluster/cluster.cpp \
            src/core/cluster/emptyCluster.cpp \
            src/core/cluster/nodeCluster.cpp \
            src/core/cluster/edgeCluster.cpp \
            src/core/cluster/clusterHandler.cpp \
            src/core/processing/cpuProcessingUnit.cpp \
            src/core/processing/processingUnit.cpp \
            src/core/processing/processingUnitHandler.cpp \
            src/core/networkManager.cpp \
            src/settings/initLogger.cpp \
            src/settings/database.cpp \
            src/settings/config.cpp \
            src/control/statusreporter.cpp \
            tests/clusterTest.cpp \
    src/core/messaging/messageQueue.cpp \
    tests/test.cpp

HEADERS +=\
            src/common/typedefs.h \
            src/common/structs.h \
            src/KyoChanNetwork.h \
            src/core/cluster/clusterHandler.h \
            src/core/cluster/cluster.h \
            src/core/cluster/emptyCluster.h \
            src/core/cluster/nodeCluster.h \
            src/core/cluster/edgeCluster.h \
            src/core/networkManager.h \
            src/core/processing/processingUnit.h \
            src/core/processing/cpuProcessingUnit.h \
            src/core/processing/processingUnitHandler.h \
            src/settings/initLogger.h \
            src/settings/database.h \
            src/settings/config.h \
            src/control/statusreporter.h \
            tests/clusterTest.h \
    src/core/messaging/messageQueue.h \
    src/common/enums.h \
    tests/test.h

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
