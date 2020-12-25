#include <iostream>
#include "logging.h"
#include "utils.hpp"
#include "vulkan/vulkan.hpp"
#include "shader_bank.h"


int main(int argc, char const *argv[])
{
	  initLogger();

    LOGI("compile time: {} {}", __DATE__, __TIME__);

    vk::UniqueInstance instance = vk::su::createInstance( "test_main", "test_engine" );

    #if !defined( NDEBUG )
    	vk::UniqueDebugUtilsMessengerEXT debugUtilsMessenger = vk::su::createDebugUtilsMessenger( instance );
	#endif

    vk::PhysicalDevice physicalDevice = instance->enumeratePhysicalDevices().front();

    std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

    size_t graphicsQueueFamilyIndex = std::distance(
      queueFamilyProperties.begin(),
      std::find_if(
        queueFamilyProperties.begin(), queueFamilyProperties.end(), []( vk::QueueFamilyProperties const & qfp ) {
          return qfp.queueFlags & vk::QueueFlagBits::eGraphics;
        } ) );
    assert( graphicsQueueFamilyIndex < queueFamilyProperties.size() );

    float                     queuePriority = 0.0f;
    vk::DeviceQueueCreateInfo deviceQueueCreateInfo(
      vk::DeviceQueueCreateFlags(), static_cast<uint32_t>( graphicsQueueFamilyIndex ), 1, &queuePriority );
    vk::UniqueDevice device =
      physicalDevice.createDeviceUnique( vk::DeviceCreateInfo( vk::DeviceCreateFlags(), deviceQueueCreateInfo ) );

	LOGI("device: {}", (void*) device.get());

    auto shader_code = shader_bank::read("basic.comp");
	LOGI("shader code: {}", shader_code);


	LOGI("done");
	destroyLogger();

    return 0;
}
