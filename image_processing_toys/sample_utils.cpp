#include "image_processing_toys/sample_utils.h"
#include "gltf_utils.h"
#include <stb_image.h>
#include "utils/resource_manager.h"
#include "image_processing_toys/euroc_io.h"
#include <stb_image.h>

void prepare(SampleContext& context, const char* EngineName, const char* AppName)
{
  LOGI("{}:{}", __FILE__, __LINE__);
    context.instance = vk::su::createInstance( AppName, EngineName, {"VK_LAYER_KHRONOS_validation"}, vk::su::getInstanceExtensions() );
    LOGI("{}:{}", __FILE__, __LINE__);
#if !NDEBUG
  context.debugUtilsMessenger =
      context.instance.createDebugUtilsMessengerEXT( vk::su::makeDebugUtilsMessengerCreateInfoEXT() );
  LOGI("{}:{}", __FILE__, __LINE__);
#endif

  context.physicalDevice = context.instance.enumeratePhysicalDevices().front();

  std::cout << __FILE__ << ":" << __LINE__ << std::endl;
  context.pSurfaceData = std::make_shared<vk::su::SurfaceData>(context.physicalDevice, context.instance, AppName, vk::Extent2D( euroc::resolution()[0], euroc::resolution()[1] ));

  std::pair<uint32_t, uint32_t> graphicsAndPresentQueueFamilyIndex =
    vk::su::findGraphicsAndPresentQueueFamilyIndex(context.physicalDevice, context.pSurfaceData->surface );
  context.graphicsQueueIndex = graphicsAndPresentQueueFamilyIndex.first;
  context.computeQueueFamilyIndex = vk::su::findQueueFamilyIndex(context.physicalDevice.getQueueFamilyProperties(), vk::QueueFlagBits::eCompute);
  context.device =
    vk::su::createDevice(context.physicalDevice, graphicsAndPresentQueueFamilyIndex.first, vk::su::getDeviceExtensions(context.physicalDevice) );

  context.graphicsQueue = context.device.getQueue( graphicsAndPresentQueueFamilyIndex.first, 0 );
  context.presentQueue  = context.device.getQueue( graphicsAndPresentQueueFamilyIndex.second, 0 );
  context.computeQueue = context.device.getQueue(context.computeQueueFamilyIndex, 0);

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

  LOGI("{}:{}", __FILE__, __LINE__);

  context.renderPass = vk::su::createRenderPass(
    context.device,
    vk::su::pickSurfaceFormat(context.physicalDevice.getSurfaceFormatsKHR( context.pSurfaceData->surface ) ).format,
    context.pDepthBuffer->format);

  LOGI("{}:{}", __FILE__, __LINE__);

  context.framebuffers = vk::su::createFramebuffers(
    context.device, context.renderPass, context.swapChainData.imageViews, context.pDepthBuffer->imageView, context.pSurfaceData->extent );

  context.commandPool = vk::su::createCommandPool( context.device, context.graphicsQueueIndex );
}

void tearDown(SampleContext& context)
{
  context.device.destroyCommandPool( context.commandPool );

  for ( auto framebuffer : context.framebuffers )
  {
    context.device.destroyFramebuffer( framebuffer );
  }
  context.device.destroyRenderPass( context.renderPass );

  context.pDepthBuffer->clear( context.device );
  context.swapChainData.clear( context.device );
  context.device.destroy();
  context.instance.destroySurfaceKHR( context.pSurfaceData->surface );
#if !NDEBUG
  context.instance.destroyDebugUtilsMessengerEXT( context.debugUtilsMessenger );
#endif
  context.instance.destroy();
}

