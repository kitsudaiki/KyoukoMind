/**
 *  @file    messageBufferTest.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef MESSAGEBUFFERTEST_H
#define MESSAGEBUFFERTEST_H

#include <common.h>
#include <testing/commonTest.h>
#include <kyoukoNetwork.h>

namespace KyoukoMind
{

class MessageBufferTest: public Kitsune::CommonTest
{
public:
    MessageBufferTest();

private:
    void initTest();
    void testAppandMessageBlock();
    void testAddMessageBlock();
    void testGetMessage();
    void testGetNextMessage();
    void testFinishMessage();
    void clostTest();

    KyoukoMind::KyoukoNetwork* m_network = nullptr;
    Brick* m_testBrick = nullptr;
};

}

#endif // MESSAGEBUFFERTEST_H
