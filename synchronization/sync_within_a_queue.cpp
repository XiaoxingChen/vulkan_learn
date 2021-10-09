#include "synchronization/sample_utils.h"
#include "logging.h"

// Reference: 
// [1] https://gpuopen.com/learn/vulkan-barriers-explained/
void syncByExecutionBarrier(const ComputeSampleContext& context, const ComputePipelineResource& initPipeline, const ComputePipelineResource& timePipeline)
{
    auto commandBuffers = context.device.allocateCommandBuffers( vk::CommandBufferAllocateInfo(
                                                            context.commandPools.at(0), vk::CommandBufferLevel::ePrimary, 2 ) );
    {
        // command buffer
        commandBuffers.at(0).begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlags()));

        commandBuffers.at(0).bindPipeline(vk::PipelineBindPoint::eCompute, initPipeline.self);
        commandBuffers.at(0).bindDescriptorSets(vk::PipelineBindPoint::eCompute, initPipeline.layout, 0, initPipeline.descriptorSet, nullptr);
        commandBuffers.at(0).dispatch((uint32_t)ceil(WIDTH / float(WORKGROUP_SIZE)), (uint32_t)ceil(HEIGHT / float(WORKGROUP_SIZE)), 1);

        commandBuffers.at(0).end();

    }

    {
        // command buffer
        commandBuffers.at(1).begin(vk::CommandBufferBeginInfo(vk::CommandBufferUsageFlags()));
        // commandBuffers.at(1).pipelineBarrier(
        //     vk::PipelineStageFlagBits::eBottomOfPipe, vk::PipelineStageFlagBits::eTopOfPipe, {},
        //     nullptr, nullptr, nullptr);

        commandBuffers.at(1).bindPipeline(vk::PipelineBindPoint::eCompute, timePipeline.self);
        commandBuffers.at(1).bindDescriptorSets(vk::PipelineBindPoint::eCompute, timePipeline.layout, 0, timePipeline.descriptorSet, nullptr);
        commandBuffers.at(1).dispatch((uint32_t)ceil(WIDTH / float(WORKGROUP_SIZE)), (uint32_t)ceil(HEIGHT / float(WORKGROUP_SIZE)), 1);

        commandBuffers.at(1).end();
    }
    auto submitInfo = vk::SubmitInfo(nullptr, nullptr, commandBuffers);
    
    context.computeQueues.at(0).submit(submitInfo, nullptr);

    context.computeQueues.at(0).waitIdle();

}