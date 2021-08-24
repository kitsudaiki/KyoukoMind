#include "abstract_segment.h"

AbstractSegment::AbstractSegment()
{

}

AbstractSegment::~AbstractSegment()
{

}

SegmentTypes
AbstractSegment::getType() const
{
    return m_type;
}

bool
AbstractSegment::isReady()
{
    for(uint8_t i = 0; i < 12; i++)
    {
        if(segmentNeighbors->neighbors[i].inUse == 1
                && segmentNeighbors->neighbors[i].inputReady == 0)
        {
            return false;
        }
    }

    return true;
}
