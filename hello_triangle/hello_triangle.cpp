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

#include "geometries.hpp"
#include "math.hpp"
#include "shaders.hpp"
#include "utils.hpp"
#include "SPIRV/GlslangToSpv.h"
#include "vulkan/vulkan.hpp"
#include "logging.h"

#include <iostream>
#include <thread>
#include <memory>

#define DEPTH_BUFFER_DATA 0

static char const * AppName    = "15_DrawCube";
static char const * EngineName = "Vulkan.hpp";

struct SampleContext
{
  vk::Instance instance;
  vk::PhysicalDevice physicalDevice;
  vk::Device device;
  int32_t graphicsQueueIndex = -1;
  vk::Queue graphicsQueue;
  vk::Queue presentQueue;
  vk::su::SwapChainData swapChainData;
  std::shared_ptr<vk::su::SurfaceData> pSurfaceData = nullptr;
  vk::PipelineLayout pipelineLayout;
  vk::RenderPass renderPass;
  vk::Pipeline graphicsPipeline;
  std::vector<vk::Framebuffer> framebuffers;
  vk::DescriptorSet descriptorSet;
  vk::PipelineCache pipelineCache;
  vk::DescriptorPool descriptorPool;
  vk::ShaderModule vertexShaderModule;
  vk::ShaderModule fragmentShaderModule;
  vk::DescriptorSetLayout descriptorSetLayout;
};

void prepare(SampleContext& context)
{
  context.physicalDevice = context.instance.enumeratePhysicalDevices().front();

  context.pSurfaceData = std::make_shared<vk::su::SurfaceData>(context.instance, AppName, vk::Extent2D( 500, 500 ));

  std::pair<uint32_t, uint32_t> graphicsAndPresentQueueFamilyIndex =
    vk::su::findGraphicsAndPresentQueueFamilyIndex(context.physicalDevice, context.pSurfaceData->surface );
  context.graphicsQueueIndex = graphicsAndPresentQueueFamilyIndex.first;
  context.device =
    vk::su::createDevice(context.physicalDevice, graphicsAndPresentQueueFamilyIndex.first, vk::su::getDeviceExtensions() );

  context.graphicsQueue = context.device.getQueue( graphicsAndPresentQueueFamilyIndex.first, 0 );
  context.presentQueue  = context.device.getQueue( graphicsAndPresentQueueFamilyIndex.second, 0 );

  context.swapChainData = vk::su::SwapChainData(context.physicalDevice,
                                        context.device,
                                        context.pSurfaceData->surface,
                                        context.pSurfaceData->extent,
                                        vk::ImageUsageFlagBits::eColorAttachment |
                                          vk::ImageUsageFlagBits::eTransferSrc,
                                        {},
                                        graphicsAndPresentQueueFamilyIndex.first,
                                        graphicsAndPresentQueueFamilyIndex.second );

  context.descriptorSetLayout = vk::su::createDescriptorSetLayout(
    context.device, { { vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex } } );
  context.pipelineLayout = context.device.createPipelineLayout(
    vk::PipelineLayoutCreateInfo( vk::PipelineLayoutCreateFlags(), context.descriptorSetLayout ) );
  LOGI("{}:{}", __FILE__, __LINE__);

  context.renderPass = vk::su::createRenderPass(
    context.device,
    vk::su::pickSurfaceFormat(context.physicalDevice.getSurfaceFormatsKHR( context.pSurfaceData->surface ) ).format,
    vk::Format::eUndefined);

  LOGI("{}:{}", __FILE__, __LINE__);

  glslang::InitializeProcess();
  context.vertexShaderModule =
    vk::su::createShaderModule( context.device, vk::ShaderStageFlagBits::eVertex, vertexShaderText_PC_C );
  context.fragmentShaderModule =
    vk::su::createShaderModule( context.device, vk::ShaderStageFlagBits::eFragment, fragmentShaderText_C_C );
  glslang::FinalizeProcess();
  context.framebuffers = vk::su::createFramebuffers(
    context.device, context.renderPass, context.swapChainData.imageViews, nullptr, context.pSurfaceData->extent );

  context.descriptorPool =
    vk::su::createDescriptorPool( context.device, { { vk::DescriptorType::eUniformBuffer, 1 } } );
  vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo( context.descriptorPool, context.descriptorSetLayout );
  context.descriptorSet = context.device.allocateDescriptorSets( descriptorSetAllocateInfo ).front();

  context.pipelineCache = context.device.createPipelineCache( vk::PipelineCacheCreateInfo() );
  context.graphicsPipeline = vk::su::createGraphicsPipeline(
    context.device,
    context.pipelineCache,
    std::make_pair( context.vertexShaderModule, nullptr ),
    std::make_pair( context.fragmentShaderModule, nullptr ),
    sizeof( coloredCubeData[0] ),
    { { vk::Format::eR32G32B32A32Sfloat, 0 }, { vk::Format::eR32G32B32A32Sfloat, 16 } },
    vk::FrontFace::eClockwise,
    true,
    context.pipelineLayout,
    context.renderPass );
}

