#if !defined(__PIPELINE_RESOURCE_H__)
#define __PIPELINE_RESOURCE_H__

#include "vulkan/vulkan.hpp"
#include "utils.hpp"
#include "shaders.hpp"
#include "SPIRV/GlslangToSpv.h"
#include "logging.h"

class ComputePipelineResource
{
protected:
    vk::Pipeline self_;
    vk::PipelineLayout layout_;
    vk::DescriptorSetLayout descriptorSetLayout_;
    vk::DescriptorSet descriptorSet_;
    vk::PipelineCache cache_;
    vk::DescriptorPool descriptorPool_;
    vk::ShaderModule computeShaderModule_;
    vk::Device device_ = nullptr;

public:
    void prepare(const vk::Device& device, const std::vector<vk::DescriptorType>& descriptors, const std::string& shaderName, const std::string& shaderMacros);
    void tearDown();
    virtual void record(vk::CommandBuffer& commandBuffer) const {}

};

#endif // __PIPELINE_RESOURCE_H__
