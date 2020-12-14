/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#include "validation.h"

#include <core/objects/item_buffer.h>
#include <core/objects/segment.h>

#include <core/objects/global_values.h>
#include <core/objects/brick.h>
#include <core/objects/node.h>
#include <core/objects/synapses.h>
#include <core/objects/edges.h>

void
validateStructSizes()
{
    assert(sizeof(Brick) < 4096);

    assert(sizeof(Synapse) == 16);
    std::cout<<"sizeof(SynapseSection): "<<sizeof(SynapseSection)<<std::endl;

    assert(sizeof(SynapseSection) == 256);
    assert(sizeof(GlobalValues) == 256);
    assert(sizeof(EdgeSection) == 4096);

    return;
}
