/**
 * @file        blossom_initializing.h
 *
 * @author      Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 * @copyright   Apache License Version 2.0
 *
 *      Copyright 2019 Tobias Anker
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#ifndef KYOUKOMIND_BLOSSOM_INITIALIZING_H
#define KYOUKOMIND_BLOSSOM_INITIALIZING_H

#include <common.h>

#include <libKitsunemimiSakuraLang/sakura_lang_interface.h>
#include <libKitsunemimiConfig/config_handler.h>
#include <libKitsunemimiCommon/common_methods/file_methods.h>
#include <libKitsunemimiCommon/files/text_file.h>
#include <libKitsunemimiCommon/logger.h>

#include <libKitsunemimiHanamiEndpoints/endpoint.h>

#include <api/v1/cluster/create_cluster.h>
#include <api/v1/cluster/show_cluster.h>
#include <api/v1/cluster/list_cluster.h>
#include <api/v1/cluster/delete_cluster.h>
#include <api/v1/cluster/save_cluster.h>
#include <api/v1/cluster/load_cluster.h>

#include <api/v1/template/create_template.h>
#include <api/v1/template/delete_template.h>
#include <api/v1/template/list_templates.h>
#include <api/v1/template/show_template.h>

#include <api/v1/task/create_image_learn_task.h>
#include <api/v1/task/create_image_request_task.h>
#include <api/v1/task/create_graph_learn_task.h>
#include <api/v1/task/create_graph_request_task.h>
#include <api/v1/task/show_task.h>
#include <api/v1/task/list_task.h>
#include <api/v1/task/delete_task.h>

using Kitsunemimi::Sakura::SakuraLangInterface;

/**
 * @brief initClusterBlossoms
 */
void
initClusterBlossoms()
{
    Kitsunemimi::Hanami::Endpoint* endpoints = Kitsunemimi::Hanami::Endpoint::getInstance();
    SakuraLangInterface* interface = SakuraLangInterface::getInstance();
    const std::string group = "cluster";

    assert(interface->addBlossom(group, "create", new CreateCluster()));
    endpoints->addEndpoint("v1/cluster",
                           Kitsunemimi::Hanami::POST_TYPE,
                           Kitsunemimi::Hanami::BLOSSOM_TYPE,
                           group,
                           "create");

    assert(interface->addBlossom(group, "show", new ShowCluster()));
    endpoints->addEndpoint("v1/cluster",
                           Kitsunemimi::Hanami::GET_TYPE,
                           Kitsunemimi::Hanami::BLOSSOM_TYPE,
                           group,
                           "show");

    assert(interface->addBlossom(group, "list", new ListCluster()));
    endpoints->addEndpoint("v1/cluster/all",
                           Kitsunemimi::Hanami::GET_TYPE,
                           Kitsunemimi::Hanami::BLOSSOM_TYPE,
                           group,
                           "list");

    assert(interface->addBlossom(group, "delete", new DeleteCluster()));
    endpoints->addEndpoint("v1/cluster",
                           Kitsunemimi::Hanami::DELETE_TYPE,
                           Kitsunemimi::Hanami::BLOSSOM_TYPE,
                           group,
                           "delete");

    assert(interface->addBlossom(group, "save", new SaveCluster()));
    endpoints->addEndpoint("v1/cluster/save",
                           Kitsunemimi::Hanami::POST_TYPE,
                           Kitsunemimi::Hanami::BLOSSOM_TYPE,
                           group,
                           "save");

    assert(interface->addBlossom(group, "load", new LoadCluster()));
    endpoints->addEndpoint("v1/cluster/load",
                           Kitsunemimi::Hanami::POST_TYPE,
                           Kitsunemimi::Hanami::BLOSSOM_TYPE,
                           group,
                           "load");
}

/**
 * @brief initTemplateBlossoms
 */
