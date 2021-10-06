#include "synchronization/sample_utils.h"
#include "logging.h"

void syncBetweenQueues(const ComputeSampleContext& context, const ComputePipelineResource& initPipeline, const ComputePipelineResource& timePipeline)
{
    if(context.computeQueues.size() < 2)
    {
        LOGI("compute queue less than 2, skip between queues synchronization demo.");
        return;
    }
    vk::Semaphore semaphore = context.device.createSemaphore(vk::SemaphoreCreateInfo());
    vk::PipelineStageFlags waitDestinationStageMask( vk::PipelineStageFlagBits::eComputeShader );

    {
        vk::CommandBuffer commandBuffer = std::move( context.device.allocateCommandBuffers( vk::CommandBufferAllocateInfo(
                                                            context.commandPools.at(0), vk::CommandBufferLevel::ePrimary, 1 ) )
                                                            .front() );

        // command buffer
        commandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlags()));

        commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, initPipeline.self);
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, initPipeline.layout, 0, initPipeline.descriptorSet, nullptr);
        commandBuffer.dispatch((uint32_t)ceil(WIDTH / float(WORKGROUP_SIZE)), (uint32_t)ceil(HEIGHT / float(WORKGROUP_SIZE)), 1);

        commandBuffer.end();

        // finish command buffer recording

        auto submitInfo = vk::SubmitInfo(nullptr, nullptr, commandBuffer, semaphore);
        context.computeQueues.at(0).submit(submitInfo, nullptr);
    }

    {
        vk::CommandBuffer commandBuffer = std::move( context.device.allocateCommandBuffers( vk::CommandBufferAllocateInfo(
                                                            context.commandPools.at(1), vk::CommandBufferLevel::ePrimary, 1 ) )
                                                            .front() );
        // command buffer
        commandBuffer.begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlags()));

        commandBuffer.bindPipeline(vk::PipelineBindPoint::eCompute, timePipeline.self);
        commandBuffer.bindDescriptorSets(vk::PipelineBindPoint::eCompute, timePipeline.layout, 0, timePipeline.descriptorSet, nullptr);
        commandBuffer.dispatch((uint32_t)ceil(WIDTH / float(WORKGROUP_SIZE)), (uint32_t)ceil(HEIGHT / float(WORKGROUP_SIZE)), 1);

        commandBuffer.end();


        auto submitInfo = vk::SubmitInfo(semaphore, waitDestinationStageMask, commandBuffer);

        context.computeQueues.at(1).submit(submitInfo, nullptr);
    }

    // context.computeQueues.at(0).waitIdle();
    context.computeQueues.at(1).waitIdle();
    context.device.destroySemaphore(semaphore);
}