void prepareRectangle(GraphicsPipelineResource& pipeline, const SampleContext& context)
{
  pipeline.descriptorSetLayout = vk::su::createDescriptorSetLayout(
    context.device, {
      { vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eFragment }
      } );
  pipeline.layout = context.device.createPipelineLayout(
    vk::PipelineLayoutCreateInfo( vk::PipelineLayoutCreateFlags(), pipeline.descriptorSetLayout ) );

  glslang::InitializeProcess();

  pipeline.vertexShaderModule =
    vk::su::createShaderModule( context.device, vk::ShaderStageFlagBits::eVertex, readShaderSource("imshow.vert") );
  pipeline.fragmentShaderModule =
    vk::su::createShaderModule( context.device, vk::ShaderStageFlagBits::eFragment, readShaderSource("imshow.frag") );
  glslang::FinalizeProcess();

  pipeline.descriptorPool =
    vk::su::createDescriptorPool( context.device, {
      { vk::DescriptorType::eCombinedImageSampler, 1 }
      } );
  vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo( pipeline.descriptorPool, pipeline.descriptorSetLayout );
  pipeline.descriptorSet = context.device.allocateDescriptorSets( descriptorSetAllocateInfo ).front();

  pipeline.cache = context.device.createPipelineCache( vk::PipelineCacheCreateInfo() );
  pipeline.self = vk::su::createGraphicsPipeline(
    context.device,
    pipeline.cache,
    std::make_pair( pipeline.vertexShaderModule, nullptr ),
    std::make_pair( pipeline.fragmentShaderModule, nullptr ),
    sizeof( float ) * 6,
    { { vk::Format::eR32G32B32A32Sfloat, 0 }, { vk::Format::eR32G32Sfloat, 16 } },
    vk::FrontFace::eCounterClockwise,
    true,
    pipeline.layout,
    context.renderPass );
}

void tearDown(const GraphicsPipelineResource& pipeline, const SampleContext& context)
{
  context.device.destroyPipeline( pipeline.self );
  context.device.destroyPipelineCache( pipeline.cache );
  context.device.destroyDescriptorPool( pipeline.descriptorPool );
  context.device.destroyShaderModule( pipeline.fragmentShaderModule );
  context.device.destroyShaderModule( pipeline.vertexShaderModule );
  context.device.destroyPipelineLayout( pipeline.layout );
  context.device.destroyDescriptorSetLayout( pipeline.descriptorSetLayout );
}

void prepareCompute(ComputePipelineResource& pipeline, const SampleContext& context, const std::string& shaderName)
{
  pipeline.descriptorSetLayout = vk::su::createDescriptorSetLayout(
      context.device, {
        { vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute },
        { vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eCompute },
        { vk::DescriptorType::eStorageBuffer, 1, vk::ShaderStageFlagBits::eCompute } } );

  pipeline.descriptorPool = vk::su::createDescriptorPool( context.device, {
    { vk::DescriptorType::eStorageBuffer, 1 },
    { vk::DescriptorType::eUniformBuffer, 1 },
    { vk::DescriptorType::eStorageBuffer, 1 } } );
  pipeline.descriptorSet = std::move(
      context.device.allocateDescriptorSets( vk::DescriptorSetAllocateInfo( pipeline.descriptorPool, pipeline.descriptorSetLayout ) )
        .front() );
  glslang::InitializeProcess();
  pipeline.computeShaderModule = vk::su::createShaderModule(context.device, vk::ShaderStageFlagBits::eCompute,
    readShaderSource(shaderName));
  glslang::FinalizeProcess();
  pipeline.layout = context.device.createPipelineLayout(
      vk::PipelineLayoutCreateInfo( vk::PipelineLayoutCreateFlags(), pipeline.descriptorSetLayout )
    );

  pipeline.self = vk::su::createComputePipeline(context.device, pipeline.computeShaderModule, pipeline.layout);
}

void tearDown(const ComputePipelineResource& pipeline, const SampleContext& context)
{
  context.device.destroyPipeline( pipeline.self );
  context.device.destroyPipelineCache( pipeline.cache );
  context.device.destroyDescriptorPool( pipeline.descriptorPool );
  context.device.destroyShaderModule( pipeline.computeShaderModule );
  context.device.destroyPipelineLayout( pipeline.layout );
  context.device.destroyDescriptorSetLayout( pipeline.descriptorSetLayout );
}

