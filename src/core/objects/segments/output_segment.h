#ifndef OUTPUT_SEGMENTS_H
#define OUTPUT_SEGMENTS_H

#include <common.h>

#include <core/objects/segments/abstract_segment.h>

class OutputSegment : public AbstractSegment
{
public:
    OutputSegment();
    ~OutputSegment();

    float lastTotalError = 0.0f;
    float actualTotalError = 0.0f;

    OutputNode* outputs = nullptr;
};

#endif // OUTPUT_SEGMENTS_H
