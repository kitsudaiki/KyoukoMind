/**
 *  @file    messasgeTest.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef MESSAGETEST_H
#define MESSAGETEST_H

#include <common.h>
#include <commonTest.h>

namespace Networking
{
class Message;
class DataMessage;
}

namespace KyoukoMind
{

class MessageTest : public Kitsune::CommonTest
{

public:
    MessageTest();

private:
    void initTestCase();
    void checkDataMessage();
    void checkReplyMessage();
    void cleanupTestCase();
};

}

#endif // MESSAGETEST_H
