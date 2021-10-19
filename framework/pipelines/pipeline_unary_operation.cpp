#include "pipeline_unary_operation.h"
#include "utils/resource_manager.h"

namespace pip
{

void UnaryOperation::prepare(const vk::PhysicalDevice& physicalDevice,
    const vk::Device& device_,
    uint32_t elementNum,
    const std::string& shaderFilename,
    const std::string& shaderMacro)
{
    device = device_;
	outputVectorSize_ = elementNum;
    std::vector<vk::DescriptorType> descriptors{
      vk::DescriptorType::eStorageBuffer,
      vk::DescriptorType::eUniformBuffer,
      vk::DescriptorType::eStorageBuffer};

    std::vector<std::tuple<vk::DescriptorType, uint32_t, vk::ShaderStageFlags>> bindingData;
    std::vector<vk::DescriptorPoolSize> poolSizes;

    for(auto & descriptor : descriptors)
    {
        bindingData.push_back({ descriptor, 1, vk::ShaderStageFlagBits::eCompute });
        poolSizes.push_back({descriptor, 1});
    }

    descriptorSetLayout = vk::su::createDescriptorSetLayout(
    device, bindingData );

    descriptorPool = vk::su::createDescriptorPool( device, poolSizes );
    descriptorSet = std::move(
        device.allocateDescriptorSets( vk::DescriptorSetAllocateInfo( descriptorPool, descriptorSetLayout ) )
          .front() );
    glslang::InitializeProcess();
    computeShaderModule = vk::su::createShaderModule(
      device, vk::ShaderStageFlagBits::eCompute, readShaderSource(shaderFilename), shaderMacro);
    glslang::FinalizeProcess();
    layout = device.createPipelineLayout(
        vk::PipelineLayoutCreateInfo( vk::PipelineLayoutCreateFlags(), descriptorSetLayout )
      );

    self = vk::su::createComputePipeline(device, computeShaderModule, layout);

    {
      pConstBuffer = std::make_shared<vk::su::BufferData>(
      physicalDevice, device, sizeof(uint32_t), vk::BufferUsageFlagBits::eUniformBuffer );
      pConstBuffer->upload(device, elementNum);
    }
}

void UnaryOperation::record(vk::CommandBuffer& commandBuffer) const
{
  commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, self);
  commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, layout, 0, descriptorSet, nullptr);
  commandBuffer.dispatch(
      (uint32_t)ceil(outputVectorSize_ / float(256)), 1, 1);
}

void UnaryOperation::updateDescriptorSets(const vk::su::BufferData& inputBuffer, const vk::su::BufferData& outputBuffer)
{
  vk::su::updateDescriptorSets(device, descriptorSet,
  {
    { vk::DescriptorType::eStorageBuffer, inputBuffer.buffer, {} },
    { vk::DescriptorType::eUniformBuffer, pConstBuffer->buffer, {} },
    { vk::DescriptorType::eStorageBuffer, outputBuffer.buffer, {} }
  }, {});
}

void UnaryOperation::tearDown()
{
  if(pConstBuffer)
  {
    pConstBuffer->clear(device);
    pConstBuffer.reset();
  }

  ComputePipelineResource::tearDown();
}

}//namespace pip