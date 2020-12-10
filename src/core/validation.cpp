/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#include "validation.h"

#include <core/processing/objects/item_buffer.h>
#include <core/processing/objects/segment.h>

#include <core/processing/objects/global_values.h>
#include <core/processing/objects/brick.h>
#include <core/processing/objects/node.h>
#include <core/processing/objects/synapses.h>
#include <core/processing/objects/edges.h>
#include <core/processing/objects/container_definitions.h>

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