vk::CommandBuffer createCommandBuffer(
  const SampleContext& context,
  const ComputePipelineResource& pipeline,
  const ModelResource& modelResource,
  const BufferList& bufferList)
{
  vk::CommandBuffer commandBuffer = std::move( context.device.allocateCommandBuffers( vk::CommandBufferAllocateInfo(
                                                           context.commandPool, vk::CommandBufferLevel::ePrimary, 1 ) )
                                                         .front() );
    // std::cout << "shader: \n" << readShaderSource("test.comp") << std::endl;


    // command buffer
    commandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlags()));
    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, pipeline.self);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, pipeline.layout, 0, pipeline.descriptorSet, nullptr);
    commandBuffer.dispatch(
      (uint32_t)ceil(euroc::resolution()[0] / float(WORKGROUP_SIZE)),
      (uint32_t)ceil(euroc::resolution()[1] / float(WORKGROUP_SIZE)), 1);
    // commandBuffer.dispatch(2,2, 1);
    modelResource.pTextureData->setImage(context.device, commandBuffer, *bufferList.pComputeOutput);
    commandBuffer.end();
    return commandBuffer;
}

vk::CommandBuffer createCommandBuffer(
  const SampleContext& context,
  const ComputePipelineResource& rawGrayU8ToF32pipeline,
  const ComputePipelineResource& f32ToU8RGBApipeline,
  const ModelResource& modelResource,
  const BufferList& bufferList)
{
  vk::CommandBuffer commandBuffer = std::move( context.device.allocateCommandBuffers( vk::CommandBufferAllocateInfo(
                                                           context.commandPool, vk::CommandBufferLevel::ePrimary, 1 ) )
                                                         .front() );
    // command buffer
    commandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlags()));

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, rawGrayU8ToF32pipeline.self);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, rawGrayU8ToF32pipeline.layout, 0, rawGrayU8ToF32pipeline.descriptorSet, nullptr);
    commandBuffer.dispatch(
      (uint32_t)ceil(euroc::resolution()[0] / float(WORKGROUP_SIZE)),
      (uint32_t)ceil(euroc::resolution()[1] / float(WORKGROUP_SIZE)), 1);

    commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, f32ToU8RGBApipeline.self);
    commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, f32ToU8RGBApipeline.layout, 0, f32ToU8RGBApipeline.descriptorSet, nullptr);
    commandBuffer.dispatch(
      (uint32_t)ceil(euroc::resolution()[0] / float(WORKGROUP_SIZE)),
      (uint32_t)ceil(euroc::resolution()[1] / float(WORKGROUP_SIZE)), 1);

    modelResource.pTextureData->setImage(context.device, commandBuffer, *bufferList.pComputeOutput);
    commandBuffer.end();
    return commandBuffer;
}

