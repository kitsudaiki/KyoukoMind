QT -= qt core gui

CONFcIG += c++17

TARGET = KyoukoMind
CONFIG += console
CONFIG -= app_bundle

TEMPLATE = app

LIBS += -L../libKitsuneCommon/src -lKitsuneCommon
LIBS += -L../libKitsuneCommon/src/debug -lKitsuneCommon
LIBS += -L../libKitsuneCommon/src/release -lKitsuneCommon
INCLUDEPATH += ../libKitsuneCommon/include/libKitsuneCommon

LIBS += -L../libKitsuneJson/src -lKitsuneJson
LIBS += -L../libKitsuneJson/src/debug -lKitsuneJson
LIBS += -L../libKitsuneJson/src/release -lKitsuneJson
INCLUDEPATH += ../libKitsuneJson/include/libKitsuneJson

LIBS += -L../libKitsuneNetwork/src -lKitsuneNetwork
LIBS += -L../libKitsuneNetwork/src/debug -lKitsuneNetwork
LIBS += -L../libKitsuneNetwork/src/release -lKitsuneNetwork
INCLUDEPATH += ../libKitsuneNetwork/include/libKitsuneNetwork

LIBS += -L../libKitsuneChanCommon -lKitsuneChanCommon
LIBS += -L../libKitsuneChanCommon/debug -lKitsuneChanCommon
LIBS += -L../libKitsuneChanCommon/release -lKitsuneChanCommon
INCLUDEPATH += ../libKitsuneChanCommon/include/libKitsuneChanCommon


INCLUDEPATH += $$PWD \
            src

SOURCES += \
    src/core/processing/processingUnit.cpp \
    src/core/processing/processingUnitHandler.cpp \
    src/core/networkManager.cpp \
    src/settings/config.cpp \
    main.cpp \
    src/core/bricks/globalValuesHandler.cpp \
    tests/runUnitTests.cpp \
    src/core/networkInteraction/connectionTrigger.cpp \
    src/initializing/axonInitializer.cpp \
    src/initializing/networkInitializer.cpp \
    src/core/bricks/brickMethods/bufferControlMethods.cpp \
    tests/initializing/initTest.cpp \
    tests/core/bricks/brickMethods/bufferControlMethodsTest.cpp \
    src/core/bricks/brickMethods/commonBrickMethods.cpp \
    src/core/bricks/brickHandler.cpp \
    src/core/processing/processingMethods/brickProcessingMethods.cpp \
    src/initializing/fileParser.cpp \
    tests/core/messaging/messageBufferTest.cpp \
    tests/core/messaging/messageMarker/incomingBufferTest.cpp \
    src/core/messaging/messageBlockBuffer.cpp \
    src/kyoukoNetwork.cpp \
    src/core/networkInteraction/mindClient.cpp


HEADERS += \
    src/common/defines.h \
    src/common/enums.h \
    src/common/includes.h \
    src/common/typedefs.h \
    src/core/processing/processingUnit.h \
    src/core/processing/processingUnitHandler.h \
    src/core/networkManager.h \
    src/settings/config.h \
    src/common.h \
    src/core/bricks/globalValuesHandler.h \
    tests/runUnitTests.h \
    src/common/methods.h \
    src/common/using.h \
    src/core/networkInteraction/connectionTrigger.h \
    src/initializing/axonInitializer.h \
    src/initializing/networkInitializer.h \
    src/initializing/initMetaData.h \
    src/core/bricks/brickMethods/bufferControlMethods.h \
    src/core/processing/processingMethods/messageProcessingMethods.h \
    tests/initializing/initTest.h \
    tests/core/bricks/brickMethods/bufferControlMethodsTest.h \
    src/core/bricks/brickMethods/commonBrickMethods.h \
    src/core/bricks/brickObjects/brick.h \
    src/core/bricks/brickHandler.h \
    src/core/processing/processingMethods/brickProcessingMethods.h \
    src/core/bricks/brickObjects/edges.h \
    src/core/bricks/brickObjects/node.h \
    src/initializing/fileParser.h \
    src/core/messaging/messageObjects/messages.h \
    src/core/messaging/messageObjects/messageBlock.h \
    src/core/messaging/messageMarker/outgoingBuffer.h \
    src/core/messaging/messageMarker/incomingBuffer.h \
    tests/core/messaging/messageBufferTest.h \
    tests/core/messaging/messageMarker/incomingBufferTest.h \
    src/core/messaging/messageObjects/contentContainer.h \
    src/core/messaging/messageBlockBuffer.h \
    src/kyoukoNetwork.h \
    src/core/networkInteraction/mindClient.h
