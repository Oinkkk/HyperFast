#include "ScreenManager.h"

namespace HyperFast
{
	ScreenManager::ScreenManager(
		Vulkan::Instance &instance, Vulkan::PhysicalDevice &physicalDevice,
		const uint32_t graphicsQueueFamilyIndex, const VkDevice device,
		const Vulkan::DeviceProcedure &deviceProc, const VkQueue graphicsQueue) noexcept :
		__instance{ instance }, __physicalDevice{ physicalDevice },
		__graphicsQueueFamilyIndex{ graphicsQueueFamilyIndex },
		__device{ device }, __deviceProc{ deviceProc }, __graphicsQueue{ graphicsQueue }
	{}

	std::unique_ptr<ScreenManager::ScreenImpl> ScreenManager::create(Win::Window &window) noexcept
	{
		return std::make_unique<ScreenImpl>(
			__instance, __physicalDevice, __graphicsQueueFamilyIndex,
			__device, __deviceProc, __graphicsQueue, window);
	}
}