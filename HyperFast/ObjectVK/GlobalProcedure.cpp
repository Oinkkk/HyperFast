#include "GlobalProcedure.h"
#include <exception>

namespace ObjectVK
{
	GlobalProcedure::~GlobalProcedure()
	{
		try
		{
			free();
		}
		catch (...)
		{ }
	}

	void GlobalProcedure::load()
	{
		if (__loaderHandle)
			return;

		__loaderHandle = LoadLibrary("vulkan-1.dll");
		if (!__loaderHandle)
			throw std::exception{ "Cannot find a vulkan loader." };
	}

	void GlobalProcedure::free()
	{
		if (!__loaderHandle)
			return;

		if (!FreeLibrary(__loaderHandle))
			throw std::exception{ "Cannot free a vulkan loader." };

		__loaderHandle = nullptr;
	}

	GlobalProcedure &GlobalProcedure::getInstance() noexcept
	{
		static GlobalProcedure instance;
		return instance;
	}
}