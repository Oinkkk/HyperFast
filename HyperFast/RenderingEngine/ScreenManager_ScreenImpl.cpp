#include "ScreenManager.h"
#include "RenderingEngine.h"

namespace HyperFast
{
	ScreenManager::ScreenImpl::ScreenImpl(
		Vulkan::Instance &instance, Vulkan::PhysicalDevice &physicalDevice,
		const uint32_t queueFamilyIndex, Vulkan::Device &device,
		Vulkan::Queue &queue, LifeCycle &lifeCycle,
		CommandSubmitter &commandSubmitter, Infra::TemporalDeleter &resourceDeleter, Win::Window &window) :
		__instance{ instance }, __physicalDevice{ physicalDevice },
		__queueFamilyIndex{ queueFamilyIndex }, __device{ device },
		__queue{ queue }, __lifeCycle{ lifeCycle },
		__commandSubmitter{ commandSubmitter }, __resourceDeleter{ resourceDeleter },
		__window{ window }
	{
		__initListeners();
		__registerListeners();
		__createSurface();
		__createPipelineFactory();

		__initSubmitInfos();
		__initSecondaryCommandBufferBeginInfos();
	}

	ScreenManager::ScreenImpl::~ScreenImpl() noexcept
	{
		__destroy();
	}

	void ScreenManager::ScreenImpl::setDrawcall(Drawcall *const pDrawcall) noexcept
	{
		Drawcall *&pCurDrawcall{ __pDrawcall };
		Drawcall *const pNewDrawcall{ pDrawcall };

		if (pCurDrawcall)
		{
			pCurDrawcall->getMeshBufferChangeEvent() -= __pDrawcallMeshBufferChangeEventListener;
			pCurDrawcall->getIndirectBufferUpdateEvent() -= __pDrawcallIndirectBufferUpdateEventListener;
			pCurDrawcall->getIndirectBufferCreateEvent() -= __pDrawcallIndirectBufferCreateEventListener;
		}

		if (pNewDrawcall)
		{
			pNewDrawcall->getMeshBufferChangeEvent() += __pDrawcallMeshBufferChangeEventListener;
			pNewDrawcall->getIndirectBufferUpdateEvent() += __pDrawcallIndirectBufferUpdateEventListener;
			pNewDrawcall->getIndirectBufferCreateEvent() += __pDrawcallIndirectBufferCreateEventListener;
		}

		pCurDrawcall = pNewDrawcall;
		__pipelineDependencyDirty = true;
	}

	bool ScreenManager::ScreenImpl::__isUpdatable() const noexcept
	{
		if (__destroyed)
			return false;

		const bool validSize{ __window.getWidth() && __window.getHeight() };
		if (!validSize)
			return false;

		return true;
	}

	bool ScreenManager::ScreenImpl::__isRenderable() const noexcept
	{
		if (!(__isUpdatable()))
			return false;

		if (!__needToRender)
			return false;

		return true;
	}

	bool ScreenManager::ScreenImpl::__isPresentable() const noexcept
	{
		if (!(__isUpdatable()))
			return false;

		if (!__needToPresent)
			return false;

		return true;
	}

	void ScreenManager::ScreenImpl::__update()
	{
		if (__swapchainDependencyDirty)
			__updateSwapchainDependencies();

		if (__pipelineDependencyDirty)
			__updatePipelineDependencies();

		if (__commandBufferDirty)
			__updateCommandBuffers();
	}

