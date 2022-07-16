#include "ScreenManager.h"

namespace HyperFast
{
	ScreenManager::ScreenManager(
		RenderingEngine &renderingEngine,
		Vulkan::Instance &instance, Vulkan::PhysicalDevice &physicalDevice,
		const uint32_t graphicsQueueFamilyIndex,
		Vulkan::Device &device, Vulkan::Queue &queue) noexcept :
		__renderingEngine{ renderingEngine }, __instance { instance },
		__physicalDevice{physicalDevice},
		__queueFamilyIndex{ graphicsQueueFamilyIndex },
		__device{ device }, __queue{ queue }
	{}

	std::unique_ptr<ScreenManager::ScreenImpl> ScreenManager::create(Win::Window &window) noexcept
	{
		return std::make_unique<ScreenImpl>(
			__renderingEngine, __instance, __physicalDevice,
			__queueFamilyIndex, __device, __queue, window);
	}
}