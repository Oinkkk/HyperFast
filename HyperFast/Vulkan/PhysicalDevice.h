#pragma once

#include "Instance.h"

namespace Vulkan
{
	class PhysicalDevice final : public Handle<VkPhysicalDevice>
	{
	public:
		PhysicalDevice(Instance &instance, const VkPhysicalDevice handle) noexcept;
		virtual ~PhysicalDevice() noexcept = default;

		void vkGetPhysicalDeviceProperties2(
			VkPhysicalDeviceProperties2 *const pProperties) const noexcept;

		void vkGetPhysicalDeviceQueueFamilyProperties(
			uint32_t *const pQueueFamilyPropertyCount,
			VkQueueFamilyProperties *const pQueueFamilyProperties) const noexcept;

		void vkGetPhysicalDeviceMemoryProperties2(
			VkPhysicalDeviceMemoryProperties2 *const pMemoryProperties) const noexcept;

		[[nodiscard]]
		VkBool32 vkGetPhysicalDeviceWin32PresentationSupportKHR(const uint32_t queueFamilyIndex) const noexcept;

		VkResult vkGetPhysicalDeviceSurfaceSupportKHR(
			const uint32_t queueFamilyIndex, const VkSurfaceKHR surface,
			VkBool32 *const pSupported) const noexcept;

		VkResult vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
			const VkSurfaceKHR surface,
			VkSurfaceCapabilitiesKHR *const pSurfaceCapabilities) const noexcept;

		VkResult vkGetPhysicalDeviceSurfaceFormatsKHR(
			const VkSurfaceKHR surface, uint32_t *const pSurfaceFormatCount,
			VkSurfaceFormatKHR *const pSurfaceFormats) const noexcept;

		VkResult vkGetPhysicalDeviceSurfacePresentModesKHR(
			const VkSurfaceKHR surface, uint32_t *const pPresentModeCount,
			VkPresentModeKHR *const pPresentModes) const noexcept;

		VkResult vkCreateDevice(
			const VkDeviceCreateInfo *const pCreateInfo,
			const VkAllocationCallbacks *const pAllocator, VkDevice *const pDevice) noexcept;

	private:
		Instance &__instance;
	};
}