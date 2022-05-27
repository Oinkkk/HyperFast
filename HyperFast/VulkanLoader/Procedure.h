#pragma once

#include <vulkan/vulkan.h>

namespace VKL
{
	class GlobalProcedure
	{
	public:
		PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr{};
		PFN_vkEnumerateInstanceVersion vkEnumerateInstanceVersion{};
		PFN_vkEnumerateInstanceExtensionProperties vkEnumerateInstanceExtensionProperties{};
		PFN_vkEnumerateInstanceLayerProperties vkEnumerateInstanceLayerProperties{};
		PFN_vkCreateInstance vkCreateInstance{};
	};

	class InstanceProcedure
	{
	public:
		PFN_vkDestroyInstance vkDestroyInstance{};
		PFN_vkCreateDebugUtilsMessengerEXT vkCreateDebugUtilsMessengerEXT{};
		PFN_vkDestroyDebugUtilsMessengerEXT vkDestroyDebugUtilsMessengerEXT{};
		PFN_vkEnumeratePhysicalDevices vkEnumeratePhysicalDevices{};
		PFN_vkEnumeratePhysicalDeviceGroups vkEnumeratePhysicalDeviceGroups{};
		PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties{};
		PFN_vkGetPhysicalDeviceProperties2 vkGetPhysicalDeviceProperties2{};
		PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties{};
		PFN_vkGetPhysicalDeviceQueueFamilyProperties2 vkGetPhysicalDeviceQueueFamilyProperties2{};
		PFN_vkGetPhysicalDeviceWin32PresentationSupportKHR vkGetPhysicalDeviceWin32PresentationSupportKHR{};
		PFN_vkCreateWin32SurfaceKHR vkCreateWin32SurfaceKHR;
		PFN_vkDestroySurfaceKHR vkDestroySurfaceKHR{};
		PFN_vkGetPhysicalDeviceSurfaceSupportKHR vkGetPhysicalDeviceSurfaceSupportKHR{};
		PFN_vkGetPhysicalDeviceSurfaceCapabilitiesKHR vkGetPhysicalDeviceSurfaceCapabilitiesKHR{};
		PFN_vkGetPhysicalDeviceSurfaceFormatsKHR vkGetPhysicalDeviceSurfaceFormatsKHR{};
		PFN_vkGetPhysicalDeviceSurfacePresentModesKHR vkGetPhysicalDeviceSurfacePresentModesKHR{};
		PFN_vkCreateDevice vkCreateDevice{};
		PFN_vkGetDeviceProcAddr vkGetDeviceProcAddr{};
	};

	class DeviceProcedure
	{
	public:
		PFN_vkDestroyDevice vkDestroyDevice{};
		PFN_vkGetDeviceQueue vkGetDeviceQueue{};
		PFN_vkCreateCommandPool vkCreateCommandPool{};
		PFN_vkDestroyCommandPool vkDestroyCommandPool{};
		PFN_vkCreateShaderModule vkCreateShaderModule{};
		PFN_vkDestroyShaderModule vkDestroyShaderModule{};
	};
}