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
#include <core/structure/network_cluster.h>

StorageIO::StorageIO() {}

/**
 * @brief StorageIO::writeToDisc
 * @param dirPath
 * @return
 */
bool
StorageIO::writeToDisc(const std::string &dirPath)
{
    const std::string coreSegPath = dirPath + "/core_segment_01";
    /*DataBuffer* coreBuffer = &KyoukoRoot::m_networkCluster->synapseSegment->persistenBuffer;
    if(writeBufferToFile(coreSegPath, *coreBuffer) == false) {
        return false;
    }*/

    return true;
}

/**
 * @brief StorageIO::readFromDisc
 * @param dirPath
 * @return
 */
bool
StorageIO::readFromDisc(const std::string &dirPath)
{
    const std::string coreSegPath = dirPath + "/core_segment_01";
    /*DataBuffer* coreBuffer = &KyoukoRoot::m_networkCluster->synapseSegment->persistenBuffer;
    if(readBufferFromFile(coreSegPath, *coreBuffer) == false) {
        return false;
    }*/

    return true;
}

/**
 * @brief StorageIO::writeBufferToFile
 * @param filePath
 * @param buffer
 * @return
 */
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

/**
 * @brief StorageIO::readBufferFromFile
 * @param filePath
 * @param buffer
 * @return
 */
bool
StorageIO::readBufferFromFile(const std::string &filePath,
                              Kitsunemimi::DataBuffer &buffer)
{
    bool ret = true;
    Kitsunemimi::Persistence::BinaryFile segReader(filePath, true);
    if(segReader.readCompleteFile(buffer) == false) {
        ret = false;
    }
    segReader.closeFile();

    return ret;
}
