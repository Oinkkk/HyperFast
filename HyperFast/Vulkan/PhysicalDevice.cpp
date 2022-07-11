#include "PhysicalDevice.h"

namespace Vulkan
{
	void PhysicalDevice::vkGetPhysicalDeviceProperties2(
		VkPhysicalDeviceProperties2 *const pProperties) const noexcept
	{
		__instance.vkGetPhysicalDeviceProperties2(_getConstHandle(), pProperties);
	}

	void PhysicalDevice::vkGetPhysicalDeviceQueueFamilyProperties(
		uint32_t *const pQueueFamilyPropertyCount,
		VkQueueFamilyProperties *const pQueueFamilyProperties) const noexcept
	{
		__instance.vkGetPhysicalDeviceQueueFamilyProperties(
			_getConstHandle(), pQueueFamilyPropertyCount, pQueueFamilyProperties);
	}

	void PhysicalDevice::vkGetPhysicalDeviceMemoryProperties2(
		VkPhysicalDeviceMemoryProperties2 *const pMemoryProperties) const noexcept
	{
		__instance.vkGetPhysicalDeviceMemoryProperties2(_getConstHandle(), pMemoryProperties);
	}

	VkBool32 PhysicalDevice::vkGetPhysicalDeviceWin32PresentationSupportKHR(
		const uint32_t queueFamilyIndex) const noexcept
	{
		return __instance.vkGetPhysicalDeviceWin32PresentationSupportKHR(
			_getConstHandle(), queueFamilyIndex);
	}

	VkResult PhysicalDevice::vkGetPhysicalDeviceSurfaceSupportKHR(
		const uint32_t queueFamilyIndex, const VkSurfaceKHR surface,
		VkBool32 *const pSupported) const noexcept
	{
		return __instance.vkGetPhysicalDeviceSurfaceSupportKHR(
			_getConstHandle(), queueFamilyIndex, surface, pSupported);
	}

	VkResult PhysicalDevice::vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
		const VkSurfaceKHR surface,
		VkSurfaceCapabilitiesKHR *const pSurfaceCapabilities) const noexcept
	{
		return __instance.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
			_getConstHandle(), surface, pSurfaceCapabilities);
	}

	VkResult PhysicalDevice::vkGetPhysicalDeviceSurfaceFormatsKHR(
		const VkSurfaceKHR surface, uint32_t *const pSurfaceFormatCount,
		VkSurfaceFormatKHR *const pSurfaceFormats) const noexcept
	{
		return __instance.vkGetPhysicalDeviceSurfaceFormatsKHR(
			_getConstHandle(), surface, pSurfaceFormatCount, pSurfaceFormats);
	}

	VkResult PhysicalDevice::vkGetPhysicalDeviceSurfacePresentModesKHR(
		const VkSurfaceKHR surface, uint32_t *const pPresentModeCount,
		VkPresentModeKHR *const pPresentModes) const noexcept
	{
		return __instance.vkGetPhysicalDeviceSurfacePresentModesKHR(
			_getConstHandle(), surface, pPresentModeCount, pPresentModes);
	}

	VkResult PhysicalDevice::vkCreateDevice(
		const VkDeviceCreateInfo *const pCreateInfo,
		const VkAllocationCallbacks *const pAllocator, VkDevice *const pDevice) noexcept
	{
		return __instance.vkCreateDevice(getHandle(), pCreateInfo, pAllocator, pDevice);
	}
}