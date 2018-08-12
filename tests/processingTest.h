/**
 *  @file    processingTest.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef PROCESSINGTEST_H
#define PROCESSINGTEST_H

#include <common.h>
#include <commonTest.h>

namespace Networking
{
class IncomingMessageBuffer;
class OutgoingMessageBuffer;
}

namespace KyoukoMind
{
class NodeCluster;

class NextChooser;
class EdgeClusterProcessing;

class ProcessingTest : public Kitsune::CommonTest
{
public:
    ProcessingTest();

private:
    void initTestCase();
    void checkProcessing();
    void cleanupTestCase();

    NodeCluster *m_nodeCluster1 = nullptr;
    NodeCluster *m_nodeCluster2 = nullptr;
    NodeCluster *m_nodeCluster3 = nullptr;

    EdgeClusterProcessing* m_nodeProcessing = nullptr;
};

}

#endif // PROCESSINGTEST_H