void
initTemplateBlossoms()
{
    Kitsunemimi::Hanami::Endpoint* endpoints = Kitsunemimi::Hanami::Endpoint::getInstance();
    SakuraLangInterface* interface = SakuraLangInterface::getInstance();
    const std::string group = "template";

    assert(interface->addBlossom(group, "create", new CreateTemplate()));
    endpoints->addEndpoint("v1/template",
                           Kitsunemimi::Hanami::POST_TYPE,
                           Kitsunemimi::Hanami::BLOSSOM_TYPE,
                           group,
                           "create");

    assert(interface->addBlossom(group, "show", new ShowTemplate()));
    endpoints->addEndpoint("v1/template",
                           Kitsunemimi::Hanami::GET_TYPE,
                           Kitsunemimi::Hanami::BLOSSOM_TYPE,
                           group,
                           "show");

    assert(interface->addBlossom(group, "list", new ListTemplates()));
    endpoints->addEndpoint("v1/template/all",
                           Kitsunemimi::Hanami::GET_TYPE,
                           Kitsunemimi::Hanami::BLOSSOM_TYPE,
                           group,
                           "list");

    assert(interface->addBlossom(group, "delete", new DeleteTemplate()));
    endpoints->addEndpoint("v1/template",
                           Kitsunemimi::Hanami::DELETE_TYPE,
                           Kitsunemimi::Hanami::BLOSSOM_TYPE,
                           group,
                           "delete");
}

/**
 * @brief initTaskBlossoms
 */
void
initTaskBlossoms()
{
    Kitsunemimi::Hanami::Endpoint* endpoints = Kitsunemimi::Hanami::Endpoint::getInstance();
    SakuraLangInterface* interface = SakuraLangInterface::getInstance();
    const std::string group = "task";

    assert(interface->addBlossom(group, "create_image_learn", new CreateImageLearnTask()));
    endpoints->addEndpoint("v1/task/image/learn",
                           Kitsunemimi::Hanami::POST_TYPE,
                           Kitsunemimi::Hanami::BLOSSOM_TYPE,
                           group,
                           "create_image_learn");

    assert(interface->addBlossom(group, "create_image_request", new CreateImageRequestTask()));
    endpoints->addEndpoint("v1/task/image/request",
                           Kitsunemimi::Hanami::POST_TYPE,
                           Kitsunemimi::Hanami::BLOSSOM_TYPE,
                           group,
                           "create_image_request");

    assert(interface->addBlossom(group, "create_graph_learn", new CreateGraphLearnTask()));
    endpoints->addEndpoint("v1/task/graph/learn",
                           Kitsunemimi::Hanami::POST_TYPE,
                           Kitsunemimi::Hanami::BLOSSOM_TYPE,
                           group,
                           "create_graph_learn");

    assert(interface->addBlossom(group, "create_graph_request", new CreateGraphRequestTask()));
    endpoints->addEndpoint("v1/task/graph/request",
                           Kitsunemimi::Hanami::POST_TYPE,
                           Kitsunemimi::Hanami::BLOSSOM_TYPE,
                           group,
                           "create_graph_request");

    assert(interface->addBlossom(group, "show", new ShowTask()));
    endpoints->addEndpoint("v1/task",
                           Kitsunemimi::Hanami::GET_TYPE,
                           Kitsunemimi::Hanami::BLOSSOM_TYPE,
                           group,
                           "show");

    assert(interface->addBlossom(group, "list", new ListTask()));
    endpoints->addEndpoint("v1/task/all",
                           Kitsunemimi::Hanami::GET_TYPE,
                           Kitsunemimi::Hanami::BLOSSOM_TYPE,
                           group,
                           "list");

    assert(interface->addBlossom(group, "delete", new DeleteTask()));
    endpoints->addEndpoint("v1/task",
                           Kitsunemimi::Hanami::DELETE_TYPE,
                           Kitsunemimi::Hanami::BLOSSOM_TYPE,
                           group,
                           "delete");
}

/**
 * @brief initBlossoms
 */
void
initBlossoms()
{
    initClusterBlossoms();
    initTemplateBlossoms();
    initTaskBlossoms();
}

#endif // KYOUKOMIND_BLOSSOM_INITIALIZING_H
