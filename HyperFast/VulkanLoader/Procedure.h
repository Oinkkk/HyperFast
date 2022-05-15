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
		PFN_vkGetPhysicalDeviceProperties vkGetPhysicalDeviceProperties{};
		PFN_vkGetPhysicalDeviceProperties2 vkGetPhysicalDeviceProperties2{};
		PFN_vkGetPhysicalDeviceQueueFamilyProperties vkGetPhysicalDeviceQueueFamilyProperties{};
		PFN_vkGetPhysicalDeviceQueueFamilyProperties2 vkGetPhysicalDeviceQueueFamilyProperties2{};
		PFN_vkEnumeratePhysicalDeviceGroups vkEnumeratePhysicalDeviceGroups{};
	};
}