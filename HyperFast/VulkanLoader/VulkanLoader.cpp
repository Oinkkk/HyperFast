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

		__global_vkGetInstanceProcAddr =
			reinterpret_cast<PFN_vkGetInstanceProcAddr>(
				GetProcAddress(__handle, "vkGetInstanceProcAddr"));

		__loadGlobalProc();
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

		PFN_vkGetInstanceProcAddr instance_vkGetInstanceProcAddr =
			reinterpret_cast<PFN_vkGetInstanceProcAddr>(
				__global_vkGetInstanceProcAddr(instance, "vkGetInstanceProcAddr"));

		retVal.vkGetInstanceProcAddr = instance_vkGetInstanceProcAddr;

		retVal.vkDestroyInstance = reinterpret_cast<PFN_vkDestroyInstance>(
			instance_vkGetInstanceProcAddr(instance, "vkDestroyInstance"));

		return retVal;
	}

	VulkanLoader &VulkanLoader::getInstance() noexcept
	{
		static VulkanLoader instance;
		return instance;
	}

	void VulkanLoader::__loadGlobalProc() noexcept
	{
		__globalProc.vkEnumerateInstanceVersion =
			reinterpret_cast<PFN_vkEnumerateInstanceVersion>(
				__global_vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceVersion"));

		__globalProc.vkEnumerateInstanceExtensionProperties =
			reinterpret_cast<PFN_vkEnumerateInstanceExtensionProperties>(
				__global_vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceExtensionProperties"));

		__globalProc.vkEnumerateInstanceLayerProperties =
			reinterpret_cast<PFN_vkEnumerateInstanceLayerProperties>(
				__global_vkGetInstanceProcAddr(nullptr, "vkEnumerateInstanceLayerProperties"));

		__globalProc.vkCreateInstance =
			reinterpret_cast<PFN_vkCreateInstance>(
				__global_vkGetInstanceProcAddr(nullptr, "vkCreateInstance"));
	}
}