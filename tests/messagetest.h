#ifndef MESSAGETEST_H
#define MESSAGETEST_H

#include <common.h>
#include <tests/commontest.h>

namespace KyoukoMind
{

class Message;
class DataMessage;
class ReplyMessage;
class LearningMessage;
class LearningReplyMessage;

class MessageTest : public CommonTest
{

public:
    MessageTest();

private:
    void initTestCase();
    void checkDataMessage();
    void checkReplyMessage();
    void checkLerningMessage();
    void checkLearingReplyMessage();
    void cleanupTestCase();
};

}

#endif // MESSAGETEST_H
