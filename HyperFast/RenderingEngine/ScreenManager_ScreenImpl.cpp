﻿#include "ScreenManager.h"
#include "RenderingEngine.h"

namespace HyperFast
{
	ScreenManager::ScreenImpl::ScreenImpl(
		RenderingEngine &renderingEngine,
		Vulkan::Instance &instance, Vulkan::PhysicalDevice &physicalDevice,
		const uint32_t queueFamilyIndex, Vulkan::Device &device,
		Vulkan::Queue &queue, Win::Window &window) :
		__renderingEngine{ renderingEngine }, __instance { instance },
		__physicalDevice{ physicalDevice }, __queueFamilyIndex{ queueFamilyIndex },
		__device{ device }, __queue{ queue }, __window{ window }
	{
		__initListeners();
		__createResourceChain();
		__createSurface();
		__initSubmitInfo();
		__initSurfaceDependencies();
	}

	ScreenManager::ScreenImpl::~ScreenImpl() noexcept
	{
		__destroy();
	}

	void ScreenManager::ScreenImpl::setDrawcall(Drawcall *const pDrawcall) noexcept
	{
		Drawcall *&pCurrentDrawCall{ __resourceParam.pDrawcall };

		if (pCurrentDrawCall == pDrawcall)
			return;

		if (pCurrentDrawCall)
		{
			pCurrentDrawCall->getAttributeFlagsUpdateEvent() -= __pAttribFlagsUpdateEventListener;
			pCurrentDrawCall->getIndirectBufferUpdateEvent() -= __pIndirectBufferUpdateListener;
			pCurrentDrawCall->getIndirectBufferCreateEvent() -= __pIndirectBufferCreateListener;
		}

		pCurrentDrawCall = pDrawcall;

		if (pCurrentDrawCall)
		{
			pCurrentDrawCall->getAttributeFlagsUpdateEvent() += __pAttribFlagsUpdateEventListener;
			pCurrentDrawCall->getIndirectBufferUpdateEvent() += __pIndirectBufferUpdateListener;
			pCurrentDrawCall->getIndirectBufferCreateEvent() += __pIndirectBufferCreateListener;
		}

		__needToUpdatePipelineDependencies = true;
	}

	void ScreenManager::ScreenImpl::render()
	{
		if(!__isValid())
			return;

		__update();

		if (__needToRender)
			__render();
	}

	void ScreenManager::ScreenImpl::present() noexcept
	{
		if (!__isValid())
			return;

		if (__needToPresent)
			__present();
	}

	void ScreenManager::ScreenImpl::__update()
	{
		ScreenResource &backResource{ __getBackResource() };
		if (!(backResource.isIdle()))
			return;

		if (__needToSwapResources)
		{
			__swapResources();
			__needToSwapResources = false;
			__needToRender = true;
		}

		if (__needToUpdateSurfaceDependencies)
		{
			backResource.updateSwapchainDependencies();
			__needToUpdateSurfaceDependencies = false;
			__needToUpdatePipelineDependencies = false;
			__needToUpdateMainCommands = false;
			__needToSwapResources = true;
		}

		if (__needToUpdatePipelineDependencies)
		{
			backResource.updatePipelineDependencies();
			__needToUpdatePipelineDependencies = false;
			__needToUpdateMainCommands = false;
			__needToSwapResources = true;
		}

		if (__needToUpdateMainCommands)
		{
			backResource.updateMainCommands();
			__needToUpdateMainCommands = false;
			__needToSwapResources = true;
		}
	}

