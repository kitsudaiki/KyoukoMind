/**
 *  @file    message_buffer_test.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef MESSAGE_BUFFER_TEST_H
#define MESSAGE_BUFFER_TEST_H

#include <common.h>
#include <libKitsunemimiCommon/test.h>
#include <kyouko_network.h>

namespace KyoukoMind
{

class MessageBufferTest
        : public Kitsunemimi::Test
{
public:
    MessageBufferTest();

private:
    void initTest();
    void appandMessageBlock_test();
    void addMessageBlock_test();
    void getMessage_test();
    void getNextMessage_test();
    void finishMessage_test();
    void clostTest();

    KyoukoMind::KyoukoNetwork* m_network = nullptr;
    Brick* m_testBrick = nullptr;
};

} // namespace KyoukoMind

#endif // MESSAGE_BUFFER_TEST_H
