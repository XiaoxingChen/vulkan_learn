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
struct SampleContext
{
  vk::Instance instance;
  vk::DebugUtilsMessengerEXT debugUtilsMessenger;
  vk::PhysicalDevice physicalDevice;
  vk::Device device;
  int32_t graphicsQueueIndex = -1;
  vk::Queue graphicsQueue;
  vk::Queue presentQueue;
  vk::su::SwapChainData swapChainData;
  std::shared_ptr<vk::su::SurfaceData> pSurfaceData = nullptr;
  std::shared_ptr<vk::su::DepthBufferData> pDepthBuffer = nullptr;
  vk::PipelineLayout pipelineLayout;
  vk::RenderPass renderPass;
  vk::Pipeline graphicsPipeline;
  std::vector<vk::Framebuffer> framebuffers;
  vk::CommandPool commandPool;
  vk::DescriptorSet descriptorSet;
  vk::PipelineCache pipelineCache;
  vk::DescriptorPool descriptorPool;
  vk::ShaderModule vertexShaderModule;
  vk::ShaderModule fragmentShaderModule;
  vk::DescriptorSetLayout descriptorSetLayout;
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

void prepare(SampleContext& context, const char* EngineName, const char* AppName);
void tearDown(SampleContext& context);
std::vector<vk::CommandBuffer> createCommandBuffers(
  const SampleContext& context,
  const ModelResource& modelResource);
void prepare(FrameResource& frame, SampleContext& context, ModelResource& modelResource);
void tearDown(FrameResource& frame, SampleContext& context);
vk::Result draw(SampleContext& context, FrameResource& frame);

void prepare(ModelResource& modelResource, const SampleContext& context);
void tearDown(ModelResource& modelResource, const SampleContext& context);

void handleSurfaceChange(SampleContext& context, const ModelResource& modelResource, FrameResource& frame);

#endif // _SAMPLE_UTILS_H_
