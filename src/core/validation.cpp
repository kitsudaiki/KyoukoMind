/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#include "validation.h"

#include <core/object_handling/brick.h>
#include <core/object_handling/item_buffer.h>
#include <core/object_handling/segment.h>
#include <core/object_handling/global_values.h>

#include <core/processing/objects/node.h>
#include <core/processing/objects/synapses.h>
#include <core/processing/objects/edges.h>
#include <core/processing/objects/container_definitions.h>

namespace KyoukoMind
{

void
validateStructSizes()
{
    assert(sizeof(Brick) < 4096);

    assert(sizeof(Synapse) == 12);
    std::cout<<"sizeof(SynapseSection): "<<sizeof(SynapseSection)<<std::endl;

    assert(sizeof(SynapseSection) == 256);
    assert(sizeof(GlobalValues) == 256);
    assert(sizeof(EdgeSection) == 4096);

    return;
}

} // namespace KyoukoMind
