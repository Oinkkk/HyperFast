#include "ScreenManager.h"
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
		__registerListeners();
		__createResourceChain();
		__createSurface();
		__initSubmitInfo();
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

	void ScreenManager::ScreenImpl::__update()
	{
		if (__needToUpdateSurfaceDependencies)
			__updateSurfaceDependencies();

		if (__needToUpdatePipelineDependencies)
			__updatePipelineDependencies();

		if (__needToUpdateMainCommands)
			__updateMainCommands();

		if (__needToUpdateResource)
			__updateResource();

		if (__needToAdvanceResource)
			__advanceResource();
	}

	void ScreenManager::ScreenImpl::__render() noexcept
	{
		Vulkan::Semaphore &imageAcquireSemaphore{ __getCurrentImageAcquireSemaphore() };

		const bool validAcquire{ __acquireNextSwapchainImageIdx(imageAcquireSemaphore) };
		if (!validAcquire)
			return;

		TimelineSemaphore &renderCompletionTimelineSemaphore{ __getCurrentRenderCompletionTimelineSemaphore() };
		const VkResult waitResult{ renderCompletionTimelineSemaphore.wait(0ULL) };

		// 앞전에 submit된 command buffer가 아직 처리 중
		if (waitResult == VkResult::VK_TIMEOUT)
			return;

		 __submitWaitInfo.semaphore = imageAcquireSemaphore.getHandle();

		Vulkan::CommandBuffer &renderCommandBuffer{ __getCurrentRenderCommandBuffer() };
		__submitCommandBufferInfo.commandBuffer = renderCommandBuffer.getHandle();

		VkSemaphoreSubmitInfo &binarySignalInfo{ __submitSignalInfos[0] };
		VkSemaphoreSubmitInfo &timelineSignalInfo{ __submitSignalInfos[1] };

		Vulkan::Semaphore &renderCompletionBinarySemaphore{ __getCurrentRenderCompletionBinarySemaphore() };
		binarySignalInfo.semaphore = renderCompletionBinarySemaphore.getHandle();

		renderCompletionTimelineSemaphore.advance();
		timelineSignalInfo.semaphore = renderCompletionTimelineSemaphore.getHandle();
		timelineSignalInfo.value = renderCompletionTimelineSemaphore.getValue();

		__renderingEngine.enqueueCommands(
			SubmitLayerType::GRAPHICS,
			1U, &__submitWaitInfo, 1U, &__submitCommandBufferInfo,
			uint32_t(std::size(__submitSignalInfos)), __submitSignalInfos);

		__getCurrentResource().addSubmitDependency(renderCompletionTimelineSemaphore);

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

		for (auto &pResource : __resourceChain)
			pResource = nullptr;

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

		__pScreenUpdateListener =
			Infra::EventListener<>::bind(&ScreenManager::ScreenImpl::__onScreenUpdate, this);

		__pRenderListener =
			Infra::EventListener<>::bind(&ScreenManager::ScreenImpl::__onRender, this);

		__pPresentListener =
			Infra::EventListener<>::bind(&ScreenManager::ScreenImpl::__onPresent, this);
	}

	void ScreenManager::ScreenImpl::__registerListeners() noexcept
	{
		__window.getResizeEvent() += __pResizeEventListener;
		__window.getDrawEvent() += __pDrawEventListener;
		__window.getDestroyEvent() += __pDestroyEventListener;

		__renderingEngine.getLifeCycleEvent(LifeCycleType::SCREEN_UPDATE) += __pScreenUpdateListener;
		__renderingEngine.getLifeCycleEvent(LifeCycleType::RENDER) += __pRenderListener;
		__renderingEngine.getLifeCycleEvent(LifeCycleType::PRESENT) += __pPresentListener;
	}

	void ScreenManager::ScreenImpl::__createResourceChain() noexcept
	{
		for (auto &pResource : __resourceChain)
		{
			pResource = std::make_unique<ScreenResource>(
				__device, __resourceParam, __queueFamilyIndex);
		}
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

	void ScreenManager::ScreenImpl::__updateSurfaceDependencies()
	{
		tf::Taskflow taskflow;

		__pOldSwapchain = std::move(__pSwapchain);
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
		__frameCursor %= numSwapchainImages;

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

		for (auto &pResource : __resourceChain)
			pResource->needToUpdateSwapchainDependencies();

		__needToUpdateSurfaceDependencies = false;
		__needToUpdatePipelineDependencies = false;
		__needToUpdateMainCommands = false;
		__needToUpdateResource = true;
		__needToRender = false;
	}

	void ScreenManager::ScreenImpl::__updatePipelineDependencies()
	{
		for (auto &pResource : __resourceChain)
			pResource->needToUpdatePipelineDependencies();

		__needToUpdatePipelineDependencies = false;
		__needToUpdateMainCommands = false;
		__needToUpdateResource = true;
	}

	void ScreenManager::ScreenImpl::__updateMainCommands()
	{
		for (auto &pResource : __resourceChain)
			pResource->needToUpdateMainCommands();

		__needToUpdateMainCommands = false;
		__needToUpdateResource = true;
	}

	void ScreenManager::ScreenImpl::__updateResource()
	{
		ScreenResource &nextResource{ __getNextResource() };

		// 한장도 못그리고 연속해서 resource 업데이트 요청이 들어온 경우
		if (!(nextResource.isIdle()))
			return;

		// 아직 이전 제출된 command buffer가 처리되지 않음
		if (nextResource.isSubmitDependent())
			return;

		nextResource.commit();
		__needToUpdateResource = false;
		__needToAdvanceResource = true;
	}

	void ScreenManager::ScreenImpl::__advanceResource() noexcept
	{
		ScreenResource &nextResource{ __getNextResource() };

		// resource update 진행 중
		if (!(nextResource.isIdle()))
			return;

		__resourceCursor = ((__resourceCursor + 1ULL) % std::size(__resourceChain));
		__resourceChainInit = true;
		__pOldSwapchain = nullptr;
		__needToAdvanceResource = false;
		__needToRender = true;
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
			.oldSwapchain = (__pOldSwapchain ? __pOldSwapchain->getHandle() : VK_NULL_HANDLE)
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
			std::make_unique<TimelineSemaphore>(__device);
	}

	bool ScreenManager::ScreenImpl::__isValid() const noexcept
	{
		if (__destroyed)
			return false;

		const bool validSize{ __window.getWidth() && __window.getHeight() };
		return validSize;
	}

	bool ScreenManager::ScreenImpl::__isRenderable() const noexcept
	{
		// resource advance 진행 중
		if (__pOldSwapchain)
			return false;

		// resource chain 초기화 안됨
		if (!__resourceChainInit)
			return false;

		return true;
	}

	ScreenResource &ScreenManager::ScreenImpl::__getCurrentResource() noexcept
	{
		return *__resourceChain[__resourceCursor];
	}

	ScreenResource &ScreenManager::ScreenImpl::__getNextResource() noexcept
	{
		const size_t nextCursor{ (__resourceCursor + 1ULL) % std::size(__resourceChain) };
		return *__resourceChain[nextCursor];
	}

	Vulkan::Semaphore &ScreenManager::ScreenImpl::__getCurrentImageAcquireSemaphore() noexcept
	{
		return *__imageAcquireSemaphores[__frameCursor];
	}

	Vulkan::CommandBuffer &ScreenManager::ScreenImpl::__getCurrentRenderCommandBuffer() noexcept
	{
		return __getCurrentResource().getRenderCommandBuffer(__imageIdx);
	}

	Vulkan::Semaphore &ScreenManager::ScreenImpl::__getCurrentRenderCompletionBinarySemaphore() noexcept
	{
		return *__renderCompletionBinarySemaphores[__imageIdx];
	}

	TimelineSemaphore &ScreenManager::ScreenImpl::__getCurrentRenderCompletionTimelineSemaphore() noexcept
	{
		return *__renderCompletionTimelineSemaphores[__imageIdx];
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

	void ScreenManager::ScreenImpl::__onScreenUpdate()
	{
		if (!(__isValid()))
			return;

		__update();
	}

	void ScreenManager::ScreenImpl::__onRender() noexcept
	{
		if (!__needToRender)
			return;

		if (!(__isValid()))
			return;
			
		if (!(__isRenderable()))
			return;

		__render();
	}

	void ScreenManager::ScreenImpl::__onPresent() noexcept
	{
		if (!__needToPresent)
			return;

		if (!(__isValid()))
			return;

		__present();
	}
}