/**
 * @file        gpu_processing_static.cpp
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

#include "gpu_processing_static.h"

#include <core/processing/cpu/processing.h>

#include <core/processing/gpu/gpu_processing_uint.h>
#include <libKitsunemimiOpencl/gpu_interface.h>

GpuProcessingStatic::GpuProcessingStatic()
    : StaticProcessing()
{
    m_gpuHandler = new Kitsunemimi::Opencl::GpuHandler();
    assert(m_gpuHandler->m_interfaces.size() >= 1);
    m_gpu = new GpuProcessingUnit(m_gpuHandler->m_interfaces.at(0));
    assert(m_gpu->initializeGpu(KyoukoRoot::m_networkCluster));
}

void
GpuProcessingStatic::executeStep()
{

}

void
GpuProcessingStatic::reductionLearning()
{

}

void
GpuProcessingStatic::updateLearning()
{

}
