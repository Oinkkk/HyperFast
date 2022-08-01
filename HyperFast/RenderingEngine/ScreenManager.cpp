#include "ScreenManager.h"

namespace HyperFast
{
	ScreenManager::ScreenManager(
		Vulkan::Instance &instance, Vulkan::PhysicalDevice &physicalDevice,
		const uint32_t graphicsQueueFamilyIndex, Vulkan::Device &device, Vulkan::Queue &queue,
		LifeCycle &lifeCycle, CommandSubmitter &commandSubmitter,
		Infra::TemporalDeleter &resourceDeleter) noexcept :
		__instance{ instance }, __physicalDevice{ physicalDevice },
		__queueFamilyIndex{ graphicsQueueFamilyIndex }, __device{ device }, __queue{ queue },
		__lifeCycle{ lifeCycle }, __commandSubmitter{ commandSubmitter },
		__resourceDeleter{ resourceDeleter }
	{}

	std::unique_ptr<ScreenManager::ScreenImpl> ScreenManager::create(Win::Window &window) noexcept
	{
		return std::make_unique<ScreenImpl>(
			__instance, __physicalDevice, __queueFamilyIndex,
			__device, __queue, __lifeCycle, __commandSubmitter, __resourceDeleter, window);
	}
}