	void ScreenManager::ScreenImpl::__render() noexcept
	{
		Vulkan::Semaphore &imageAcquireSemaphore{ __getCurrentImageAcquireSemaphore() };

		const bool validAcquire{ __acquireNextSwapchainImageIdx(imageAcquireSemaphore) };
		if (!validAcquire)
			return;

		Vulkan::Semaphore &renderCompletionTimelineSemaphore{ __getCurrentRenderCompletionTimelineSemaphore() };
		uint64_t &renderCompletionSemaphoreValue{ __getCurrentRenderCompletionSemaphoreValue() };

		const VkResult waitResult
		{
			renderCompletionTimelineSemaphore.wait(renderCompletionSemaphoreValue, 0ULL)
		};

		// 앞전에 submit된 command buffer가 아직 처리 중
		if (waitResult == VkResult::VK_TIMEOUT)
			return;

		renderCompletionSemaphoreValue++;

		 __submitWaitInfo.semaphore = imageAcquireSemaphore.getHandle();

		Vulkan::CommandBuffer &renderCommandBuffer{ __getCurrentRenderCommandBuffer() };
		__submitCommandBufferInfo.commandBuffer = renderCommandBuffer.getHandle();

		VkSemaphoreSubmitInfo &binarySignalInfo{ __submitSignalInfos[0] };
		VkSemaphoreSubmitInfo &timelineSignalInfo{ __submitSignalInfos[1] };

		Vulkan::Semaphore &renderCompletionBinarySemaphore{ __getCurrentRenderCompletionBinarySemaphore() };
		binarySignalInfo.semaphore = renderCompletionBinarySemaphore.getHandle();

		timelineSignalInfo.semaphore = renderCompletionTimelineSemaphore.getHandle();
		timelineSignalInfo.value = renderCompletionSemaphoreValue;

		__renderingEngine.enqueueCommands(
			SubmitLayerType::GRAPHICS,
			1U, &__submitWaitInfo, 1U, &__submitCommandBufferInfo,
			uint32_t(std::size(__submitSignalInfos)), __submitSignalInfos);

		__needToRender = false;
		__needToPresent = true;
	}

	void ScreenManager::ScreenImpl::__present() noexcept
	{
		Vulkan::Semaphore &attachmentOutputSemaphore{ __getCurrentRenderCompletionBinarySemaphore() };

		const VkPresentInfoKHR presentInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.waitSemaphoreCount = 1U,
			.pWaitSemaphores = &(attachmentOutputSemaphore.getHandle()),
			.swapchainCount = 1U,
			.pSwapchains = &(__pSwapchain->getHandle()),
			.pImageIndices = &__imageIdx
		};

		const VkResult presentResult{ __queue.vkQueuePresentKHR(&presentInfo) };
		__imageAcquired = false;

		if ((presentResult == VkResult::VK_SUBOPTIMAL_KHR) ||
			(presentResult == VkResult::VK_ERROR_OUT_OF_DATE_KHR))
		{
			__needToUpdateSurfaceDependencies = true;
			__needToRender = true;
		}

