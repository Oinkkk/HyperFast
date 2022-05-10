#include "GlobalProcedureLoader.h"
#include <exception>

namespace ObjectVK
{
	GlobalProcedureLoader::~GlobalProcedureLoader()
	{
		try
		{
			free();
		}
		catch (...)
		{ }
	}

	void GlobalProcedureLoader::load()
	{
		if (__loaderHandle)
			return;

		__loaderHandle = LoadLibrary("vulkan-1.dll");
		if (!__loaderHandle)
			throw std::exception{ "Cannot find a vulkan loader." };

		const PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr
		{
			reinterpret_cast<PFN_vkGetInstanceProcAddr>(
				GetProcAddress(__loaderHandle, "vkGetInstanceProcAddr"))
		};

		__loadProcedure(vkGetInstanceProcAddr);
	}

	void GlobalProcedureLoader::free()
	{
		if (!__loaderHandle)
			return;

		if (!FreeLibrary(__loaderHandle))
			throw std::exception{ "Cannot free a vulkan loader." };

		__loaderHandle = nullptr;
	}

	GlobalProcedureLoader &GlobalProcedureLoader::getInstance() noexcept
	{
		static GlobalProcedureLoader instance;
		return instance;
	}

	void GlobalProcedureLoader::__loadProcedure(const PFN_vkGetInstanceProcAddr vkGetInstanceProcAddr) noexcept
	{
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