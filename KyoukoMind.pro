QT -= qt core gui

TARGET = KyoukoMind
CONFIG += console
CONFIG += c++17

LIBS += -L../libSagiriArchive/src -lSagiriArchive
LIBS += -L../libSagiriArchive/src/debug -lSagiriArchive
LIBS += -L../libSagiriArchive/src/release -lSagiriArchive
INCLUDEPATH += ../libSagiriArchive/include

LIBS += -L../libAzukiHeart/src -lAzukiHeart
LIBS += -L../libAzukiHeart/src/debug -lAzukiHeart
LIBS += -L../libAzukiHeart/src/release -lAzukiHeart
INCLUDEPATH += ../libAzukiHeart/include

LIBS += -L../libMisakaGuard/src -lMisakaGuard
LIBS += -L../libMisakaGuard/src/debug -lMisakaGuard
LIBS += -L../libMisakaGuard/src/release -lMisakaGuard
INCLUDEPATH += ../libMisakaGuard/include

LIBS += -L../libKitsunemimiHanamiSdk/src -lKitsunemimiHanamiSdk
LIBS += -L../libKitsunemimiHanamiSdk/src/debug -lKitsunemimiHanamiSdk
LIBS += -L../libKitsunemimiHanamiSdk/src/release -lKitsunemimiHanamiSdk
INCLUDEPATH += ../libKitsunemimiHanamiSdk/cpp/include

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

CONFIG(run_tests) {
TARGET = KyoukoMind_Test
DEFINES += USE_DEV_MODE
}

HEADERS += \
    src/api/blossom_initializing.h \
    src/api/v1/cluster/create_cluster.h \
    src/api/v1/cluster/delete_cluster.h \
    src/api/v1/cluster/list_cluster.h \
    src/api/v1/cluster/load_cluster.h \
    src/api/v1/cluster/save_cluster.h \
    src/api/v1/cluster/set_cluster_mode.h \
    src/api/v1/cluster/show_cluster.h \
    src/api/v1/task/create_graph_learn_task.h \
    src/api/v1/task/create_graph_request_task.h \
    src/api/v1/task/create_image_learn_task.h \
    src/api/v1/task/create_image_request_task.h \
    src/api/v1/task/delete_task.h \
    src/api/v1/task/list_task.h \
    src/api/v1/task/show_task.h \
    src/api/v1/template/delete_template.h \
    src/api/v1/template/generate_template.h \
    src/api/v1/template/list_templates.h \
    src/api/v1/template/show_template.h \
    src/api/v1/template/upload_template.h \
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
    src/core/cluster/cluster.h \
    src/core/cluster/cluster_handler.h \
    src/core/cluster/cluster_init.h \
    src/core/cluster/cluster_meta.h \
    src/core/cluster/statemachine_init.h \
    src/core/cluster/states/cycle_finish_state.h \
    src/core/cluster/states/graphs/graph_interpolation_state.h \
    src/core/cluster/states/graphs/graph_learn_forward_state.h \
    src/core/cluster/states/images/image_identify_state.h \
    src/core/cluster/states/images/image_learn_forward_state.h \
    src/core/cluster/states/snapshots/restore_cluster_state.h \
    src/core/cluster/states/snapshots/save_cluster_state.h \
    src/core/cluster/states/task_handle_state.h \
    src/core/cluster/task.h \
    src/core/processing/cpu_processing_unit.h \
    src/core/processing/processing_unit_handler.h \
    src/core/processing/segment_queue.h \
    src/core/routing_functions.h \
    src/core/segments/abstract_segment.h \
    src/core/segments/brick.h \
    src/core/segments/dynamic_segment/backpropagation.h \
    src/core/segments/dynamic_segment/dynamic_segment.h \
    src/core/segments/dynamic_segment/objects.h \
    src/core/segments/dynamic_segment/processing.h \
    src/core/segments/dynamic_segment/reduction.h \
    src/core/segments/input_segment/input_segment.h \
    src/core/segments/input_segment/objects.h \
    src/core/segments/input_segment/processing.h \
    src/core/segments/output_segment/backpropagation.h \
    src/core/segments/output_segment/objects.h \
    src/core/segments/output_segment/output_segment.h \
    src/core/segments/output_segment/processing.h \
    src/core/segments/segment_meta.h \
    src/core/struct_validation.h \
    src/core/templates/dynamic_segments.h \
    src/core/templates/input_segments.h \
    src/core/templates/output_segments.h \
    src/core/templates/template_creator.h \
    src/database/cluster_table.h \
    src/database/template_table.h \
    src/dev_test.h \
    src/kyouko_root.h

SOURCES += \
    src/api/v1/cluster/create_cluster.cpp \
    src/api/v1/cluster/delete_cluster.cpp \
    src/api/v1/cluster/list_cluster.cpp \
    src/api/v1/cluster/load_cluster.cpp \
    src/api/v1/cluster/save_cluster.cpp \
    src/api/v1/cluster/set_cluster_mode.cpp \
    src/api/v1/cluster/show_cluster.cpp \
    src/api/v1/task/create_graph_learn_task.cpp \
    src/api/v1/task/create_graph_request_task.cpp \
    src/api/v1/task/create_image_learn_task.cpp \
    src/api/v1/task/create_image_request_task.cpp \
    src/api/v1/task/delete_task.cpp \
    src/api/v1/task/list_task.cpp \
    src/api/v1/task/show_task.cpp \
    src/api/v1/template/delete_template.cpp \
    src/api/v1/template/generate_template.cpp \
    src/api/v1/template/list_templates.cpp \
    src/api/v1/template/show_template.cpp \
    src/api/v1/template/upload_template.cpp \
    src/callbacks.cpp \
    src/core/cluster/cluster.cpp \
    src/core/cluster/cluster_handler.cpp \
    src/core/cluster/cluster_init.cpp \
    src/core/cluster/statemachine_init.cpp \
    src/core/cluster/states/cycle_finish_state.cpp \
    src/core/cluster/states/graphs/graph_interpolation_state.cpp \
    src/core/cluster/states/graphs/graph_learn_forward_state.cpp \
    src/core/cluster/states/images/image_identify_state.cpp \
    src/core/cluster/states/images/image_learn_forward_state.cpp \
    src/core/cluster/states/snapshots/restore_cluster_state.cpp \
    src/core/cluster/states/snapshots/save_cluster_state.cpp \
    src/core/cluster/states/task_handle_state.cpp \
    src/core/processing/cpu_processing_unit.cpp \
    src/core/processing/processing_unit_handler.cpp \
    src/core/processing/segment_queue.cpp \
    src/core/segments/abstract_segment.cpp \
    src/core/segments/dynamic_segment/dynamic_segment.cpp \
    src/core/segments/input_segment/input_segment.cpp \
    src/core/segments/output_segment/output_segment.cpp \
    src/core/struct_validation.cpp \
    src/database/cluster_table.cpp \
    src/database/template_table.cpp \
    src/dev_test.cpp \
    src/kyouko_root.cpp \
    src/main.cpp

