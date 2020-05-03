QT -= qt core gui

TARGET = KyoukoMind
CONFIG += console
CONFIG += c++14

LIBS += -L../libKitsunemimiArgs/src -lKitsunemimiArgs
LIBS += -L../libKitsunemimiArgs/src/debug -lKitsunemimiArgs
LIBS += -L../libKitsunemimiArgs/src/release -lKitsunemimiArgs
INCLUDEPATH += ../libKitsunemimiArgs/include

LIBS += -L../libKitsunemimiConfig/src -lKitsunemimiConfig
LIBS += -L../libKitsunemimiConfig/src/debug -lKitsunemimiConfig
LIBS += -L../libKitsunemimiConfig/src/release -lKitsunemimiConfig
INCLUDEPATH += ../libKitsunemimiConfig/include

INCLUDEPATH += ../libKitsunemimiKyoukoCommon/include

LIBS += -L../libKitsunemimiProjectNetwork/src -lKitsunemimiProjectNetwork
LIBS += -L../libKitsunemimiProjectNetwork/src/debug -lKitsunemimiProjectNetwork
LIBS += -L../libKitsunemimiProjectNetwork/src/release -lKitsunemimiProjectNetwork
INCLUDEPATH += ../libKitsunemimiProjectNetwork/include

LIBS += -L../libKitsunemimiNetwork/src -lKitsunemimiNetwork
LIBS += -L../libKitsunemimiNetwork/src/debug -lKitsunemimiNetwork
LIBS += -L../libKitsunemimiNetwork/src/release -lKitsunemimiNetwork
INCLUDEPATH += ../libKitsunemimiNetwork/include

LIBS += -L../libKitsunemimiObj/src -lKitsunemimiObj
LIBS += -L../libKitsunemimiObj/src/debug -lKitsunemimiObj
LIBS += -L../libKitsunemimiObj/src/release -lKitsunemimiObj
INCLUDEPATH += ../libKitsunemimiObj/include

LIBS += -L../libKitsunemimiCommon/src -lKitsunemimiCommon
LIBS += -L../libKitsunemimiCommon/src/debug -lKitsunemimiCommon
LIBS += -L../libKitsunemimiCommon/src/release -lKitsunemimiCommon
INCLUDEPATH += ../libKitsunemimiCommon/include

LIBS += -L../libKitsunemimiIni/src -lKitsunemimiIni
LIBS += -L../libKitsunemimiIni/src/debug -lKitsunemimiIni
LIBS += -L../libKitsunemimiIni/src/release -lKitsunemimiIni
INCLUDEPATH += ../libKitsunemimiIni/include

LIBS += -L../libKitsunemimiJson/src -lKitsunemimiJson
LIBS += -L../libKitsunemimiJson/src/debug -lKitsunemimiJson
LIBS += -L../libKitsunemimiJson/src/release -lKitsunemimiJson
INCLUDEPATH += ../libKitsunemimiJson/include

LIBS += -L../libKitsunemimiPersistence/src -lKitsunemimiPersistence
LIBS += -L../libKitsunemimiPersistence/src/debug -lKitsunemimiPersistence
LIBS += -L../libKitsunemimiPersistence/src/release -lKitsunemimiPersistence
INCLUDEPATH += ../libKitsunemimiPersistence/include

LIBS +=  -lboost_filesystem -lboost_system -lssl -lcrypt

INCLUDEPATH += $$PWD \
               src

HEADERS += \
    src/common/defines.h \
    src/common/enums.h \
    src/common/includes.h \
    src/common/typedefs.h \
    src/core/objects/brick.h \
    src/core/objects/brick_pos.h \
    src/core/objects/container_definitions.h \
    src/core/objects/data_connection.h \
    src/core/objects/edges.h \
    src/core/objects/empty_placeholder.h \
    src/core/objects/neighbor.h \
    src/core/objects/node.h \
    src/core/objects/synapses.h \
    src/core/processing/processing_unit.h \
    src/core/processing/processing_unit_handler.h \
    src/core/methods/brick_initializing_methods.h \
    src/core/methods/brick_item_methods.h \
    src/core/processing/methods/brick_processing.h \
    src/core/global_values_handler.h \
    src/core/network_manager.h \
    src/initializing/axon_initializer.h \
    src/initializing/file_parser.h \
    src/initializing/init_meta_data.h \
    src/initializing/network_initializer.h \
    src/common.h \
    src/root_object.h \
    src/args.h \
    src/config.h \
    src/io/network_callbacks.h \
    src/core/processing/methods/edge_container_processing.h \
    src/core/processing/methods/synapse_container_processing.h \
    src/core/methods/edge_methods.h \
    src/core/methods/neighbor_methods.h \
    src/core/methods/synapse_methods.h \
    src/io/client_processing.h \
    src/io/control_processing.h \
    src/core/obj_converter.h \
    src/core/objects/network_segment.h \
    src/core/methods/network_segment_methods.h \
    src/core/methods/data_connection_methods.h \
    src/core/brick_queue.h \
    src/core/processing/methods/node_processing.h

SOURCES += \
    src/core/processing/processing_unit.cpp \
    src/core/processing/processing_unit_handler.cpp \
    src/core/methods/brick_initializing_methods.cpp \
    src/core/processing/methods/brick_processing.cpp \
    src/core/global_values_handler.cpp \
    src/core/network_manager.cpp \
    src/initializing/axon_initializer.cpp \
    src/initializing/file_parser.cpp \
    src/initializing/network_initializer.cpp \
    src/root_object.cpp \
    src/core/obj_converter.cpp \
    src/core/methods/network_segment_methods.cpp \
    src/core/methods/data_connection_methods.cpp \
    src/core/brick_queue.cpp


CONFIG(run_tests) {
TARGET = KyoukoMind_Test

HEADERS +=

SOURCES += \
    tests/main_tests.cpp
} else {
SOURCES += \
    src/main.cpp
}




