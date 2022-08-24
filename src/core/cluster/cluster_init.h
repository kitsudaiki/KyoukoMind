/**
 * @file        cluster_init.h
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

#ifndef KYOUKOMIND_CLUSTERINIT_H
#define KYOUKOMIND_CLUSTERINIT_H

#include <common.h>

class InputSegment;
class OutputSegment;
class AbstractSegment;

class Cluster;

bool reinitPointer(Cluster* cluster, const std::string &uuid);

bool initNewCluster(Cluster* cluster,
                    const JsonItem &clusterTemplate,
                    const std::map<std::string, JsonItem> &segmentTemplates,
                    const std::string &uuid);

AbstractSegment* addInputSegment(Cluster* cluster,
                                 const std::string &name,
                                 const JsonItem &clusterTemplatePart);
AbstractSegment* addOutputSegment(Cluster* cluster,
                                  const std::string &name,
                                  const JsonItem &clusterTemplatePart);
AbstractSegment* addDynamicSegment(Cluster* cluster,
                                   const std::string &name,
                                   const JsonItem &clusterTemplatePart,
                                   const JsonItem &segmentTemplate);

Position convertPosition(const JsonItem &parsedContent);

#endif // KYOUKOMIND_CLUSTERINIT_H
