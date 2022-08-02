﻿#include "ScreenManager_Old.h"
#include "RenderingEngine.h"

namespace HyperFast
{
	ScreenManager_Old::ScreenImpl::ScreenImpl(
		Vulkan::Instance &instance, Vulkan::PhysicalDevice &physicalDevice,
		const uint32_t queueFamilyIndex, Vulkan::Device &device,
		Vulkan::Queue &queue, Win::Window &window, LifeCycle &lifeCycle,
		CommandSubmitter &commandSubmitter, Infra::TemporalDeleter &resourceDeleter) :
		__instance{ instance }, __physicalDevice{ physicalDevice },
		__queueFamilyIndex{ queueFamilyIndex }, __device{ device },
		__queue{ queue }, __window{ window }, __lifeCycle{ lifeCycle },
		__commandSubmitter{ commandSubmitter }, __resourceDeleter{ resourceDeleter }
	{
		__initListeners();
		__registerListeners();
		__createResourceChain();
		__createSurface();
		__initSubmitInfo();
	}

	ScreenManager_Old::ScreenImpl::~ScreenImpl() noexcept
	{
		__destroy();
	}

	void ScreenManager_Old::ScreenImpl::setDrawcall(Drawcall *const pDrawcall) noexcept
	{
		if (__pDrawcall == pDrawcall)
			return;

		if (__pDrawcall)
		{
			__pDrawcall->getMeshBufferChangeEvent()
				-= __pDrawcallMeshBufferChangeEventListener;
			
			__pDrawcall->getIndirectBufferUpdateEvent()
				-= __pDrawcallIndirectBufferUpdateEventListener;
			
			__pDrawcall->getIndirectBufferCreateEvent()
				-= __pDrawcallIndirectBufferCreateEventListener;
		}

		__pDrawcall = pDrawcall;

		if (__pDrawcall)
		{
			__pDrawcall->getMeshBufferChangeEvent()
				+= __pDrawcallMeshBufferChangeEventListener;

			__pDrawcall->getIndirectBufferUpdateEvent()
				+= __pDrawcallIndirectBufferUpdateEventListener;

			__pDrawcall->getIndirectBufferCreateEvent()
				+= __pDrawcallIndirectBufferCreateEventListener;
		}

		__pipelineDependencyDirty = true;
	}

	void ScreenManager_Old::ScreenImpl::__update()
	{
		if (__swapchainDependencDirty)
			__updateSwapchainDependencies();

		if (__pipelineDependencyDirty)
			__updatePipelineDependencies();

		if (__commandBufferDirty)
			__updateCommandBuffers();

		if (__needToUpdateResource)
			__updateResource();

		if (__needToAdvanceResource)
			__advanceResource();
	}

	void ScreenManager_Old::ScreenImpl::__render() noexcept
	{
		Vulkan::Semaphore &imageAcquireSemaphore{ __getCurrentImageAcquireSemaphore() };

		const bool validAcquire{ __acquireNextSwapchainImageIdx(imageAcquireSemaphore) };
		if (!validAcquire)
			return;

		Vulkan::Semaphore &renderCompletionTimelineSemaphore{ __getCurrentRenderCompletionTimelineSemaphore() };
		uint64_t &renderCompletionSemaphoreValue{ __getCurrentRenderCompletionSemaphoreValue() };

		const VkResult waitResult
		{
			renderCompletionTimelineSemaphore.wait(
				renderCompletionSemaphoreValue, 0ULL)
		};

		// 앞전에 submit된 command buffer가 아직 처리 중
		if (waitResult == VkResult::VK_TIMEOUT)
			return;

		 __submitWaitInfo.semaphore = imageAcquireSemaphore.getHandle();

		Vulkan::CommandBuffer &renderCommandBuffer{ __getCurrentPrimaryCommandBuffer() };
		__submitCommandBufferInfo.commandBuffer = renderCommandBuffer.getHandle();

		VkSemaphoreSubmitInfo &binarySignalInfo{ __submitSignalInfos[0] };
		VkSemaphoreSubmitInfo &timelineSignalInfo{ __submitSignalInfos[1] };

		Vulkan::Semaphore &renderCompletionBinarySemaphore{ __getCurrentRenderCompletionBinarySemaphore() };
		binarySignalInfo.semaphore = renderCompletionBinarySemaphore.getHandle();

		renderCompletionSemaphoreValue++;
		timelineSignalInfo.semaphore = renderCompletionTimelineSemaphore.getHandle();
		timelineSignalInfo.value = renderCompletionSemaphoreValue;

		__commandSubmitter.enqueue(
			SubmitLayerType::GRAPHICS,
			1U, &__submitWaitInfo, 1U, &__submitCommandBufferInfo,
			uint32_t(std::size(__submitSignalInfos)), __submitSignalInfos);

		__submitDependencyManager.setDependency(
			renderCompletionTimelineSemaphore, renderCompletionSemaphoreValue);

		__needToRender = false;
		__needToPresent = true;
	}

