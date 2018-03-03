#ifndef INITLOGGER_H
#define INITLOGGER_H

namespace Persistence {
class Logger;
}

namespace KyoukoMind
{

class Logger
{
public:
    Logger();
    Persistence::Logger *initLogger(bool *ok);

};

}

#endif // INITLOGGER_H
