QT -= core
QT -= gui

CONFIG += c++17

TARGET = KyoukoMind
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

LIBS += -L../libPerformanceIO -lPerformanceIO
LIBS += -L../libPerformanceIO/debug -lPerformanceIO
LIBS += -L../libPerformanceIO/release -lPerformanceIO
INCLUDEPATH += ../libPerformanceIO/include/libPerformanceIO

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
            src/settings/config.cpp \
            src/control/statusreporter.cpp \
            src/core/messaging/messages/message.cpp \
            src/core/messaging/messages/datamessage.cpp \
            src/core/messaging/messages/replymessage.cpp \
            src/core/messaging/messages/learningmessage.cpp \
            src/core/messaging/messages/learningreplymessage.cpp \
            src/core/messaging/timeouthandler.cpp \
            src/core/messaging/messagecontroller.cpp \
    src/core/messaging/messages/cyclefinishmessage.cpp \
    tests/messagetest.cpp \
    tests/clustertest.cpp \
    src/core/messaging/messageQueues/incomingMessageBuffer.cpp \
    src/core/messaging/messageQueues/messageBuffer.cpp \
    src/core/messaging/messageQueues/outgoingMessageBuffer.cpp \
    tests/messagebuffertest.cpp

HEADERS +=\
            src/common/typedefs.h \
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
            src/settings/config.h \
            src/control/statusreporter.h \
            src/common/enums.h \
            src/core/messaging/messages/message.h \
            src/core/messaging/messages/datamessage.h \
            src/core/messaging/messages/replymessage.h \
            src/core/messaging/messages/learningmessage.h \
            src/core/messaging/messages/learningreplymessage.h \
            src/core/messaging/timeouthandler.h \
            src/core/messaging/messagecontroller.h \
            src/common/clusterstructs.h \
            src/common/messagestructs.h \
            src/common/netstructs.h \
            src/common/includes.h \
            common.h \
    src/core/messaging/messages/cyclefinishmessage.h \
    tests/messagetest.h \
    tests/clustertest.h \
    src/common/defines.h \
    src/core/messaging/messageQueues/incomingMessageBuffer.h \
    src/core/messaging/messageQueues/messageBuffer.h \
    src/core/messaging/messageQueues/outgoingMessageBuffer.h \
    tests/messagebuffertest.h

        # The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
