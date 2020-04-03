#ifndef DATA_CONNECTION_H
#define DATA_CONNECTION_H

#include <common.h>

#include <core/objects/node.h>
#include <core/objects/edges.h>
#include <core/objects/synapses.h>

namespace KyoukoMind
{

//==================================================================================================

enum DataConnectionTypes
{
    EDGE_DATA = 0,
    NODE_DATA = 1,
    SYNAPSE_DATA = 2
};

//==================================================================================================

struct DataConnection
{
    uint8_t inUse = 0;
    uint32_t itemSize = 0;
    uint32_t numberOfItems = 0;
    uint32_t numberOfItemBlocks = 0;
    uint32_t numberOfDeletedDynamicItems = 0;
    DataBuffer buffer;

    uint32_t bytePositionOfFirstEmptyBlock = UNINIT_STATE_32;
    uint32_t bytePositionOfLastEmptyBlock = UNINIT_STATE_32;
    uint32_t numberOfEmptyBlocks = 0;

} __attribute__((packed));


// getter
inline Node*
getNodeBlock(DataConnection* data)
{
    return static_cast<Node*>(data->buffer.data);
}

inline SynapseSection*
getSynapseSectionBlock(const DataConnection* data)
{
    return static_cast<SynapseSection*>(data->buffer.data);
}

inline EdgeSection*
getEdgeBlock(const DataConnection* data)
{
    return static_cast<EdgeSection*>(data->buffer.data);
}

}

#endif // DATA_CONNECTION_H