void tearDown(SampleContext& context)
{
  context.device.destroyPipeline( context.graphicsPipeline );
  context.device.destroyPipelineCache( context.pipelineCache );
  context.device.destroyDescriptorPool( context.descriptorPool );

  for ( auto framebuffer : context.framebuffers )
  {
    context.device.destroyFramebuffer( framebuffer );
  }
  context.device.destroyShaderModule( context.fragmentShaderModule );
  context.device.destroyShaderModule( context.vertexShaderModule );
  context.device.destroyRenderPass( context.renderPass );
  context.device.destroyPipelineLayout( context.pipelineLayout );
  context.device.destroyDescriptorSetLayout( context.descriptorSetLayout );

  // depthBufferData.clear( context.device );
  context.swapChainData.clear( context.device );
  context.device.destroy();
  context.instance.destroySurfaceKHR( context.pSurfaceData->surface );
}

std::vector<vk::CommandBuffer> createCommandBuffers(
  const SampleContext& context,
  const vk::CommandPool& commandPool,
  const vk::su::BufferData& vertexBufferData,
  size_t num)
{
  auto allocateInfo = vk::CommandBufferAllocateInfo( commandPool, vk::CommandBufferLevel::ePrimary, num );
  std::vector<vk::CommandBuffer> commandBuffers = context.device.allocateCommandBuffers( allocateInfo );
  for(size_t i = 0; i < commandBuffers.size(); i++)
  {
    auto & commandBuffer(commandBuffers.at(i));
    commandBuffer.begin( vk::CommandBufferBeginInfo( vk::CommandBufferUsageFlags() ) );
    std::array<vk::ClearValue, 2> clearValues;
    clearValues[0].color        = vk::ClearColorValue( std::array<float, 4>( { { 0.2f, 0.2f, 0.2f, 0.2f } } ) );
    clearValues[1].depthStencil = vk::ClearDepthStencilValue( 1.0f, 0 );

    vk::RenderPassBeginInfo renderPassBeginInfo( context.renderPass,
                                                 context.framebuffers[i],
                                                 vk::Rect2D( vk::Offset2D( 0, 0 ), context.pSurfaceData->extent ),
                                                 clearValues );

    commandBuffer.beginRenderPass( renderPassBeginInfo, vk::SubpassContents::eInline );
    commandBuffer.bindPipeline( vk::PipelineBindPoint::eGraphics, context.graphicsPipeline );
    commandBuffer.bindDescriptorSets( vk::PipelineBindPoint::eGraphics, context.pipelineLayout, 0, context.descriptorSet, nullptr );

    commandBuffer.bindVertexBuffers( 0, vertexBufferData.buffer, { 0 } );
    commandBuffer.setViewport( 0,
                               vk::Viewport( 0.0f,
                                             0.0f,
                                             static_cast<float>( context.pSurfaceData->extent.width ),
                                             static_cast<float>( context.pSurfaceData->extent.height ),
                                             0.0f,
                                             1.0f ) );
    commandBuffer.setScissor( 0, vk::Rect2D( vk::Offset2D( 0, 0 ), context.pSurfaceData->extent ) );

    commandBuffer.draw( 12 * 3, 1, 0, 0 );
    commandBuffer.endRenderPass();
    commandBuffer.end();
  }

  return commandBuffers;
}

