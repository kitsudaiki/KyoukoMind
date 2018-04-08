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
    AxonInitializer(std::vector<std::vector<MetaDataEntry>> *networkMetaStructure,
                    uint32_t networkDimensionX,
                    uint32_t networkDimensionY);

    bool createAxons();

private:
    std::vector<std::vector<MetaDataEntry>>* m_networkMetaStructure = nullptr;
    uint32_t m_networkDimensionX = 0;
    uint32_t m_networkDimensionY = 0;
    NextChooser* m_chooser = nullptr;

    uint32_t getNextAxonPathStep(const uint32_t x,
                                 const uint32_t y,
                                 const uint8_t inputSide,
                                 const uint32_t currentPath,
                                 const uint32_t currentStep);
    std::pair<uint32_t, uint32_t> calculatePosition(const uint32_t clusterId);
};

}

#endif // AXONINITIALIZER_H
