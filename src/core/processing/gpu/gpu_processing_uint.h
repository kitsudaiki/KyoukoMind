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

#ifndef KYOUKOMIND_GPU_PROCESSING_UNIT_H
#define KYOUKOMIND_GPU_PROCESSING_UNIT_H

#include <common.h>
#include <core/data_structure/segments/dynamic_segment.h>
#include <core/data_structure/cluster.h>

#include <libKitsunemimiOpencl/gpu_handler.h>
#include <libKitsunemimiOpencl/gpu_interface.h>

class GpuProcessingUnit
{
public:
    GpuProcessingUnit(Kitsunemimi::Opencl::GpuInterface* gpuInterface);

    bool initializeGpu(Cluster *);

    bool learn(Kitsunemimi::ErrorContainer &error);
    bool execute(Kitsunemimi::ErrorContainer &error);

private:
    Kitsunemimi::Opencl::GpuHandler* m_gpuHandler = nullptr;
    Kitsunemimi::Opencl::GpuInterface* m_gpuInterface = nullptr;

    Kitsunemimi::Opencl::GpuData oclData;

    bool closeDevice();
};

#endif // KYOUKOMIND_GPU_PROCESSING_UNIT_H
