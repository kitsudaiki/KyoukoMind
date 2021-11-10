/**
 * @file        init_blossom.h
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

#ifndef KYOUKOMIND_INIT_BLOSSOM_H
#define KYOUKOMIND_INIT_BLOSSOM_H

#include <libKitsunemimiSakuraLang/blossom.h>

class InitBlossom
        : public Kitsunemimi::Sakura::Blossom
{
public:
    InitBlossom();

protected:
    bool runTask(Kitsunemimi::Sakura::BlossomLeaf &blossomLeaf,
                 Kitsunemimi::Sakura::BlossomStatus &status,
                 std::string &errorMessage);
};

#endif // KYOUKOMIND_INIT_BLOSSOM_H
