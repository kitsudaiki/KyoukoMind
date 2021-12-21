/**
 * @file        create_learn_task.h
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

#ifndef KYOUKOMIND_CREATELEARNTASK_H
#define KYOUKOMIND_CREATELEARNTASK_H

#include <libKitsunemimiSakuraLang/blossom.h>

class CreateLearnTask
        : public Kitsunemimi::Sakura::Blossom
{
public:
    CreateLearnTask();

protected:
    bool runTask(Kitsunemimi::Sakura::BlossomLeaf &blossomLeaf,
                 const Kitsunemimi::DataMap &,
                 Kitsunemimi::Sakura::BlossomStatus &status,
                 Kitsunemimi::ErrorContainer &error);
};

#endif // KYOUKOMIND_CREATELEARNTASK_H
