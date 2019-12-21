QT -= qt core gui

TARGET = KyoukoMind
CONFIG += console
CONFIG -= app_bundle
CONFIG += c++14

LIBS += -L../libKitsunemimiCommon/src -lKitsunemimiCommon
LIBS += -L../libKitsunemimiCommon/src/debug -lKitsunemimiCommon
LIBS += -L../libKitsunemimiCommon/src/release -lKitsunemimiCommon
INCLUDEPATH += ../libKitsunemimiCommon/include

LIBS += -L../libKitsunemimiJson/src -lKitsunemimiJson
LIBS += -L../libKitsunemimiJson/src/debug -lKitsunemimiJson
LIBS += -L../libKitsunemimiJson/src/release -lKitsunemimiJson
INCLUDEPATH += ../libKitsunemimiJson/include

#LIBS += -L../libKitsunemimiNetwork/src -lKitsunemimiNetwork
#LIBS += -L../libKitsunemimiNetwork/src/debug -lKitsunemimiNetwork
#LIBS += -L../libKitsunemimiNetwork/src/release -lKitsunemimiNetwork
#INCLUDEPATH += ../libKitsunemimiNetwork/include

#LIBS += -L../libKitsunemimiKyoukoCommon -lKitsunemimiKyoukoCommon
#LIBS += -L../libKitsunemimiKyoukoCommon/debug -lKitsunemimiKyoukoCommon
#LIBS += -L../libKitsunemimiKyoukoCommon/release -lKitsunemimiKyoukoCommon
#INCLUDEPATH += ../libKitsunemimiKyoukoCommon/include

LIBS +=  -lboost_filesystem -lboost_system -lboost_program_options  -lssl -lcrypt

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
