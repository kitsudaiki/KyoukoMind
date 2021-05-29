/**
 * @file        enums.h
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

#ifndef ENUMS_H
#define ENUMS_H

enum ThreadTask {
    NOTHING = 0,
    OUTPUT_LEARN = 1,
    NODE_PROCESSING = 2,
    UPDATE_CORE = 3,
    CORE_PROCESSING = 4,
    OUTPUT_PROCESSING = 5,
};

#endif // ENUMS_H
