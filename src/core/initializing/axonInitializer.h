/**
 *  @file    axonInitializer.h
 *  @author  Tobias Anker
 *
 *  @section DESCRIPTION
 *
 *  TODO: Description
 */

#ifndef AXONINITIALIZER_H
#define AXONINITIALIZER_H

#include <common.h>
#include <core/structs/clusterMeta.h>

namespace KyoukoMind
{
class NextChooser;

class AxonInitializer
{
public:
    AxonInitializer(std::vector<std::vector<InitMetaDataEntry>> *networkMetaStructure,
                    const uint32_t networkDimensionX,
                    const uint32_t networkDimensionY);

    bool createAxons();

private:
    struct NewAxon {
        uint32_t targetX = 0;
        uint32_t targetY = 0;
        uint32_t targetPath = 0;
    };

    std::vector<std::vector<InitMetaDataEntry>>* m_networkMetaStructure = nullptr;
    uint32_t m_networkDimensionX = 0;
    uint32_t m_networkDimensionY = 0;

    NewAxon getNextAxonPathStep(const uint32_t x,
                                const uint32_t y,
                                const uint8_t inputSide,
                                const uint32_t currentPath,
                                const uint32_t currentStep);
};

}

#endif // AXONINITIALIZER_H
