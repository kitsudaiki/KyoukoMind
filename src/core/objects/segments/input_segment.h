#ifndef INPUT_SEGMENTS_H
#define INPUT_SEGMENTS_H

#include <common.h>

#include <core/objects/segments/abstract_segment.h>

class InputSegment : public AbstractSegment
{
public:
    InputSegment();
    ~InputSegment();

    InputNode* inputs = nullptr;
};

#endif // INPUT_SEGMENTS_H
