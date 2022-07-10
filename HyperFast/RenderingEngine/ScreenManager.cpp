﻿#include "ScreenManager.h"

namespace HyperFast
{
	ScreenManager::ScreenManager(
		Vulkan::Instance &instance, Vulkan::PhysicalDevice &physicalDevice,
		const uint32_t graphicsQueueFamilyIndex,
		Vulkan::Device &device, const VkQueue graphicsQueue) noexcept :
		__instance{ instance }, __physicalDevice{ physicalDevice },
		__graphicsQueueFamilyIndex{ graphicsQueueFamilyIndex },
		__device{ device }, __graphicsQueue{ graphicsQueue }
	{}

	std::unique_ptr<ScreenManager::ScreenImpl> ScreenManager::create(Win::Window &window) noexcept
	{
		return std::make_unique<ScreenImpl>(
			__instance, __physicalDevice, __graphicsQueueFamilyIndex,
			__device, __graphicsQueue, window);
	}
}