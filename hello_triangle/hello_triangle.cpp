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

#include "hello_triangle/sample_utils.h"

#define DEPTH_BUFFER_DATA 0

static char const * AppName    = "15_DrawCube";
static char const * EngineName = "Vulkan.hpp";


int main( int /*argc*/, char ** /*argv*/ )
{
  initLogger();
  SampleContext context;

  try
  {

  prepare(context,EngineName, AppName);

    /* VULKAN_KEY_START */
    vk::su::BufferData uniformBufferData = vk::su::BufferData(
     context.physicalDevice, context.device, sizeof( glm::mat4x4 ), vk::BufferUsageFlagBits::eUniformBuffer );

    glm::mat4x4 mvpcMatrix = vk::su::createModelViewProjectionClipMatrix( context.pSurfaceData->extent );
    vk::su::copyToDevice( context.device, uniformBufferData.deviceMemory, mvpcMatrix );

    vk::su::updateDescriptorSets(
      context.device, context.descriptorSet, { { vk::DescriptorType::eUniformBuffer, uniformBufferData.buffer, {} } }, {} );

    vk::su::BufferData vertexBufferData(
     context.physicalDevice, context.device, sizeof( coloredCubeData ), vk::BufferUsageFlagBits::eVertexBuffer );
    vk::su::copyToDevice( context.device,
                          vertexBufferData.deviceMemory,
                          coloredCubeData,
                          sizeof( coloredCubeData ) / sizeof( coloredCubeData[0] ) );

    FrameResource frame;
    prepare(frame, context, vertexBufferData);

    float angle = 0;

    while (!glfwWindowShouldClose(context.pSurfaceData->window.handle)) {
            glfwPollEvents();

      angle += 0.05;
      glm::mat4x4 mvpcMatrix = vk::su::createModelViewProjectionClipMatrix( context.pSurfaceData->extent , angle);
      uniformBufferData.upload(context.device, mvpcMatrix);
      auto result = draw(context, frame);
      if(result == vk::Result::eSuboptimalKHR)
      {
        handleSurfaceChange(context, vertexBufferData, frame);
      }

    }

    context.device.waitIdle();
    uniformBufferData.clear( context.device );
    vertexBufferData.clear( context.device );

    /* VULKAN_KEY_END */
    tearDown(frame, context);
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