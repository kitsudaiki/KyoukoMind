QT -= qt core gui

TARGET = KyoukoMind
CONFIG += console
CONFIG += c++17

LIBS += -L../libKitsunemimiHanamiPredefinitions/src -lKitsunemimiHanamiPredefinitions
LIBS += -L../libKitsunemimiHanamiPredefinitions/src/debug -lKitsunemimiHanamiPredefinitions
LIBS += -L../libKitsunemimiHanamiPredefinitions/src/release -lKitsunemimiHanamiPredefinitions
INCLUDEPATH += ../libKitsunemimiHanamiPredefinitions/include

LIBS += -L../libKitsunemimiHanamiMessaging/src -lKitsunemimiHanamiMessaging
LIBS += -L../libKitsunemimiHanamiMessaging/src/debug -lKitsunemimiHanamiMessaging
LIBS += -L../libKitsunemimiHanamiMessaging/src/release -lKitsunemimiHanamiMessaging
INCLUDEPATH += ../libKitsunemimiHanamiMessaging/include

LIBS += -L../libKitsunemimiHanamiEndpoints/src -lKitsunemimiHanamiEndpoints
LIBS += -L../libKitsunemimiHanamiEndpoints/src/debug -lKitsunemimiHanamiEndpoints
LIBS += -L../libKitsunemimiHanamiEndpoints/src/release -lKitsunemimiHanamiEndpoints
INCLUDEPATH += ../libKitsunemimiHanamiEndpoints/include

LIBS += -L../libKitsunemimiHanamiDatabase/src -lKitsunemimiHanamiDatabase
LIBS += -L../libKitsunemimiHanamiDatabase/src/debug -lKitsunemimiHanamiDatabase
LIBS += -L../libKitsunemimiHanamiDatabase/src/release -lKitsunemimiHanamiDatabase
INCLUDEPATH += ../libKitsunemimiHanamiDatabase/include

LIBS += -L../libKitsunemimiHanamiCommon/src -lKitsunemimiHanamiCommon
LIBS += -L../libKitsunemimiHanamiCommon/src/debug -lKitsunemimiHanamiCommon
LIBS += -L../libKitsunemimiHanamiCommon/src/release -lKitsunemimiHanamiCommon
INCLUDEPATH += ../libKitsunemimiHanamiCommon/include

LIBS += -L../libKitsunemimiArgs/src -lKitsunemimiArgs
LIBS += -L../libKitsunemimiArgs/src/debug -lKitsunemimiArgs
LIBS += -L../libKitsunemimiArgs/src/release -lKitsunemimiArgs
INCLUDEPATH += ../libKitsunemimiArgs/include

LIBS += -L../libKitsunemimiConfig/src -lKitsunemimiConfig
LIBS += -L../libKitsunemimiConfig/src/debug -lKitsunemimiConfig
LIBS += -L../libKitsunemimiConfig/src/release -lKitsunemimiConfig
INCLUDEPATH += ../libKitsunemimiConfig/include

LIBS += -L../libKitsunemimiSakuraDatabase/src -lKitsunemimiSakuraDatabase
LIBS += -L../libKitsunemimiSakuraDatabase/src/debug -lKitsunemimiSakuraDatabase
LIBS += -L../libKitsunemimiSakuraDatabase/src/release -lKitsunemimiSakuraDatabase
INCLUDEPATH += ../libKitsunemimiSakuraDatabase/include

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

LIBS += -L../libKitsunemimiSqlite/src -lKitsunemimiSqlite
LIBS += -L../libKitsunemimiSqlite/src/debug -lKitsunemimiSqlite
LIBS += -L../libKitsunemimiSqlite/src/release -lKitsunemimiSqlite
INCLUDEPATH += ../libKitsunemimiSqlite/include

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

LIBS += -L../libKitsunemimiJwt/src -lKitsunemimiJwt
LIBS += -L../libKitsunemimiJwt/src/debug -lKitsunemimiJwt
LIBS += -L../libKitsunemimiJwti/src/release -lKitsunemimiJwt
INCLUDEPATH += ../libKitsunemimiJwt/include

LIBS += -L../libKitsunemimiCrypto/src -lKitsunemimiCrypto
LIBS += -L../libKitsunemimiCrypto/src/debug -lKitsunemimiCrypto
LIBS += -L../libKitsunemimiCrypto/src/release -lKitsunemimiCrypto
INCLUDEPATH += ../libKitsunemimiCrypto/include

LIBS += -lcryptopp -lssl -lsqlite3 -luuid -lcrypto -lOpenCL

INCLUDEPATH += $$PWD \
               src


HEADERS += \
    src/api/blossom_initializing.h \
    src/api/v1/cluster/create_cluster.h \
    src/api/v1/cluster/delete_cluster.h \
    src/api/v1/cluster/list_cluster.h \
    src/api/v1/cluster/show_cluster.h \
    src/api/v1/task/create_learn_task.h \
    src/api/v1/task/create_request_task.h \
    src/api/v1/task/delete_task.h \
    src/api/v1/task/list_task.h \
    src/api/v1/task/show_task.h \
    src/args.h \
    src/callbacks.h \
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
    src/database/cluster_table.h \
    src/dev_test.h \
    src/kyouko_root.h

SOURCES += \
    src/api/v1/cluster/create_cluster.cpp \
    src/api/v1/cluster/delete_cluster.cpp \
    src/api/v1/cluster/list_cluster.cpp \
    src/api/v1/cluster/show_cluster.cpp \
    src/api/v1/task/create_learn_task.cpp \
    src/api/v1/task/create_request_task.cpp \
    src/api/v1/task/delete_task.cpp \
    src/api/v1/task/list_task.cpp \
    src/api/v1/task/show_task.cpp \
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
    src/database/cluster_table.cpp \
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
