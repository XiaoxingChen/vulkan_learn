#include "hello_triangle/sample_utils.h"

void prepare(SampleContext& context, const char* EngineName, const char* AppName)
{
  LOGI("{}:{}", __FILE__, __LINE__);
    context.instance = vk::su::createInstance( AppName, EngineName, {}, vk::su::getInstanceExtensions() );
    LOGI("{}:{}", __FILE__, __LINE__);

  context.debugUtilsMessenger =
      context.instance.createDebugUtilsMessengerEXT( vk::su::makeDebugUtilsMessengerCreateInfoEXT() );

  context.physicalDevice = context.instance.enumeratePhysicalDevices().front();

  context.pSurfaceData = std::make_shared<vk::su::SurfaceData>(context.instance, AppName, vk::Extent2D( 500, 500 ));

  std::pair<uint32_t, uint32_t> graphicsAndPresentQueueFamilyIndex =
    vk::su::findGraphicsAndPresentQueueFamilyIndex(context.physicalDevice, context.pSurfaceData->surface );
  context.graphicsQueueIndex = graphicsAndPresentQueueFamilyIndex.first;
  context.device =
    vk::su::createDevice(context.physicalDevice, graphicsAndPresentQueueFamilyIndex.first, vk::su::getDeviceExtensions() );

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

  context.descriptorSetLayout = vk::su::createDescriptorSetLayout(
    context.device, { { vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eVertex } } );
  context.pipelineLayout = context.device.createPipelineLayout(
    vk::PipelineLayoutCreateInfo( vk::PipelineLayoutCreateFlags(), context.descriptorSetLayout ) );
  LOGI("{}:{}", __FILE__, __LINE__);

  context.renderPass = vk::su::createRenderPass(
    context.device,
    vk::su::pickSurfaceFormat(context.physicalDevice.getSurfaceFormatsKHR( context.pSurfaceData->surface ) ).format,
    vk::Format::eUndefined);

  LOGI("{}:{}", __FILE__, __LINE__);

  glslang::InitializeProcess();
  context.vertexShaderModule =
    vk::su::createShaderModule( context.device, vk::ShaderStageFlagBits::eVertex, vertexShaderText_PC_C );
  context.fragmentShaderModule =
    vk::su::createShaderModule( context.device, vk::ShaderStageFlagBits::eFragment, fragmentShaderText_C_C );
  glslang::FinalizeProcess();
  context.framebuffers = vk::su::createFramebuffers(
    context.device, context.renderPass, context.swapChainData.imageViews, nullptr, context.pSurfaceData->extent );

  context.descriptorPool =
    vk::su::createDescriptorPool( context.device, { { vk::DescriptorType::eUniformBuffer, 1 } } );
  vk::DescriptorSetAllocateInfo descriptorSetAllocateInfo( context.descriptorPool, context.descriptorSetLayout );
  context.descriptorSet = context.device.allocateDescriptorSets( descriptorSetAllocateInfo ).front();

  context.pipelineCache = context.device.createPipelineCache( vk::PipelineCacheCreateInfo() );
  context.graphicsPipeline = vk::su::createGraphicsPipeline(
    context.device,
    context.pipelineCache,
    std::make_pair( context.vertexShaderModule, nullptr ),
    std::make_pair( context.fragmentShaderModule, nullptr ),
    sizeof( coloredCubeData[0] ),
    { { vk::Format::eR32G32B32A32Sfloat, 0 }, { vk::Format::eR32G32B32A32Sfloat, 16 } },
    vk::FrontFace::eClockwise,
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

  // depthBufferData.clear( context.device );
  context.swapChainData.clear( context.device );
  context.device.destroy();
  context.instance.destroySurfaceKHR( context.pSurfaceData->surface );

  context.instance.destroyDebugUtilsMessengerEXT( context.debugUtilsMessenger );

  context.instance.destroy();
}

std::vector<vk::CommandBuffer> createCommandBuffers(
  const SampleContext& context,
  const vk::CommandPool& commandPool,
  const vk::su::BufferData& vertexBufferData,
  size_t num)
{
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

    commandBuffer.bindVertexBuffers( 0, vertexBufferData.buffer, { 0 } );
    commandBuffer.setViewport( 0,
                               vk::Viewport( 0.0f,
                                             0.0f,
                                             static_cast<float>( context.pSurfaceData->extent.width ),
                                             static_cast<float>( context.pSurfaceData->extent.height ),
                                             0.0f,
                                             1.0f ) );
    commandBuffer.setScissor( 0, vk::Rect2D( vk::Offset2D( 0, 0 ), context.pSurfaceData->extent ) );

    commandBuffer.draw( 12 * 3, 1, 0, 0 );
    commandBuffer.endRenderPass();
    commandBuffer.end();
  }

  return commandBuffers;
}

void prepare(FrameResource& frame, SampleContext& context, vk::su::BufferData& vertexBufferData)
{
  frame.imageNum = context.swapChainData.images.size();
  frame.commandBuffers = createCommandBuffers(context, context.commandPool, vertexBufferData, frame.imageNum);
  frame.drawFences.resize(frame.imageNum);
  frame.imageAcquiredSemaphores.resize(frame.imageNum);
  for(size_t i = 0; i < frame.imageNum; i++)
  {
    frame.drawFences.at(i) = context.device.createFence( vk::FenceCreateInfo() );
    frame.imageAcquiredSemaphores.at(i) = context.device.createSemaphore( vk::SemaphoreCreateInfo() );
  }
}

void tearDown(FrameResource& frame, SampleContext& context)
{
    for(size_t i = 0; i < frame.imageNum; i++)
    {
      context.device.destroyFence(frame.drawFences.at(i));
      context.device.destroySemaphore( frame.imageAcquiredSemaphores.at(i));
    }
}
void draw(SampleContext& context, FrameResource& frame)
{
  auto & commandBuffer = frame.commandBuffers.front();
  auto & drawFence = frame.drawFences.front();
  auto & imageAcquiredSemaphore = frame.imageAcquiredSemaphores.front();
  // Get the index of the next available swapchain image:

  vk::ResultValue<uint32_t> currentBuffer =
    context.device.acquireNextImageKHR( context.swapChainData.swapChain, vk::su::FenceTimeout, imageAcquiredSemaphore, nullptr );
  assert( currentBuffer.result == vk::Result::eSuccess );
  assert( currentBuffer.value < context.framebuffers.size() );

  vk::PipelineStageFlags waitDestinationStageMask( vk::PipelineStageFlagBits::eColorAttachmentOutput );
  vk::SubmitInfo         submitInfo( imageAcquiredSemaphore, waitDestinationStageMask, frame.commandBuffers.at(currentBuffer.value) );
  context.device.resetFences( drawFence);
  context.graphicsQueue.submit( submitInfo, drawFence );

  while ( vk::Result::eTimeout == context.device.waitForFences( drawFence, VK_TRUE, vk::su::FenceTimeout ) );
  // LOGI("{}:{}", __FILE__, __LINE__);
  vk::Result result =
    context.presentQueue.presentKHR( vk::PresentInfoKHR( {}, context.swapChainData.swapChain, currentBuffer.value ) );
  switch ( result )
  {
    case vk::Result::eSuccess: break;
    case vk::Result::eSuboptimalKHR:
      std::cout << "vk::Queue::presentKHR returned vk::Result::eSuboptimalKHR !\n";
      break;
    default: assert( false );  // an unexpected result is returned !
  }
}
