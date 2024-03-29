#include "model_loading/sample_utils.h"
#include "gltf_utils.h"
#include <stb_image.h>
#include "utils/resource_manager.h"

void prepare(SampleContext& context, const char* EngineName, const char* AppName)
{
  LOGI("{}:{}", __FILE__, __LINE__);
    context.instance = vk::su::createInstance( AppName, EngineName, {}, vk::su::getInstanceExtensions() );
    LOGI("{}:{}", __FILE__, __LINE__);
#if !NDEBUG
  context.debugUtilsMessenger =
      context.instance.createDebugUtilsMessengerEXT( vk::su::makeDebugUtilsMessengerCreateInfoEXT() );
  LOGI("{}:{}", __FILE__, __LINE__);
#endif

  context.physicalDevice = context.instance.enumeratePhysicalDevices().front();

  context.pSurfaceData = std::make_shared<vk::su::SurfaceData>(context.physicalDevice, context.instance, AppName, vk::Extent2D( 500, 500 ));

  std::pair<uint32_t, uint32_t> graphicsAndPresentQueueFamilyIndex =
    vk::su::findGraphicsAndPresentQueueFamilyIndex(context.physicalDevice, context.pSurfaceData->surface );
  context.graphicsQueueIndex = graphicsAndPresentQueueFamilyIndex.first;
  context.device =
    vk::su::createDevice(context.physicalDevice, graphicsAndPresentQueueFamilyIndex.first, vk::su::getDeviceExtensions(context.physicalDevice) );

  context.graphicsQueue = context.device.getQueue( graphicsAndPresentQueueFamilyIndex.first, 0 );
  context.presentQueue  = context.device.getQueue( graphicsAndPresentQueueFamilyIndex.second, 0 );

  context.swapChainData = vk::su::SwapChainData(context.physicalDevice,
                                        context.device,
                                        context.pSurfaceData->surface,
                                        context.pSurfaceData->extent,
                                        vk::ImageUsageFlagBits::eColorAttachment |
                                          vk::ImageUsageFlagBits::eTransferSrc,
                                        {},
                                        graphicsAndPresentQueueFamilyIndex.first,
                                        graphicsAndPresentQueueFamilyIndex.second );

  context.pDepthBuffer = std::make_shared<vk::su::DepthBufferData>(
      context.physicalDevice,
      context.device,
      vk::Format::eD16Unorm,
      context.pSurfaceData->extent);

  context.descriptorSetLayout = vk::su::createDescriptorSetLayout(
    context.device, {
      { vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex },
      { vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment }
      } );
  context.pipelineLayout = context.device.createPipelineLayout(
    vk::PipelineLayoutCreateInfo( vk::PipelineLayoutCreateFlags(), context.descriptorSetLayout ) );
  LOGI("{}:{}", __FILE__, __LINE__);

  context.renderPass = vk::su::createRenderPass(
    context.device,
    vk::su::pickSurfaceFormat(context.physicalDevice.getSurfaceFormatsKHR( context.pSurfaceData->surface ) ).format,
    context.pDepthBuffer->format);

  LOGI("{}:{}", __FILE__, __LINE__);

  glslang::InitializeProcess();
  context.vertexShaderModule =
    vk::su::createShaderModule( context.device, vk::ShaderStageFlagBits::eVertex, vertexShaderText_PT_T );
  context.fragmentShaderModule =
    vk::su::createShaderModule( context.device, vk::ShaderStageFlagBits::eFragment, fragmentShaderText_T_C );
  glslang::FinalizeProcess();
  context.framebuffers = vk::su::createFramebuffers(
    context.device, context.renderPass, context.swapChainData.imageViews, context.pDepthBuffer->imageView, context.pSurfaceData->extent );

  context.descriptorPool =
    vk::su::createDescriptorPool( context.device, {
      { vk::DescriptorType::eUniformBuffer, 1 },
      { vk::DescriptorType::eCombinedImageSampler, 1 }
      } );
  vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo( context.descriptorPool, context.descriptorSetLayout );
  context.descriptorSet = context.device.allocateDescriptorSets( descriptorSetAllocateInfo ).front();

  context.pipelineCache = context.device.createPipelineCache( vk::PipelineCacheCreateInfo() );
  context.graphicsPipeline = vk::su::createGraphicsPipeline(
    context.device,
    context.pipelineCache,
    std::make_pair( context.vertexShaderModule, nullptr ),
    std::make_pair( context.fragmentShaderModule, nullptr ),
    sizeof( float ) * 6,
    { { vk::Format::eR32G32B32A32Sfloat, 0 }, { vk::Format::eR32G32Sfloat, 16 } },
    vk::FrontFace::eCounterClockwise,
    true,
    context.pipelineLayout,
    context.renderPass );

  context.commandPool = vk::su::createCommandPool( context.device, context.graphicsQueueIndex );
}