	void ScreenManager_Old::ScreenImpl::__present() noexcept
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
			__swapchainDependencDirty = true;
		}

		if (presentResult == VkResult::VK_SUCCESS)
			__needToPresent = false;
	}

	void ScreenManager_Old::ScreenImpl::__destroy() noexcept
	{
		if (__destroyed)
			return;

		__pWindowResizeEventListener = nullptr;
		__pWindowDrawEventListener = nullptr;
		__pWindowDestroyEventListener = nullptr;
		__pDrawcallMeshBufferChangeEventListener = nullptr;
		__pDrawcallIndirectBufferUpdateEventListener = nullptr;
		__pDrawcallIndirectBufferCreateEventListener = nullptr;
		__pScreenUpdateListener = nullptr;
		__pRenderListener = nullptr;
		__pPresentListener = nullptr;

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

	void ScreenManager_Old::ScreenImpl::__initListeners() noexcept
	{
		__pWindowResizeEventListener =
			Infra::EventListener<Win::Window &, Win::Window::ResizingType>::bind(
				&ScreenManager_Old::ScreenImpl::__onWindowResize, this,
				std::placeholders::_1, std::placeholders::_2);

		__pWindowDrawEventListener =
			Infra::EventListener<Win::Window &>::bind(
				&ScreenManager_Old::ScreenImpl::__onWindowDraw, this, std::placeholders::_1);

		__pWindowDestroyEventListener =
			Infra::EventListener<Win::Window &>::bind(
				&ScreenManager_Old::ScreenImpl::__onWindowDestroy, this, std::placeholders::_1);

		__pDrawcallMeshBufferChangeEventListener =
			Infra::EventListener<Drawcall &, size_t>::bind(
				&ScreenManager_Old::ScreenImpl::__onDrawcallMeshBufferChange, this,
				std::placeholders::_1, std::placeholders::_2);

		__pDrawcallIndirectBufferUpdateEventListener =
			Infra::EventListener<Drawcall &, size_t>::bind(
				&ScreenManager_Old::ScreenImpl::__onDrawcallIndirectBufferUpdate, this,
				std::placeholders::_1, std::placeholders::_2);

		__pDrawcallIndirectBufferCreateEventListener =
			Infra::EventListener<Drawcall &, size_t>::bind(
				&ScreenManager_Old::ScreenImpl::__onDrawcallIndirectBufferCreate, this,
				std::placeholders::_1, std::placeholders::_2);

		__pScreenUpdateListener =
			Infra::EventListener<>::bind(&ScreenManager_Old::ScreenImpl::__onScreenUpdate, this);

		__pRenderListener =
			Infra::EventListener<>::bind(&ScreenManager_Old::ScreenImpl::__onRender, this);

		__pPresentListener =
			Infra::EventListener<>::bind(&ScreenManager_Old::ScreenImpl::__onPresent, this);
	}

	void ScreenManager_Old::ScreenImpl::__registerListeners() noexcept
	{
		__window.getResizeEvent() += __pWindowResizeEventListener;
		__window.getDrawEvent() += __pWindowDrawEventListener;
		__window.getDestroyEvent() += __pWindowDestroyEventListener;

		__lifeCycle.getSignalEvent(LifeCycleType::SCREEN_UPDATE) += __pScreenUpdateListener;
		__lifeCycle.getSignalEvent(LifeCycleType::RENDER) += __pRenderListener;
		__lifeCycle.getSignalEvent(LifeCycleType::PRESENT) += __pPresentListener;
	}

	void ScreenManager_Old::ScreenImpl::__createResourceChain() noexcept
	{
		for (auto &pResource : __resourceChain)
			pResource = std::make_unique<ScreenResource>(__device, __queueFamilyIndex);
	}

	void ScreenManager_Old::ScreenImpl::__createSurface()
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

	void ScreenManager_Old::ScreenImpl::__updateSwapchainDependencies()
	{
		tf::Taskflow taskflow;

		__pOldSwapchain = std::move(__pSwapchain);
		__checkSurfaceSupport();
		__querySurfaceCapabilities();
		__querySupportedSurfaceFormats();
		__querySupportedSurfacePresentModes();
		__createSwapchain();

		const size_t numSwapchainImages{ __swapchainParam.swapChainImages.size() };
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

		__swapchainDependencDirty = false;
		__pipelineDependencyDirty = false;
		__commandBufferDirty = false;
		__needToUpdateResource = true;
		__needToRender = false;
	}

	void ScreenManager_Old::ScreenImpl::__updatePipelineDependencies()
	{
		for (auto &pResource : __resourceChain)
			pResource->needToUpdatePipelineDependencies();

		__pipelineDependencyDirty = false;
		__commandBufferDirty = false;
		__needToUpdateResource = true;
	}

	void ScreenManager_Old::ScreenImpl::__updateCommandBuffers()
	{
		for (auto &pResource : __resourceChain)
			pResource->needToUpdateCommandBuffer();

		__commandBufferDirty = false;
		__needToUpdateResource = true;
	}

	void ScreenManager_Old::ScreenImpl::__updateResource()
	{
		ScreenResource &nextResource{ __getNextResource() };
		if (!(nextResource.isIdle()))
			return;

		nextResource.update(__swapchainParam, __pDrawcall);
		__needToUpdateResource = false;
		__needToAdvanceResource = true;
	}

	void ScreenManager_Old::ScreenImpl::__advanceResource() noexcept
	{
		ScreenResource &nextResource{ __getNextResource() };
		if (!(nextResource.isIdle()))
			return;

		__resourceCursor = ((__resourceCursor + 1ULL) % std::size(__resourceChain));

		__submitDependencyManager.nextGroup();
		__pCurrentSubmitDependency = std::make_shared<SemaphoreDependency>(__submitDependencyManager);

		nextResource.addSemaphoreDependency(__pCurrentSubmitDependency);
		/*if (__pDrawcall)
			__pDrawcall->addSemaphoreDependency(__pCurrentSubmitDependency);*/

		__resourceChainInit = true;
		__pOldSwapchain = nullptr;
		__needToAdvanceResource = false;
		__needToRender = true;
	}

	void ScreenManager_Old::ScreenImpl::__checkSurfaceSupport() const
	{
		VkBool32 surfaceSupported{};
		__physicalDevice.vkGetPhysicalDeviceSurfaceSupportKHR(
			__queueFamilyIndex, __pSurface->getHandle(), &surfaceSupported);

		if (!surfaceSupported)
			throw std::exception{ "The physical device doesn't support the surface." };
	}

	void ScreenManager_Old::ScreenImpl::__querySurfaceCapabilities() noexcept
	{
		__physicalDevice.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
			__pSurface->getHandle(), &__surfaceCapabilities);
	}

	void ScreenManager_Old::ScreenImpl::__querySupportedSurfaceFormats() noexcept
	{
		uint32_t numFormats{};
		__physicalDevice.vkGetPhysicalDeviceSurfaceFormatsKHR(
			__pSurface->getHandle(), &numFormats, nullptr);

		__supportedSurfaceFormats.resize(numFormats);
		__physicalDevice.vkGetPhysicalDeviceSurfaceFormatsKHR(
			__pSurface->getHandle(), &numFormats, __supportedSurfaceFormats.data());
	}

	void ScreenManager_Old::ScreenImpl::__querySupportedSurfacePresentModes() noexcept
	{
		uint32_t numModes{};
		__physicalDevice.vkGetPhysicalDeviceSurfacePresentModesKHR(
			__pSurface->getHandle(), &numModes, nullptr);

		__supportedSurfacePresentModes.resize(numModes);
		__physicalDevice.vkGetPhysicalDeviceSurfacePresentModesKHR(
			__pSurface->getHandle(), &numModes, __supportedSurfacePresentModes.data());
	}

	void ScreenManager_Old::ScreenImpl::__createSwapchain()
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
		__swapchainParam.swapchainFormat = createInfo.imageFormat;
		__swapchainParam.swapchainExtent = createInfo.imageExtent;

		uint32_t numImages{};
		__pSwapchain->vkGetSwapchainImagesKHR(&numImages, nullptr);

		__swapchainParam.swapChainImages.resize(numImages);
		__pSwapchain->vkGetSwapchainImagesKHR(&numImages, __swapchainParam.swapChainImages.data());
	}

	void ScreenManager_Old::ScreenImpl::__createRenderSemaphores(const size_t imageIdx)
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

	bool ScreenManager_Old::ScreenImpl::__isUpdatable() const noexcept
	{
		if (__destroyed)
			return false;

		const bool validSize{ __window.getWidth() && __window.getHeight() };
		return validSize;
	}

	bool ScreenManager_Old::ScreenImpl::__isRenderable() const noexcept
	{
		// resource advance 진행 중
		if (__pOldSwapchain)
			return false;

		// resource chain 초기화 안됨
		if (!__resourceChainInit)
			return false;

		return true;
	}

	ScreenResource &ScreenManager_Old::ScreenImpl::__getCurrentResource() noexcept
	{
		return *__resourceChain[__resourceCursor];
	}

	ScreenResource &ScreenManager_Old::ScreenImpl::__getNextResource() noexcept
	{
		const size_t nextCursor{ (__resourceCursor + 1ULL) % std::size(__resourceChain) };
		return *__resourceChain[nextCursor];
	}

	Vulkan::Semaphore &ScreenManager_Old::ScreenImpl::__getCurrentImageAcquireSemaphore() noexcept
	{
		return *__imageAcquireSemaphores[__frameCursor];
	}

	Vulkan::CommandBuffer &ScreenManager_Old::ScreenImpl::__getCurrentPrimaryCommandBuffer() noexcept
	{
		return __getCurrentResource().getPrimaryCommandBuffer(__imageIdx);
	}

	Vulkan::Semaphore &ScreenManager_Old::ScreenImpl::__getCurrentRenderCompletionBinarySemaphore() noexcept
	{
		return *__renderCompletionBinarySemaphores[__imageIdx];
	}

	Vulkan::Semaphore &ScreenManager_Old::ScreenImpl::__getCurrentRenderCompletionTimelineSemaphore() noexcept
	{
		return *__renderCompletionTimelineSemaphores[__imageIdx];
	}

	uint64_t &ScreenManager_Old::ScreenImpl::__getCurrentRenderCompletionSemaphoreValue() noexcept
	{
		return __renderCompletionSemaphoreValues[__imageIdx];
	}

	bool ScreenManager_Old::ScreenImpl::__acquireNextSwapchainImageIdx(Vulkan::Semaphore &semaphore) noexcept
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
			__swapchainDependencDirty = true;
		}

		return false;
	}

	void ScreenManager_Old::ScreenImpl::__onWindowResize(
		Win::Window &window, const Win::Window::ResizingType resizingType) noexcept
	{
		if (resizingType == Win::Window::ResizingType::MINIMIZED)
			return;

		__swapchainDependencDirty = true;
	}

	void ScreenManager_Old::ScreenImpl::__onDrawcallMeshBufferChange(
		Drawcall &drawcall, const size_t segmentIndex) noexcept
	{
		__commandBufferDirty = true;

		for (auto &pResource : __resourceChain)
			pResource->needToUpdateSecondaryCommandBuffer(segmentIndex);
	}

	void ScreenManager_Old::ScreenImpl::__onDrawcallIndirectBufferUpdate(
		Drawcall &drawcall, const size_t segmentIndex) noexcept
	{
		__needToRender = true;
	}

	void ScreenManager_Old::ScreenImpl::__onDrawcallIndirectBufferCreate(
		Drawcall &drawcall, const size_t segmentIndex) noexcept
	{
		__commandBufferDirty = true;

		for (auto &pResource : __resourceChain)
			pResource->needToUpdateSecondaryCommandBuffer(segmentIndex);
	}

	void ScreenManager_Old::ScreenImpl::__onWindowDraw(Win::Window &window) noexcept
	{
		__needToRender = true;
	}

	void ScreenManager_Old::ScreenImpl::__onWindowDestroy(Win::Window &window) noexcept
	{
		__destroy();
	}

	void ScreenManager_Old::ScreenImpl::__onScreenUpdate()
	{
		if (!(__isUpdatable()))
			return;

		__update();
	}

	void ScreenManager_Old::ScreenImpl::__onRender() noexcept
	{
		if (!__needToRender)
			return;

		if (!(__isUpdatable()))
			return;
			
		if (!(__isRenderable()))
			return;

		__render();
	}

	void ScreenManager_Old::ScreenImpl::__onPresent() noexcept
	{
		if (!__needToPresent)
			return;

		if (!(__isUpdatable()))
			return;

		__present();
	}
}