/**
 *  @file    init_meta_data.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef INIT_META_DATA_H
#define INIT_META_DATA_H

#include <common.h>

namespace KyoukoMind
{

struct Brick;

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
    BrickID brickId = 0;
    Brick* brick = nullptr;
    uint32_t numberOfAxons = 0;
};

} // namespace KyoukoMind

#endif // INIT_META_DATA_H
