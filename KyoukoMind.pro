QT -= qt core gui

CONFIG += c++14

TARGET = KyoukoMind
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

LIBS += -L../libKitsuneCommon -lKitsuneCommon
LIBS += -L../libKitsuneCommon/debug -lKitsuneCommon
LIBS += -L../libKitsuneCommon/release -lKitsuneCommon
INCLUDEPATH += ../libKitsuneCommon/include/libKitsuneCommon

LIBS += -L../libKitsuneJson/src -lKitsuneJson
LIBS += -L../libKitsuneJson/src/debug -lKitsuneJson
LIBS += -L../libKitsuneJson/src/release -lKitsuneJson
INCLUDEPATH += ../libKitsuneJson/include/libKitsuneJson

LIBS += -L../libKitsunePersistence -lKitsunePersistence
LIBS += -L../libKitsunePersistence/debug -lKitsunePersistence
LIBS += -L../libKitsunePersistence/release -lKitsunePersistence
INCLUDEPATH += ../libKitsunePersistence/include/libKitsunePersistence

LIBS += -L../libKitsuneNetwork/src -lKitsuneNetwork
LIBS += -L../libKitsuneNetwork/src/debug -lKitsuneNetwork
LIBS += -L../libKitsuneNetwork/src/release -lKitsuneNetwork
INCLUDEPATH += ../libKitsuneNetwork/include/libKitsuneNetwork

LIBS += -L../libKitsuneChanCommunication -lKitsuneChanCommunication
LIBS += -L../libKitsuneChanCommunication/debug -lKitsuneChanCommunication
LIBS += -L../libKitsuneChanCommunication/release -lKitsuneChanCommunication
INCLUDEPATH += ../libKitsuneChanCommunication/include/libKitsuneChanCommunication

INCLUDEPATH += $$PWD \
            src

SOURCES += \
    src/core/clustering/cluster/cluster.cpp \
    src/core/clustering/cluster/edgeCluster.cpp \
    src/core/clustering/cluster/nodeCluster.cpp \
    src/core/clustering/clusterHandler.cpp \
    src/core/clustering/clusterQueue.cpp \
    src/core/messaging/messages/dataMessage.cpp \
    src/core/messaging/messages/message.cpp \
    src/core/messaging/incomingMessageBuffer.cpp \
    src/core/messaging/outgoingMessageBuffer.cpp \
    src/core/processing/cpuProcessingUnit.cpp \
    src/core/processing/processingUnit.cpp \
    src/core/processing/processingUnitHandler.cpp \
    src/core/networkManager.cpp \
    src/initializing/axonInitializer.cpp \
    src/initializing/clusterInitilizer.cpp \
    src/initializing/networkInitializer.cpp \
    src/settings/config.cpp \
    src/kyoChanNetwork.cpp \
    tests/initializing/initTest.cpp \
    tests/core/messaging/message/messageTest.cpp \
    main.cpp \
    src/core/clustering/globalValuesHandler.cpp \
    tests/core/clustering/cluster/clusterTest.cpp \
    tests/core/clustering/cluster/edgeClusterTest.cpp \
    tests/core/clustering/cluster/nodeClusterTest.cpp \
    tests/runUnitTests.cpp \
    src/core/networkInteraction/connectionTrigger.cpp


HEADERS += \
    src/common/defines.h \
    src/common/enums.h \
    src/common/includes.h \
    src/common/typedefs.h \
    src/core/clustering/cluster/cluster.h \
    src/core/clustering/cluster/edgeCluster.h \
    src/core/clustering/cluster/nodeCluster.h \
    src/core/clustering/clusterHandler.h \
    src/core/clustering/clusterQueue.h \
    src/core/messaging/messages/dataMessage.h \
    src/core/messaging/messages/message.h \
    src/core/messaging/incomingMessageBuffer.h \
    src/core/messaging/outgoingMessageBuffer.h \
    src/core/processing/cpuProcessingUnit.h \
    src/core/processing/processingUnit.h \
    src/core/processing/processingUnitHandler.h \
    src/core/structs/clusterMeta.h \
    src/core/structs/kyochanEdges.h \
    src/core/structs/kyochanNodes.h \
    src/core/structs/messageContainer.h \
    src/core/structs/messageHeader.h \
    src/core/networkManager.h \
    src/initializing/axonInitializer.h \
    src/initializing/clusterInitilizer.h \
    src/initializing/networkInitializer.h \
    src/settings/config.h \
    src/common.h \
    src/kyoChanNetwork.h \
    tests/initializing/initTest.h \
    tests/core/messaging/message/messageTest.h \
    src/core/common/weightmap.h \
    src/core/structs/globalValues.h \
    src/core/clustering/globalValuesHandler.h \
    tests/core/clustering/cluster/clusterTest.h \
    tests/core/clustering/cluster/edgeClusterTest.h \
    tests/core/clustering/cluster/nodeClusterTest.h \
    tests/runUnitTests.h \
    src/common/methods.h \
    src/common/using.h \
    src/core/networkInteraction/connectionTrigger.h
