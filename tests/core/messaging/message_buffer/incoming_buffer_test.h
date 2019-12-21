/**
 *  @file    incoming_buffer_test.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef INCOMING_BUFFER_TEST_H
#define INCOMING_BUFFER_TEST_H

#include <common.h>
#include <libKitsunemimiCommon/test.h>

namespace KyoukoMind
{

class IncomingBufferTest
        : public Kitsunemimi::Common::Test
{
public:
    IncomingBufferTest();

private:
    void addMessageAndFinish_test();
    void getMessage_test();
    void isReady_test();
};

} // namespace KyoukoMind

#endif // INCOMING_BUFFER_TEST_H
