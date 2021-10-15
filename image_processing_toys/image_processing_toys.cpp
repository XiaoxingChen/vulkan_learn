// Copyright(c) 2019, NVIDIA CORPORATION. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// VulkanHpp Samples : 15_DrawCube
//                     Draw a cube

#include "math.hpp"
// #include "shaders.hpp"
#include "utils.hpp"
#include "SPIRV/GlslangToSpv.h"
#include "vulkan/vulkan.hpp"
#include "logging.h"

#include <iostream>
#include <thread>
#include <memory>
#include <chrono>

#include "image_processing_toys/sample_utils.h"
#include "event_manager.h"

#define DEPTH_BUFFER_DATA 0

static char const * AppName    = "img_proc_toys";
static char const * EngineName = "Vulkan.hpp";


int main( int /*argc*/, char ** /*argv*/ )
{
  initLogger();
  SampleContext context;
  ModelResource modelResource;
  GraphicsPipelineResource graphicsPipelineResource;
  ComputePipelineResource computePipelineResource;
  BufferList bufferList;
  uint32_t frameCounter = 0;
  auto timePrev = std::chrono::steady_clock::now();
  ComputeUniformInfo uniformInfo;
  try
  {

  prepare(context,EngineName, AppName);
  prepareRectangle(graphicsPipelineResource, context);
  prepareCompute(computePipelineResource, context);
  prepare(modelResource, context);
  bufferList.prepare(context, modelResource);
  uniformInfo.imgSize = glm::ivec4(modelResource.pTextureData->extent.width, modelResource.pTextureData->extent.height, 0, 0);
  uniformInfo.timestamp = 0;
  
  LOGI("{}:{}", __FILE__, __LINE__);


    /* VULKAN_KEY_START */

    vk::su::updateDescriptorSets(
      context.device, computePipelineResource.descriptorSet,
      {
        { vk::DescriptorType::eStorageBuffer, bufferList.pComputeOutput->buffer, {} },
        { vk::DescriptorType::eUniformBuffer, bufferList.pComputeUniform->buffer, {} }
      }, {});

    vk::su::updateDescriptorSets(
      context.device, graphicsPipelineResource.descriptorSet,
      {},
      *modelResource.pTextureData );

    FrameResource frame;
    prepare(frame, context);
    auto computeCommandBuffer = createCommandBuffer(context, computePipelineResource, modelResource);

    frame.commandBuffers = createCommandBuffers(context, graphicsPipelineResource, modelResource);

#if 1
    while (!glfwWindowShouldClose(context.pSurfaceData->window.handle)) {
            glfwPollEvents();

      if (handleExit(vk::su::eventList())) break;

      vk::su::eventList().clear();

      {
        uniformInfo.timestamp += 1e-1;
        bufferList.pComputeUniform->upload(context.device, uniformInfo);
        auto submitInfo = vk::SubmitInfo(nullptr, nullptr, computeCommandBuffer);
        auto fence = context.device.createFence(vk::FenceCreateInfo());
        context.computeQueue.submit(submitInfo, fence);
        while ( vk::Result::eTimeout == context.device.waitForFences( fence, VK_TRUE, vk::su::FenceTimeout ) );
        context.device.destroyFence(fence);
      }
      vk::su::oneTimeSubmit(context.device, context.commandPool, context.graphicsQueue,
      [&](vk::CommandBuffer const & commandBuffer)
      {
          modelResource.pTextureData->setImage(context.device, commandBuffer, *bufferList.pComputeOutput);
      });

      auto result = draw(context, frame);
      if(result == vk::Result::eSuboptimalKHR)
      {
        handleSurfaceChange(context, modelResource, frame, graphicsPipelineResource);
      }
      
    }
#endif
    context.device.waitIdle();
    bufferList.tearDown(context);

    /* VULKAN_KEY_END */
    tearDown(modelResource, context);
    tearDown(frame, context);
    tearDown(graphicsPipelineResource, context);
    tearDown(computePipelineResource, context);
    tearDown(context);
  }
  catch ( vk::SystemError & err )
  {
    std::cout << "vk::SystemError: " << err.what() << std::endl;
    exit( -1 );
  }
  catch ( std::exception & err )
  {
    std::cout << "std::exception: " << err.what() << std::endl;
    exit( -1 );
  }
  catch ( ... )
  {
    std::cout << "unknown error\n";
    exit( -1 );
  }
  LOGI("{}:{}", __FILE__, __LINE__);
  destroyLogger();
  return 0;
}