	void ScreenManager::ScreenImpl::__render() noexcept
	{
		Vulkan::Semaphore &imageAcquireSemaphore{ __getImageAcquireSemaphore() };

		const bool validAcquire{ __acquireNextSwapchainImageIdx(imageAcquireSemaphore) };
		if (!validAcquire)
			return;

		Vulkan::Semaphore &renderCompletionTimelineSemaphore{ __getRenderCompletionTimelineSemaphore() };
		uint64_t &renderCompletionSemaphoreValue{ __getRenderCompletionSemaphoreValue() };

		const VkResult waitResult
		{
			renderCompletionTimelineSemaphore.wait(renderCompletionSemaphoreValue, 0ULL)
		};

		// 앞전에 submit된 command buffer가 아직 처리 중
		if (waitResult == VkResult::VK_TIMEOUT)
			return;

		__submitWaitInfo.semaphore = imageAcquireSemaphore.getHandle();

		Vulkan::CommandBuffer &renderCommandBuffer{ __getPrimaryCommandBuffer() };
		__submitCommandBufferInfo.commandBuffer = renderCommandBuffer.getHandle();

		VkSemaphoreSubmitInfo &binarySignalInfo{ __submitSignalInfos[0] };
		VkSemaphoreSubmitInfo &timelineSignalInfo{ __submitSignalInfos[1] };

		Vulkan::Semaphore &renderCompletionBinarySemaphore{ __getRenderCompletionBinarySemaphore() };
		binarySignalInfo.semaphore = renderCompletionBinarySemaphore.getHandle();

		renderCompletionSemaphoreValue++;
		timelineSignalInfo.semaphore = renderCompletionTimelineSemaphore.getHandle();
		timelineSignalInfo.value = renderCompletionSemaphoreValue;

		__commandSubmitter.enqueue(
			SubmitLayerType::GRAPHICS,
			1U, &__submitWaitInfo, 1U, &__submitCommandBufferInfo,
			uint32_t(std::size(__submitSignalInfos)), __submitSignalInfos);

		__needToRender = false;
		__needToPresent = true;
	}

	void ScreenManager::ScreenImpl::__present() noexcept
	{
		Vulkan::Semaphore &attachmentOutputSemaphore{ __getRenderCompletionBinarySemaphore() };

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
			__swapchainDependencyDirty = true;
		}