int main( int /*argc*/, char ** /*argv*/ )
{
  initLogger();
  SampleContext context;

  try
  {
    LOGI("{}:{}", __FILE__, __LINE__);
    context.instance = vk::su::createInstance( AppName, EngineName, {}, vk::su::getInstanceExtensions() );
    LOGI("{}:{}", __FILE__, __LINE__);
#if !defined( NDEBUG )
    vk::DebugUtilsMessengerEXT debugUtilsMessenger =
      context.instance.createDebugUtilsMessengerEXT( vk::su::makeDebugUtilsMessengerCreateInfoEXT() );
#endif
  prepare(context);

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

    vk::CommandPool commandPool = vk::su::createCommandPool( context.device, context.graphicsQueueIndex );
#if 0
    vk::CommandBuffer commandBuffer =
      context.device.allocateCommandBuffers( vk::CommandBufferAllocateInfo( commandPool, vk::CommandBufferLevel::ePrimary, 1 ) )
        .front();
#endif
    auto commandBuffers = createCommandBuffers(context, commandPool, vertexBufferData, context.swapChainData.images.size());
    std::vector<vk::Fence> drawFences(commandBuffers.size());
    std::vector<vk::Semaphore> imageAcquiredSemaphores(commandBuffers.size());
    for(size_t i = 0; i < commandBuffers.size(); i++)
    {
      drawFences.at(i) = context.device.createFence( vk::FenceCreateInfo() );
      imageAcquiredSemaphores.at(i) = context.device.createSemaphore( vk::SemaphoreCreateInfo() );
    }
    auto & commandBuffer = commandBuffers.front();
    auto & drawFence = drawFences.front();
    auto & imageAcquiredSemaphore = imageAcquiredSemaphores.front();

    float angle = 0;

    while (!glfwWindowShouldClose(context.pSurfaceData->window.handle)) {
            glfwPollEvents();

      // Get the index of the next available swapchain image:

      vk::ResultValue<uint32_t> currentBuffer =
        context.device.acquireNextImageKHR( context.swapChainData.swapChain, vk::su::FenceTimeout, imageAcquiredSemaphore, nullptr );
      assert( currentBuffer.result == vk::Result::eSuccess );
      assert( currentBuffer.value < context.framebuffers.size() );

      angle += 0.05;
      glm::mat4x4 mvpcMatrix = vk::su::createModelViewProjectionClipMatrix( context.pSurfaceData->extent , angle);
      uniformBufferData.upload(context.device, mvpcMatrix);

      vk::PipelineStageFlags waitDestinationStageMask( vk::PipelineStageFlagBits::eColorAttachmentOutput );
      vk::SubmitInfo         submitInfo( imageAcquiredSemaphore, waitDestinationStageMask, commandBuffers.at(currentBuffer.value) );
      context.device.resetFences( drawFence);
      context.graphicsQueue.submit( submitInfo, drawFence );

      while ( vk::Result::eTimeout == context.device.waitForFences( drawFence, VK_TRUE, vk::su::FenceTimeout ) );
      // LOGI("{}:{}", __FILE__, __LINE__);
      vk::Result result =
        context.presentQueue.presentKHR( vk::PresentInfoKHR( {}, context.swapChainData.swapChain, currentBuffer.value ) );
      switch ( result )
      {
        case vk::Result::eSuccess: break;
        case vk::Result::eSuboptimalKHR:
          std::cout << "vk::Queue::presentKHR returned vk::Result::eSuboptimalKHR !\n";
          break;
        default: assert( false );  // an unexpected result is returned !
      }

    }

    context.device.waitIdle();
    uniformBufferData.clear( context.device );
    vertexBufferData.clear( context.device );

    // context.device.destroyFence( drawFence );
    // context.device.destroySemaphore( imageAcquiredSemaphore );
    for(size_t i = 0; i < commandBuffers.size(); i++)
    {
      context.device.destroyFence(drawFences.at(i));
      context.device.destroySemaphore( imageAcquiredSemaphores.at(i));
    }
    context.device.destroyCommandPool( commandPool );

    /* VULKAN_KEY_END */
    tearDown(context);
#if !defined( NDEBUG )
    context.instance.destroyDebugUtilsMessengerEXT( debugUtilsMessenger );
#endif
    context.instance.destroy();
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