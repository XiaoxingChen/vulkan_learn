#include "pipeline_resource.h"
#include <tuple>
#include <vector>
#include "resource_manager.h"

void ComputePipelineResource::prepare(
    const vk::Device& device,
    const std::vector<vk::DescriptorType>& descriptors,
    const std::string& shaderName,
    const std::string& shaderMacros)
{
    device_ = device;
    // auto & pipeline = *this;
    std::vector<std::tuple<vk::DescriptorType, uint32_t, vk::ShaderStageFlags>> bindingData;
    std::vector<vk::DescriptorPoolSize> poolSizes;

    for(auto & descriptor : descriptors)
    {
        bindingData.push_back({ descriptor, 1, vk::ShaderStageFlagBits::eCompute });
        poolSizes.push_back({descriptor, 1});
    }

    descriptorSetLayout_ = vk::su::createDescriptorSetLayout(
    device_, bindingData );

    descriptorPool_ = vk::su::createDescriptorPool( device_, poolSizes );
    descriptorSet_ = std::move(
        device_.allocateDescriptorSets( vk::DescriptorSetAllocateInfo( descriptorPool_, descriptorSetLayout_ ) )
          .front() );
    glslang::InitializeProcess();
    computeShaderModule_ = vk::su::createShaderModule(device_, vk::ShaderStageFlagBits::eCompute,
      readShaderSource(shaderName), shaderMacros);
    glslang::FinalizeProcess();
    layout_ = device_.createPipelineLayout(
        vk::PipelineLayoutCreateInfo( vk::PipelineLayoutCreateFlags(), descriptorSetLayout_ )
      );

    self_ = vk::su::createComputePipeline(device_, computeShaderModule_, layout_);
}

void ComputePipelineResource::tearDown()
{
  if(!device_) return;
  // auto & pipeline = *this;
  device_.destroyPipeline( self_ );
  device_.destroyPipelineCache( cache_ );
  device_.destroyDescriptorPool( descriptorPool_ );
  device_.destroyShaderModule( computeShaderModule_ );
  device_.destroyPipelineLayout( layout_ );
  device_.destroyDescriptorSetLayout( descriptorSetLayout_ );
  device_ = nullptr;
}