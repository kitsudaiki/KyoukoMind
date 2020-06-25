#include "validation.h"

#include <core/objects/brick.h>
#include <core/objects/brick_pos.h>
#include <core/objects/container_definitions.h>
#include <core/objects/data_connection.h>
#include <core/objects/edges.h>
#include <core/objects/empty_placeholder.h>
#include <core/objects/neighbor.h>
#include <core/objects/network_segment.h>
#include <core/objects/node.h>
#include <core/objects/synapses.h>

namespace KyoukoMind
{

void
validateStructSizes()
{
    assert(sizeof(Synapse) == 12);
    std::cout<<"sizeof(SynapseSection): "<<sizeof(SynapseSection)<<std::endl;

    assert(sizeof(SynapseSection) == 256);
    return;
}

} // namespace KyoukoMind
