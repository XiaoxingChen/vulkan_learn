#if !defined(__PIPELINE_U8_TO_F32_H__)
#define __PIPELINE_U8_TO_F32_H__
#include "pipeline_resource.h"
#include "utils.hpp"

namespace pip
{
class UnaryOperation: public ComputePipelineResource
{
public:
    UnaryOperation() {};
    void prepare(
        const vk::PhysicalDevice& physicalDevice,
        const vk::Device& device,
        uint32_t elementNum,
        const std::string& shaderFilename,
        const std::string& shaderMacro);
    void updateDescriptorSets(const vk::su::BufferData& inputBuffer, const vk::su::BufferData& outputBuffer);
    virtual void record(vk::CommandBuffer& commandBuffer) const override;
    void tearDown();

private:
    uint32_t outputVectorSize_ = 0;
    std::shared_ptr<vk::su::BufferData> pConstBuffer = nullptr;

};

inline UnaryOperation createU8C1ToU8C4(
    const vk::PhysicalDevice& physicalDevice,
    const vk::Device& device,
    uint32_t elementNum)
{
    UnaryOperation ret;
    ret.prepare(physicalDevice, device, elementNum, "framework/pipelines/shaders/u8c1_to_u8c4.comp", "");
    return ret;
}

} // namespace pip



#endif // __PIPELINE_U8_TO_F32_H__
