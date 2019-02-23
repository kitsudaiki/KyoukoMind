/**
 *  @file    globalValuesHandler.h
 *
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 *
 *  Apache License Version 2.0
 */

#ifndef GLOBALVALUESHANDLER_H
#define GLOBALVALUESHANDLER_H

#include <common.h>
#include <core/structs/globalValues.h>
#include <core/structs/clusterMeta.h>

namespace KyoukoMind
{

class GlobalValuesHandler
{
public:
    GlobalValuesHandler();

    GlobalValues getGlobalValues();
    void setGlobalValues(const GlobalValues &values);

private:
    GlobalValues m_globalValues;

    std::mutex m_mutex;
};

}

#endif // GLOBALVALUESHANDLER_H
