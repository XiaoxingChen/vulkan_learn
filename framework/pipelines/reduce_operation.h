#if !defined(__REDUCE_OPERATION_H__)
#define __REDUCE_OPERATION_H__

#include "pipeline_resource.h"
#include "utils.hpp"

namespace pip
{
class ReduceOperation: public ComputePipelineResource
{
private:
    vk::PhysicalDevice physicalDevice_ = nullptr;
    std::shared_ptr<vk::su::BufferData> pStagingBuffer_ = nullptr;
    vk::Buffer inputBuffer_ = nullptr;
    vk::Buffer outputBuffer_ = nullptr;
    VkDeviceSize outputBufferOffset_ = 0;

    uint32_t elementNum_ = 0;
    VkDeviceSize elementSize_;

public:
    ReduceOperation(/* args */);

    void prepare(
        const vk::PhysicalDevice& physicalDevice,
        const vk::Device& device,
        const std::string& shaderFilename,
        const std::string& shaderMacro,
        VkDeviceSize elementSize = sizeof(float));
    void configIO(
        uint32_t elementNum,
        const vk::Buffer& inputBuffer,
        const vk::Buffer& outputBuffer,
        VkDeviceSize outputBufferOffset);
    virtual void record(vk::CommandBuffer& commandBuffer) const override;
    void tearDown();

    ~ReduceOperation();
};

template<typename DType>
ReduceOperation createReduceToFirst(
    const vk::PhysicalDevice& physicalDevice,
    const vk::Device& device)
{
    assert(false);
}

template<>
inline ReduceOperation createReduceToFirst<float>(
    const vk::PhysicalDevice& physicalDevice,
    const vk::Device& device)
{
    ReduceOperation ret;
    ret.prepare(physicalDevice, device, "framework/pipelines/shaders/reduce.comp", "", sizeof(float));
    return ret;
}

} // namespace pip



#endif // __REDUCE_OPERATION_H__
