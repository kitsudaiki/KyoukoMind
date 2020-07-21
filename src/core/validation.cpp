/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#include "validation.h"

#include <core/object_handling/brick.h>
#include <core/processing/internal/objects/container_definitions.h>
#include <core/object_handling/item_buffer.h>
#include <core/processing/internal/objects/edges.h>
#include <core/object_handling/segment.h>
#include <core/processing/internal/objects/node.h>
#include <core/processing/internal/objects/synapses.h>

namespace KyoukoMind
{

void
validateStructSizes()
{
    assert(sizeof(Synapse) == 12);
    std::cout<<"sizeof(SynapseSection): "<<sizeof(SynapseSection)<<std::endl;

    assert(sizeof(SynapseSection) == 256);
    assert(sizeof(EdgeSection) == 4096);

    return;
}

} // namespace KyoukoMind
