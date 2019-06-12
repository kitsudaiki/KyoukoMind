/**
 *  @file    incomingBufferTest.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef INCOMINGBUFFERTEST_H
#define INCOMINGBUFFERTEST_H

#include <common.h>
#include <testing/commonTest.h>

namespace KyoukoMind
{

class IncomingBufferTest: public Kitsune::CommonTest
{
public:
    IncomingBufferTest();

private:
    void testAddMessageAndFinish();
    void testGetMessage();
    void testIsReady();
};

}

#endif // INCOMINGBUFFERTEST_H
