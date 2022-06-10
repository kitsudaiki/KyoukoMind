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

/**
 * @brief constructor
 */
InputSegment::InputSegment()
    : AbstractSegment()
{
    m_type = INPUT_SEGMENT;
}

InputSegment::InputSegment(const void* data, const uint64_t dataSize)
    : AbstractSegment(data, dataSize)
{
    m_type = INPUT_SEGMENT;
}

/**
 * @brief destructor
 */
InputSegment::~InputSegment() {}

/**
 * @brief initalize segment
 *
 * @param parsedContent json-object with the segment-description
 *
 * @return true, if successful, else false
 */
bool
InputSegment::initSegment(const JsonItem &parsedContent)
{
    const uint32_t numberOfInputs = parsedContent.get("number_of_inputs").getInt();
    const uint32_t totalBorderSize = parsedContent.get("total_border_size").getInt();

    SegmentHeader header = createNewHeader(numberOfInputs, totalBorderSize);
    header.position = convertPosition(parsedContent);

    allocateSegment(header);
    initSegmentPointer(header);
    initBorderBuffer(parsedContent);
    connectBorderBuffer();

    return true;
}

/**
 * @brief InputSegment::reinitPointer
 * @return
 */
bool
InputSegment::reinitPointer(const uint64_t numberOfBytes)
{
    uint8_t* dataPtr = static_cast<uint8_t*>(segmentData.staticData);

    uint64_t pos = 0;
    uint64_t byteCounter = 0;
    segmentHeader = reinterpret_cast<SegmentHeader*>(dataPtr + pos);
    byteCounter += sizeof(SegmentHeader);

    pos = segmentHeader->settings.bytePos;
    dynamicSegmentSettings = reinterpret_cast<DynamicSegmentSettings*>(dataPtr + pos);
    byteCounter += sizeof(DynamicSegmentSettings);

    pos = segmentHeader->neighborList.bytePos;
    segmentNeighbors = reinterpret_cast<SegmentNeighborList*>(dataPtr + pos);
    byteCounter += segmentHeader->neighborList.count * sizeof(SegmentNeighborList);

    pos = segmentHeader->inputTransfers.bytePos;
    inputTransfers = reinterpret_cast<float*>(dataPtr + pos);
    byteCounter += segmentHeader->inputTransfers.count * sizeof(float);

    pos = segmentHeader->outputTransfers.bytePos;
    outputTransfers = reinterpret_cast<float*>(dataPtr + pos);
    byteCounter += segmentHeader->outputTransfers.count * sizeof(float);

    pos = segmentHeader->inputs.bytePos;
    inputs = reinterpret_cast<InputNode*>(dataPtr + pos);
    byteCounter += segmentHeader->inputs.count * sizeof(InputNode);

    // check result
    if(byteCounter != numberOfBytes) {
        return false;
    }

    return true;
}

/**
 * @brief init border-buffer
 *
 * @return true, if successful, else false
 */
bool
InputSegment::connectBorderBuffer()
{
    for(uint32_t i = 0; i < segmentHeader->inputs.count; i++)
    {
        inputs[i] = InputNode();
        inputs[i].targetBorderId = i;
    }

    return true;
}

/**
 * @brief create new segment-header with size and position information
 *
 * @param numberOfInputs number of inputs
 * @param borderbufferSize size of border-buffer
 *
 * @return new segment-header
 */
SegmentHeader
InputSegment::createNewHeader(const uint32_t numberOfInputs,
                              const uint64_t borderbufferSize)
{
    SegmentHeader segmentHeader;
    segmentHeader.segmentType = m_type;
    uint32_t segmentDataPos = createGenericNewHeader(segmentHeader, borderbufferSize);

    // init bricks
    segmentHeader.inputs.count = numberOfInputs;
    segmentHeader.inputs.bytePos = segmentDataPos;
    segmentDataPos += numberOfInputs * sizeof(InputNode);

    segmentHeader.staticDataSize = segmentDataPos;

    return segmentHeader;
}

/**
 * @brief init pointer within the segment-header
 *
 * @param header segment-header
 */
void
InputSegment::initSegmentPointer(const SegmentHeader &header)
{
    uint8_t* dataPtr = static_cast<uint8_t*>(segmentData.staticData);
    uint64_t pos = 0;

    segmentHeader = reinterpret_cast<SegmentHeader*>(dataPtr + pos);
    segmentHeader[0] = header;

    pos = 256;
    dynamicSegmentSettings = reinterpret_cast<DynamicSegmentSettings*>(dataPtr + pos);
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
 * @brief allocate memory for the segment
 *
 * @param header header with the size-information
 */
void
InputSegment::allocateSegment(SegmentHeader &header)
{
    const uint32_t numberOfBlocks = (header.staticDataSize / 4096) + 1;
    header.staticDataSize = numberOfBlocks * 4096;   
    segmentData.initBuffer(header.staticDataSize);
}