void tearDown(SampleContext& context)
{
  context.device.destroyCommandPool( context.commandPool );
  context.device.destroyPipeline( context.graphicsPipeline );
  context.device.destroyPipelineCache( context.pipelineCache );
  context.device.destroyDescriptorPool( context.descriptorPool );

  for ( auto framebuffer : context.framebuffers )
  {
    context.device.destroyFramebuffer( framebuffer );
  }
  context.device.destroyShaderModule( context.fragmentShaderModule );
  context.device.destroyShaderModule( context.vertexShaderModule );
  context.device.destroyRenderPass( context.renderPass );
  context.device.destroyPipelineLayout( context.pipelineLayout );
  context.device.destroyDescriptorSetLayout( context.descriptorSetLayout );

  context.pDepthBuffer->clear( context.device );
  context.swapChainData.clear( context.device );
  context.device.destroy();
  context.instance.destroySurfaceKHR( context.pSurfaceData->surface );
#if !NDEBUG
  context.instance.destroyDebugUtilsMessengerEXT( context.debugUtilsMessenger );
#endif
  context.instance.destroy();
}

std::vector<vk::CommandBuffer> createCommandBuffers(
  const SampleContext& context,
  const ModelResource& modelResource)
{
  size_t num = context.swapChainData.imageViews.size();
  const vk::CommandPool& commandPool = context.commandPool;

  auto allocateInfo = vk::CommandBufferAllocateInfo( commandPool, vk::CommandBufferLevel::ePrimary, num );
  std::vector<vk::CommandBuffer> commandBuffers = context.device.allocateCommandBuffers( allocateInfo );
  for(size_t i = 0; i < commandBuffers.size(); i++)
  {
    auto & commandBuffer(commandBuffers.at(i));
    commandBuffer.begin( vk::CommandBufferBeginInfo( vk::CommandBufferUsageFlags() ) );

    std::array<vk::ClearValue, 2> clearValues;
    clearValues[0].color        = vk::ClearColorValue( std::array<float, 4>( { { 0.2f, 0.2f, 0.2f, 0.2f } } ) );
    clearValues[1].depthStencil = vk::ClearDepthStencilValue( 1.0f, 0 );

    vk::RenderPassBeginInfo renderPassBeginInfo( context.renderPass,
                                                 context.framebuffers[i],
                                                 vk::Rect2D( vk::Offset2D( 0, 0 ), context.pSurfaceData->extent ),
                                                 clearValues );

    commandBuffer.beginRenderPass( renderPassBeginInfo, vk::SubpassContents::eInline );
    commandBuffer.bindPipeline( vk::PipelineBindPoint::eGraphics, context.graphicsPipeline );
    commandBuffer.bindDescriptorSets( vk::PipelineBindPoint::eGraphics, context.pipelineLayout, 0, context.descriptorSet, nullptr );

    commandBuffer.bindVertexBuffers( 0, modelResource.pVertexBuffer->buffer, { 0 } );
    commandBuffer.bindIndexBuffer(modelResource.pIndexBuffer->buffer, 0, vk::IndexType::eUint16);
    commandBuffer.setViewport( 0,
                               vk::Viewport( 0.0f,
                                             0.0f,
                                             static_cast<float>( context.pSurfaceData->extent.width ),
                                             static_cast<float>( context.pSurfaceData->extent.height ),
                                             0.0f,
                                             1.0f ) );
    commandBuffer.setScissor( 0, vk::Rect2D( vk::Offset2D( 0, 0 ), context.pSurfaceData->extent ) );

    // commandBuffer.draw( 12 * 3, 1, 0, 0 );
    commandBuffer.drawIndexed(modelResource.indexNum, 1, 0, 0, 0);
    commandBuffer.endRenderPass();
    commandBuffer.end();
  }

  return commandBuffers;
}

