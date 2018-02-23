QT += core
QT += sql
QT += xml
QT -= gui

CONFIG += c++11

TARGET = KyoukoMind
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

LIBS += -L../libNetworkConnection -lNetworkConnection
LIBS += -L../libNetworkConnection/debug -lPNetworkConnection
LIBS += -L../libNetworkConnection/release -lNetworkConnection
INCLUDEPATH += ../libNetworkConnection/include/libNetworkConnection

LIBS += -L../libCrypto -lCrypto
LIBS += -L../libCrypto/debug -lCrypto
LIBS += -L../libCrypto/release -lCrypto
INCLUDEPATH += ../libCrypto/include/libCrypto

LIBS += -L../libPersistence -lPersistence
LIBS += -L../libPersistence/debug -lPersistence
LIBS += -L../libPersistence/release -lPersistence
INCLUDEPATH += ../libPersistence/include/libPersistence

INCLUDEPATH += $$PWD

SOURCES += src/main.cpp \
            core/cluster/cluster.cpp \
            core/networkManager.cpp \
            core/cluster/emptyCluster.cpp \
            core/cluster/nodeCluster.cpp \
            core/cluster/edgeCluster.cpp \
            core/processing/cpuProcessingUnit.cpp \
            core/processing/processingUnit.cpp \
            KyoChanNetwork.cpp \
            core/cluster/clusterHandler.cpp \
            core/processing/processingUnitHandler.cpp \
            persistence/initLogger.cpp \
            persistence/initialFileInput.cpp \
            persistence/database.cpp \
            persistence/config.cpp \
            control/statusreporter.cpp \
            tests/clusterTest.cpp

HEADERS +=\
            core/cluster/cluster.h \
            core/networkManager.h \
            common/typedefs.h \
            core/cluster/emptyCluster.h \
            core/cluster/nodeCluster.h \
            core/cluster/edgeCluster.h \
            core/processing/processingUnit.h \
            core/processing/cpuProcessingUnit.h \
            KyoChanNetwork.h \
            core/cluster/clusterHandler.h \
            core/processing/processingUnitHandler.h \
            common/structs.h \
            persistence/initLogger.h \
            persistence/initialFileInput.h \
            persistence/database.h \
            persistence/config.h \
            control/statusreporter.h \
            tests/clusterTest.h

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
