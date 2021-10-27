#include "pipeline_unary_operation.h"
#include "utils/resource_manager.h"

namespace pip
{

void UnaryOperation::prepare(const vk::PhysicalDevice& physicalDevice,
    const vk::Device& device,
    const std::string& shaderFilename,
    const std::string& shaderMacro)
{
    device_ = device;
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

    descriptorSetLayout_ = vk::su::createDescriptorSetLayout(
    device_, bindingData );

    descriptorPool_ = vk::su::createDescriptorPool( device_, poolSizes );
    descriptorSet_ = std::move(
        device_.allocateDescriptorSets( vk::DescriptorSetAllocateInfo( descriptorPool_, descriptorSetLayout_ ) )
          .front() );
    glslang::InitializeProcess();
    computeShaderModule_ = vk::su::createShaderModule(
      device_, vk::ShaderStageFlagBits::eCompute, readShaderSource(shaderFilename), shaderMacro);
    glslang::FinalizeProcess();
    layout_ = device_.createPipelineLayout(
        vk::PipelineLayoutCreateInfo( vk::PipelineLayoutCreateFlags(), descriptorSetLayout_ )
      );

    self_ = vk::su::createComputePipeline(device_, computeShaderModule_, layout_);

    {
      pConstBuffer_ = std::make_shared<vk::su::BufferData>(
      physicalDevice, device_, sizeof(uint32_t), vk::BufferUsageFlagBits::eUniformBuffer );
      // pConstBuffer_->upload(device_, elementNum);
    }
}

void UnaryOperation::record(vk::CommandBuffer& commandBuffer) const
{
  commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, self_);
  commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, layout_, 0, descriptorSet_, nullptr);
  commandBuffer.dispatch(
      (uint32_t)ceil(outputVectorSize_ / float(256)), 1, 1);
}

void UnaryOperation::configIO(const vk::su::BufferData& inputBuffer, const vk::su::BufferData& outputBuffer, uint32_t elementNum)
{
  outputVectorSize_ = elementNum;
  pConstBuffer_->upload(device_, elementNum);
  vk::su::updateDescriptorSets(device_, descriptorSet_,
  {
    { vk::DescriptorType::eStorageBuffer, inputBuffer.buffer, {} },
    { vk::DescriptorType::eUniformBuffer, pConstBuffer_->buffer, {} },
    { vk::DescriptorType::eStorageBuffer, outputBuffer.buffer, {} }
  }, {});
}

void UnaryOperation::tearDown()
{
  if(pConstBuffer_)
  {
    pConstBuffer_->clear(device_);
    pConstBuffer_.reset();
  }

  ComputePipelineResource::tearDown();
}

}//namespace pip