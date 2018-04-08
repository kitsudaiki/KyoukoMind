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
            src/core/messaging/messages/message.cpp \
            src/core/messaging/messageQueues/incomingMessageBuffer.cpp \
            src/core/messaging/messageQueues/messageBuffer.cpp \
            src/core/messaging/messageQueues/outgoingMessageBuffer.cpp \
            src/core/initializing/networkInitializer.cpp \
    src/core/processing/nextChooser.cpp \
    src/control/statusReporter.cpp \
    src/core/initializing/axonInitializer.cpp \
    src/core/initializing/clusterInitilizer.cpp \
    src/core/messaging/messages/cycleFinishMessage.cpp \
    src/core/messaging/messages/dataMessage.cpp \
    src/core/messaging/messages/learningMessage.cpp \
    src/core/messaging/messages/learningReplyMessage.cpp \
    src/core/messaging/messages/replyMessage.cpp \
    src/core/messaging/messageController.cpp \
    src/core/messaging/timeoutHandler.cpp \
    src/core/processing/commonThread.cpp \
    src/kyochanNetwork.cpp \
    tests/proofOfConcept/demoIO.cpp \
    tests/clusterTest.cpp \
    tests/commonTest.cpp \
    tests/messageBufferTest.cpp \
    tests/messageTest.cpp

HEADERS +=\
            src/common/typedefs.h \
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
            src/common/enums.h \
            src/core/messaging/messages/message.h \
            src/common/includes.h \
            common.h \
            src/common/defines.h \
            src/core/messaging/messageQueues/incomingMessageBuffer.h \
            src/core/messaging/messageQueues/messageBuffer.h \
            src/core/messaging/messageQueues/outgoingMessageBuffer.h \
            src/core/initializing/networkInitializer.h \
            src/core/structs/kyochanEdges.h \
            src/core/structs/kyochanNodes.h \
            src/core/structs/learningMessages.h \
            src/core/structs/clusterMeta.h \
    src/core/processing/nextChooser.h \
    src/control/statusReporter.h \
    src/core/initializing/axonInitializer.h \
    src/core/initializing/clusterInitilizer.h \
    src/core/messaging/messages/cycleFinishMessage.h \
    src/core/messaging/messages/dataMessage.h \
    src/core/messaging/messages/learningMessage.h \
    src/core/messaging/messages/learningReplyMessage.h \
    src/core/messaging/messages/replyMessage.h \
    src/core/messaging/messageController.h \
    src/core/messaging/timeoutHandler.h \
    src/core/processing/commonThread.h \
    src/kyochanNetwork.h \
    tests/proofOfConcept/demIO.h \
    tests/clusterTest.h \
    tests/commonTest.h \
    tests/messageBufferTest.h \
    tests/messageTest.h

        # The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
