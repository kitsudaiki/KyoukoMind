/**
 *  @author  Tobias Anker
 *  Contact: tobias.anker@kitsunemimi.moe
 */

#ifndef NETWORK_INITIALIZER_H
#define NETWORK_INITIALIZER_H

#include <common.h>
#include "init_meta_data.h"

class BrickQueue;
class Segment;

class NetworkInitializer
{

public:
    NetworkInitializer();

    bool createNewNetwork(const std::string &fileContent);

private:
    std::vector<std::vector<InitMetaDataEntry>> m_networkMetaStructure;

    void connectAllBricks(Segment &segment);
    void addBricks(Segment &segment);

    uint32_t getNumberOfBricks();
    uint32_t getNumberOfNodeBricks();

    std::pair<uint32_t, uint32_t> getNext(const uint32_t x,
                                          const uint32_t y,
                                          const uint8_t side);

    uint32_t getDistantToNextNodeBrick(const uint32_t x,
                                       const uint32_t y,
                                       const uint8_t side);

};

#endif // NETWORK_INITIALIZER_H