		if (presentResult == VkResult::VK_SUCCESS)
			__needToPresent = false;
	}

	void ScreenManager::ScreenImpl::__destroy() noexcept
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

		__perImageCommandBufferResources.clear();
		__resetSwapchainDependencies();
		__resourceDeleter.reserve(__pSurface);

		__destroyed = true;
	}

	void ScreenManager::ScreenImpl::__initListeners() noexcept
	{
		__pWindowResizeEventListener =
			Infra::EventListener<Win::Window &, Win::Window::ResizingType>::bind(
				&ScreenManager::ScreenImpl::__onWindowResize, this,
				std::placeholders::_1, std::placeholders::_2);

		__pWindowDrawEventListener =
			Infra::EventListener<Win::Window &>::bind(
				&ScreenManager::ScreenImpl::__onWindowDraw, this, std::placeholders::_1);

		__pWindowDestroyEventListener =
			Infra::EventListener<Win::Window &>::bind(
				&ScreenManager::ScreenImpl::__onWindowDestroy, this, std::placeholders::_1);

		__pDrawcallMeshBufferChangeEventListener =
			Infra::EventListener<Drawcall &, size_t>::bind(
				&ScreenManager::ScreenImpl::__onDrawcallMeshBufferChange, this,
				std::placeholders::_1, std::placeholders::_2);

		__pDrawcallIndirectBufferUpdateEventListener =
			Infra::EventListener<Drawcall &, size_t>::bind(
				&ScreenManager::ScreenImpl::__onDrawcallIndirectBufferUpdate, this,
				std::placeholders::_1, std::placeholders::_2);

		__pDrawcallIndirectBufferCreateEventListener =
			Infra::EventListener<Drawcall &, size_t>::bind(
				&ScreenManager::ScreenImpl::__onDrawcallIndirectBufferCreate, this,
				std::placeholders::_1, std::placeholders::_2);

		__pScreenUpdateListener =
			Infra::EventListener<>::bind(&ScreenManager::ScreenImpl::__onScreenUpdate, this);

		__pRenderListener =
			Infra::EventListener<>::bind(&ScreenManager::ScreenImpl::__onRender, this);

		__pPresentListener =
			Infra::EventListener<>::bind(&ScreenManager::ScreenImpl::__onPresent, this);
	}

	void ScreenManager::ScreenImpl::__registerListeners() noexcept
	{
		__window.getResizeEvent() += __pWindowResizeEventListener;
		__window.getDrawEvent() += __pWindowDrawEventListener;
		__window.getDestroyEvent() += __pWindowDestroyEventListener;

		__lifeCycle.getSignalEvent(LifeCycleType::SCREEN_UPDATE) += __pScreenUpdateListener;
		__lifeCycle.getSignalEvent(LifeCycleType::RENDER) += __pRenderListener;
		__lifeCycle.getSignalEvent(LifeCycleType::PRESENT) += __pPresentListener;
	}

	void ScreenManager::ScreenImpl::__createSurface()
	{
		const VkWin32SurfaceCreateInfoKHR createInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
			.hinstance = __window.getClass().getHInstance(),
			.hwnd = __window.getHandle()
		};

		__pSurface = new Vulkan::Surface{ __instance, createInfo };
		__swapchainDependencyDirty = true;
	}

	void ScreenManager::ScreenImpl::__createPipelineFactory() noexcept
	{
		__pPipelineFactory = std::make_unique<PipelineFactory>(__device, __resourceDeleter);
	}

	void ScreenManager::ScreenImpl::__resetSwapchainDependencies() noexcept
	{
		__resourceDeleter.reserve(__pFramebuffer);
		__resourceDeleter.reserve(__pRenderPass);

		for (Vulkan::ImageView *const pImageView : __swapChainImageViews)
			__resourceDeleter.reserve(pImageView);

		__swapChainImageViews.clear();
		__resourceDeleter.reserve(__pSwapchain);
	}

	void ScreenManager::ScreenImpl::__updateSwapchainDependencies()
	{
		__checkSurfaceSupport();
		__querySurfaceCapabilities();
		__querySupportedSurfaceFormats();
		__querySupportedSurfacePresentModes();

		__resetSwapchainDependencies();

		__createSwapchain();
		__createSwapchainImageViews();
		__createRenderPass();
		__createFramebuffer();
		__populateSecondaryCommandBufferInheritanceInfo();
		__frameIdx %= __swapChainImages.size();

		__updatePipelineDependencies();
		__swapchainDependencyDirty = false;
	}

	void ScreenManager::ScreenImpl::__updatePipelineDependencies()
	{
		__pPipelineFactory->reset();

		tf::Taskflow taskflow;
		tf::Task t1
		{
			taskflow.emplace([this](tf::Subflow &subflow)
			{
				__buildPipelines(subflow);
			})
		};

		tf::Task t2
		{
			taskflow.emplace([this](tf::Subflow &subflow)
			{
				__dirtyAllDrawcallSegments();

				const size_t numImages{ __swapChainImages.size() };
				for (size_t imageIdx = 0ULL; imageIdx < numImages; imageIdx++)
				{
					PerImageCommandBufferResource &commandBufferResource{ __getPerImageCommandBufferResource(imageIdx) };

					tf::Task t1
					{
						subflow.emplace([this, &commandBufferResource](tf::Subflow &subflow)
						{
							__updateSecondaryCommandBuffers(commandBufferResource, subflow);
						})
					};

					subflow.emplace([this, &commandBufferResource, imageIdx](tf::Subflow &subflow)
					{
						__updateSecondaryCommandBufferHandles(commandBufferResource);
						__updatePrimaryCommandBuffer(commandBufferResource, imageIdx);
					}).succeed(t1);
				}
			})
		};
		t2.succeed(t1);

		taskflow.emplace([this]
		{
			__drawcallSegmentDirties.clear();
		}).succeed(t2);

		tf::Executor &executor{ Infra::Environment::getInstance().getTaskflowExecutor() };
		executor.run(taskflow).wait();

		__pipelineDependencyDirty = false;
		__commandBufferDirty = false;
		__needToRender = true;
	}

	void ScreenManager::ScreenImpl::__updateCommandBuffers()
	{
		tf::Taskflow taskflow;
		tf::Task t1
		{
			taskflow.emplace([this](tf::Subflow &subflow)
			{
				const size_t numImages{ __swapChainImages.size() };
				for (size_t imageIdx = 0ULL; imageIdx < numImages; imageIdx++)
				{
					PerImageCommandBufferResource &commandBufferResource{ __getPerImageCommandBufferResource(imageIdx) };

					tf::Task t1
					{
						subflow.emplace([this, &commandBufferResource](tf::Subflow &subflow)
						{
							__updateSecondaryCommandBuffers(commandBufferResource, subflow);
						})
					};

					subflow.emplace([this, &commandBufferResource, imageIdx](tf::Subflow &subflow)
					{
						__updateSecondaryCommandBufferHandles(commandBufferResource);
						__updatePrimaryCommandBuffer(commandBufferResource, imageIdx);
					}).succeed(t1);
				}
			})
		};

		taskflow.emplace([this]
		{
			__drawcallSegmentDirties.clear();
		}).succeed(t1);

		tf::Executor &executor{ Infra::Environment::getInstance().getTaskflowExecutor() };
		executor.run(taskflow).wait();

		__commandBufferDirty = false;
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

	void ScreenManager::ScreenImpl::__populateSecondaryCommandBufferInheritanceInfo() noexcept
	{
		__secondaryCommandBufferInheritanceInfo.renderPass = __pRenderPass->getHandle();
		__secondaryCommandBufferInheritanceInfo.framebuffer = __pFramebuffer->getHandle();
	}

	void ScreenManager::ScreenImpl::__dirtyAllDrawcallSegments() noexcept
	{
		if (!__pDrawcall)
			return;

		const size_t numSegments{ __pDrawcall->getNumSegments() };
		for (size_t segmentIter = 0ULL; segmentIter < numSegments; segmentIter++)
			__drawcallSegmentDirties.emplace(segmentIter);
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

		__pSwapchain = new Vulkan::Swapchain{ __device, createInfo };
		__swapchainFormat = createInfo.imageFormat;
		__swapchainExtent = createInfo.imageExtent;

		uint32_t numImages{};
		__pSwapchain->vkGetSwapchainImagesKHR(&numImages, nullptr);

		__swapChainImages.resize(numImages);
		__pSwapchain->vkGetSwapchainImagesKHR(&numImages, __swapChainImages.data());
	}

	void ScreenManager::ScreenImpl::__createSwapchainImageViews()
	{
		for (const VkImage swapchainImage : __swapChainImages)
		{
			const VkImageViewCreateInfo createInfo
			{
				.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
				.image = swapchainImage,
				.viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D,
				.format = __swapchainFormat,
				.components =
				{
					.r = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY,
					.g = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY,
					.b = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY,
					.a = VkComponentSwizzle::VK_COMPONENT_SWIZZLE_IDENTITY
				},
				.subresourceRange =
				{
					.aspectMask = VkImageAspectFlagBits::VK_IMAGE_ASPECT_COLOR_BIT,
					.baseMipLevel = 0U,
					.levelCount = 1U,
					.baseArrayLayer = 0U,
					.layerCount = 1U
				}
			};

			__swapChainImageViews.emplace_back(new Vulkan::ImageView{ __device, createInfo });
		}
	}

	void ScreenManager::ScreenImpl::__createRenderPass()
	{
		std::vector<VkAttachmentDescription2> attachments;
		std::vector<VkSubpassDescription2> subpasses;
		std::vector<VkSubpassDependency2> dependencies;

		VkAttachmentDescription2 &colorAttachment{ attachments.emplace_back() };
		colorAttachment.sType = VkStructureType::VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
		colorAttachment.format = __swapchainFormat;
		colorAttachment.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		const VkAttachmentReference2 colorAttachmentRef
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_ATTACHMENT_REFERENCE_2,
			.attachment = 0U,
			.layout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		};

		VkSubpassDescription2 &subpass{ subpasses.emplace_back() };
		subpass.sType = VkStructureType::VK_STRUCTURE_TYPE_SUBPASS_DESCRIPTION_2;
		subpass.pipelineBindPoint = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1U;
		subpass.pColorAttachments = &colorAttachmentRef;

		const VkMemoryBarrier2 subpassBarrier
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
			.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,

			// 암묵적 메모리 디펜던시
			.srcAccessMask = 0ULL, 
			.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT
		};

		VkSubpassDependency2 &dependency{ dependencies.emplace_back() };
		dependency.sType = VkStructureType::VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2;
		dependency.pNext = &subpassBarrier;
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;
		dependency.dstSubpass = 0U;
		dependency.dependencyFlags = VkDependencyFlagBits::VK_DEPENDENCY_BY_REGION_BIT;

		const VkRenderPassCreateInfo2 createInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO_2,
			.attachmentCount = uint32_t(attachments.size()),
			.pAttachments = attachments.data(),
			.subpassCount = uint32_t(subpasses.size()),
			.pSubpasses = subpasses.data(),
			.dependencyCount = uint32_t(dependencies.size()),
			.pDependencies = dependencies.data()
		};

		__pRenderPass = new Vulkan::RenderPass{ __device, createInfo };
	}

	void ScreenManager::ScreenImpl::__createFramebuffer()
	{
		std::vector<VkFramebufferAttachmentImageInfo> attachmentImageInfos;

		VkFramebufferAttachmentImageInfo &colorAttachmentImageInfo{ attachmentImageInfos.emplace_back() };
		colorAttachmentImageInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO;
		colorAttachmentImageInfo.usage = VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		colorAttachmentImageInfo.width = __swapchainExtent.width;
		colorAttachmentImageInfo.height = __swapchainExtent.height;
		colorAttachmentImageInfo.layerCount = 1U;
		colorAttachmentImageInfo.viewFormatCount = 1U;
		colorAttachmentImageInfo.pViewFormats = &__swapchainFormat;

		const VkFramebufferAttachmentsCreateInfo attachmentInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENTS_CREATE_INFO,
			.attachmentImageInfoCount = uint32_t(attachmentImageInfos.size()),
			.pAttachmentImageInfos = attachmentImageInfos.data()
		};

		const VkFramebufferCreateInfo createInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			.pNext = &attachmentInfo,
			.flags = VkFramebufferCreateFlagBits::VK_FRAMEBUFFER_CREATE_IMAGELESS_BIT,
			.renderPass = __pRenderPass->getHandle(),
			.attachmentCount = 1U,
			.width = __swapchainExtent.width,
			.height = __swapchainExtent.height,
			.layers = 1U
		};

		__pFramebuffer = new Vulkan::Framebuffer{ __device, createInfo };
	}

	void ScreenManager::ScreenImpl::__buildPipelines(tf::Subflow &subflow)
	{
		__pipelineBuildParam.renderPass = __pRenderPass->getHandle();
		__pipelineBuildParam.viewport =
		{
			.x = 0.0f,
			.y = 0.0f,
			.width = float(__swapchainExtent.width),
			.height = float(__swapchainExtent.height),
			.minDepth = 0.0f,
			.maxDepth = 1.0f
		};

		__pipelineBuildParam.scissor =
		{
			.offset = { 0, 0 },
			.extent = __swapchainExtent
		};

		__pPipelineFactory->build(__pipelineBuildParam, subflow);
	}

	void ScreenManager::ScreenImpl::__updateSecondaryCommandBuffers(
		PerImageCommandBufferResource &commandBufferResource, tf::Subflow &subflow) noexcept
	{
		if (!__pDrawcall)
			return;

		for (const size_t segmentIdx : __drawcallSegmentDirties)
		{
			Vulkan::CommandBuffer &commandBuffer
			{
				__nextSecondaryCommandBuffer(commandBufferResource, segmentIdx)
			};

			subflow.emplace([this, segmentIdx, &commandBuffer](tf::Subflow &subflow)
			{
				commandBuffer.vkBeginCommandBuffer(&__secondaryCommandBufferBeginInfo);
				commandBuffer.vkCmdBindPipeline(
					VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, __pPipelineFactory->get());

				__pDrawcall->draw(segmentIdx, commandBuffer);

				commandBuffer.vkEndCommandBuffer();
			});
		}
	}

	void ScreenManager::ScreenImpl::__updatePrimaryCommandBuffer(
		PerImageCommandBufferResource &commandBufferResource, const size_t imageIdx) noexcept
	{
		static constexpr VkCommandBufferBeginInfo commandBufferBeginInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
		};

		const VkRenderPassAttachmentBeginInfo renderPassAttachmentInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_RENDER_PASS_ATTACHMENT_BEGIN_INFO,
			.attachmentCount = 1U,
			.pAttachments = &(__swapChainImageViews[imageIdx]->getHandle())
		};

		static constexpr VkClearValue clearColor
		{
			.color = {.float32 = { 0.004f, 0.004f, 0.004f, 1.0f } }
		};

		const VkRenderPassBeginInfo renderPassBeginInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.pNext = &renderPassAttachmentInfo,
			.renderPass = __pRenderPass->getHandle(),
			.framebuffer = __pFramebuffer->getHandle(),
			.renderArea =
			{
				.offset = { 0, 0 },
				.extent = __swapchainExtent
			},
			.clearValueCount = 1U,
			.pClearValues = &clearColor
		};

		const std::vector<VkCommandBuffer> &secondaryCommandBufferHandles
		{
			commandBufferResource.secondaryCommandBufferHandles
		};

		Vulkan::CommandBuffer &commandBuffer{ __nextPrimaryCommandBuffer(commandBufferResource) };
		commandBuffer.vkBeginCommandBuffer(&commandBufferBeginInfo);

		if (__pDrawcall)
		{
			commandBuffer.vkCmdBeginRenderPass(
				&renderPassBeginInfo, VkSubpassContents::VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

			commandBuffer.vkCmdExecuteCommands(
				uint32_t(secondaryCommandBufferHandles.size()),
				secondaryCommandBufferHandles.data());
		}
		else
		{
			commandBuffer.vkCmdBeginRenderPass(
				&renderPassBeginInfo, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);
		}

		commandBuffer.vkCmdEndRenderPass();
		commandBuffer.vkEndCommandBuffer();
	}

	void ScreenManager::ScreenImpl::__onWindowResize(
		Win::Window &window, const Win::Window::ResizingType resizingType) noexcept
	{
		if (resizingType == Win::Window::ResizingType::MINIMIZED)
			return;

		__swapchainDependencyDirty = true;
	}

	void ScreenManager::ScreenImpl::__onDrawcallMeshBufferChange(
		Drawcall &drawcall, const size_t segmentIdx) noexcept
	{
		__commandBufferDirty = true;
		__drawcallSegmentDirties.emplace(segmentIdx);
	}

	void ScreenManager::ScreenImpl::__onDrawcallIndirectBufferUpdate(Drawcall &, size_t) noexcept
	{
		__needToRender = true;
	}

	void ScreenManager::ScreenImpl::__onDrawcallIndirectBufferCreate(
		Drawcall &drawcall, const size_t segmentIdx) noexcept
	{
		__commandBufferDirty = true;
		__drawcallSegmentDirties.emplace(segmentIdx);
	}

	void ScreenManager::ScreenImpl::__onWindowDraw(Win::Window &window) noexcept
	{
		__needToRender = true;
	}

	void ScreenManager::ScreenImpl::__onWindowDestroy(Win::Window &window) noexcept
	{
		__destroy();
	}

	void ScreenManager::ScreenImpl::__onScreenUpdate()
	{
		if (!(__isUpdatable()))
			return;

		__update();
	}

	void ScreenManager::ScreenImpl::__onRender() noexcept
	{
		if (!(__isRenderable()))
			return;

		__render();
	}

	void ScreenManager::ScreenImpl::__onPresent() noexcept
	{
		if (!(__isPresentable()))
			return;

		__present();
	}

	ScreenManager::ScreenImpl::PerImageCommandBufferResource &
		ScreenManager::ScreenImpl::__getPerImageCommandBufferResource(const size_t imageIdx) noexcept
	{
		auto &pPerImageCommandBufferResource{ __perImageCommandBufferResources[imageIdx] };
		if (!pPerImageCommandBufferResource)
			pPerImageCommandBufferResource = std::make_unique<PerImageCommandBufferResource>();

		return *pPerImageCommandBufferResource;
	}

	Vulkan::CommandBuffer &ScreenManager::ScreenImpl::__nextSecondaryCommandBuffer(
		PerImageCommandBufferResource &resource, const size_t segmentIdx)
	{
		auto &pSecondaryManager{ resource.secondaryManagerMap[segmentIdx] };
		if (!pSecondaryManager)
		{
			pSecondaryManager = std::make_unique<CommandBufferManager>(
				__device, __queueFamilyIndex,
				VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_SECONDARY, __resourceDeleter);
		}
		else
			pSecondaryManager->advance();

		return pSecondaryManager->get();
	}

	void ScreenManager::ScreenImpl::__updateSecondaryCommandBufferHandles(
		PerImageCommandBufferResource &commandBufferResource) noexcept
	{
		std::vector<VkCommandBuffer> &handles{ commandBufferResource.secondaryCommandBufferHandles };
		handles.clear();

		for (const auto &[_, pManager] : commandBufferResource.secondaryManagerMap)
		{
			const VkCommandBuffer handle{ pManager->get().getHandle() };
			handles.emplace_back(handle);
		}
	}

	Vulkan::CommandBuffer &ScreenManager::ScreenImpl::__nextPrimaryCommandBuffer(
		PerImageCommandBufferResource &resource)
	{
		auto &pPrimaryManager{ resource.primaryManager };
		if (!pPrimaryManager)
		{
			pPrimaryManager = std::make_unique<CommandBufferManager>(
				__device, __queueFamilyIndex,
				VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY, __resourceDeleter);
		}
		else
			pPrimaryManager->advance();

		return pPrimaryManager->get();
	}

	Vulkan::Semaphore &ScreenManager::ScreenImpl::__getImageAcquireSemaphore() noexcept
	{
		auto &pSemaphore{ __imageAcquireSemaphores[__frameIdx] };
		if (!pSemaphore)
		{
			static constexpr VkSemaphoreCreateInfo createInfo
			{
				.sType = VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
			};

			pSemaphore = std::make_unique<Vulkan::Semaphore>(__device, createInfo);
		}

		return *pSemaphore;
	}

	[[nodiscard]]
	Vulkan::CommandBuffer &ScreenManager::ScreenImpl::__getPrimaryCommandBuffer() noexcept
	{
		return __getPerImageCommandBufferResource(__imageIdx).primaryManager->get();
	}

	Vulkan::Semaphore &ScreenManager::ScreenImpl::__getRenderCompletionBinarySemaphore() noexcept
	{
		auto &pSemaphore{ __renderCompletionBinarySemaphores[__imageIdx] };
		if (!pSemaphore)
		{
			static constexpr VkSemaphoreCreateInfo createInfo
			{
				.sType = VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
			};

			pSemaphore = std::make_unique<Vulkan::Semaphore>(__device, createInfo);
		}

		return *pSemaphore;
	}

	Vulkan::Semaphore &ScreenManager::ScreenImpl::__getRenderCompletionTimelineSemaphore() noexcept
	{
		auto &pSemaphore{ __renderCompletionTimelineSemaphores[__imageIdx] };
		if (!pSemaphore)
		{
			static constexpr VkSemaphoreTypeCreateInfo timelineInfo
			{
				.sType = VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
				.semaphoreType = VkSemaphoreType::VK_SEMAPHORE_TYPE_TIMELINE
			};

			static constexpr VkSemaphoreCreateInfo createInfo
			{
				.sType = VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
				.pNext = &timelineInfo
			};

			pSemaphore = std::make_unique<Vulkan::Semaphore>(__device, createInfo);
		}

		return *pSemaphore;
	}

	uint64_t &ScreenManager::ScreenImpl::__getRenderCompletionSemaphoreValue() noexcept
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
			__frameIdx = ((__frameIdx + 1ULL) % __swapChainImages.size());
			return true;
		}

		if ((acquireResult == VkResult::VK_SUBOPTIMAL_KHR) ||
			(acquireResult == VkResult::VK_ERROR_OUT_OF_DATE_KHR))
		{
			__swapchainDependencyDirty = true;
		}

		return false;
	}
}