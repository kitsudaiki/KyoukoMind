/**
 * @file        input_segment.cpp
 *
 * @author      Tobias Anker <tobias.anker@kitsunemimi.moe>
 *
 * @copyright   Apache License Version 2.0
 *
 *      Copyright 2019 Tobias Anker
 *
 *      Licensed under the Apache License, Version 2.0 (the "License");
 *      you may not use this file except in compliance with the License.
 *      You may obtain a copy of the License at
 *
 *          http://www.apache.org/licenses/LICENSE-2.0
 *
 *      Unless required by applicable law or agreed to in writing, software
 *      distributed under the License is distributed on an "AS IS" BASIS,
 *      WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *      See the License for the specific language governing permissions and
 *      limitations under the License.
 */

#include "input_segment.h"

InputSegment::InputSegment()
    : AbstractSegment()
{
    m_type = INPUT_SEGMENT;
}

InputSegment::~InputSegment()
{

}

bool
InputSegment::initSegment(JsonItem &parsedContent)
{
    const uint32_t numberOfInputs = parsedContent["number_of_inputs"].getInt();
    const uint32_t totalBorderSize = parsedContent["total_border_size"].getInt();

    SegmentHeader header = createNewHeader(numberOfInputs,
                                           totalBorderSize);
    allocateSegment(header);
    initSegmentPointer(header);
    initPosition(parsedContent);
    connectBorderBuffer();

    return true;
}

/**
 * @brief InputSegment::connectBorderBuffer
 * @return
 */
bool
InputSegment::connectBorderBuffer()
{
    for(uint32_t i = 0; i < segmentHeader->inputs.count; i++) {
        inputs[i].targetBorderId = i;
    }

    return true;
}

/**
 * @brief InputSegment::createNewHeader
 * @param numberOfInputs
 * @return
 */
SegmentHeader
InputSegment::createNewHeader(const uint32_t numberOfInputs,
                              const uint64_t borderbufferSize)
{
    SegmentHeader segmentHeader;
    segmentHeader.segmentType = INPUT_SEGMENT;
    uint32_t segmentDataPos = 0;

    // init header
    segmentDataPos += 1 * sizeof(SegmentHeader);

    // init settings
    segmentHeader.settings.count = 1;
    segmentHeader.settings.bytePos = segmentDataPos;
    segmentDataPos += 1 * sizeof(SegmentSettings);

    // init neighborList
    segmentHeader.neighborList.count = 1;
    segmentHeader.neighborList.bytePos = segmentDataPos;
    segmentDataPos += 1 * sizeof(SegmentNeighborList);

    // init inputTransfers
    segmentHeader.inputTransfers.count = borderbufferSize;
    segmentHeader.inputTransfers.bytePos = segmentDataPos;
    segmentDataPos += borderbufferSize * sizeof(float);

    // init outputTransfers
    segmentHeader.outputTransfers.count = borderbufferSize;
    segmentHeader.outputTransfers.bytePos = segmentDataPos;
    segmentDataPos += borderbufferSize * sizeof(float);

    // init bricks
    segmentHeader.inputs.count = numberOfInputs;
    segmentHeader.inputs.bytePos = segmentDataPos;
    segmentDataPos += numberOfInputs * sizeof(InputNode);

    segmentHeader.staticDataSize = segmentDataPos;

    return segmentHeader;
}

/**
 * @brief InputSegment::initSegmentPointer
 * @param header
 */
void
InputSegment::initSegmentPointer(const SegmentHeader &header)
{
    uint8_t* dataPtr = static_cast<uint8_t*>(staticSegmentData.data);
    uint64_t pos = 0;

    segmentHeader = reinterpret_cast<SegmentHeader*>(dataPtr + pos);
    segmentHeader[0] = header;

    pos = 256;
    segmentSettings = reinterpret_cast<SegmentSettings*>(dataPtr + pos);
    pos = segmentHeader->neighborList.bytePos;
    segmentNeighbors = reinterpret_cast<SegmentNeighborList*>(dataPtr + pos);
    pos = segmentHeader->inputTransfers.bytePos;
    inputTransfers = reinterpret_cast<float*>(dataPtr + pos);
    pos = segmentHeader->outputTransfers.bytePos;
    outputTransfers = reinterpret_cast<float*>(dataPtr + pos);
    pos = segmentHeader->inputs.bytePos;
    inputs = reinterpret_cast<InputNode*>(dataPtr + pos);
}

/**
 * @brief InputSegment::allocateSegment
 * @param header
 */
void
InputSegment::allocateSegment(SegmentHeader &header)
{
    const uint32_t numberOfBlocks = (header.staticDataSize / 4096) + 1;
    header.staticDataSize = numberOfBlocks * 4096;
    Kitsunemimi::allocateBlocks_DataBuffer(staticSegmentData, numberOfBlocks);
}
