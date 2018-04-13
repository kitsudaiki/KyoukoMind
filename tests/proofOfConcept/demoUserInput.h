/**
 *  @file    demoUserInput.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef DEMOUSERINPUT_H
#define DEMOUSERINPUT_H

#include <common.h>

namespace KyoukoMind
{
class DemoIO;
class MessageController;

class DemoUserInput
{
public:
    DemoUserInput(MessageController* messageController);

    void inputLoop();

private:
    DemoIO* m_dataio = nullptr;
};

}

#endif // DEMOUSERINPUT_H
