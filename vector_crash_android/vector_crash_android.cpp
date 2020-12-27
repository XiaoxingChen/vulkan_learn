#include "utils.hpp"
#include "vulkan/vulkan.hpp"
#include "logging.h"

#include <iostream>

int main(int argc, char const *argv[])
{
    initLogger();
    spdlog::set_level(spdlog::level::debug);
    LOGD("{}:{}", __FILE__, __LINE__);
    vk::UniqueInstance instance = vk::su::createInstance( "simple_compute", "xx_engine" );
    LOGD("{}:{}", __FILE__, __LINE__);
#if !defined( NDEBUG )
    vk::UniqueDebugUtilsMessengerEXT debugUtilsMessenger = vk::su::createDebugUtilsMessenger( instance );
#endif
    LOGD("{}:{}", __FILE__, __LINE__);
    vk::PhysicalDevice physicalDevice = instance->enumeratePhysicalDevices().front();

    uint32_t computeQueueFamilyIndex =
      vk::su::findQueueFamilyIndex<vk::QueueFlagBits::eCompute>
      ( physicalDevice.getQueueFamilyProperties() );
    LOGD("{}:{}", __FILE__, __LINE__);
    vk::UniqueDevice device = vk::su::createDevice( physicalDevice, computeQueueFamilyIndex );

    /* VULKAN_HPP_KEY_START */
    LOGD("{}:{}", __FILE__, __LINE__);
    std::vector<double> testData{1., 2., 3., 4.};
    LOGI("testData.size: {}, addr: {}", testData.size(), ((void**)&testData)[0]);

    vk::su::BufferData storageBufferData(
      physicalDevice, device, sizeof(testData[0]) * testData.size() , vk::BufferUsageFlagBits::eStorageBuffer );
    LOGD("{}:{}", __FILE__, __LINE__);
    LOGI("testData.size: {}, data: {}, addr: {}", testData.size(), (void*) testData.data(), ((void**)&testData)[0]);

    LOGI("address of main: {}", (void*)main);

    LOGI("finished...");
    destroyLogger();
    return 0;
}
