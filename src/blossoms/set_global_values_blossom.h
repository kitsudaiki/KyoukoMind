/**
 * @file        get_node_brick_ids_blossom.h
 *
 * @author      Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 * @copyright   Apache License Version 2.0
 *
 *      Copyright 2020 Tobias Anker
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

#ifndef SET_GLOBAL_VALUES_BLOSSOM_H
#define SET_GLOBAL_VALUES_BLOSSOM_H

#include <libKitsunemimiSakuraLang/blossom.h>

class SetGlobalValues_Blossom
        : public Kitsunemimi::Sakura::Blossom
{
public:
    SetGlobalValues_Blossom();

protected:
    bool runTask(Kitsunemimi::Sakura::BlossomLeaf &blossomLeaf,
                 std::string &);
};

#endif // SET_GLOBAL_VALUES_BLOSSOM_H
