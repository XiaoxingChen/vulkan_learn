#include "utils.hpp"
#include "vulkan/vulkan.hpp"
#include "logging.h"
#include "SPIRV/GlslangToSpv.h"
#include "shaders.hpp"
#include "resource_manager.h"
#include "synchronization/sample_utils.h"
#include <iostream>

void prepare(ComputeSampleContext& context)
{
    #ifdef __ANDROID__
    context.instance = vk::su::createInstance( "simple_compute", "xx_engine", {}, vk::su::getInstanceExtensions() );
    #else
    context.instance = vk::su::createInstance( "simple_compute", "xx_engine", {"VK_LAYER_KHRONOS_validation"}, vk::su::getInstanceExtensions() );
    #endif
    LOGI("{}:{}", __FILE__, __LINE__);
#if !defined( NDEBUG )
    LOGI("{}:{}", __FILE__, __LINE__);
    context.debugUtilsMessenger =
      context.instance.createDebugUtilsMessengerEXT( vk::su::makeDebugUtilsMessengerCreateInfoEXT() );
#endif
    LOGI("{}:{}", __FILE__, __LINE__);
    context.physicalDevice = context.instance.enumeratePhysicalDevices().front();

    context.computeQueueFamilyIndices =
      vk::su::findQueueFamilyIndices( context.physicalDevice.getQueueFamilyProperties(), vk::QueueFlagBits::eCompute );
    // assert(context.computeQueueFamilyIndices.size() > 1);
    // context.computeQueueFamilyIndices.resize(2);

    {
      vk::PhysicalDeviceFeatures const * physicalDeviceFeatures = nullptr;
      auto extensions = vk::su::getDeviceExtensions(context.physicalDevice);
      auto & device = context.device;
      std::vector<char const *> enabledExtensions;
      enabledExtensions.reserve( extensions.size() );
      for ( auto const & ext : extensions )
      {
        enabledExtensions.push_back( ext.data() );
      }

      float                     queuePriority = 0.0f;

      std::vector<vk::DeviceQueueCreateInfo> deviceQueueCreateInfos(context.computeQueueFamilyIndices.size());
      for(size_t i = 0;i < deviceQueueCreateInfos.size(); i++)
      {
        deviceQueueCreateInfos.at(i) = vk::DeviceQueueCreateInfo ( {}, context.computeQueueFamilyIndices.at(i), 1, &queuePriority );
      }


      vk::DeviceCreateInfo deviceCreateInfo( {}, deviceQueueCreateInfos, {}, enabledExtensions, physicalDeviceFeatures );
      deviceCreateInfo.pNext = nullptr;

      device = context.physicalDevice.createDevice( deviceCreateInfo );
#if ( VULKAN_HPP_DISPATCH_LOADER_DYNAMIC == 1 )
      // initialize function pointers for instance
      VULKAN_HPP_DEFAULT_DISPATCHER.init( device );
#endif
    }

    // context.device = vk::su::createDevice( context.physicalDevice, context.computeQueueFamilyIndex,  );
    for(size_t i = 0; i < context.computeQueueFamilyIndices.size(); i++)
    {
      context.computeQueues.push_back( context.device.getQueue(context.computeQueueFamilyIndices.at(i), 0));
      auto commandPoolCreateInfo = vk::CommandPoolCreateInfo( vk::CommandPoolCreateFlags(), context.computeQueueFamilyIndices.at(i) ) ;
      context.commandPools.push_back( context.device.createCommandPool( commandPoolCreateInfo ));
    }

}

void prepareUnaryInplaceCompute(ComputePipelineResource& pipeline, const ComputeSampleContext& context, const std::string& shaderName)
{
  pipeline.descriptorSetLayout = vk::su::createDescriptorSetLayout(
      context.device, { { vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute } } );
  pipeline.descriptorPool = vk::su::createDescriptorPool( context.device, { { vk::DescriptorType::eStorageBuffer, 1 } } );
  pipeline.descriptorSet = std::move(
      context.device.allocateDescriptorSets( vk::DescriptorSetAllocateInfo( pipeline.descriptorPool, pipeline.descriptorSetLayout ) )
        .front() );
  glslang::InitializeProcess();
  pipeline.computeShaderModule = vk::su::createShaderModule(context.device, vk::ShaderStageFlagBits::eCompute, readShaderSource(shaderName));
  glslang::FinalizeProcess();
  pipeline.layout = context.device.createPipelineLayout(
      vk::PipelineLayoutCreateInfo( vk::PipelineLayoutCreateFlags(), pipeline.descriptorSetLayout )
    );

  pipeline.self = vk::su::createComputePipeline(context.device, pipeline.computeShaderModule, pipeline.layout);
}

void tearDown(ComputeSampleContext& context)
{
  for(const auto & commandPool : context.commandPools)
    context.device.destroyCommandPool( commandPool );

  context.device.destroy();
#if !NDEBUG
  context.instance.destroyDebugUtilsMessengerEXT( context.debugUtilsMessenger );
#endif
  context.instance.destroy();
}