std::vector<vk::CommandBuffer> createCommandBuffers(
  const SampleContext& context,
  const GraphicsPipelineResource& pipeline,
  const ModelResource& modelResource)
{
  size_t num = context.swapChainData.images.size();
  auto allocateInfo = vk::CommandBufferAllocateInfo( context.commandPool, vk::CommandBufferLevel::ePrimary, num );
  std::vector<vk::CommandBuffer> commandBuffers = context.device.allocateCommandBuffers( allocateInfo );
  for(size_t i = 0; i < commandBuffers.size(); i++)
  {
    auto & commandBuffer(commandBuffers.at(i));
    commandBuffer.begin( vk::CommandBufferBeginInfo( vk::CommandBufferUsageFlags() ) );

    // modelResource.pTextureData->setImage(context.device, commandBuffer, *modelResource.pTextureGenerator);

    std::array<vk::ClearValue, 2> clearValues;
    clearValues[0].color        = vk::ClearColorValue( std::array<float, 4>( { { 0.2f, 0.2f, 0.2f, 0.2f } } ) );
    clearValues[1].depthStencil = vk::ClearDepthStencilValue( 1.0f, 0 );

    vk::RenderPassBeginInfo renderPassBeginInfo( context.renderPass,
                                                 context.framebuffers[i],
                                                 vk::Rect2D( vk::Offset2D( 0, 0 ), context.pSurfaceData->extent ),
                                                 clearValues );

    commandBuffer.beginRenderPass( renderPassBeginInfo, vk::SubpassContents::eInline );
    commandBuffer.bindPipeline( vk::PipelineBindPoint::eGraphics, pipeline.self );
    commandBuffer.bindDescriptorSets( vk::PipelineBindPoint::eGraphics, pipeline.layout, 0, pipeline.descriptorSet, nullptr );

    commandBuffer.bindVertexBuffers( 0, modelResource.pVertexBuffer->buffer, { 0 } );
    // commandBuffer.bindIndexBuffer(modelResource.pIndexBuffer->buffer, 0, vk::IndexType::eUint16);
    commandBuffer.setViewport( 0,
                               vk::Viewport( 0.0f,
                                             0.0f,
                                             static_cast<float>( context.pSurfaceData->extent.width ),
                                             static_cast<float>( context.pSurfaceData->extent.height ),
                                             0.0f,
                                             1.0f ) );
    commandBuffer.setScissor( 0, vk::Rect2D( vk::Offset2D( 0, 0 ), context.pSurfaceData->extent ) );

    commandBuffer.draw( 6, 1, 0, 0 );
    // commandBuffer.drawIndexed(modelResource.indexNum, 1, 0, 0, 0);
    commandBuffer.endRenderPass();
    commandBuffer.end();
  }

  return commandBuffers;
}

void prepare(FrameResource& frame, SampleContext& context)
{
  frame.imageNum = context.swapChainData.images.size();
  // frame.commandBuffers = createCommandBuffers(context, pipeline, modelResource, frame.imageNum);
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
    for(auto & semaphore: frame.recycledSemaphores)
    {
      context.device.destroySemaphore(semaphore);
    }
}

void handleSurfaceChange(SampleContext& context, const ModelResource& modelResource, FrameResource& frame, const GraphicsPipelineResource& pipe)
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
  frame.commandBuffers = createCommandBuffers(context, pipe, modelResource);
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

  vk::ResultValue<uint32_t> currentBuffer =
    context.device.acquireNextImageKHR( context.swapChainData.swapChain, vk::su::FenceTimeout, imageAcquiredSemaphore, nullptr );
  if(currentBuffer.result != vk::Result::eSuccess)
  {
    context.device.destroySemaphore(imageAcquiredSemaphore);
    return currentBuffer.result;
  }

  assert( currentBuffer.value < context.framebuffers.size() );

  vk::PipelineStageFlags waitDestinationStageMask( vk::PipelineStageFlagBits::eColorAttachmentOutput );
  vk::SubmitInfo         submitInfo( imageAcquiredSemaphore, waitDestinationStageMask, frame.commandBuffers.at(currentBuffer.value) );
  auto & drawFence = frame.drawFences.at(currentBuffer.value);
  context.device.resetFences( drawFence);
  context.graphicsQueue.submit( submitInfo, drawFence );

  while ( vk::Result::eTimeout == context.device.waitForFences( drawFence, VK_TRUE, vk::su::FenceTimeout ) );
  frame.recycledSemaphores.push_back(imageAcquiredSemaphore);
  // LOGI("{}:{}", __FILE__, __LINE__);
  vk::Result result =
    context.presentQueue.presentKHR( vk::PresentInfoKHR( {}, context.swapChainData.swapChain, currentBuffer.value ) );
  if(result != vk::Result::eSuccess) return result;

