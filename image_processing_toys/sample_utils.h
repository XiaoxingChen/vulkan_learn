#if !defined(_SAMPLE_UTILS_H_)
#define _SAMPLE_UTILS_H_

#include "vulkan/vulkan.hpp"
#include "utils.hpp"
#include "geometries.hpp"
#include "math.hpp"
#include "shaders.hpp"
#include "SPIRV/GlslangToSpv.h"
#include "vulkan/vulkan.hpp"
#include "logging.h"

const uint32_t WORKGROUP_SIZE = 32;
struct GraphicsPipelineResource
{
  vk::Pipeline self;
  vk::PipelineLayout layout;
  vk::DescriptorSetLayout descriptorSetLayout;
  vk::DescriptorSet descriptorSet;
  vk::PipelineCache cache;
  vk::DescriptorPool descriptorPool;
  vk::ShaderModule vertexShaderModule;
  vk::ShaderModule fragmentShaderModule;
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

struct SampleContext
{
  vk::Instance instance;
  vk::DebugUtilsMessengerEXT debugUtilsMessenger;
  vk::PhysicalDevice physicalDevice;
  vk::Device device;
  int32_t graphicsQueueIndex = -1;
  int32_t computeQueueFamilyIndex = -1;
  vk::Queue computeQueue;
  vk::Queue graphicsQueue;
  vk::Queue presentQueue;
  vk::su::SwapChainData swapChainData;
  std::shared_ptr<vk::su::SurfaceData> pSurfaceData = nullptr;
  std::shared_ptr<vk::su::DepthBufferData> pDepthBuffer = nullptr;

  vk::RenderPass renderPass;
  std::vector<vk::Framebuffer> framebuffers;
  vk::CommandPool commandPool;
};

struct FrameResource
{
  size_t imageNum = 0;
  size_t counter = 0;
  decltype(std::chrono::steady_clock::now()) timeStamp;
  std::vector<vk::CommandBuffer> commandBuffers;
  std::vector<vk::Fence> drawFences;
  std::vector<vk::Semaphore> recycledSemaphores;
};

struct ModelResource
{
  std::shared_ptr<vk::su::BufferData> pVertexBuffer = nullptr;
  std::shared_ptr<vk::su::BufferData> pIndexBuffer = nullptr;
  std::shared_ptr<vk::su::TextureData> pTextureData = nullptr;
  std::shared_ptr<vk::su::PixelsImageGenerator> pTextureGenerator = nullptr;
  size_t indexNum = 0;
  float scale = 1.f;
};

struct BufferList
{
  std::shared_ptr<vk::su::BufferData> pComputeOutput = nullptr;
  std::shared_ptr<vk::su::BufferData> pComputeUniform = nullptr;
  void prepare(SampleContext& context, const ModelResource& modelResource);
  void tearDown(SampleContext& context);
};

struct ComputeUniformInfo{
  glm::ivec4 imgSize;
  float timestamp;
};

void prepare(SampleContext& context, const char* EngineName, const char* AppName);
void tearDown(SampleContext& context);
std::vector<vk::CommandBuffer> createCommandBuffers(
  const SampleContext& context,
  const GraphicsPipelineResource& pipe,
  const ModelResource& modelResource);
  
vk::CommandBuffer createCommandBuffer(
  const SampleContext& context,
  const ComputePipelineResource& pipeline,
  const ModelResource& modelResource);

void prepare(FrameResource& frame, SampleContext& context);
void tearDown(FrameResource& frame, SampleContext& context);
vk::Result draw(SampleContext& context, FrameResource& frame);

void prepare(ModelResource& modelResource, const SampleContext& context);
void tearDown(ModelResource& modelResource, const SampleContext& context);

void prepareRectangle(GraphicsPipelineResource& pipe, const SampleContext& context);
void tearDown(const GraphicsPipelineResource& pipe, const SampleContext& context);

void prepareCompute(ComputePipelineResource& pipe, const SampleContext& context);
void tearDown(const ComputePipelineResource& pipe, const SampleContext& context);

void handleSurfaceChange(SampleContext& context, const ModelResource& modelResource, FrameResource& frame, const GraphicsPipelineResource& pipe);

#endif // _SAMPLE_UTILS_H_
