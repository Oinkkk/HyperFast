#include "ScreenManager.h"

namespace HyperFast
{
	ScreenManager::ScreenManager(
		const VkInstance instance, const VKL::InstanceProcedure &instanceProc,
		const VkPhysicalDevice firstPhysicalDevice, const uint32_t graphicsQueueFamilyIndex,
		const VkDevice device, const VKL::DeviceProcedure &deviceProc) noexcept :
		__instance{ instance }, __instanceProc{ instanceProc },
		__firstPhysicalDevice{ firstPhysicalDevice }, __graphicsQueueFamilyIndex{ graphicsQueueFamilyIndex },
		__device{ device }, __deviceProc{ deviceProc }
	{}

	std::unique_ptr<ScreenManager::ScreenImpl> ScreenManager::create(Win::Window &window) noexcept
	{
		return std::make_unique<ScreenImpl>(
			__instance, __instanceProc,
			__firstPhysicalDevice, __graphicsQueueFamilyIndex,
			__device, __deviceProc, window);
	}

	ScreenManager::ScreenImpl::ScreenImpl(
		const VkInstance instance, const VKL::InstanceProcedure &instanceProc,
		const VkPhysicalDevice firstPhysicalDevice, const uint32_t graphicsQueueFamilyIndex,
		const VkDevice device, const VKL::DeviceProcedure &deviceProc, Win::Window &window) :
		__instance{ instance }, __instanceProc{ instanceProc },
		__firstPhysicalDevice{ firstPhysicalDevice }, __graphicsQueueFamilyIndex{ graphicsQueueFamilyIndex },
		__device{ device }, __deviceProc{ deviceProc }, __window{ window },
		__surface{ __createSurface(instance, instanceProc, window) },
		__pipelineFactory{ device, deviceProc }
	{
		if (!__surface)
			throw std::exception{ "Cannot create a surface." };

		__checkSurfaceSupport();
		__querySurfaceCapabilities();
		__querySupportedSurfaceFormats();
		__initPipelineFactoryBuildParam();
		__pipelineFactory.build(__pipelineFactoryBuildParam);
	}

	ScreenManager::ScreenImpl::~ScreenImpl() noexcept
	{
		__destroySurface();
	}

	void ScreenManager::ScreenImpl::__destroySurface() noexcept
	{
		__instanceProc.vkDestroySurfaceKHR(__instance, __surface, nullptr);
	}

	void ScreenManager::ScreenImpl::__checkSurfaceSupport() const
	{
		VkBool32 surfaceSupported{};
		__instanceProc.vkGetPhysicalDeviceSurfaceSupportKHR(
			__firstPhysicalDevice, __graphicsQueueFamilyIndex, __surface, &surfaceSupported);

		if (!surfaceSupported)
			throw std::exception{ "The physical device doesn't support the surface." };
	}

	void ScreenManager::ScreenImpl::__querySurfaceCapabilities() noexcept
	{
		__instanceProc.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
			__firstPhysicalDevice, __surface, &__surfaceCapabilities);
	}

	void ScreenManager::ScreenImpl::__querySupportedSurfaceFormats() noexcept
	{
		uint32_t numFormats{};
		__instanceProc.vkGetPhysicalDeviceSurfaceFormatsKHR(
			__firstPhysicalDevice, __surface, &numFormats, nullptr);

		__supportedSurfaceFormats.resize(numFormats);
		__instanceProc.vkGetPhysicalDeviceSurfaceFormatsKHR(
			__firstPhysicalDevice, __surface, &numFormats, __supportedSurfaceFormats.data());
	}

	void ScreenManager::ScreenImpl::__initPipelineFactoryBuildParam() noexcept
	{
		__pipelineFactoryBuildParam.viewportWidth = float(__window.getWidth());
		__pipelineFactoryBuildParam.viewportHeight = float(__window.getHeight());
	}

	VkSurfaceKHR ScreenManager::ScreenImpl::__createSurface(
		const VkInstance instance, const VKL::InstanceProcedure &instanceProc, Win::Window &window)
	{
		Win::WindowClass &windowClass{ window.getClass() };

		const VkWin32SurfaceCreateInfoKHR createInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
			.hinstance = windowClass.getHInstance(),
			.hwnd = window.getHandle()
		};

		VkSurfaceKHR retVal{};
		instanceProc.vkCreateWin32SurfaceKHR(instance, &createInfo, nullptr, &retVal);

		return retVal;
	}
}