		if (presentResult == VkResult::VK_SUCCESS)
			__needToPresent = false;
	}

	void ScreenManager::ScreenImpl::__destroy() noexcept
	{
		if (__destroyed)
			return;

		__device.vkDeviceWaitIdle();

		__renderCompletionTimelineSemaphores.clear();
		__renderCompletionBinarySemaphores.clear();
		__imageAcquireSemaphores.clear();

		__resourceChain[1] = nullptr;
		__resourceChain[0] = nullptr;

		__pSwapchain = nullptr;
		__pSurface = nullptr;
	
		__destroyed = true;
	}

	void ScreenManager::ScreenImpl::__initListeners() noexcept
	{
		__pResizeEventListener = Infra::EventListener<Win::Window &, Win::Window::ResizingType>::
			make([this](Win::Window &window, const Win::Window::ResizingType resizingType)
		{
			if (resizingType == Win::Window::ResizingType::MINIMIZED)
				return;

			__needToUpdateSurfaceDependencies = true;
		});

		__pDrawEventListener = Infra::EventListener<Win::Window &>::make([this] (Win::Window &window)
		{
			__needToRender = true;
		});

		__pDestroyEventListener = Infra::EventListener<Win::Window &>::make([this] (Win::Window &)
		{
			__destroy();
		});

		__pAttribFlagsUpdateEventListener = Infra::EventListener<Drawcall &>::make([this] (Drawcall &)
		{
			__needToUpdatePipelineDependencies = true;
		});

		__pIndirectBufferUpdateListener = Infra::EventListener<Drawcall &>::make([this](Drawcall &)
		{
			__needToRender = true;
		});

		__pIndirectBufferCreateListener = Infra::EventListener<Drawcall &>::make([this](Drawcall &)
		{
			__needToUpdateMainCommands = true;
		});

		__window.getResizeEvent() += __pResizeEventListener;
		__window.getDrawEvent() += __pDrawEventListener;
		__window.getDestroyEvent() += __pDestroyEventListener;
	}

	void ScreenManager::ScreenImpl::__createResourceChain() noexcept
	{
		__resourceChain[0] = std::make_unique<ScreenResource>(
			__device, __resourceParam, __queueFamilyIndex);

		__resourceChain[1] = std::make_unique<ScreenResource>(
			__device, __resourceParam, __queueFamilyIndex);
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

		__pSurface = std::make_unique<Vulkan::Surface>(__instance, createInfo);
	}

	void ScreenManager::ScreenImpl::__initSurfaceDependencies()
	{
		tf::Taskflow taskflow;

		__checkSurfaceSupport();
		__querySurfaceCapabilities();
		__querySupportedSurfaceFormats();
		__querySupportedSurfacePresentModes();
		__createSwapchain();

		const size_t numSwapchainImages{ __resourceParam.swapChainImages.size() };
		__imageAcquireSemaphores.resize(numSwapchainImages);
		__renderCompletionBinarySemaphores.resize(numSwapchainImages);
		__renderCompletionTimelineSemaphores.resize(numSwapchainImages);
		__renderCompletionSemaphoreValues.resize(numSwapchainImages);

		taskflow.emplace([this, numSwapchainImages](tf::Subflow &subflow)
		{
			for (
				size_t swapchainImageIter = 0ULL;
				swapchainImageIter < numSwapchainImages;
				swapchainImageIter++)
			{
				subflow.emplace([this, imageIdx = swapchainImageIter]
				{
					__createRenderSemaphores(imageIdx);
				});
			}
		});

		tf::Executor &executor{ Infra::Environment::getInstance().getTaskflowExecutor() };
		executor.run(taskflow).wait();

		__needToUpdateSurfaceDependencies = true;
	}

	void ScreenManager::ScreenImpl::__checkSurfaceSupport() const
	{
		VkBool32 surfaceSupported{};
		__physicalDevice.vkGetPhysicalDeviceSurfaceSupportKHR(
			__queueFamilyIndex, __pSurface->getHandle(), &surfaceSupported);

		if (!surfaceSupported)
			throw std::exception{ "The physical device doesn't support the surface." };
	}

	void ScreenManager::ScreenImpl::__querySurfaceCapabilities() noexcept
	{
		__physicalDevice.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
			__pSurface->getHandle(), &__surfaceCapabilities);
	}

	void ScreenManager::ScreenImpl::__querySupportedSurfaceFormats() noexcept
	{
		uint32_t numFormats{};
		__physicalDevice.vkGetPhysicalDeviceSurfaceFormatsKHR(
			__pSurface->getHandle(), &numFormats, nullptr);

		__supportedSurfaceFormats.resize(numFormats);
		__physicalDevice.vkGetPhysicalDeviceSurfaceFormatsKHR(
			__pSurface->getHandle(), &numFormats, __supportedSurfaceFormats.data());
	}

	void ScreenManager::ScreenImpl::__querySupportedSurfacePresentModes() noexcept
	{
		uint32_t numModes{};
		__physicalDevice.vkGetPhysicalDeviceSurfacePresentModesKHR(
			__pSurface->getHandle(), &numModes, nullptr);

		__supportedSurfacePresentModes.resize(numModes);
		__physicalDevice.vkGetPhysicalDeviceSurfacePresentModesKHR(
			__pSurface->getHandle(), &numModes, __supportedSurfacePresentModes.data());
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
			pDesiredFormat = __supportedSurfaceFormats.data();

		VkExtent2D desiredExtent{};
		if (__surfaceCapabilities.currentExtent.width == 0xFFFFFFFFU)
		{
			desiredExtent.width = uint32_t(__window.getWidth());
			desiredExtent.height = uint32_t(__window.getHeight());
		}
		else
			desiredExtent = __surfaceCapabilities.currentExtent;

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
			.surface = __pSurface->getHandle(),
			.minImageCount = numDesiredImages,
			.imageFormat = pDesiredFormat->format,
			.imageColorSpace = pDesiredFormat->colorSpace,
			.imageExtent = desiredExtent,
			.imageArrayLayers = 1U,
			.imageUsage = VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
			.imageSharingMode = VkSharingMode::VK_SHARING_MODE_EXCLUSIVE,
			.preTransform = __surfaceCapabilities.currentTransform,
			.compositeAlpha = compositeAlpha,
			.presentMode = desiredPresentMode,
			.clipped = VK_TRUE,
			.oldSwapchain = (__pSwapchain ? __pSwapchain->getHandle() : VK_NULL_HANDLE)
		};

		__pSwapchain = std::make_unique<Vulkan::Swapchain>(__device, createInfo);
		__resourceParam.swapchainFormat = createInfo.imageFormat;
		__resourceParam.swapchainExtent = createInfo.imageExtent;

		uint32_t numImages{};
		__pSwapchain->vkGetSwapchainImagesKHR(&numImages, nullptr);

		__resourceParam.swapChainImages.resize(numImages);
		__pSwapchain->vkGetSwapchainImagesKHR(&numImages, __resourceParam.swapChainImages.data());
	}

	void ScreenManager::ScreenImpl::__createRenderSemaphores(const size_t imageIdx)
	{
		if (__imageAcquireSemaphores[imageIdx])
			return;

		static constexpr VkSemaphoreCreateInfo binaryCreateInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
		};

		static constexpr VkSemaphoreTypeCreateInfo timelineInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
			.semaphoreType = VkSemaphoreType::VK_SEMAPHORE_TYPE_TIMELINE
		};

		static constexpr VkSemaphoreCreateInfo timelineCreateInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
			.pNext = &timelineInfo
		};

		__imageAcquireSemaphores[imageIdx] =
			std::make_unique<Vulkan::Semaphore>(__device, binaryCreateInfo);

		__renderCompletionBinarySemaphores[imageIdx] =
			std::make_unique<Vulkan::Semaphore>(__device, binaryCreateInfo);

		__renderCompletionTimelineSemaphores[imageIdx] =
			std::make_unique<Vulkan::Semaphore>(__device, timelineCreateInfo);
	}

	bool ScreenManager::ScreenImpl::__isValid() const noexcept
	{
		if (__destroyed)
			return false;

		const bool validSize{ __window.getWidth() && __window.getHeight() };
		return validSize;
	}

	ScreenResource &ScreenManager::ScreenImpl::__getFrontResource() noexcept
	{
		return *__resourceChain[__resourceCursor];
	}

	ScreenResource &ScreenManager::ScreenImpl::__getBackResource() noexcept
	{
		const size_t nextCursor{ (__resourceCursor + 1ULL) % 2ULL };
		return *__resourceChain[nextCursor];
	}

	Vulkan::Semaphore &ScreenManager::ScreenImpl::__getCurrentImageAcquireSemaphore() noexcept
	{
		return *__imageAcquireSemaphores[__frameCursor];
	}

	Vulkan::CommandBuffer &ScreenManager::ScreenImpl::__getCurrentRenderCommandBuffer() noexcept
	{
		return __getFrontResource().getRenderCommandBuffer(__imageIdx);
	}

	Vulkan::Semaphore &ScreenManager::ScreenImpl::__getCurrentRenderCompletionBinarySemaphore() noexcept
	{
		return *__renderCompletionBinarySemaphores[__imageIdx];
	}

	Vulkan::Semaphore &ScreenManager::ScreenImpl::__getCurrentRenderCompletionTimelineSemaphore() noexcept
	{
		return *__renderCompletionTimelineSemaphores[__imageIdx];
	}

	uint64_t &ScreenManager::ScreenImpl::__getCurrentRenderCompletionSemaphoreValue() noexcept
	{
		return __renderCompletionSemaphoreValues[__imageIdx];
	}

	bool ScreenManager::ScreenImpl::__acquireNextSwapchainImageIdx(Vulkan::Semaphore &semaphore) noexcept
	{
		if (__imageAcquired)
			return true;

		const VkResult acquireResult
		{
			__pSwapchain->vkAcquireNextImageKHR(
				0ULL, semaphore.getHandle(), VK_NULL_HANDLE, &__imageIdx)
		};

		const bool valid{ acquireResult == VkResult::VK_SUCCESS };
		if (valid)
		{
			__imageAcquired = true;
			__advanceFrameCursor();
			return true;
		}

		if ((acquireResult == VkResult::VK_SUBOPTIMAL_KHR) ||
			(acquireResult == VkResult::VK_ERROR_OUT_OF_DATE_KHR))
		{
			__needToUpdateSurfaceDependencies = true;
		}

		return false;
	}
}