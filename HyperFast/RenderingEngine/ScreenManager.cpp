#include "ScreenManager.h"

namespace HyperFast
{
	ScreenManager::ScreenManager(
		Vulkan::Instance &instance, Vulkan::PhysicalDevice &physicalDevice,
		const uint32_t graphicsQueueFamilyIndex, Vulkan::Device &device, Vulkan::Queue &queue,
		LifeCycle &lifeCycle, CommandSubmitter &commandSubmitter) noexcept :
		__instance{instance}, __physicalDevice{ physicalDevice },
		__queueFamilyIndex{ graphicsQueueFamilyIndex }, __device{ device }, __queue{ queue },
		__lifeCycle{ lifeCycle }, __commandSubmitter{ commandSubmitter }
	{}

	std::unique_ptr<ScreenManager::ScreenImpl> ScreenManager::create(Win::Window &window) noexcept
	{
		return std::make_unique<ScreenImpl>(
			__instance, __physicalDevice, __queueFamilyIndex,
			__device, __queue, window, __lifeCycle, __commandSubmitter);
	}
}