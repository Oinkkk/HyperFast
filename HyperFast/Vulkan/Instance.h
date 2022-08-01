#pragma once

#include "VulkanLoader.h"
#include "Handle.h"

namespace Vulkan
{
	class Instance final : public Handle<VkInstance>
	{
	public:
		Instance(const VkInstanceCreateInfo &createInfo);
		virtual ~Instance() noexcept;

		VkResult vkCreateDebugUtilsMessengerEXT(
			const VkDebugUtilsMessengerCreateInfoEXT *const pCreateInfo,
			const VkAllocationCallbacks *const pAllocator, VkDebugUtilsMessengerEXT *const pMessenger) noexcept;

		void vkDestroyDebugUtilsMessengerEXT(
			const VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks *const pAllocator) noexcept;

		VkResult vkEnumeratePhysicalDevices(
			uint32_t *const pPhysicalDeviceCount, VkPhysicalDevice *const pPhysicalDevices) const noexcept;

		void vkGetPhysicalDeviceProperties(
			const VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties *const pProperties) const noexcept;

		void vkGetPhysicalDeviceProperties2(
			const VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2 *const pProperties) const noexcept;

		void vkGetPhysicalDeviceQueueFamilyProperties(
			const VkPhysicalDevice physicalDevice, uint32_t *const pQueueFamilyPropertyCount,
			VkQueueFamilyProperties *const pQueueFamilyProperties) const noexcept;

		void vkGetPhysicalDeviceMemoryProperties2(
			const VkPhysicalDevice physicalDevice,
			VkPhysicalDeviceMemoryProperties2 *const pMemoryProperties) const noexcept;

		[[nodiscard]]
		VkBool32 vkGetPhysicalDeviceWin32PresentationSupportKHR(
			const VkPhysicalDevice physicalDevice, const uint32_t queueFamilyIndex) const noexcept;

		VkResult vkCreateWin32SurfaceKHR(
			const VkWin32SurfaceCreateInfoKHR *const pCreateInfo,
			const VkAllocationCallbacks *const pAllocator, VkSurfaceKHR *const pSurface) noexcept;

		void vkDestroySurfaceKHR(
			const VkSurfaceKHR surface, const VkAllocationCallbacks *const pAllocator) noexcept;

		VkResult vkGetPhysicalDeviceSurfaceSupportKHR(
			const VkPhysicalDevice physicalDevice, const uint32_t queueFamilyIndex,
			const VkSurfaceKHR surface, VkBool32 *const pSupported) const noexcept;

		VkResult vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
			const VkPhysicalDevice physicalDevice, const VkSurfaceKHR surface,
			VkSurfaceCapabilitiesKHR *const pSurfaceCapabilities) const noexcept;

		VkResult vkGetPhysicalDeviceSurfaceFormatsKHR(
			const VkPhysicalDevice physicalDevice, const VkSurfaceKHR surface,
			uint32_t *const pSurfaceFormatCount, VkSurfaceFormatKHR *const pSurfaceFormats) const noexcept;

		VkResult vkGetPhysicalDeviceSurfacePresentModesKHR(
			const VkPhysicalDevice physicalDevice, const VkSurfaceKHR surface,
			uint32_t *const pPresentModeCount, VkPresentModeKHR *const pPresentModes) const noexcept;

		VkResult vkCreateDevice(
			const VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *const pCreateInfo,
			const VkAllocationCallbacks *const pAllocator, VkDevice *const pDevice) noexcept;

		[[nodiscard]]
		DeviceProcedure queryDeviceProcedure(const VkDevice device) const noexcept;

	private:
		InstanceProcedure __proc{};

		void __queryProc() noexcept;
		void __destroy() noexcept;

		static VkInstance __create(const VkInstanceCreateInfo &createInfo);
	};
}
