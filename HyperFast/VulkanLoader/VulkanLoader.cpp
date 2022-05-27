#include "VulkanLoader.h"
#include <exception>
#include <cassert>

namespace VKL
{
	VulkanLoader::~VulkanLoader()
	{
		try
		{
			free();
		}
		catch (...)
		{ }
	}

	void VulkanLoader::load()
	{
		if (__handle)
			return;

		__handle = LoadLibrary("vulkan-1.dll");
		if (!__handle)
			throw std::exception{ "Cannot find a vulkan loader." };

		const PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr
		{
			reinterpret_cast<PFN_vkGetInstanceProcAddr>(
				GetProcAddress(__handle, "vkGetInstanceProcAddr"))
		};

		__loadGlobalProc(vkGetInstanceProcAddr);
	}

	void VulkanLoader::free()
	{
		if (!__handle)
			return;

		if (!FreeLibrary(__handle))
			throw std::exception{ "Cannot free a vulkan loader." };

		__handle = nullptr;
	}

	InstanceProcedure VulkanLoader::queryInstanceProcedure(const VkInstance instance) noexcept
	{
		assert(__handle);

		InstanceProcedure retVal;

		retVal.vkDestroyInstance =
			reinterpret_cast<PFN_vkDestroyInstance>(
				__globalProc.vkGetInstanceProcAddr(instance, "vkDestroyInstance"));

		retVal.vkCreateDebugUtilsMessengerEXT =
			reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(
				__globalProc.vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT"));

		retVal.vkDestroyDebugUtilsMessengerEXT =
			reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(
				__globalProc.vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT"));

		retVal.vkEnumeratePhysicalDevices =
			reinterpret_cast<PFN_vkEnumeratePhysicalDevices>(
				__globalProc.vkGetInstanceProcAddr(instance, "vkEnumeratePhysicalDevices"));

		retVal.vkEnumeratePhysicalDeviceGroups =
			reinterpret_cast<PFN_vkEnumeratePhysicalDeviceGroups>(
				__globalProc.vkGetInstanceProcAddr(instance, "vkEnumeratePhysicalDeviceGroups"));

		retVal.vkGetPhysicalDeviceProperties =
			reinterpret_cast<PFN_vkGetPhysicalDeviceProperties>(
				__globalProc.vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceProperties"));

		retVal.vkGetPhysicalDeviceProperties2 =
			reinterpret_cast<PFN_vkGetPhysicalDeviceProperties2>(
				__globalProc.vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceProperties2"));

		retVal.vkGetPhysicalDeviceQueueFamilyProperties =
			reinterpret_cast<PFN_vkGetPhysicalDeviceQueueFamilyProperties>(
				__globalProc.vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceQueueFamilyProperties"));

		retVal.vkGetPhysicalDeviceQueueFamilyProperties2 =
			reinterpret_cast<PFN_vkGetPhysicalDeviceQueueFamilyProperties2>(
				__globalProc.vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceQueueFamilyProperties2"));

		retVal.vkGetPhysicalDeviceWin32PresentationSupportKHR =
			reinterpret_cast<PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR>(
				__globalProc.vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceWin32PresentationSupportKHR"));

		retVal.vkCreateWin32SurfaceKHR =
			reinterpret_cast<PFN_vkCreateWin32SurfaceKHR>(
				__globalProc.vkGetInstanceProcAddr(instance, "vkCreateWin32SurfaceKHR"));

		retVal.vkDestroySurfaceKHR =
			reinterpret_cast<PFN_vkDestroySurfaceKHR>(
				__globalProc.vkGetInstanceProcAddr(instance, "vkDestroySurfaceKHR"));

		retVal.vkGetPhysicalDeviceSurfaceSupportKHR =
			reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceSupportKHR>(
				__globalProc.vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceSupportKHR"));

		retVal.vkGetPhysicalDeviceSurfaceCapabilitiesKHR =
			reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR>(
				__globalProc.vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceCapabilitiesKHR"));

		retVal.vkGetPhysicalDeviceSurfaceFormatsKHR =
			reinterpret_cast<PFN_vkGetPhysicalDeviceSurfaceFormatsKHR>(
				__globalProc.vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfaceFormatsKHR"));

		retVal.vkGetPhysicalDeviceSurfacePresentModesKHR =
			reinterpret_cast<PFN_vkGetPhysicalDeviceSurfacePresentModesKHR>(
				__globalProc.vkGetInstanceProcAddr(instance, "vkGetPhysicalDeviceSurfacePresentModesKHR"));

		retVal.vkCreateDevice =
			reinterpret_cast<PFN_vkCreateDevice>(
				__globalProc.vkGetInstanceProcAddr(instance, "vkCreateDevice"));

		retVal.vkGetDeviceProcAddr =
			reinterpret_cast<PFN_vkGetDeviceProcAddr>(
				__globalProc.vkGetInstanceProcAddr(instance, "vkGetDeviceProcAddr"));

		return retVal;
	}

	DeviceProcedure VulkanLoader::queryDeviceProcedure(
		const PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr, const VkDevice device) noexcept
	{
		assert(__handle);

		DeviceProcedure retVal;

		retVal.vkDestroyDevice =
			reinterpret_cast<PFN_vkDestroyDevice>(
				vkGetDeviceProcAddr(device, "vkDestroyDevice"));

		retVal.vkGetDeviceQueue =
			reinterpret_cast<PFN_vkGetDeviceQueue>(
				vkGetDeviceProcAddr(device, "vkGetDeviceQueue"));

		retVal.vkCreateCommandPool =
			reinterpret_cast<PFN_vkCreateCommandPool>(
				vkGetDeviceProcAddr(device, "vkCreateCommandPool"));

		retVal.vkDestroyCommandPool =
			reinterpret_cast<PFN_vkDestroyCommandPool>(
				vkGetDeviceProcAddr(device, "vkDestroyCommandPool"));

		retVal.vkCreateShaderModule =
			reinterpret_cast<PFN_vkCreateShaderModule>(
				vkGetDeviceProcAddr(device, "vkCreateShaderModule"));

		retVal.vkDestroyShaderModule =
			reinterpret_cast<PFN_vkDestroyShaderModule>(
				vkGetDeviceProcAddr(device, "vkDestroyShaderModule"));

		return retVal;
	}

	VulkanLoader &VulkanLoader::getInstance() noexcept
	{
		static VulkanLoader instance;
		return instance;
	}

	void VulkanLoader::__loadGlobalProc(const PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr) noexcept
	{
		// vkGetInstanceProcAddr(instance, "vkGetInstanceProcAddr") 반환 값이 nullptr 나옴
		__globalProc.vkGetInstanceProcAddr = vkGetInstanceProcAddr;

		__globalProc.vkEnumerateInstanceVersion =
			reinterpret_cast<PFN_vkEnumerateInstanceVersion>(
				vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceVersion"));

		__globalProc.vkEnumerateInstanceExtensionProperties =
			reinterpret_cast<PFN_vkEnumerateInstanceExtensionProperties>(
				vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceExtensionProperties"));

		__globalProc.vkEnumerateInstanceLayerProperties =
			reinterpret_cast<PFN_vkEnumerateInstanceLayerProperties>(
				vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceLayerProperties"));

		__globalProc.vkCreateInstance =
			reinterpret_cast<PFN_vkCreateInstance>(
				vkGetInstanceProcAddr(nullptr, "vkCreateInstance"));
	}
}