void prepare(FrameResource& frame, SampleContext& context, ModelResource& modelResource)
{
  frame.imageNum = context.swapChainData.images.size();
  frame.commandBuffers = createCommandBuffers(context, modelResource);
  frame.drawFences.resize(frame.imageNum);
  frame.recycledSemaphores.resize(frame.imageNum);
  for(size_t i = 0; i < frame.imageNum; i++)
  {
    frame.drawFences.at(i) = context.device.createFence( vk::FenceCreateInfo() );
    frame.recycledSemaphores.at(i) = context.device.createSemaphore( vk::SemaphoreCreateInfo() );
  }
}

void tearDown(FrameResource& frame, SampleContext& context)
{
    for(size_t i = 0; i < frame.imageNum; i++)
    {
      context.device.destroyFence(frame.drawFences.at(i));
    }
    for(size_t i = 0; i < frame.recycledSemaphores.size(); i++)
    {
      context.device.destroySemaphore( frame.recycledSemaphores.at(i));
    }
}

void handleSurfaceChange(SampleContext& context, const ModelResource& modelResource, FrameResource& frame)
{
  auto surfaceCapabilities = context.physicalDevice.getSurfaceCapabilitiesKHR(context.pSurfaceData->surface);
  const auto & newExtent = surfaceCapabilities.currentExtent;
  if (newExtent.width == 0xFFFFFFFF) return;
  if(newExtent.width == context.pSurfaceData->extent.width &&
    newExtent.height == context.pSurfaceData->extent.height) return;

  context.device.waitIdle();
  for ( auto framebuffer : context.framebuffers )
  {
    context.device.destroyFramebuffer( framebuffer );
  }
  context.swapChainData.clear( context.device );
  context.framebuffers.clear();
  context.pDepthBuffer->clear( context.device );

  context.pSurfaceData->extent = newExtent;
  context.swapChainData = vk::su::SwapChainData(context.physicalDevice,
                                        context.device,
                                        context.pSurfaceData->surface,
                                        context.pSurfaceData->extent,
                                        vk::ImageUsageFlagBits::eColorAttachment |
                                          vk::ImageUsageFlagBits::eTransferSrc,
                                        {},
                                        context.graphicsQueueIndex,
                                        context.graphicsQueueIndex);

  context.pDepthBuffer = std::make_shared<vk::su::DepthBufferData>(
      context.physicalDevice,
      context.device,
      vk::Format::eD16Unorm,
      context.pSurfaceData->extent);
  context.framebuffers = vk::su::createFramebuffers(
    context.device, context.renderPass, context.swapChainData.imageViews, context.pDepthBuffer->imageView, context.pSurfaceData->extent );
  frame.commandBuffers = createCommandBuffers(context, modelResource);
}

vk::Result draw(SampleContext& context, FrameResource& frame)
{
  vk::Semaphore imageAcquiredSemaphore;
  if(frame.recycledSemaphores.empty())
  {
    imageAcquiredSemaphore = context.device.createSemaphore(vk::SemaphoreCreateInfo());
  }else
  {
    imageAcquiredSemaphore = frame.recycledSemaphores.back();
    frame.recycledSemaphores.pop_back();
  }
  // Get the index of the next available swapchain image:
  vk::ResultValue<uint32_t> currentBuffer(vk::Result::eSuccess, 0);
  try
  {
    currentBuffer =
      context.device.acquireNextImageKHR( context.swapChainData.swapChain, vk::su::FenceTimeout, imageAcquiredSemaphore, nullptr );
  }
  catch(const vk::SystemError& e)
  {
    std::cerr << e.what() << ", code: " << e.code().value() << '\n';
    currentBuffer.result = static_cast<vk::Result>(e.code().value());
  }
  if(currentBuffer.result != vk::Result::eSuccess)
  {
    context.device.destroySemaphore(imageAcquiredSemaphore);
    return currentBuffer.result;
  }

  // assert( currentBuffer.value < context.framebuffers.size() );

  vk::PipelineStageFlags waitDestinationStageMask( vk::PipelineStageFlagBits::eColorAttachmentOutput );
  vk::SubmitInfo         submitInfo( imageAcquiredSemaphore, waitDestinationStageMask, frame.commandBuffers.at(currentBuffer.value) );
  auto & drawFence = frame.drawFences.at(currentBuffer.value);
  context.device.resetFences( drawFence);
  context.graphicsQueue.submit( submitInfo, drawFence );

  while ( vk::Result::eTimeout == context.device.waitForFences( drawFence, VK_TRUE, vk::su::FenceTimeout ) );
  frame.recycledSemaphores.push_back(imageAcquiredSemaphore);
  // LOGI("{}:{}", __FILE__, __LINE__);
  vk::Result result = vk::Result::eSuccess;
  try
  {
    result = context.presentQueue.presentKHR( vk::PresentInfoKHR( {}, context.swapChainData.swapChain, currentBuffer.value ) );
  }
  catch(const vk::SystemError& e)
  {
    std::cerr << e.what() << ", code: " << e.code() << '\n';
    result = static_cast<vk::Result>(e.code().value());
  }
  if(result != vk::Result::eSuccess) return result;

  frame.counter++;
  static const size_t FPS_PERIOD_BITS = 8;
  static const size_t FPS_PERIOD = ((1 << FPS_PERIOD_BITS) - 1);
  if((frame.counter & FPS_PERIOD) == 0)
  {
      auto timeCurr = std::chrono::steady_clock::now();
      std::chrono::duration<double> elapsedSeconds = timeCurr - frame.timeStamp;
      std::cout << "FPS: " << (FPS_PERIOD)/elapsedSeconds.count() << std::endl;
      frame.timeStamp = timeCurr;
  }
  return vk::Result::eSuccess;
}