void tearDown(const ComputePipelineResource& pipeline, const ComputeSampleContext& context)
{
  context.device.destroyPipeline( pipeline.self );
  context.device.destroyPipelineCache( pipeline.cache );
  context.device.destroyDescriptorPool( pipeline.descriptorPool );
  context.device.destroyShaderModule( pipeline.computeShaderModule );
  context.device.destroyPipelineLayout( pipeline.layout );
  context.device.destroyDescriptorSetLayout( pipeline.descriptorSetLayout );
}

int main(int argc, char const *argv[])
{
    ComputeSampleContext context;
    ComputePipelineResource initPipeline;
    ComputePipelineResource timesPipeline;
    prepare(context);
    prepareUnaryInplaceCompute(initPipeline, context, "init_buffer.comp");
    prepareUnaryInplaceCompute(timesPipeline, context, "times_two.comp");
    /* VULKAN_HPP_KEY_START */
    LOGI("{}:{}", __FILE__, __LINE__);
    // std::vector<float> testData(WIDTH * HEIGHT);
    // LOGI("testData.size: {}, addr: {}", testData.size(), ((void**)&testData)[0]);

    size_t storageBufferSize = sizeof(float) * WIDTH * HEIGHT;

    vk::su::BufferData storageBufferData(
      context.physicalDevice, context.device, storageBufferSize, vk::BufferUsageFlagBits::eStorageBuffer );
    LOGI("{}:{}", __FILE__, __LINE__);

    vk::DescriptorBufferInfo descriptorBufferInfo( storageBufferData.buffer, 0, storageBufferSize);
    context.device.updateDescriptorSets(
      vk::WriteDescriptorSet( initPipeline.descriptorSet, 0, 0, vk::DescriptorType::eStorageBuffer, {}, descriptorBufferInfo ),
      {} );

    context.device.updateDescriptorSets(
      vk::WriteDescriptorSet( timesPipeline.descriptorSet, 0, 0, vk::DescriptorType::eStorageBuffer, {}, descriptorBufferInfo ),
      {} );

    LOGI("{}:{}", __FILE__, __LINE__);
    #if 0
    // allocate a CommandBuffer from the CommandPool
    vk::CommandBuffer commandBuffer = std::move( context.device.allocateCommandBuffers( vk::CommandBufferAllocateInfo(
                                                           context.commandPool, vk::CommandBufferLevel::ePrimary, 1 ) )
                                                         .front() );
    // std::cout << "shader: \n" << readShaderSource("test.comp") << std::endl;


    // command buffer
    commandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlags()));

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, initPipeline.self);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, initPipeline.layout, 0, initPipeline.descriptorSet, nullptr);
    commandBuffer.dispatch((uint32_t)ceil(WIDTH / float(WORKGROUP_SIZE)), (uint32_t)ceil(HEIGHT / float(WORKGROUP_SIZE)), 1);

    commandBuffer.end();

    // finish command buffer recording

    auto submitInfo = vk::SubmitInfo(nullptr, nullptr, commandBuffer);
    auto fence = context.device.createFence(vk::FenceCreateInfo());
    context.computeQueue.submit(submitInfo, fence);
    while ( vk::Result::eTimeout == context.device.waitForFences( fence, VK_TRUE, vk::su::FenceTimeout ) );


    vk::su::oneTimeSubmit(context.device, context.commandPools.at(1), context.computeQueues.at(1), [&](vk::CommandBuffer const & commandBuffer){
      commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, initPipeline.self);
      commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, initPipeline.layout, 0, initPipeline.descriptorSet, nullptr);
      commandBuffer.dispatch((uint32_t)ceil(WIDTH / float(WORKGROUP_SIZE)), (uint32_t)ceil(HEIGHT / float(WORKGROUP_SIZE)), 1);
    });

    vk::su::oneTimeSubmit(context.device, context.commandPools.at(0), context.computeQueues.at(0), [&](vk::CommandBuffer const & commandBuffer){
      commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, timesPipeline.self);
      commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, timesPipeline.layout, 0, timesPipeline.descriptorSet, nullptr);
      commandBuffer.dispatch((uint32_t)ceil(WIDTH / float(WORKGROUP_SIZE)), (uint32_t)ceil(HEIGHT / float(WORKGROUP_SIZE)), 1);
    });
    #endif
    syncBetweenQueues( context, initPipeline, timesPipeline);

    auto computeResult = storageBufferData.download<float>(context.device, storageBufferSize);
    for(const auto & val : computeResult)
    {
      std::cout << val << " ";
    } std::cout << "\n";
    context.device.waitIdle();
    storageBufferData.clear(context.device);
    // context.device.destroyFence(fence);

    tearDown(initPipeline, context);
    tearDown(timesPipeline, context);
    tearDown(context);

    LOGI("finished...");
    return 0;
}
