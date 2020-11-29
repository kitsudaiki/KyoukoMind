/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#ifndef INIT_META_DATA_H
#define INIT_META_DATA_H

#include <common.h>

class Brick;

enum BrickTypes
{
    UNDEFINED_BRICK = 0,
    EMPTY_BRICK = 1,
    EDGE_BRICK = 2,
    NODE_BRICK = 3
};


/**
 * @brief The InitMetaDataEntry struct
 */
struct InitMetaDataEntry
{
    uint8_t type = EMPTY_BRICK;
    Brick* brick = nullptr;
};

#endif // INIT_META_DATA_H
