/**
 *  @file    dummy_input.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *
 */

#ifndef DUMMY_INPUT_H
#define DUMMY_INPUT_H

#include <common.h>
#include <libKitsunemimiCommon/threading/thread.h>
#include <core/objects/network_segment.h>

namespace KyoukoMind
{

class DummyInput
        : public Kitsunemimi::Thread
{
public:
    DummyInput();

    void run();
private:
    Brick* inputBrick = nullptr;

    float m_inputValue = 150.0f;

    float m_gliaValue = 1.1f;
    float m_memTemp = 0.0f;
    float m_memOffset = 0.5f;
    float m_learnTemp = 0.0f;
    float m_learnOffset = 0.5f;

    void initialize();
};

}

#endif // DUMMY_INPUT_H
