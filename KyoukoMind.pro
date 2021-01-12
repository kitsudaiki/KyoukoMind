QT -= qt core gui

TARGET = KyoukoMind
CONFIG += console
CONFIG += c++14

LIBS += -L../libKitsunemimiSakuraMessaging/src -lKitsunemimiSakuraMessaging
LIBS += -L../libKitsunemimiSakuraMessaging/src/debug -lKitsunemimiSakuraMessaging
LIBS += -L../libKitsunemimiSakuraMessaging/src/release -lKitsunemimiSakuraMessaging
INCLUDEPATH += ../libKitsunemimiSakuraMessaging/include

LIBS += -L../libKitsunemimiAiParser/src -lKitsunemimiAiParser
LIBS += -L../libKitsunemimiAiParser/src/debug -lKitsunemimiAiParser
LIBS += -L../libKitsunemimiAiParser/src/release -lKitsunemimiAiParser
INCLUDEPATH += ../libKitsunemimiAiParser/include

LIBS += -L../libKitsunemimiArgs/src -lKitsunemimiArgs
LIBS += -L../libKitsunemimiArgs/src/debug -lKitsunemimiArgs
LIBS += -L../libKitsunemimiArgs/src/release -lKitsunemimiArgs
INCLUDEPATH += ../libKitsunemimiArgs/include

LIBS += -L../libKitsunemimiConfig/src -lKitsunemimiConfig
LIBS += -L../libKitsunemimiConfig/src/debug -lKitsunemimiConfig
LIBS += -L../libKitsunemimiConfig/src/release -lKitsunemimiConfig
INCLUDEPATH += ../libKitsunemimiConfig/include

LIBS += -L../libKitsunemimiSakuraNetwork/src -lKitsunemimiSakuraNetwork
LIBS += -L../libKitsunemimiSakuraNetwork/src/debug -lKitsunemimiSakuraNetwork
LIBS += -L../libKitsunemimiSakuraNetwork/src/release -lKitsunemimiSakuraNetwork
INCLUDEPATH += ../libKitsunemimiSakuraNetwork/include

LIBS += -L../libKitsunemimiSakuraLang/src -lKitsunemimiSakuraLang
LIBS += -L../libKitsunemimiSakuraLang/src/debug -lKitsunemimiSakuraLang
LIBS += -L../libKitsunemimiSakuraLang/src/release -lKitsunemimiSakuraLang
INCLUDEPATH += ../libKitsunemimiSakuraLang/include

LIBS += -L../libKitsunemimiOpencl/src -lKitsunemimiOpencl
LIBS += -L../libKitsunemimiOpencl/src/debug -lKitsunemimiOpencl
LIBS += -L../libKitsunemimiOpencl/src/release -lKitsunemimiOpencl
INCLUDEPATH += ../libKitsunemimiOpencl/include

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

LIBS += -L../libKitsunemimiJinja2/src -lKitsunemimiJinja2
LIBS += -L../libKitsunemimiJinja2/src/debug -lKitsunemimiJinja2
LIBS += -L../libKitsunemimiJinja2/src/release -lKitsunemimiJinja2
INCLUDEPATH += ../libKitsunemimiJinja2/include

LIBS += -L../libKitsunemimiJson/src -lKitsunemimiJson
LIBS += -L../libKitsunemimiJson/src/debug -lKitsunemimiJson
LIBS += -L../libKitsunemimiJson/src/release -lKitsunemimiJson
INCLUDEPATH += ../libKitsunemimiJson/include

LIBS += -L../libKitsunemimiPersistence/src -lKitsunemimiPersistence
LIBS += -L../libKitsunemimiPersistence/src/debug -lKitsunemimiPersistence
LIBS += -L../libKitsunemimiPersistence/src/release -lKitsunemimiPersistence
INCLUDEPATH += ../libKitsunemimiPersistence/include

LIBS +=  -lboost_filesystem -lboost_system -lssl -lcrypt -lOpenCL

