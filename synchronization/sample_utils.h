#if !defined(_SAMPLE_UTILS_H)
#define _SAMPLE_UTILS_H

#include "utils.hpp"
#include "vulkan/vulkan.hpp"
#include <math.h> 
#include <iostream>

struct ComputeSampleContext
{
  vk::Instance instance;
  vk::DebugUtilsMessengerEXT debugUtilsMessenger;
  vk::PhysicalDevice physicalDevice;
  vk::Device device;

  std::vector<uint32_t> computeQueueFamilyIndices;
  std::vector<vk::Queue> computeQueues;
  std::vector<vk::CommandPool> commandPools;
};

struct ComputePipelineResource
{
  vk::Pipeline self;
  vk::PipelineLayout layout;
  vk::DescriptorSetLayout descriptorSetLayout;
  vk::DescriptorSet descriptorSet;
  vk::PipelineCache cache;
  vk::DescriptorPool descriptorPool;
  vk::ShaderModule computeShaderModule;
};

const int WIDTH = 1; // Size of rendered mandelbrot set.
const int HEIGHT = 32; // Size of renderered mandelbrot set.
const int WORKGROUP_SIZE = 32; // Workgroup size in compute shader.

void syncBetweenQueues(const ComputeSampleContext& context, const ComputePipelineResource& initPipeline, const ComputePipelineResource& timePipeline);

#endif // _SAMPLE_UTILS_H
