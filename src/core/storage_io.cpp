/**
 * @file        storage_io.cpp
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

#include "storage_io.h"

#include <libKitsunemimiPersistence/files/binary_file.h>
#include <kyouko_root.h>
#include <core/objects/network_cluster.h>

StorageIO::StorageIO() {}

bool
StorageIO::writeToDisc(const std::string &dirPath)
{
    const std::string coreSegPath = dirPath + "/core_segment_01";
    DataBuffer* coreBuffer = &KyoukoRoot::m_networkCluster->synapseSegment->buffer;
    if(writeBufferToFile(coreSegPath, *coreBuffer) == false) {
        return false;
    }

    const std::string outputSegPath = dirPath + "/output_segment_01";
    DataBuffer* outputBuffer = &KyoukoRoot::m_networkCluster->outputSegment->buffer;
    if(writeBufferToFile(outputSegPath, *outputBuffer) == false) {
        return false;
    }

    return true;
}

bool
StorageIO::readFromDisc(const std::string &dirPath)
{
    return true;
}

bool
StorageIO::writeBufferToFile(const std::string &filePath,
                             Kitsunemimi::DataBuffer &buffer)
{
    bool ret = true;
    Kitsunemimi::Persistence::BinaryFile segWriter(filePath, true);
    if(segWriter.writeCompleteFile(buffer) == false) {
        ret = false;
    }
    segWriter.closeFile();

    return ret;
}
