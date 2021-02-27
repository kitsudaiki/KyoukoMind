#ifndef SECTION_HANDLING_H
#define SECTION_HANDLING_H

#include <common.h>

#include <kyouko_root.h>
#include <core/objects/brick.h>
#include <core/objects/node.h>
#include <core/objects/segment.h>
#include <core/objects/synapses.h>
#include <core/objects/global_values.h>

#include <libKitsunemimiCommon/buffer/item_buffer.h>

/**
 * @brief findNewSectioin
 * @param synapseSections
 * @param oldSectionId
 * @return
 */
inline bool
findNewSectioin(SynapseSection* synapseSections,
                const uint64_t oldSectionId,
                const uint32_t sourceNodeBrickId)
{
    Brick** nodeBricks = KyoukoRoot::m_segment->nodeBricks;

    // check if section is new and schould be created
    SynapseSection newSection;
    newSection.randomPos = rand() % 1024;

    const uint64_t pos = KyoukoRoot::m_segment->synapses.addNewItem(newSection);
    if(pos == UNINIT_STATE_64)
    {
        // not allowed in while testing
        // TODO: remove assert again
        assert(false);
        return false;
    }

    synapseSections[pos].prev = oldSectionId;
    synapseSections[oldSectionId].next = pos;

    Brick* sourceBrick = nodeBricks[sourceNodeBrickId];
    synapseSections[pos].nodeBrickId = sourceBrick->possibleTargetNodeBrickIds[rand() % 1000];
    assert(synapseSections[pos].nodeBrickId != UNINIT_STATE_32);

    return true;
}

/**
 * @brief removeSection
 * @param synapseSections
 * @param pos
 */
inline void
removeSection(SynapseSection* synapseSections, const uint64_t pos)
{
    SynapseSection* section = &synapseSections[pos];
    SynapseSection* prev = &synapseSections[section->prev];

    if(section->next != UNINIT_STATE_64)
    {
        SynapseSection* next = &synapseSections[section->next];
        next->prev = section->prev;
    }

    prev->next = section->next;

    assert(KyoukoRoot::m_segment->synapses.deleteItem(pos));
}

#endif // SECTION_HANDLING_H
