#include "pipeline_resource.h"
#include <tuple>
#include <vector>
#include "resource_manager.h"

void ComputePipelineResource::prepare(
    const vk::Device& device_,
    const std::vector<vk::DescriptorType>& descriptors,
    const std::string& shaderName,
    const std::string& shaderMacros)
{
    device = device_;
    auto & pipeline = *this;
    std::vector<std::tuple<vk::DescriptorType, uint32_t, vk::ShaderStageFlags>> bindingData;
    std::vector<vk::DescriptorPoolSize> poolSizes;

    for(auto & descriptor : descriptors)
    {
        bindingData.push_back({ descriptor, 1, vk::ShaderStageFlagBits::eCompute });
        poolSizes.push_back({descriptor, 1});
    }

    pipeline.descriptorSetLayout = vk::su::createDescriptorSetLayout(
    device, bindingData );

    pipeline.descriptorPool = vk::su::createDescriptorPool( device, poolSizes );
    pipeline.descriptorSet = std::move(
        device.allocateDescriptorSets( vk::DescriptorSetAllocateInfo( pipeline.descriptorPool, pipeline.descriptorSetLayout ) )
          .front() );
    glslang::InitializeProcess();
    pipeline.computeShaderModule = vk::su::createShaderModule(device, vk::ShaderStageFlagBits::eCompute,
      readShaderSource(shaderName), shaderMacros);
    glslang::FinalizeProcess();
    pipeline.layout = device.createPipelineLayout(
        vk::PipelineLayoutCreateInfo( vk::PipelineLayoutCreateFlags(), pipeline.descriptorSetLayout )
      );

    pipeline.self = vk::su::createComputePipeline(device, pipeline.computeShaderModule, pipeline.layout);
}

void ComputePipelineResource::tearDown()
{
  if(!device) return;
  auto & pipeline = *this;
  device.destroyPipeline( pipeline.self );
  device.destroyPipelineCache( pipeline.cache );
  device.destroyDescriptorPool( pipeline.descriptorPool );
  device.destroyShaderModule( pipeline.computeShaderModule );
  device.destroyPipelineLayout( pipeline.layout );
  device.destroyDescriptorSetLayout( pipeline.descriptorSetLayout );
  device = nullptr;
}