/**
 *  @file    demoUserInput.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef DEMOUSERINPUT_H
#define DEMOUSERINPUT_H

#include <common.h>

namespace KyoukoMind
{
class DemoIO;
class ClusterHandler;

class DemoUserInput
{
public:
    DemoUserInput(ClusterHandler* clusterHandler);

    void inputLoop();

private:
    DemoIO* m_dataio = nullptr;
};

}

#endif // DEMOUSERINPUT_H
