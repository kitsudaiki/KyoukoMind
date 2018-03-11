#ifndef MESSAGETEST_H
#define MESSAGETEST_H

#include <QObject>
#include <QString>
#include <QtTest>
#include <QFile>
#include <common/enums.h>

namespace KyoukoMind
{

class Message;
class DataMessage;
class ReplyMessage;
class LearningMessage;
class LearningReplyMessage;

class MessageTest : public QObject
{
    Q_OBJECT

public:
    MessageTest();

private slots:
    void initTestCase();
    void checkDataMessage();
    void checkReplyMessage();
    void checkLerningMessage();
    void checkLearingReplyMessage();
    void cleanupTestCase();
};

}

#endif // MESSAGETEST_H
