#if !defined(__PIPELINE_RESOURCE_H__)
#define __PIPELINE_RESOURCE_H__

#include "vulkan/vulkan.hpp"
#include "utils.hpp"
#include "shaders.hpp"
#include "SPIRV/GlslangToSpv.h"
#include "logging.h"

class ComputePipelineResource
{
public:

    vk::Pipeline self;
    vk::PipelineLayout layout;
    vk::DescriptorSetLayout descriptorSetLayout;
    vk::DescriptorSet descriptorSet;
    vk::PipelineCache cache;
    vk::DescriptorPool descriptorPool;
    vk::ShaderModule computeShaderModule;
    vk::Device device = nullptr;

    void prepare(const vk::Device& device, const std::vector<vk::DescriptorType>& descriptors, const std::string& shaderName, const std::string& shaderMacros);
    void tearDown();
    virtual void record(vk::CommandBuffer& commandBuffer) const {}

};

#endif // __PIPELINE_RESOURCE_H__
