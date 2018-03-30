#ifndef MESSAGETEST_H
#define MESSAGETEST_H

#include <common.h>

namespace KyoukoMind
{

class Message;
class DataMessage;
class ReplyMessage;
class LearningMessage;
class LearningReplyMessage;

class MessageTest
{

public:
    MessageTest();

protected:
    void initTestCase();
    void checkDataMessage();
    void checkReplyMessage();
    void checkLerningMessage();
    void checkLearingReplyMessage();
    void cleanupTestCase();
};

}

#endif // MESSAGETEST_H
