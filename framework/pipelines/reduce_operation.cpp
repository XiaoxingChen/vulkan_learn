#include "reduce_operation.h"
#include "utils/resource_manager.h"

namespace pip
{

ReduceOperation::ReduceOperation(/* args */)
{
}

ReduceOperation::~ReduceOperation()
{
}

void ReduceOperation::prepare(
	const vk::PhysicalDevice& physicalDevice,
	const vk::Device& device,
	const std::string& shaderFilename,
	const std::string& shaderMacro,
    VkDeviceSize elementSize)
{
    elementSize_ = elementSize;
	device_ = device;
    physicalDevice_ = physicalDevice;
    std::vector<vk::DescriptorType> descriptors{vk::DescriptorType::eStorageBuffer};

    std::vector<std::tuple<vk::DescriptorType, uint32_t, vk::ShaderStageFlags>> bindingData;
    std::vector<vk::DescriptorPoolSize> poolSizes;

    for(auto & descriptor : descriptors)
    {
        bindingData.push_back({ descriptor, 1, vk::ShaderStageFlagBits::eCompute });
        poolSizes.push_back({descriptor, 1});
    }

    descriptorSetLayout_ = vk::su::createDescriptorSetLayout(device_, bindingData );

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

}

void ReduceOperation::configIO(
    uint32_t elementNum,const vk::Buffer& inputBuffer, const vk::Buffer& outputBuffer, VkDeviceSize outputBufferOffset)
{
    elementNum_ = elementNum;
    pStagingBuffer_ = std::make_shared<vk::su::BufferData>(
        physicalDevice_, device_, elementSize_ * elementNum_,
        vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc | vk::BufferUsageFlagBits::eTransferDst );
	vk::su::updateDescriptorSets(device_, descriptorSet_,
        { { vk::DescriptorType::eStorageBuffer, pStagingBuffer_->buffer, {} } }, {});

    inputBuffer_ = inputBuffer;
    outputBuffer_ = outputBuffer;
    outputBufferOffset_ = outputBufferOffset;
}

void ReduceOperation::record(vk::CommandBuffer& commandBuffer) const
{
    commandBuffer.copyBuffer(inputBuffer_, pStagingBuffer_->buffer, vk::BufferCopy());
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, self_);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, layout_, 0, descriptorSet_, nullptr);
    commandBuffer.dispatch(
        (uint32_t)ceil(elementNum_ / float(256)), 1, 1);
    commandBuffer.copyBuffer(pStagingBuffer_->buffer, outputBuffer_, vk::BufferCopy{0, outputBufferOffset_, elementSize_});
}

void ReduceOperation::tearDown()
{
    if(pStagingBuffer_)
    {
        pStagingBuffer_->clear(device_);
        pStagingBuffer_.reset();
    }

    ComputePipelineResource::tearDown();
}

} // namespace pip