std::shared_ptr<vk::su::BufferData> createTexturedVertexBuffer(
  const SampleContext& context,
  const std::vector<float>& vertices,
  const std::vector<float>& texCoord)
{
#define USE_GLTF 1
  size_t vertexNum = vertices.size() / 3;
  assert(vertices.size() / 3 == texCoord.size() / 2);
  std::vector<float> texturedData(6 * vertexNum);
  for(size_t i = 0; i < vertexNum; i++)
  {
#if USE_GLTF
    texturedData.at(6 * i + 0) = vertices.at(3 * i + 0);
    texturedData.at(6 * i + 1) = vertices.at(3 * i + 1);
    texturedData.at(6 * i + 2) = vertices.at(3 * i + 2);
    texturedData.at(6 * i + 3) = 1.;
    texturedData.at(6 * i + 4) = texCoord.at(2 * i + 0);
    texturedData.at(6 * i + 5) = texCoord.at(2 * i + 1);
#else
    texturedData.at(6 * i + 0) = texturedCubeData[i].x;
    texturedData.at(6 * i + 1) = texturedCubeData[i].y;
    texturedData.at(6 * i + 2) = texturedCubeData[i].z;
    texturedData.at(6 * i + 3) = texturedCubeData[i].w;
    texturedData.at(6 * i + 4) = texturedCubeData[i].u;
    texturedData.at(6 * i + 5) = texturedCubeData[i].v;
#endif
  }
  // std::cout << vk::su::to_string(texturedData) << std::endl;
  // std::cout << "memcmp:" << memcmp(texturedCubeData, texturedData.data(), texturedData.size() * sizeof(float)) << std::endl;
#if USE_GLTF
  std::cout << "sizeof( float ) * vertexNum * 6: " << sizeof( float ) * vertexNum * 6 << std::endl;
  std::cout << "vertexNum: " << vertexNum  << std::endl;
  auto pVertexBufferData = std::make_shared<vk::su::BufferData> (
     context.physicalDevice, context.device, sizeof( float ) * vertexNum * 6, vk::BufferUsageFlagBits::eVertexBuffer );
  vk::su::copyToDevice( context.device,
                          pVertexBufferData->deviceMemory,
                          texturedData.data(),
                          texturedData.size(),
                          sizeof(float));
#else
  std::cout << "sizeof( texturedCubeData ): " << sizeof( texturedCubeData ) << std::endl;
  std::cout << "sizeof( texturedCubeData ) / sizeof( texturedCubeData[0] ) ): " << sizeof( texturedCubeData ) / sizeof( texturedCubeData[0] )  << std::endl;
  auto pVertexBufferData = std::make_shared<vk::su::BufferData>(
      context.physicalDevice, context.device, sizeof( texturedCubeData ), vk::BufferUsageFlagBits::eVertexBuffer );
  vk::su::copyToDevice( context.device,
                          pVertexBufferData->deviceMemory,
                          texturedCubeData,
                          sizeof( texturedCubeData ) / sizeof( texturedCubeData[0] ),
                          sizeof( texturedCubeData[0] ) );
#endif
  return pVertexBufferData;
}

