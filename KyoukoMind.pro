QT -= core
QT -= gui

CONFIG += c++17

TARGET = KyoukoMind
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

LIBS += -L../libKitsuneCommon -lKitsuneCommon
LIBS += -L../libKitsuneCommon/debug -lKitsuneCommon
LIBS += -L../libKitsuneCommon/release -lKitsuneCommon
INCLUDEPATH += ../libKitsuneCommon/include/libKitsuneCommon

LIBS += -L../libKitsuneChanNetwork -lKitsuneChanNetwork
LIBS += -L../libKitsuneChanNetwork/debug -lKitsuneChanNetwork
LIBS += -L../libKitsuneChanNetwork/release -lKitsuneChanNetwork
INCLUDEPATH += ../libKitsuneChanNetwork/include/libKitsuneChanNetwork

LIBS += -L../libKitsunePersistence -lKitsunePersistence
LIBS += -L../libKitsunePersistence/debug -lKitsunePersistence
LIBS += -L../libKitsunePersistence/release -lKitsunePersistence
INCLUDEPATH += ../libKitsunePersistence/include/libKitsunePersistence

INCLUDEPATH += $$PWD \
            src

SOURCES += main.cpp \
            src/core/clustering/cluster/nodeCluster.cpp \
            src/core/clustering/clusterHandler.cpp \
            src/core/processing/cpu/cpuProcessingUnit.cpp \
            src/core/processing/processingUnit.cpp \
            src/core/processing/processingUnitHandler.cpp \
            src/core/networkManager.cpp \
            src/settings/config.cpp \
            src/core/initializing/networkInitializer.cpp \
            src/control/statusReporter.cpp \
            src/core/initializing/axonInitializer.cpp \
            src/core/initializing/clusterInitilizer.cpp \
            src/kyochanNetwork.cpp \
            tests/proofOfConcept/demoIO.cpp \
            tests/clusterTest.cpp \
            tests/commonTest.cpp \
            tests/messageBufferTest.cpp \
            tests/messageTest.cpp \
            src/core/clustering/clusterQueue.cpp \
            tests/proofOfConcept/demoUserInput.cpp \
            tests/dataBufferTest.cpp \
            tests/storageMemoryTest.cpp \
            tests/processingTest.cpp \
            src/core/clustering/cluster/edgeCluster.cpp \
            tests/structTest.cpp \
            tests/initTest.cpp \
            src/core/clustering/cluster/cluster.cpp \
            src/core/processing/cpu/edgeClusterProcessing.cpp \
            src/core/processing/cpu/nodeClusterProcessing.cpp

HEADERS +=\
            src/common/typedefs.h \
            src/core/clustering/clusterHandler.h \
            src/core/clustering/cluster/nodeCluster.h \
            src/core/networkManager.h \
            src/core/processing/processingUnit.h \
            src/core/processing/cpu/cpuProcessingUnit.h \
            src/core/processing/processingUnitHandler.h \
            src/settings/config.h \
            src/common/enums.h \
            src/common/includes.h \
            common.h \
            src/common/defines.h \
            src/core/initializing/networkInitializer.h \
            src/core/structs/kyochanEdges.h \
            src/core/structs/kyochanNodes.h \
            src/core/structs/clusterMeta.h \
            src/control/statusReporter.h \
            src/core/initializing/axonInitializer.h \
            src/core/initializing/clusterInitilizer.h \
            src/kyochanNetwork.h \
            tests/clusterTest.h \
            tests/commonTest.h \
            tests/messageBufferTest.h \
            tests/messageTest.h \
            src/core/clustering/clusterQueue.h \
            tests/proofOfConcept/demoUserInput.h \
            tests/proofOfConcept/demoIO.h \
            tests/dataBufferTest.h \
            tests/storageMemoryTest.h \
            src/core/structs/kyochanEmotes.h \
            tests/processingTest.h \
            src/core/clustering/cluster/edgeCluster.h \
            tests/structTest.h \
            tests/initTest.h \
            src/core/structs/messageContainer.h \
            src/core/clustering/cluster/cluster.h \
            src/core/processing/cpu/edgeClusterProcessing.h \
            src/core/processing/cpu/nodeClusterProcessing.h

        # The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0
