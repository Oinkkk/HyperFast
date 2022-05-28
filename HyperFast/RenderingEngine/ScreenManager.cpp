#include "ScreenManager.h"

namespace HyperFast
{
	ScreenManager::ScreenManager(
		const VkInstance instance, const VKL::InstanceProcedure &instanceProc,
		const VkPhysicalDevice physicalDevice, const uint32_t graphicsQueueFamilyIndex,
		const VkDevice device, const VKL::DeviceProcedure &deviceProc, Infra::Logger &logger) noexcept :
		__instance{ instance }, __instanceProc{ instanceProc },
		__physicalDevice{ physicalDevice }, __graphicsQueueFamilyIndex{ graphicsQueueFamilyIndex },
		__device{ device }, __deviceProc{ deviceProc }, __logger{ logger }
	{}

	std::unique_ptr<ScreenManager::ScreenImpl> ScreenManager::create(Win::Window &window) noexcept
	{
		return std::make_unique<ScreenImpl>(
			__instance, __instanceProc,
			__physicalDevice, __graphicsQueueFamilyIndex,
			__device, __deviceProc, window, __logger);
	}

	ScreenManager::ScreenImpl::ScreenImpl(
		const VkInstance instance, const VKL::InstanceProcedure &instanceProc,
		const VkPhysicalDevice physicalDevice, const uint32_t graphicsQueueFamilyIndex,
		const VkDevice device, const VKL::DeviceProcedure &deviceProc, Win::Window &window,
		Infra::Logger &logger) :
		__instance{ instance }, __instanceProc{ instanceProc },
		__physicalDevice{ physicalDevice }, __graphicsQueueFamilyIndex{ graphicsQueueFamilyIndex },
		__device{ device }, __deviceProc{ deviceProc }, __window{ window }, __logger{ logger },
		__pipelineFactory{ device, deviceProc }
	{
		__createSurface();
	}

	ScreenManager::ScreenImpl::~ScreenImpl() noexcept
	{
		__reset();
		__destroySurface();
	}

	void ScreenManager::ScreenImpl::draw() noexcept
	{
		if (!__swapchain)
			__init();
	}

	void ScreenManager::ScreenImpl::__init()
	{
		__checkSurfaceSupport();
		__querySurfaceCapabilities();
		__querySupportedSurfaceFormats();
		__querySupportedSurfacePresentModes();
		__createSwapchain();
		__retrieveSwapchainImages();
	}

	void ScreenManager::ScreenImpl::__reset() noexcept
	{
		__destroySwapchain();
		__swapchain = nullptr;
	}

	void ScreenManager::ScreenImpl::__createSurface()
	{
		Win::WindowClass &windowClass{ __window.getClass() };

		const VkWin32SurfaceCreateInfoKHR createInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
			.hinstance = windowClass.getHInstance(),
			.hwnd = __window.getHandle()
		};