std::shared_ptr<vk::su::BufferData> createIndexBuffer(
  const SampleContext& context,
  const std::vector<uint16_t>& indices)
{
  size_t triangleNum = indices.size() / 3;
  auto pIndexBufferData = std::make_shared<vk::su::BufferData> (
     context.physicalDevice,
     context.device,
     sizeof( uint16_t ) * indices.size(),
     vk::BufferUsageFlagBits::eIndexBuffer );

  vk::su::copyToDevice( context.device,
                        pIndexBufferData->deviceMemory,
                        indices.data(),
                        indices.size() );
  return pIndexBufferData;
}

std::shared_ptr<vk::su::PixelsImageGenerator> createImageGenerator(const std::string& filename)
{
  int texWidth, texHeight, texChannels;
  stbi_uc* pixels = stbi_load(filename.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
  // std::cout << "original texChannels: " << texChannels << std::endl;
  if(3 == texChannels)
  {
    texChannels = 4;
  }

  if (!pixels) {
      throw std::runtime_error("failed to load texture image!");
  }
  auto img = std::make_shared<vk::su::PixelsImageGenerator>(vk::Extent2D(texWidth, texHeight), texChannels, pixels);
  return img;
}

void prepare(ModelResource& modelResource, const SampleContext& context)
{
  tinygltf::Model model;
  // std::cout << "WARNING: gltf life cycle problem! Texture was destroyed before copied by command buffer." << std::endl;
  std::string filename(assetsFolder() + "Duck.gltf");
  loadModel(model, filename);

  std::vector<uint16_t> indices = loadMeshIndices(model, 0);
  modelResource.indexNum = indices.size();
  std::vector<float> vertices = loadMeshAttributes(model, 0, "POSITION");
  std::vector<float> texCoord = loadMeshAttributes(model, 0, "TEXCOORD_0");


  // std::cout << vertices.size() / 3<<  " vertices:\n" ;
  // std::cout << "vertices:\n" << vk::su::to_string(vertices) << std::endl;
  // std::cout << "texCoord:\n" << vk::su::to_string(texCoord) << std::endl;
  // std::cout << "indices:\n" << vk::su::to_string(indices) << std::endl;

  modelResource.pIndexBuffer = createIndexBuffer(context, indices);
  modelResource.pVertexBuffer = createTexturedVertexBuffer(context, vertices, texCoord);
  if(model.textures.size() > 0)
  {
    auto gltfImage = loadMeshTexture(model, 0);
    modelResource.pTextureGenerator = std::make_shared<vk::su::PixelsImageGenerator>(vk::Extent2D(gltfImage.width, gltfImage.height), 4, gltfImage.image.data());
  }

  modelResource.pTextureData = std::make_shared<vk::su::TextureData>(
    context.physicalDevice,
    context.device,
    modelResource.pTextureGenerator ? modelResource.pTextureGenerator->extent() : vk::Extent2D(256, 256),
    vk::ImageUsageFlags(),vk::FormatFeatureFlags(),false, true);


  vk::su::oneTimeSubmit(context.device, context.commandPool, context.graphicsQueue,
  [&](vk::CommandBuffer const & commandBuffer)
  {
    if(modelResource.pTextureGenerator)
    {
      modelResource.pTextureData->setImage(context.device, commandBuffer, *modelResource.pTextureGenerator);
    }else
    {
      modelResource.pTextureData->setImage(context.device, commandBuffer, vk::su::CheckerboardImageGenerator());
    }
  });

  modelResource.scale = adaptiveModelScale(model, 0, 5);

}

void tearDown(ModelResource& modelResource, const SampleContext& context)
{
  if(modelResource.pVertexBuffer)
  {
    modelResource.pVertexBuffer->clear( context.device );
    modelResource.pVertexBuffer.reset();
    LOGI("{}:{}", __FILE__, __LINE__);
  }

  if(modelResource.pIndexBuffer)
  {
    modelResource.pIndexBuffer->clear( context.device );
    modelResource.pIndexBuffer.reset();
    LOGI("{}:{}", __FILE__, __LINE__);
  }

  if(modelResource.pTextureGenerator)
  {
    modelResource.pTextureGenerator.reset();
    LOGI("{}:{}", __FILE__, __LINE__);
  }

  if(modelResource.pTextureData)
  {
    modelResource.pTextureData->clear(context.device);
    modelResource.pTextureData.reset();
    LOGI("{}:{}", __FILE__, __LINE__);
  }
}