#if 0
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
#endif
  return vk::Result::eSuccess;
}

std::shared_ptr<vk::su::BufferData> createTexturedVertexBuffer(const SampleContext& context)
{
  auto pVertexBufferData = std::make_shared<vk::su::BufferData>(
      context.physicalDevice, context.device, sizeof( texturedRectangleData ), vk::BufferUsageFlagBits::eVertexBuffer );
  vk::su::copyToDevice( context.device,
                          pVertexBufferData->deviceMemory,
                          texturedRectangleData,
                          sizeof( texturedRectangleData ) / sizeof( texturedRectangleData[0] ),
                          sizeof( texturedRectangleData[0] ) );
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

void prepare(ModelResource& modelResource, const SampleContext& context)
{
  // modelResource.pIndexBuffer = createIndexBuffer(context, indices);
  modelResource.pVertexBuffer = createTexturedVertexBuffer(context);

  // modelResource.pTextureGenerator = vk::su::createImageGenerator(assetsFolder() + "damaged_helmet.png");
  // modelResource.pTextureGenerator = vk::su::createImageGenerator(euroc::imagePaths("V1_01_easy", 0).at(0));

  modelResource.pTextureData = std::make_shared<vk::su::TextureData>(
    context.physicalDevice,
    context.device,
    modelResource.pTextureGenerator ? modelResource.pTextureGenerator->extent() : vk::Extent2D(euroc::resolution()[0], euroc::resolution()[1]),
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

  modelResource.scale = 1.f;
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

void BufferList::prepare(SampleContext& context)
{
  size_t imagePixels = euroc::resolution()[0] * euroc::resolution()[1];

  pComputeOutput = std::make_shared<vk::su::BufferData>(
      context.physicalDevice, context.device, sizeof(uint32_t) * imagePixels, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferSrc| vk::BufferUsageFlagBits::eTransferDst );
  pComputeUniform = std::make_shared<vk::su::BufferData>(
      context.physicalDevice, context.device, sizeof(shader::ComputeUniformInfo), vk::BufferUsageFlagBits::eUniformBuffer );
  pImageData = std::make_shared<vk::su::BufferData>(
      context.physicalDevice, context.device, sizeof(shader::ImageData) * imagePixels, vk::BufferUsageFlagBits::eStorageBuffer );
  pImageU8RawGray = std::make_shared<vk::su::BufferData>(
      context.physicalDevice, context.device, sizeof(uint8_t) * imagePixels, vk::BufferUsageFlagBits::eStorageBuffer | vk::BufferUsageFlagBits::eTransferDst);
}

void BufferList::updateRawImage(const SampleContext& context, const std::string imageFilename)
{
  int texWidth, texHeight, texChannels;
  stbi_uc* pixels = stbi_load(imageFilename.c_str(), &texWidth, &texHeight, &texChannels, STBI_grey);
  // std::vector<uint8_t> mock(euroc::resolution()[0] * euroc::resolution()[1]);
  // for(auto & v: mock) v = 255;
  // pImageU8RawGray->upload(context.device, mock);

  void * dataPtr = context.device.mapMemory( pImageU8RawGray->deviceMemory, 0, texWidth * texHeight );
  memcpy( dataPtr, pixels, texWidth * texHeight );
  context.device.unmapMemory( pImageU8RawGray->deviceMemory );

  free (pixels);
}

void BufferList::tearDown(SampleContext& context)
{
  if(pComputeOutput)
  {
    pComputeOutput->clear(context.device);
    pComputeOutput.reset();
  }
  if(pComputeUniform)
  {
    pComputeUniform->clear(context.device);
    pComputeUniform.reset();
  }
  if(pImageData)
  {
    pImageData->clear(context.device);
    pImageData.reset();
  }
  if(pImageU8RawGray)
  {
    pImageU8RawGray->clear(context.device);
    pImageU8RawGray.reset();
  }
}