#include "utils.hpp"
#include "vulkan/vulkan.hpp"
#include "logging.h"
#include "SPIRV/GlslangToSpv.h"
#include "shaders.hpp"
#include "shader_bank.h"

#include <iostream>

int main(int argc, char const *argv[])
{
    LOGD("{}:{}", __FILE__, __LINE__);
    vk::UniqueInstance instance = vk::su::createInstance( "simple_compute", "xx_engine" );
    LOGD("{}:{}", __FILE__, __LINE__);
#if !defined( NDEBUG )
    vk::UniqueDebugUtilsMessengerEXT debugUtilsMessenger = vk::su::createDebugUtilsMessenger( instance );
#endif
    LOGD("{}:{}", __FILE__, __LINE__);
    vk::PhysicalDevice physicalDevice = instance->enumeratePhysicalDevices().front();

    uint32_t computeQueueFamilyIndex =
      vk::su::findQueueFamilyIndex<vk::QueueFlagBits::eCompute>
      ( physicalDevice.getQueueFamilyProperties() );
    LOGD("{}:{}", __FILE__, __LINE__);
    vk::UniqueDevice device = vk::su::createDevice( physicalDevice, computeQueueFamilyIndex );

    /* VULKAN_HPP_KEY_START */
    LOGD("{}:{}", __FILE__, __LINE__);
    std::vector<double> testData{1., 2., 3., 4.};
    LOGI("testData.size: {}, addr: {}", testData.size(), ((void**)&testData)[0]);

    vk::su::BufferData storageBufferData(
      physicalDevice, device, sizeof(testData[0]) * testData.size() , vk::BufferUsageFlagBits::eStorageBuffer );
    LOGD("{}:{}", __FILE__, __LINE__);
    LOGI("testData.size: {}, data: {}, addr: {}", testData.size(), (void*) testData.data(), ((void**)&testData)[0]);
    vk::su::copyToDevice(
      device, storageBufferData.deviceMemory, testData.data(), testData.size());
    LOGD("{}:{}", __FILE__, __LINE__);
    vk::UniqueDescriptorSetLayout descriptorSetLayout = vk::su::createDescriptorSetLayout(
      device, { { vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute } } );

    /* VULKAN_HPP_KEY_START */
    LOGD("{}:{}", __FILE__, __LINE__);
    // create a descriptor pool
    vk::DescriptorPoolSize   poolSize( vk::DescriptorType::eStorageBuffer, 1 );
    vk::UniqueDescriptorPool descriptorPool = device->createDescriptorPoolUnique(
      vk::DescriptorPoolCreateInfo( vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet, 1, poolSize ) );

    LOGD("{}:{}", __FILE__, __LINE__);
    // allocate a descriptor set
    vk::UniqueDescriptorSet descriptorSet = std::move(
      device->allocateDescriptorSetsUnique( vk::DescriptorSetAllocateInfo( *descriptorPool, *descriptorSetLayout ) )
        .front() );

    vk::DescriptorBufferInfo descriptorBufferInfo( storageBufferData.buffer.get(), 0, sizeof( testData[0] ) * testData.size() );
    device->updateDescriptorSets(
      vk::WriteDescriptorSet( descriptorSet.get(), 0, 0, vk::DescriptorType::eUniformBuffer, {}, descriptorBufferInfo ),
      {} );
    LOGD("{}:{}", __FILE__, __LINE__);
    // create a UniqueCommandPool to allocate a CommandBuffer from
    vk::UniqueCommandPool commandPool = device->createCommandPoolUnique(
      vk::CommandPoolCreateInfo( vk::CommandPoolCreateFlags(), computeQueueFamilyIndex ) );
    LOGD("{}:{}", __FILE__, __LINE__);
    // allocate a CommandBuffer from the CommandPool
    vk::UniqueCommandBuffer commandBuffer = std::move( device
                                                         ->allocateCommandBuffersUnique( vk::CommandBufferAllocateInfo(
                                                           commandPool.get(), vk::CommandBufferLevel::ePrimary, 1 ) )
                                                         .front() );

    glslang::InitializeProcess();

    std::vector<unsigned int> computeShaderSPV;
    auto shaderSrc = shader_bank::read("test.comp");
    bool ok = vk::su::GLSLtoSPV( vk::ShaderStageFlagBits::eCompute, shaderSrc, computeShaderSPV );
    assert( ok );

    vk::ShaderModuleCreateInfo computeShaderModuleCreateInfo( vk::ShaderModuleCreateFlags(), computeShaderSPV );
    vk::UniqueShaderModule     computeShaderModule = device->createShaderModuleUnique( computeShaderModuleCreateInfo );


    glslang::FinalizeProcess();

    LOGI("finished...");
    return 0;
}
