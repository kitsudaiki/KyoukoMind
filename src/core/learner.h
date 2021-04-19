#ifndef LERNER_H
#define LERNER_H

#include <common.h>

struct OutputSegmentMeta;
struct Output;

class Learner
{
public:
    Learner();

    bool learnStep();
    void executeStep();

    float m_inputBuffer[800];
private:
    void finishStep();
    uint32_t checkOutput(OutputSegmentMeta *segmentMeta, Output *outputs);
};

#endif // LERNER_H
