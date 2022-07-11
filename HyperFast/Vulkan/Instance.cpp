#include "Instance.h"
#include <exception>

namespace Vulkan
{
	Instance::Instance(const VkInstanceCreateInfo &createInfo) :
		Handle{ __create(createInfo) }
	{
		__queryProc();
	}

	Instance::~Instance() noexcept
	{
		__destroy();
	}

	VkResult Instance::vkCreateDebugUtilsMessengerEXT(
		const VkDebugUtilsMessengerCreateInfoEXT *const pCreateInfo,
		const VkAllocationCallbacks *const pAllocator, VkDebugUtilsMessengerEXT *const pMessenger) noexcept
	{
		return __proc.vkCreateDebugUtilsMessengerEXT(getHandle(), pCreateInfo, pAllocator, pMessenger);
	}

	void Instance::vkDestroyDebugUtilsMessengerEXT(
		const VkDebugUtilsMessengerEXT messenger, const VkAllocationCallbacks *const pAllocator) noexcept
	{
		__proc.vkDestroyDebugUtilsMessengerEXT(getHandle(), messenger, pAllocator);
	}

	VkResult Instance::vkEnumeratePhysicalDevices(
		uint32_t *const pPhysicalDeviceCount, VkPhysicalDevice *const pPhysicalDevices) const noexcept
	{
		return __proc.vkEnumeratePhysicalDevices(
			_getConstHandle(), pPhysicalDeviceCount, pPhysicalDevices);
	}

	void Instance::vkGetPhysicalDeviceProperties(
		const VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties *const pProperties) const noexcept
	{
		__proc.vkGetPhysicalDeviceProperties(physicalDevice, pProperties);
	}

	void Instance::vkGetPhysicalDeviceProperties2(
		const VkPhysicalDevice physicalDevice, VkPhysicalDeviceProperties2 *const pProperties) const noexcept
	{
		__proc.vkGetPhysicalDeviceProperties2(physicalDevice, pProperties);
	}

	void Instance::vkGetPhysicalDeviceQueueFamilyProperties(
		const VkPhysicalDevice physicalDevice, uint32_t *const pQueueFamilyPropertyCount,
		VkQueueFamilyProperties *const pQueueFamilyProperties) const noexcept
	{
		__proc.vkGetPhysicalDeviceQueueFamilyProperties(
			physicalDevice, pQueueFamilyPropertyCount, pQueueFamilyProperties);
	}

	void Instance::vkGetPhysicalDeviceMemoryProperties2(
		const VkPhysicalDevice physicalDevice,
		VkPhysicalDeviceMemoryProperties2 *const pMemoryProperties) const noexcept
	{
		__proc.vkGetPhysicalDeviceMemoryProperties2(physicalDevice, pMemoryProperties);
	}

	VkBool32 Instance::vkGetPhysicalDeviceWin32PresentationSupportKHR(
		const VkPhysicalDevice physicalDevice, const uint32_t queueFamilyIndex) const noexcept
	{
		return __proc.vkGetPhysicalDeviceWin32PresentationSupportKHR(physicalDevice, queueFamilyIndex);
	}

	VkResult Instance::vkCreateWin32SurfaceKHR(
		const VkWin32SurfaceCreateInfoKHR *const pCreateInfo,
		const VkAllocationCallbacks *const pAllocator, VkSurfaceKHR *const pSurface) noexcept
	{
		return __proc.vkCreateWin32SurfaceKHR(getHandle(), pCreateInfo, pAllocator, pSurface);
	}

	void Instance::vkDestroySurfaceKHR(
		const VkSurfaceKHR surface, const VkAllocationCallbacks *const pAllocator) noexcept
	{
		__proc.vkDestroySurfaceKHR(getHandle(), surface, pAllocator);
	}

	VkResult Instance::vkGetPhysicalDeviceSurfaceSupportKHR(
		const VkPhysicalDevice physicalDevice, const uint32_t queueFamilyIndex,
		const VkSurfaceKHR surface, VkBool32 *const pSupported) const noexcept
	{
		return __proc.vkGetPhysicalDeviceSurfaceSupportKHR(
			physicalDevice, queueFamilyIndex, surface, pSupported);
	}

	VkResult Instance::vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
		const VkPhysicalDevice physicalDevice, const VkSurfaceKHR surface,
		VkSurfaceCapabilitiesKHR *const pSurfaceCapabilities) const noexcept
	{
		return __proc.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
			physicalDevice, surface, pSurfaceCapabilities);
	}

	VkResult Instance::vkGetPhysicalDeviceSurfaceFormatsKHR(
		const VkPhysicalDevice physicalDevice, const VkSurfaceKHR surface,
		uint32_t *const pSurfaceFormatCount, VkSurfaceFormatKHR *const pSurfaceFormats) const noexcept
	{
		return __proc.vkGetPhysicalDeviceSurfaceFormatsKHR(
			physicalDevice, surface, pSurfaceFormatCount, pSurfaceFormats);
	}

	VkResult Instance::vkGetPhysicalDeviceSurfacePresentModesKHR(
		const VkPhysicalDevice physicalDevice, const VkSurfaceKHR surface,
		uint32_t *const pPresentModeCount, VkPresentModeKHR *const pPresentModes) const noexcept
	{
		return __proc.vkGetPhysicalDeviceSurfacePresentModesKHR(
			physicalDevice, surface, pPresentModeCount, pPresentModes);
	}

	VkResult Instance::vkCreateDevice(
		const VkPhysicalDevice physicalDevice, const VkDeviceCreateInfo *const pCreateInfo,
		const VkAllocationCallbacks *const pAllocator, VkDevice *const pDevice) noexcept
	{
		return __proc.vkCreateDevice(physicalDevice, pCreateInfo, pAllocator, pDevice);
	}

	DeviceProcedure Instance::queryDeviceProcedure(const VkDevice device) const noexcept
	{
		return VulkanLoader::getInstance().
			queryDeviceProcedure(__proc.vkGetDeviceProcAddr, device);
	}

	void Instance::__queryProc() noexcept
	{
		__proc = VulkanLoader::getInstance().queryInstanceProcedure(getHandle());
	}

	void Instance::__destroy() noexcept
	{
		__proc.vkDestroyInstance(getHandle(), nullptr);
	}

	VkInstance Instance::__create(const VkInstanceCreateInfo &createInfo)
	{
		const GlobalProcedure &globalProc{ VulkanLoader::getInstance().getGlobalProcedure() };

		VkInstance retVal{};
		globalProc.vkCreateInstance(&createInfo, nullptr, &retVal);

		if (!retVal)
			throw std::exception{ "Cannot create a VkInstance." };

		return retVal;
	}
}