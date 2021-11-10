/**
 * @file        special_blossoms.h
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

#ifndef KYOUKOMIND_SPECIAL_BLOSSOMS_H
#define KYOUKOMIND_SPECIAL_BLOSSOMS_H

#include <libKitsunemimiSakuraLang/blossom.h>

//==================================================================================================
// AssertBlossom
//==================================================================================================
class AssertBlossom
        : public Kitsunemimi::Sakura::Blossom
{

public:
    AssertBlossom();

protected:
    bool runTask(Kitsunemimi::Sakura::BlossomLeaf &blossomLeaf,
                 uint64_t &status,
                 std::string &errorMessage);
};

//==================================================================================================
// ItemUpdateBlossom
//==================================================================================================
class ItemUpdateBlossom
        : public Kitsunemimi::Sakura::Blossom
{

public:
    ItemUpdateBlossom();

protected:
    bool runTask(Kitsunemimi::Sakura::BlossomLeaf &blossomLeaf,
                 uint64_t &status,
                 std::string &errorMessage);
};

//==================================================================================================
// PrintBlossom
//==================================================================================================
class PrintBlossom
        : public Kitsunemimi::Sakura::Blossom
{

public:
    PrintBlossom();

protected:
    bool runTask(Kitsunemimi::Sakura::BlossomLeaf &blossomLeaf,
                 uint64_t &status,
                 std::string &errorMessage);
};

#endif // KYOUKOMIND_SPECIAL_BLOSSOMS_H
