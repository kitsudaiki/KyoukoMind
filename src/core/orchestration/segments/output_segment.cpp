/**
 * @file        output_segment.cpp
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

#include "output_segment.h"

/**
 * @brief constructor
 */
OutputSegment::OutputSegment()
    : AbstractSegment()
{
    m_type = OUTPUT_SEGMENT;
}

/**
 * @brief destructor
 */
OutputSegment::~OutputSegment() {}

/**
 * @brief OutputSegment::initSegment
 * @param parsedContent
 * @return
 */
bool
OutputSegment::initSegment(const JsonItem &parsedContent)
{
    const uint32_t numberOfOutputs = parsedContent.get("number_of_outputs").getInt();
    const uint32_t totalBorderSize = parsedContent.get("total_border_size").getInt();

    SegmentHeader header = createNewHeader(numberOfOutputs,
                                           totalBorderSize);
    allocateSegment(header);
    initSegmentPointer(header);
    segmentHeader->position = convertPosition(parsedContent);
    initBorderBuffer(parsedContent);
    connectBorderBuffer();

    return true;
}

/**
 * @brief OutputSegment::connectBorderBuffer
 * @return
 */
bool
OutputSegment::connectBorderBuffer()
{
    for(uint32_t i = 0; i < segmentHeader->outputs.count; i++) {
        outputs[i].targetBorderId = i;
    }

    return true;
}

/**
 * @brief OutputSegment::createNewHeader
 * @param numberOfOutputs
 * @param borderbufferSize
 * @return
 */
SegmentHeader
OutputSegment::createNewHeader(const uint32_t numberOfOutputs,
                               const uint64_t borderbufferSize)
{
    SegmentHeader segmentHeader;
    segmentHeader.segmentType = m_type;
    uint32_t segmentDataPos = createGenericNewHeader(segmentHeader, borderbufferSize);

    // init outputs
    segmentHeader.outputs.count = numberOfOutputs;
    segmentHeader.outputs.bytePos = segmentDataPos;
    segmentDataPos += numberOfOutputs * sizeof(OutputNode);

    // set total size of the segment
    segmentHeader.staticDataSize = segmentDataPos;

    return segmentHeader;
}

/**
 * @brief OutputSegment::initSegmentPointer
 * @param header
 */
void
OutputSegment::initSegmentPointer(const SegmentHeader &header)
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
    pos = segmentHeader->outputs.bytePos;
    outputs = reinterpret_cast<OutputNode*>(dataPtr + pos);
}

/**
 * @brief OutputSegment::allocateSegment
 * @param header
 */
void
OutputSegment::allocateSegment(SegmentHeader &header)
{
    const uint32_t numberOfBlocks = (header.staticDataSize / 4096) + 1;
    header.staticDataSize = numberOfBlocks * 4096;
    Kitsunemimi::allocateBlocks_DataBuffer(staticSegmentData, numberOfBlocks);
}