INCLUDEPATH += $$PWD \
               src

HEADERS += \
    src/common/defines.h \
    src/common/enums.h \
    src/common/includes.h \
    src/common/typedefs.h \
    src/core/objects/brick.h \
    src/core/objects/global_values.h \
    src/core/objects/item_buffer.h \
    src/core/objects/segment.h \
    src/core/processing/cpu/cpu_processing_unit.h \
    src/core/processing/cpu/edge_processing.h \
    src/core/processing/gpu/gpu_processing_uint.h \
    src/core/objects/edges.h \
    src/core/objects/node.h \
    src/core/objects/synapses.h \
    src/core/objects/transfer_objects.h \
    src/core/processing/processing_unit_handler.h \
    src/core/network_manager.h \
    src/import_export/obj_converter.h \
    src/core/validation.h \
    src/initializing/network_initializer.h \
    src/initializing/segment_initializing.h \
    src/args.h \
    src/common.h \
    src/config.h \
    src/kyouko_root.h \
    src/core/callbacks.h \
    src/blossoms/register/register_input_blossom.h \
    src/blossoms/register/register_output_blossom.h \
    src/blossoms/special/learn_blossom.h \
    src/blossoms/metadata/metadata_blossom.h \
    src/core/connection_handler/monitoring_connection_handler.h \
    src/core/connection_handler/client_connection_handler.h \
    src/blossoms/special/special_blossoms.h \
    src/blossoms/global_values/set_global_values_blossom.h \
    src/blossoms/special/freeze_state_blossom.h \
    src/blossoms/snapshot/snapshot_blossom.h \
    src/core/objects/monitoring_container.h \
    src/initializing/blossom_initializing.h \
    src/core/processing/input_output_processing.h \
    src/core/processing/cpu/synapse_processing.h

SOURCES += \
    src/core/objects/brick.cpp \
    src/core/objects/item_buffer.cpp \
    src/core/objects/segment.cpp \
    src/core/processing/cpu/cpu_processing_unit.cpp \
    src/core/processing/processing_unit_handler.cpp \
    src/core/network_manager.cpp \
    src/import_export/obj_converter.cpp \
    src/core/validation.cpp \
    src/initializing/network_initializer.cpp \
    src/initializing/segment_initializing.cpp \
    src/kyouko_root.cpp \
    src/core/processing/gpu/gpu_processing_uint.cpp \
    src/blossoms/register/register_input_blossom.cpp \
    src/blossoms/register/register_output_blossom.cpp \
    src/blossoms/special/learn_blossom.cpp \
    src/blossoms/metadata/metadata_blossom.cpp \
    src/core/connection_handler/monitoring_connection_handler.cpp \
    src/core/connection_handler/client_connection_handler.cpp \
    src/blossoms/global_values/set_global_values_blossom.cpp \
    src/blossoms/special/freeze_state_blossom.cpp \
    src/blossoms/snapshot/snapshot_blossom.cpp \
    src/blossoms/special/special_blossoms.cpp \
    src/core/processing/input_output_processing.cpp

CONFIG(run_tests) {
TARGET = KyoukoMind_Test

HEADERS +=

SOURCES += \
    tests/main_tests.cpp
} else {
SOURCES += \
    src/main.cpp
}

GPU_KERNEL = src/core/processing/gpu/synapse_node_processing.cl

OTHER_FILES +=  \
    $$GPU_KERNEL

gpu_processing.input = GPU_KERNEL
gpu_processing.output = ${QMAKE_FILE_BASE}.h
gpu_processing.commands = xxd -i ${QMAKE_FILE_IN} | sed 's/______KyoukoMind_src_core_processing_gpu_//g' > ${QMAKE_FILE_BASE}.h
gpu_processing.variable_out = HEADERS
gpu_processing.CONFIG += target_predeps no_link

QMAKE_EXTRA_COMPILERS += gpu_processing

