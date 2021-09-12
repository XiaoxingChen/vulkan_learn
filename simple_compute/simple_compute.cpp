#include "utils.hpp"
#include "vulkan/vulkan.hpp"
#include "logging.h"
#include "SPIRV/GlslangToSpv.h"
#include "shaders.hpp"
#include "shader_bank.h"
#include "resource_manager.h"

#include <iostream>

struct ComputeSampleContext
{
  vk::Instance instance;
  vk::DebugUtilsMessengerEXT debugUtilsMessenger;
  vk::PhysicalDevice physicalDevice;
  vk::Device device;
  int32_t computeQueueFamilyIndex = -1;
  vk::Queue computeQueue;
  vk::PipelineLayout pipelineLayout;
  vk::Pipeline computePipeline;
  vk::CommandPool commandPool;
  vk::DescriptorSet descriptorSet;
  vk::PipelineCache pipelineCache;
  vk::DescriptorPool descriptorPool;
  vk::ShaderModule computeShaderModule;
  vk::DescriptorSetLayout descriptorSetLayout;
};

void prepare(ComputeSampleContext& context)
{
    LOGI("{}:{}", __FILE__, __LINE__);
    context.instance = vk::su::createInstance( "simple_compute", "xx_engine", {"VK_LAYER_KHRONOS_validation"} );
    LOGI("{}:{}", __FILE__, __LINE__);
#if !defined( NDEBUG )
    LOGI("{}:{}", __FILE__, __LINE__);
    context.debugUtilsMessenger =
      context.instance.createDebugUtilsMessengerEXT( vk::su::makeDebugUtilsMessengerCreateInfoEXT() );
#endif
    LOGI("{}:{}", __FILE__, __LINE__);
    context.physicalDevice = context.instance.enumeratePhysicalDevices().front();

    context.computeQueueFamilyIndex =
      vk::su::findQueueFamilyIndex( context.physicalDevice.getQueueFamilyProperties(), vk::QueueFlagBits::eCompute );
    LOGI("{}:{}", __FILE__, __LINE__);
    context.device = vk::su::createDevice( context.physicalDevice, context.computeQueueFamilyIndex );
    context.computeQueue = context.device.getQueue(context.computeQueueFamilyIndex, 0);

    context.descriptorSetLayout = vk::su::createDescriptorSetLayout(
      context.device, { { vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute } } );

    /* VULKAN_HPP_KEY_START */
    LOGI("{}:{}", __FILE__, __LINE__);
    // create a descriptor pool
    context.descriptorPool = vk::su::createDescriptorPool( context.device, { { vk::DescriptorType::eStorageBuffer, 1 } } );

    LOGI("{}:{}", __FILE__, __LINE__);
    // allocate a descriptor set
    context.descriptorSet = std::move(
      context.device.allocateDescriptorSets( vk::DescriptorSetAllocateInfo( context.descriptorPool, context.descriptorSetLayout ) )
        .front() );
    LOGI("{}:{}", __FILE__, __LINE__);

    LOGI("{}:{}", __FILE__, __LINE__);
    // create a CommandPool to allocate a CommandBuffer from
    context.commandPool = context.device.createCommandPool(
      vk::CommandPoolCreateInfo( vk::CommandPoolCreateFlags(), context.computeQueueFamilyIndex ) );

    glslang::InitializeProcess();
    context.computeShaderModule = vk::su::createShaderModule(context.device, vk::ShaderStageFlagBits::eCompute, readShaderSource("test.comp"));
    glslang::FinalizeProcess();

    context.pipelineLayout = context.device.createPipelineLayout(
      vk::PipelineLayoutCreateInfo( vk::PipelineLayoutCreateFlags(), context.descriptorSetLayout )
    );

    context.computePipeline = vk::su::createComputePipeline(context.device, context.computeShaderModule, context.pipelineLayout);
}

void tearDown(ComputeSampleContext& context)
{
  context.device.destroyCommandPool( context.commandPool );
  context.device.destroyPipeline( context.computePipeline );
  context.device.destroyPipelineCache( context.pipelineCache );
  context.device.destroyDescriptorPool( context.descriptorPool );

  context.device.destroyShaderModule( context.computeShaderModule );
  context.device.destroyPipelineLayout( context.pipelineLayout );
  context.device.destroyDescriptorSetLayout( context.descriptorSetLayout );

  context.device.destroy();
#if !NDEBUG
  context.instance.destroyDebugUtilsMessengerEXT( context.debugUtilsMessenger );
#endif
  context.instance.destroy();
}

vk::CommandBuffer createCommandBuffer(const ComputeSampleContext& context)
{

}
const int WIDTH = 4; // Size of rendered mandelbrot set.
const int HEIGHT = 4; // Size of renderered mandelbrot set.
const int WORKGROUP_SIZE = 32; // Workgroup size in compute shader.

int main(int argc, char const *argv[])
{
    ComputeSampleContext context;
    prepare(context);
    /* VULKAN_HPP_KEY_START */
    LOGI("{}:{}", __FILE__, __LINE__);
    // std::vector<float> testData(WIDTH * HEIGHT);
    // LOGI("testData.size: {}, addr: {}", testData.size(), ((void**)&testData)[0]);

    size_t storageBufferSize = sizeof(float) * WIDTH * HEIGHT * 4;

    vk::su::BufferData storageBufferData(
      context.physicalDevice, context.device, storageBufferSize, vk::BufferUsageFlagBits::eStorageBuffer );
    LOGI("{}:{}", __FILE__, __LINE__);

    vk::DescriptorBufferInfo descriptorBufferInfo( storageBufferData.buffer, 0, storageBufferSize);
    context.device.updateDescriptorSets(
      vk::WriteDescriptorSet( context.descriptorSet, 0, 0, vk::DescriptorType::eStorageBuffer, {}, descriptorBufferInfo ),
      {} );

    LOGI("{}:{}", __FILE__, __LINE__);
    // allocate a CommandBuffer from the CommandPool
    vk::CommandBuffer commandBuffer = std::move( context.device.allocateCommandBuffers( vk::CommandBufferAllocateInfo(
                                                           context.commandPool, vk::CommandBufferLevel::ePrimary, 1 ) )
                                                         .front() );
    // std::cout << "shader: \n" << readShaderSource("test.comp") << std::endl;


    // command buffer
    commandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlags()));
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, context.computePipeline);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, context.pipelineLayout, 0, context.descriptorSet, nullptr);
    commandBuffer.dispatch((uint32_t)ceil(WIDTH / float(WORKGROUP_SIZE)), (uint32_t)ceil(HEIGHT / float(WORKGROUP_SIZE)), 1);
    // commandBuffer.dispatch(2,2, 1);
    commandBuffer.end();
    // finish command buffer recording

    auto submitInfo = vk::SubmitInfo(nullptr, nullptr, commandBuffer);
    auto fence = context.device.createFence(vk::FenceCreateInfo());
    context.computeQueue.submit(submitInfo, fence);
    while ( vk::Result::eTimeout == context.device.waitForFences( fence, VK_TRUE, vk::su::FenceTimeout ) );
    auto computeResult = storageBufferData.download<float>(context.device, storageBufferSize);
    for(const auto & val : computeResult)
    {
      std::cout << val << " ";
    } std::cout << "\n";
    context.device.waitIdle();
    storageBufferData.clear(context.device);
    context.device.destroyFence(fence);

    tearDown(context);

    LOGI("finished...");
    return 0;
}
