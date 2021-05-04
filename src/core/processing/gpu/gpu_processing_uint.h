/**
 * @file        gpu_processing_uint.h
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

#ifndef GPU_INTERFACE_H
#define GPU_INTERFACE_H

#include <common.h>
#include <core/objects/network_cluster.h>
#include <core/objects/segment.h>

#include <libKitsunemimiCommon/threading/thread.h>
#include <libKitsunemimiOpencl/gpu_handler.h>
#include <libKitsunemimiOpencl/gpu_interface.h>

namespace Kitsunemimi {
class Barrier;
}

class GpuProcessingUnit
        : public Kitsunemimi::Thread
{
public:
    GpuProcessingUnit(Kitsunemimi::Opencl::GpuInterface* gpuInterface);

    bool initializeGpu(NetworkCluster *cluster);

    bool updateInput();
    bool synapse_processing();
    bool node_processing();
    bool output_node_processing();
    bool output_learn_step();
    bool finish();

    void run();

    Kitsunemimi::Barrier* m_phase1 = nullptr;
    Kitsunemimi::Barrier* m_phase2 = nullptr;
    Kitsunemimi::Barrier* m_phase3 = nullptr;

private:
    Kitsunemimi::Opencl::GpuHandler* m_gpuHandler = nullptr;
    Kitsunemimi::Opencl::GpuInterface* m_gpuInterface = nullptr;

    Kitsunemimi::Opencl::GpuData oclData;

    bool closeDevice();
};

#endif // GPU_INTERFACE_H
