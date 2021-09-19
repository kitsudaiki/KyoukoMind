/**
 * @file        functions.h
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

#ifndef KYOUKOMIND_FUNCTIONS_H
#define KYOUKOMIND_FUNCTIONS_H

#include <stdint.h>
#include "structs.h"
#include <uuid/uuid.h>

struct kuuid
{
    char uuid[UUID_STR_LEN];
    uint8_t padding[3];

    const std::string toString() const {
        return std::string(uuid);
    }

    // total size: 40 Bytes
};

/**
 * @brief generate a new uuid with external library
 *
 * @return new uuid
 */
inline const kuuid
generateUuid()
{
    uuid_t binaryUuid;
    kuuid result;

    uuid_generate_random(binaryUuid);
    uuid_unparse_lower(binaryUuid, result.uuid);

    return result;
}

#endif // KYOUKOMIND_FUNCTIONS_H
