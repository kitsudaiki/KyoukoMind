QT -= qt core gui

TARGET = KyoukoMind
CONFIG += console
CONFIG -= app_bundle
CONFIG += c++14

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
    src/settings/config.cpp \
    main.cpp \
    src/core/bricks/brick_methods/buffer_control_methods.cpp \
    src/core/bricks/brick_methods/common_brick_methods.cpp \
    src/core/bricks/brick_handler.cpp \
    src/core/bricks/global_values_handler.cpp \
    src/core/messaging/message_block_buffer.cpp \
    src/core/network_interaction/connection_trigger.cpp \
    src/core/network_interaction/mind_client.cpp \
    src/core/processing/processing_methods/brick_processing_methods.cpp \
    src/core/processing/processing_unit.cpp \
    src/core/processing/processing_unit_handler.cpp \
    src/core/network_manager.cpp \
    src/initializing/axon_initializer.cpp \
    src/initializing/file_parser.cpp \
    src/initializing/network_initializer.cpp \
    src/kyouko_network.cpp \
    tests/core/bricks/brick_methods/buffer_control_methods_test.cpp \
    tests/core/messaging/message_buffer/incoming_buffer_test.cpp \
    tests/core/messaging/message_buffer_test.cpp \
    tests/initializing/init_test.cpp \
    tests/run_unit_tests.cpp


HEADERS += \
    src/common/defines.h \
    src/common/enums.h \
    src/common/includes.h \
    src/common/typedefs.h \
    src/settings/config.h \
    src/common.h \
    src/common/using.h \
    src/core/bricks/brick_objects/brick.h \
    src/core/bricks/brick_objects/edges.h \
    src/core/bricks/brick_objects/node.h \
    src/core/messaging/messageObjects/messages.h \
    src/core/bricks/brick_methods/buffer_control_methods.h \
    src/core/bricks/brick_methods/common_brick_methods.h \
    src/core/bricks/brick_handler.h \
    src/core/bricks/global_values_handler.h \
    src/core/messaging/message_buffer/incoming_buffer.h \
    src/core/messaging/message_buffer/outgoing_buffer.h \
    src/core/messaging/message_objects/content_container.h \
    src/core/messaging/message_objects/message_block.h \
    src/core/messaging/message_block_buffer.h \
    src/core/network_interaction/connection_trigger.h \
    src/core/network_interaction/mind_client.h \
    src/core/processing/processing_methods/brick_processing_methods.h \
    src/core/processing/processing_methods/message_processing_methods.h \
    src/core/processing/processing_unit.h \
    src/core/processing/processing_unit_handler.h \
    src/initializing/axon_initializer.h \
    src/core/network_manager.h \
    src/initializing/file_parser.h \
    src/initializing/network_initializer.h \
    src/kyouko_network.h \
    src/initializing/init_meta_data.h \
    tests/core/bricks/brick_methods/buffer_control_methods_test.h \
    tests/core/messaging/message_buffer/incoming_buffer_test.h \
    tests/core/messaging/message_buffer_test.h \
    tests/initializing/init_test.h \
    tests/run_unit_tests.h
