#pragma once

#include <vulkan/vulkan.h>

namespace ObjectVK
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
}