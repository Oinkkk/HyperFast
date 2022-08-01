#include "ScreenManager_Old.h"

namespace HyperFast
{
	ScreenManager_Old::ScreenManager_Old(
		Vulkan::Instance &instance, Vulkan::PhysicalDevice &physicalDevice,
		const uint32_t graphicsQueueFamilyIndex, Vulkan::Device &device, Vulkan::Queue &queue,
		LifeCycle &lifeCycle, CommandSubmitter &commandSubmitter,
		Infra::TemporalDeleter &resourceDeleter) noexcept :
		__instance{instance}, __physicalDevice{ physicalDevice },
		__queueFamilyIndex{ graphicsQueueFamilyIndex }, __device{ device }, __queue{ queue },
		__lifeCycle{ lifeCycle }, __commandSubmitter{ commandSubmitter },
		__resourceDeleter{ resourceDeleter }
	{}

	std::unique_ptr<ScreenManager_Old::ScreenImpl> ScreenManager_Old::create(Win::Window &window) noexcept
	{
		return std::make_unique<ScreenImpl>(
			__instance, __physicalDevice, __queueFamilyIndex,
			__device, __queue, window, __lifeCycle, __commandSubmitter, __resourceDeleter);
	}
}