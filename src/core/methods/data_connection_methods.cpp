/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#include "data_connection_methods.h"

namespace KyoukoMind
{

//==================================================================================================

/**
 * initialize the node-list of the brick
 *
 * @return false if nodes are already initialized, esle true
*/
bool
initDataBlocks(ItemBuffer &data,
               const uint64_t numberOfItems,
               const uint32_t itemSize)
{
    assert(itemSize != 0);
    assert(data.numberOfItems == 0);

    // update meta-data of the brick
    data.itemSize = itemSize;
    data.numberOfItems = numberOfItems;
    const uint64_t requiredNumberOfBlocks = ((numberOfItems * itemSize)
                                             / data.buffer.blockSize) + 1;

    // allocate blocks in buffer
    //data.buffer = DataBuffer(requiredNumberOfBlocks);
    Kitsunemimi::allocateBlocks_DataBuffer(data.buffer, requiredNumberOfBlocks);
    data.buffer.bufferPosition = numberOfItems * itemSize;

    return true;
}

//==================================================================================================

}
