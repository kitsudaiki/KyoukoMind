QT -= qt core gui

TARGET = KyoukoMind
CONFIG += console
CONFIG += c++14

LIBS += -L../libKitsunemimiHanamiMessaging/src -lKitsunemimiHanamiMessaging
LIBS += -L../libKitsunemimiHanamiMessaging/src/debug -lKitsunemimiHanamiMessaging
LIBS += -L../libKitsunemimiHanamiMessaging/src/release -lKitsunemimiHanamiMessaging
INCLUDEPATH += ../libKitsunemimiHanamiMessaging/include

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

LIBS +=  -lboost_filesystem -lboost_system -lssl -lcrypt -lOpenCL -luuid

INCLUDEPATH += $$PWD \
               src


HEADERS += \
    src/api/blossom_initializing.h \
    src/api/io/ask_blossom.h \
    src/api/io/init_blossom.h \
    src/api/io/learn_blossom.h \
    src/api/save_load/list_save_files_blossom.h \
    src/api/save_load/restore_net_blossom.h \
    src/api/save_load/save_net_blossom.h \
    src/api/save_load/show_save_file_blossom.h \
    src/api/special/special_blossoms.h \
    src/args.h \
    src/common.h \
    src/common/defines.h \
    src/common/enums.h \
    src/common/functions.h \
    src/common/includes.h \
    src/common/structs.h \
    src/common/typedefs.h \
    src/config.h \
    src/core/callbacks.h \
    src/core/initializing/preprocess_cluster_json.h \
    src/core/initializing/struct_validation.h \
    src/core/objects/brick.h \
    src/core/objects/cluster_meta.h \
    src/core/objects/node.h \
    src/core/objects/segment_meta.h \
    src/core/objects/synapses.h \
    src/core/objects/task.h \
    src/core/orchestration/cluster_handler.h \
    src/core/orchestration/cluster_interface.h \
    src/core/orchestration/task_queue.h \
    src/core/processing/cpu/cpu_processing_unit.h \
    src/core/processing/cpu/dynamic_segment/backpropagation.h \
    src/core/processing/cpu/dynamic_segment/create_reduce.h \
    src/core/processing/cpu/dynamic_segment/processing.h \
    src/core/processing/cpu/input_segment/processing.h \
    src/core/processing/cpu/output_segment/backpropagation.h \
    src/core/processing/cpu/output_segment/processing.h \
    src/core/processing/gpu/gpu_processing_uint.h \
    src/core/processing/processing_unit_handler.h \
    src/core/processing/segment_queue.h \
    src/core/initializing/routing_functions.h \
    src/core/storage_io.h \
    src/core/orchestration/network_cluster.h \
    src/core/orchestration/segments/abstract_segment.h \
    src/core/orchestration/segments/dynamic_segment.h \
    src/core/orchestration/segments/input_segment.h \
    src/core/orchestration/segments/output_segment.h \
    src/dev_test.h \
    src/kyouko_root.h

SOURCES += \
    src/api/io/ask_blossom.cpp \
    src/api/io/init_blossom.cpp \
    src/api/io/learn_blossom.cpp \
    src/api/save_load/list_save_files_blossom.cpp \
    src/api/save_load/restore_net_blossom.cpp \
    src/api/save_load/save_net_blossom.cpp \
    src/api/save_load/show_save_file_blossom.cpp \
    src/api/special/special_blossoms.cpp \
    src/core/initializing/preprocess_cluster_json.cpp \
    src/core/initializing/struct_validation.cpp \
    src/core/orchestration/cluster_handler.cpp \
    src/core/orchestration/cluster_interface.cpp \
    src/core/orchestration/task_queue.cpp \
    src/core/processing/cpu/cpu_processing_unit.cpp \
    src/core/processing/gpu/gpu_processing_uint.cpp \
    src/core/processing/processing_unit_handler.cpp \
    src/core/processing/segment_queue.cpp \
    src/core/storage_io.cpp \
    src/core/orchestration/network_cluster.cpp \
    src/core/orchestration/segments/abstract_segment.cpp \
    src/core/orchestration/segments/dynamic_segment.cpp \
    src/core/orchestration/segments/input_segment.cpp \
    src/core/orchestration/segments/output_segment.cpp \
    src/dev_test.cpp \
    src/kyouko_root.cpp

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
