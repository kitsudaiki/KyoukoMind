#ifndef SEGMENT_CREATION_H
#define SEGMENT_CREATION_H

#include <common.h>
#include <core/objects/segment.h>

/**
 * @brief createNewHeader
 * @param numberOfBricks
 * @param numberOfNodes
 * @param numberOfSynapseSections
 * @param numberOfInputs
 * @param numberOfOutputs
 * @return
 */
SegmentHeader
createNewHeader(const uint32_t numberOfBricks,
                const uint32_t numberOfNodes,
                const uint64_t numberOfSynapseSections,
                const uint32_t numberOfInputs,
                const uint32_t numberOfOutputs)
{
    SegmentHeader segmentHeader;
    uint32_t segmentDataPos = 0;

    // init header
    segmentDataPos += 1 * sizeof(SegmentHeader);

    // init settings
    segmentHeader.settings.count = 1;
    segmentHeader.settings.bytePos = segmentDataPos;
    segmentDataPos += 1 * sizeof(SegmentSettings);

    // init bricks
    segmentHeader.bricks.count = numberOfBricks;
    segmentHeader.bricks.bytePos = segmentDataPos;
    segmentDataPos += numberOfBricks * sizeof(Brick);

    // init brick-order
    segmentHeader.brickOrder.count = numberOfBricks;
    segmentHeader.brickOrder.bytePos = segmentDataPos;
    segmentDataPos += numberOfBricks * sizeof(uint32_t);

    // init nodes
    segmentHeader.nodes.count = numberOfNodes;
    segmentHeader.nodes.bytePos = segmentDataPos;
    segmentDataPos += numberOfNodes * sizeof(Node);

    // init input
    segmentHeader.inputs.count = numberOfInputs;
    segmentHeader.inputs.bytePos = segmentDataPos;
    segmentDataPos += numberOfInputs * sizeof(InputNode);

    // init output
    segmentHeader.outputs.count = numberOfOutputs;
    segmentHeader.outputs.bytePos = segmentDataPos;
    segmentDataPos += numberOfOutputs * sizeof(OutputNode);


    segmentHeader.staticDataSize = segmentDataPos;


    // init synapse sections
    segmentDataPos = 0;
    segmentHeader.synapseSections.count = numberOfSynapseSections;
    segmentHeader.synapseSections.bytePos = segmentDataPos;
    segmentDataPos += numberOfSynapseSections * sizeof(SynapseSection);

    return segmentHeader;
}

/**
 * @brief initSegmentPointer
 * @param segment
 * @param header
 */
void
initSegmentPointer(Segment &segment,
                   const SegmentHeader &header)
{
    uint8_t* segmentData = static_cast<uint8_t*>(segment.persistenBuffer.data);
    uint64_t pos = 0;

    segment.segmentHeader = reinterpret_cast<SegmentHeader*>(segmentData + pos);
    segment.segmentHeader[0] = header;

    pos = 256;
    segment.synapseSettings = reinterpret_cast<SegmentSettings*>(segmentData + pos);
    pos = segment.segmentHeader->bricks.bytePos;
    segment.bricks = reinterpret_cast<Brick*>(segmentData + pos);
    pos = segment.segmentHeader->brickOrder.bytePos;
    segment.brickOrder = reinterpret_cast<uint32_t*>(segmentData + pos);
    pos = segment.segmentHeader->nodes.bytePos;
    segment.nodes = reinterpret_cast<Node*>(segmentData + pos);
    pos = segment.segmentHeader->inputs.bytePos;
    segment.inputs = reinterpret_cast<InputNode*>(segmentData + pos);
    pos = segment.segmentHeader->outputs.bytePos;
    segment.outputs = reinterpret_cast<OutputNode*>(segmentData + pos);

    uint8_t* dynamicData = static_cast<uint8_t*>(segment.dynamicBuffer.buffer.data);
    pos = segment.segmentHeader->synapseSections.bytePos;
    segment.synapseSections = reinterpret_cast<SynapseSection*>(dynamicData + pos);
}

/**
 * @brief allocateSegment
 * @param header
 * @return
 */
Segment*
allocateSegment(SegmentHeader &header)
{
    Segment* newSegment = new Segment();

    const uint32_t numberOfBlocks = (header.staticDataSize / 4096) + 1;
    header.staticDataSize = numberOfBlocks * 4096;
    Kitsunemimi::allocateBlocks_DataBuffer(newSegment->persistenBuffer, numberOfBlocks);

    newSegment->dynamicBuffer.initBuffer<SynapseSection>(header.synapseSections.count);
    newSegment->dynamicBuffer.deleteAll();

    return newSegment;
}

/**
 * @brief initDefaultValues
 * @param segment
 * @param numberOfBricks
 * @param numberOfNodes
 * @param numberOfSynapseSections
 * @param numberOfInputs
 * @param numberOfOutputs
 * @param numberOfRandValues
 */
void
initDefaultValues(Segment &segment,
                  const uint32_t numberOfBricks,
                  const uint32_t numberOfNodes,
                  const uint32_t numberOfInputs,
                  const uint32_t numberOfOutputs)
{

    // init header and metadata
    segment.synapseSettings[0] = SegmentSettings();

    // init bricks;
    for(uint32_t i = 0; i < numberOfBricks; i++) {
        segment.bricks[i] = Brick();
    }

    // init brick-order
    for(uint32_t i = 0; i < numberOfBricks; i++) {
        segment.brickOrder[i] = i;
    }

    // init nodes
    for(uint32_t i = 0; i < numberOfNodes; i++) {
        segment.nodes[i] = Node();
    }

    // init input
    for(uint32_t i = 0; i < numberOfInputs; i++) {
        segment.inputs[i] = InputNode();
    }

    // init output
    for(uint32_t i = 0; i < numberOfOutputs; i++) {
        segment.outputs[i] = OutputNode();
    }
}

/**
 * @brief Segment::initSynapseSegment
 * @param numberOfNodeBricks
 * @param numberOfNodes
 * @param numberOfSynapseSections
 * @param numberOfRandValues
 * @return
 */
Segment*
createNewSegment(const uint32_t numberOfBricks,
                 const uint32_t numberOfNodes,
                 const uint64_t numberOfSynapseSections,
                 const uint32_t numberOfInputs,
                 const uint32_t numberOfOutputs)
{
    SegmentHeader header = createNewHeader(numberOfBricks,
                                           numberOfNodes,
                                           numberOfSynapseSections,
                                           numberOfInputs,
                                           numberOfOutputs);
    Segment* segment = allocateSegment(header);

    initSegmentPointer(*segment, header);

    initDefaultValues(*segment,
                      numberOfBricks,
                      numberOfNodes,
                      numberOfInputs,
                      numberOfOutputs);

    return segment;
}


#endif // SEGMENT_CREATION_H