		 __instanceProc.vkCreateWin32SurfaceKHR(__instance, &createInfo, nullptr, &__surface);
		 if (!__surface)
			 throw std::exception{ "Cannot create a surface." };
	}

	void ScreenManager::ScreenImpl::__destroySurface() noexcept
	{
		__instanceProc.vkDestroySurfaceKHR(__instance, __surface, nullptr);
	}

	void ScreenManager::ScreenImpl::__checkSurfaceSupport() const
	{
		VkBool32 surfaceSupported{};
		__instanceProc.vkGetPhysicalDeviceSurfaceSupportKHR(
			__physicalDevice, __graphicsQueueFamilyIndex, __surface, &surfaceSupported);

		if (!surfaceSupported)
			throw std::exception{ "The physical device doesn't support the surface." };
	}

	void ScreenManager::ScreenImpl::__querySurfaceCapabilities() noexcept
	{
		__instanceProc.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
			__physicalDevice, __surface, &__surfaceCapabilities);

		if (__surfaceCapabilities.maxImageCount <= 2U)
		{
			__logger.log(
				Infra::LogSeverityType::WARNING,
				R"(maxImageCount of SurfaceCapabilities is lower than 3.
				Triple buffering is not supported with this surface.)"
			);
		}
	}

	void ScreenManager::ScreenImpl::__querySupportedSurfaceFormats() noexcept
	{
		uint32_t numFormats{};
		__instanceProc.vkGetPhysicalDeviceSurfaceFormatsKHR(
			__physicalDevice, __surface, &numFormats, nullptr);

		__supportedSurfaceFormats.resize(numFormats);
		__instanceProc.vkGetPhysicalDeviceSurfaceFormatsKHR(
			__physicalDevice, __surface, &numFormats, __supportedSurfaceFormats.data());
	}

	void ScreenManager::ScreenImpl::__querySupportedSurfacePresentModes() noexcept
	{
		uint32_t numModes{};
		__instanceProc.vkGetPhysicalDeviceSurfacePresentModesKHR(
			__physicalDevice, __surface, &numModes, nullptr);

		__supportedSurfacePresentModes.resize(numModes);
		__instanceProc.vkGetPhysicalDeviceSurfacePresentModesKHR(
			__physicalDevice, __surface, &numModes, __supportedSurfacePresentModes.data());
	}

	void ScreenManager::ScreenImpl::__createSwapchain()
	{
		uint32_t numDesiredImages{ std::max(3U, __surfaceCapabilities.minImageCount) };
		if (__surfaceCapabilities.maxImageCount)
			numDesiredImages = std::min(numDesiredImages, __surfaceCapabilities.maxImageCount);

		const VkSurfaceFormatKHR *pDesiredFormat{};
		for (const VkSurfaceFormatKHR &surfaceFormat : __supportedSurfaceFormats)
		{
			const bool formatValid
			{
				(surfaceFormat.format == VkFormat::VK_FORMAT_B8G8R8A8_SRGB) ||
				(surfaceFormat.format == VkFormat::VK_FORMAT_R8G8B8A8_SRGB)
			};

			const bool colorSpaceValid
			{
				(surfaceFormat.colorSpace == VkColorSpaceKHR::VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
			};

			if (!formatValid || !colorSpaceValid)
				continue;

			pDesiredFormat = &surfaceFormat;
			break;
		}

		if (!pDesiredFormat)
		{
			__logger.log(
				Infra::LogSeverityType::WARNING,
				R"(There are no suitable sRGB surface formats.
				It may occurs weird feeling of color)"
			);

			pDesiredFormat = __supportedSurfaceFormats.data();
		}

		VkExtent2D imageExtent{};
		if (__surfaceCapabilities.currentExtent.width == 0xFFFFFFFFU)
		{
			imageExtent.width = uint32_t(__window.getWidth());
			imageExtent.height = uint32_t(__window.getHeight());
		}
		else
			imageExtent = __surfaceCapabilities.currentExtent;

		VkCompositeAlphaFlagBitsKHR compositeAlpha{};
		if (__surfaceCapabilities.supportedCompositeAlpha & VkCompositeAlphaFlagBitsKHR::VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
			compositeAlpha = VkCompositeAlphaFlagBitsKHR::VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		else if (__surfaceCapabilities.supportedCompositeAlpha & VkCompositeAlphaFlagBitsKHR::VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR)
			compositeAlpha = VkCompositeAlphaFlagBitsKHR::VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;

		VkPresentModeKHR desiredPresentMode{ VkPresentModeKHR::VK_PRESENT_MODE_FIFO_KHR };
		for (const VkPresentModeKHR presendMode : __supportedSurfacePresentModes)
		{
			if (presendMode != VkPresentModeKHR::VK_PRESENT_MODE_MAILBOX_KHR)
				continue;

			desiredPresentMode = presendMode;
			break;
		}

		const VkSwapchainCreateInfoKHR createInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
			.surface = __surface,
			.minImageCount = numDesiredImages,
			.imageFormat = pDesiredFormat->format,
			.imageColorSpace = pDesiredFormat->colorSpace,
			.imageExtent = imageExtent,
			.imageArrayLayers = 1U,
			.imageUsage = VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			.imageSharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE,
			.preTransform = __surfaceCapabilities.currentTransform,
			.compositeAlpha = compositeAlpha,
			.presentMode = desiredPresentMode,
			.clipped = VK_TRUE,
			.oldSwapchain = __swapchain
		};

		__deviceProc.vkCreateSwapchainKHR(__device, &createInfo, nullptr, &__swapchain);
		if (!__swapchain)
			throw std::exception{ "Cannot create a VkSwapchainKHR." };
	}

	void ScreenManager::ScreenImpl::__destroySwapchain() noexcept
	{
		__deviceProc.vkDestroySwapchainKHR(__device, __swapchain, nullptr);
	}

	void ScreenManager::ScreenImpl::__retrieveSwapchainImages() noexcept
	{
		uint32_t numImages{};
		__deviceProc.vkGetSwapchainImagesKHR(__device, __swapchain, &numImages, nullptr);

		__swapChainImages.resize(numImages);
		__deviceProc.vkGetSwapchainImagesKHR(__device, __swapchain, &numImages, __swapChainImages.data());
	}

	void ScreenManager::ScreenImpl::__initPipelineFactoryBuildParam() noexcept
	{
		__pipelineFactoryBuildParam.viewportWidth = float(__window.getWidth());
		__pipelineFactoryBuildParam.viewportHeight = float(__window.getHeight());
	}

	void ScreenManager::ScreenImpl::__buildPipelines()
	{
		__pipelineFactory.build(__pipelineFactoryBuildParam);
	}
}