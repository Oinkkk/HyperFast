#include "ScreenManager.h"

namespace HyperFast
{
	ScreenManager::ScreenManager(const VkInstance instance, const VKL::InstanceProcedure &instanceProc) noexcept :
		__instance{ instance }, __instanceProc{ instanceProc }
	{}

	VkSurfaceKHR ScreenManager::create(Win::Window &window) noexcept
	{
		Win::WindowClass &windowClass{ window.getClass() };

		const VkWin32SurfaceCreateInfoKHR createInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
			.hinstance = windowClass.getHInstance(),
			.hwnd = window.getHandle()
		};

		VkSurfaceKHR retVal{};
		__instanceProc.vkCreateWin32SurfaceKHR(__instance, &createInfo, nullptr, &retVal);

		return retVal;
	}

	void ScreenManager::destroy(const VkSurfaceKHR handle) noexcept
	{
		__instanceProc.vkDestroySurfaceKHR(__instance, handle, nullptr);
	}
}