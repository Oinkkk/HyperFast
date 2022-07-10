#include "ScreenManager.h"

namespace HyperFast
{
	ScreenManager::ScreenImpl::ScreenImpl(
		Vulkan::Instance &instance, Vulkan::PhysicalDevice &physicalDevice,
		const uint32_t graphicsQueueFamilyIndex, Vulkan::Device &device,
		Vulkan::Queue &queue, Win::Window &window) :
		__instance{ instance }, __physicalDevice{ physicalDevice },
		__graphicsQueueFamilyIndex{ graphicsQueueFamilyIndex },
		__device{ device }, __queue{ queue },
		__window{ window }, __pipelineFactory{ device }
	{
		__initListeners();
		__createSurface();
		__updateSurfaceDependencies();
	}

	ScreenManager::ScreenImpl::~ScreenImpl() noexcept
	{
		__destroy();
	}

	void ScreenManager::ScreenImpl::setDrawcall(Drawcall *const pDrawcall) noexcept
	{
		if (__pDrawcall == pDrawcall)
			return;

		if (__pDrawcall)
		{
			__pDrawcall->getAttributeFlagsUpdateEvent() -= __pAttribFlagsUpdateEventListener;
			__pDrawcall->getIndirectBufferUpdateEvent() -= __pIndirectBufferUpdateListener;
			__pDrawcall->getIndirectBufferCreateEvent() -= __pIndirectBufferCreateListener;
		}

		__pDrawcall = pDrawcall;

		if (__pDrawcall)
		{
			__pDrawcall->getAttributeFlagsUpdateEvent() += __pAttribFlagsUpdateEventListener;
			__pDrawcall->getIndirectBufferUpdateEvent() += __pIndirectBufferUpdateListener;
			__pDrawcall->getIndirectBufferCreateEvent() += __pIndirectBufferCreateListener;
		}

		__needToUpdatePipelineDependencies = true;
	}

	void ScreenManager::ScreenImpl::draw()
	{
		if (__destroyed)
			return;

		const bool validSize{ __window.getWidth() && __window.getHeight() };
		if (!validSize)
			return;

		__update();

		if (__needToDraw)
		{
			__draw();
			__needToDraw = false;
		}
	}

	void ScreenManager::ScreenImpl::__update()
	{
		if (__needToUpdateSurfaceDependencies)
		{
			__updateSurfaceDependencies();
			__needToUpdateSurfaceDependencies = false;
			__needToUpdatePipelineDependencies = false;
			__needToUpdateMainCommands = false;
			__needToDraw = true;
		}

		if (__needToUpdatePipelineDependencies)
		{
			__updatePipelineDependencies();
			__needToUpdatePipelineDependencies = false;
			__needToUpdateMainCommands = false;
			__needToDraw = true;
		}

		if (__needToUpdateMainCommands)
		{
			__updateMainCommands();
			__needToUpdateMainCommands = false;
			__needToDraw = true;
		}
	}

	bool ScreenManager::ScreenImpl::__draw()
	{
		const VkSemaphore presentCompleteSemaphore{ __presentCompleteSemaphores[__frameCursor] };

		if (!__imageAcquired)
		{
			const VkResult acquirementResult
			{
				__pSwapchain->vkAcquireNextImageKHR(
					0ULL, presentCompleteSemaphore, VK_NULL_HANDLE, &__imageIdx)
			};

			if (acquirementResult == VkResult::VK_NOT_READY)
				return false;

			if ((acquirementResult == VkResult::VK_SUBOPTIMAL_KHR) ||
				(acquirementResult == VkResult::VK_ERROR_OUT_OF_DATE_KHR))
			{
				__needToUpdateSurfaceDependencies = true;
				return false;
			}

			__advanceFrameCursor();
			__imageAcquired = true;
		}
		
		const VkFence renderCompleteFence{ __renderCompleteFences[__imageIdx] };
		const VkResult waitResult
		{
			__device.vkWaitForFences(1U, &renderCompleteFence, VK_TRUE, 0ULL)
		};

		if (waitResult == VkResult::VK_TIMEOUT)
			return false;

		__device.vkResetFences(1U, &renderCompleteFence);

		const VkCommandBuffer mainCommandBuffer{ __mainCommandBuffers[__imageIdx] };
		const VkSemaphore renderCompleteSemaphore{ __renderCompleteSemaphores[__imageIdx] };

		const VkSemaphoreSubmitInfo waitInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
			.semaphore = presentCompleteSemaphore,
			.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT
		};

		const VkCommandBufferSubmitInfo commandBufferInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
			.commandBuffer = mainCommandBuffer
		};

		const VkSemaphoreSubmitInfo signalInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
			.semaphore = renderCompleteSemaphore,
			.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT
		};

		const VkSubmitInfo2 submitInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
			.waitSemaphoreInfoCount = 1U,
			.pWaitSemaphoreInfos = &waitInfo,
			.commandBufferInfoCount = 1U,
			.pCommandBufferInfos = &commandBufferInfo,
			.signalSemaphoreInfoCount = 1U,
			.pSignalSemaphoreInfos = &signalInfo
		};

		__queue.vkQueueSubmit2(1U, &submitInfo, renderCompleteFence);

		const VkPresentInfoKHR presentInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
			.waitSemaphoreCount = 1U,
			.pWaitSemaphores = &renderCompleteSemaphore,
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

		return (presentResult == VkResult::VK_SUCCESS);
	}

	void ScreenManager::ScreenImpl::__destroy() noexcept
	{
		if (__destroyed)
			return;

		__device.vkDeviceWaitIdle();
		__resetPipelines();
		__destroyFramebuffer();
		__destroyRenderPasses();
		__destroySyncObjects();
		__swapChainImageViews.clear();
		__destroyMainCommandBufferManagers();
		__pSwapchain = nullptr;
		__destroySurface();
	
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
			__needToDraw = true;
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
			__needToDraw = true;
		});

		__pIndirectBufferCreateListener = Infra::EventListener<Drawcall &>::make([this](Drawcall &)
		{
			__needToUpdateMainCommands = true;
		});

		__window.getResizeEvent() += __pResizeEventListener;
		__window.getDrawEvent() += __pDrawEventListener;
		__window.getDestroyEvent() += __pDestroyEventListener;
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

		__instance.vkCreateWin32SurfaceKHR(&createInfo, nullptr, &__surface);
		if (!__surface)
			throw std::exception{ "Cannot create a surface." };
	}

	void ScreenManager::ScreenImpl::__destroySurface() noexcept
	{
		__instance.vkDestroySurfaceKHR(__surface, nullptr);
	}

	void ScreenManager::ScreenImpl::__updateSurfaceDependencies()
	{
		std::unique_ptr<Vulkan::Swapchain> pOldSwapchain{ std::move(__pSwapchain) };

		tf::Taskflow taskflow;
		tf::Task t1
		{
			taskflow.emplace([this, &pOldSwapchain](tf::Subflow &subflow)
			{
				if (pOldSwapchain)
				{
					__device.vkDeviceWaitIdle();
					__resetPipelines();
					__destroyFramebuffer();
					__destroyRenderPasses();
					__swapChainImageViews.clear();
				}

				__checkSurfaceSupport();
				__querySurfaceCapabilities();
				__querySupportedSurfaceFormats();
				__querySupportedSurfacePresentModes();
				__createSwapchain(pOldSwapchain.get());
				__retrieveSwapchainImages();
				__reserveSwapchainImageDependencyPlaceholers();
				__resetFrameCursor();

				tf::Task t1
				{
					subflow.emplace([this]
					{
						__createRenderPasses();
					})
				};

				tf::Task t2
				{
					subflow.emplace([this]
					{
						__createFramebuffer();
					})
				};
				t2.succeed(t1);

				tf::Task t3
				{
					subflow.emplace([this](tf::Subflow &subflow)
					{
						__populatePipelineBuildParam();
						__buildPipelines(subflow);
					})
				};
				t3.succeed(t1);
			})
		};

		tf::Task t2
		{
			taskflow.emplace([this](tf::Subflow &subflow)
			{
				const size_t numSwapchainImages{ __swapChainImages.size() };
				for (
					size_t swapchainImageIter = 0ULL;
					swapchainImageIter < numSwapchainImages;
					swapchainImageIter++)
				{
					subflow.emplace([this, imageIdx = swapchainImageIter]
					{
						__createMainCommandBufferManager(imageIdx);
						__createSwapchainImageView(imageIdx);
						__createSyncObject(imageIdx);
						__recordMainCommand(imageIdx);
					});
				}
			})
		};
		t2.succeed(t1);

		taskflow.emplace([this, &pOldSwapchain]()
		{
			pOldSwapchain = nullptr;
		}).succeed(t1);

		tf::Executor &executor{ Infra::Environment::getInstance().getTaskflowExecutor() };
		executor.run(taskflow).wait();
	}

	void ScreenManager::ScreenImpl::__updatePipelineDependencies()
	{
		tf::Taskflow taskflow;
		taskflow.emplace([this](tf::Subflow &subflow)
		{
			__device.vkDeviceWaitIdle();
			__resetPipelines();
			__resetFrameCursor();
			__populatePipelineBuildParam();

			tf::Task t1
			{
				subflow.emplace([this](tf::Subflow &subflow)
				{
					__buildPipelines(subflow);
				})
			};

			subflow.emplace([this](tf::Subflow &subflow)
			{
				const size_t numSwapchainImages{ __swapChainImages.size() };
				for (
					size_t swapchainImageIter = 0ULL;
					swapchainImageIter < numSwapchainImages;
					swapchainImageIter++)
				{
					subflow.emplace([this, imageIdx = swapchainImageIter]
					{
						__recordMainCommand(imageIdx);
					});
				}
			}).succeed(t1);
		});

		tf::Executor &executor{ Infra::Environment::getInstance().getTaskflowExecutor() };
		executor.run(taskflow).wait();
	}

	void ScreenManager::ScreenImpl::__updateMainCommands() noexcept
	{
		tf::Taskflow taskflow;
		taskflow.emplace([this](tf::Subflow &subflow)
		{
			__device.vkDeviceWaitIdle();
			__resetFrameCursor();

			subflow.emplace([this](tf::Subflow &subflow)
			{
				const size_t numSwapchainImages{ __swapChainImages.size() };
				for (
					size_t swapchainImageIter = 0ULL;
					swapchainImageIter < numSwapchainImages;
					swapchainImageIter++)
				{
					subflow.emplace([this, imageIdx = swapchainImageIter]
					{
						__recordMainCommand(imageIdx);
					});
				}
			});
		});

		tf::Executor &executor{ Infra::Environment::getInstance().getTaskflowExecutor() };
		executor.run(taskflow).wait();
	}

	void ScreenManager::ScreenImpl::__checkSurfaceSupport() const
	{
		VkBool32 surfaceSupported{};
		__physicalDevice.vkGetPhysicalDeviceSurfaceSupportKHR(
			__graphicsQueueFamilyIndex, __surface, &surfaceSupported);

		if (!surfaceSupported)
			throw std::exception{ "The physical device doesn't support the surface." };
	}

	void ScreenManager::ScreenImpl::__querySurfaceCapabilities() noexcept
	{
		__physicalDevice.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(__surface, &__surfaceCapabilities);
	}

	void ScreenManager::ScreenImpl::__querySupportedSurfaceFormats() noexcept
	{
		uint32_t numFormats{};
		__physicalDevice.vkGetPhysicalDeviceSurfaceFormatsKHR(
			__surface, &numFormats, nullptr);

		__supportedSurfaceFormats.resize(numFormats);
		__physicalDevice.vkGetPhysicalDeviceSurfaceFormatsKHR(
			__surface, &numFormats, __supportedSurfaceFormats.data());
	}

	void ScreenManager::ScreenImpl::__querySupportedSurfacePresentModes() noexcept
	{
		uint32_t numModes{};
		__physicalDevice.vkGetPhysicalDeviceSurfacePresentModesKHR(
			__surface, &numModes, nullptr);

		__supportedSurfacePresentModes.resize(numModes);
		__physicalDevice.vkGetPhysicalDeviceSurfacePresentModesKHR(
			__surface, &numModes, __supportedSurfacePresentModes.data());
	}

	void ScreenManager::ScreenImpl::__createSwapchain(Vulkan::Swapchain *const pOldSwapchain)
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
			.surface = __surface,
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
			.oldSwapchain = (pOldSwapchain ? pOldSwapchain->getHandle() : VK_NULL_HANDLE)
		};

		__pSwapchain = std::make_unique<Vulkan::Swapchain>(__device, createInfo);
		__swapchainFormat = createInfo.imageFormat;
		__swapchainExtent = createInfo.imageExtent;
	}

	void ScreenManager::ScreenImpl::__retrieveSwapchainImages() noexcept
	{
		uint32_t numImages{};
		__pSwapchain->vkGetSwapchainImagesKHR(&numImages, nullptr);

		__swapChainImages.resize(numImages);
		__pSwapchain->vkGetSwapchainImagesKHR(&numImages, __swapChainImages.data());
	}

	void ScreenManager::ScreenImpl::__reserveSwapchainImageDependencyPlaceholers() noexcept
	{
		const size_t numSwapchainImages{ __swapChainImages.size() };

		__mainCommandBufferManagers.resize(numSwapchainImages, nullptr);
		__mainCommandBuffers.resize(numSwapchainImages);
		__swapChainImageViews.resize(numSwapchainImages);
		__presentCompleteSemaphores.resize(numSwapchainImages, VK_NULL_HANDLE);
		__renderCompleteSemaphores.resize(numSwapchainImages, VK_NULL_HANDLE);
		__renderCompleteFences.resize(numSwapchainImages, VK_NULL_HANDLE);
	}

	void ScreenManager::ScreenImpl::__createMainCommandBufferManager(const size_t imageIdx)
	{
		CommandBufferManager *&pManager{ __mainCommandBufferManagers[imageIdx] };
		if (pManager)
			return;

		pManager = new CommandBufferManager{ __device, __graphicsQueueFamilyIndex, 8ULL };
	}

	void ScreenManager::ScreenImpl::__destroyMainCommandBufferManagers() noexcept
	{
		for (CommandBufferManager *const pManager : __mainCommandBufferManagers)
			delete pManager;
	}

	void ScreenManager::ScreenImpl::__createSwapchainImageView(const size_t imageIdx)
	{
		VkImageViewCreateInfo createInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = __swapChainImages[imageIdx],
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

		__swapChainImageViews[imageIdx] =
			std::make_unique<Vulkan::ImageView>(__device, createInfo);
	}

	void ScreenManager::ScreenImpl::__createRenderPasses()
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

		/*
			세마포어나 펜스는 시그널이 들어오면 해당 큐가 모든 작업을 처리했음을 보장
			또한 모든 메모리 access에 대해 available을 보장 (암묵적 메모리 디펜던시)
			vkQueueSubmit는 host visible 메모리의 모든 access에 대해 visible함을 보장 (암묵적 메모리 디펜던시)
			세마포어 대기 요청은 모든 메모리 access에 대해 visible함을 보장 (암묵적 메모리 디펜던시)
		*/
		const VkMemoryBarrier2 subpassBarrier
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
			.srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			.srcAccessMask = 0ULL, // 암묵적 메모리 디펜던시
			.dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
			.dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT
		};

		VkSubpassDependency2 &dependency{ dependencies.emplace_back() };
		dependency.sType = VkStructureType::VK_STRUCTURE_TYPE_SUBPASS_DEPENDENCY_2;
		dependency.pNext = &subpassBarrier;

		// srcSubpass의 srcStageMask 파이프가 idle이 되고, 거기에 srcAccessMask가 모두 available해질 때까지 블록
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;

		// dstSubpass 진행에 대해 위 조건 + dstAccessMask가 visible 해질 때 까지 dstStageMask를 블록
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

		__device.vkCreateRenderPass2(&createInfo, nullptr, &__renderPass);
		if (!__renderPass)
			throw std::exception{ "Cannot create a VkRenderPass." };
	}

	void ScreenManager::ScreenImpl::__destroyRenderPasses() noexcept
	{
		__device.vkDestroyRenderPass(__renderPass, nullptr);
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
			.renderPass = __renderPass,
			.attachmentCount = 1U,
			.width = __swapchainExtent.width,
			.height = __swapchainExtent.height,
			.layers = 1U
		};

		__device.vkCreateFramebuffer(&createInfo, nullptr, &__framebuffer);
		if (!__framebuffer)
			throw std::exception{ "Cannot create a VkFramebuffer." };
	}

	void ScreenManager::ScreenImpl::__destroyFramebuffer() noexcept
	{
		__device.vkDestroyFramebuffer(__framebuffer, nullptr);
	}

	void ScreenManager::ScreenImpl::__createSyncObject(const size_t imageIdx)
	{
		if (__presentCompleteSemaphores[imageIdx] != VK_NULL_HANDLE)
			return;

		const VkSemaphoreCreateInfo semaphoreCreateInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
		};

		const VkFenceCreateInfo fenceCreateInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
			.flags = VkFenceCreateFlagBits::VK_FENCE_CREATE_SIGNALED_BIT
		};

		VkSemaphore presentCompleteSemaphore{};
		VkSemaphore renderCompleteSemaphore{};
		VkFence renderCompleteFence{};

		__device.vkCreateSemaphore(&semaphoreCreateInfo, nullptr, &presentCompleteSemaphore);
		__device.vkCreateSemaphore(&semaphoreCreateInfo, nullptr, &renderCompleteSemaphore);
		__device.vkCreateFence(&fenceCreateInfo, nullptr, &renderCompleteFence);

		if (!(presentCompleteSemaphore && renderCompleteSemaphore && renderCompleteFence))
			throw std::exception{ "Error occurred while create sync objects." };

		__presentCompleteSemaphores[imageIdx] = presentCompleteSemaphore;
		__renderCompleteSemaphores[imageIdx] = renderCompleteSemaphore;
		__renderCompleteFences[imageIdx] = renderCompleteFence;
	}

	void ScreenManager::ScreenImpl::__destroySyncObjects() noexcept
	{
		for (const VkFence renderCompleteFence : __renderCompleteFences)
			__device.vkDestroyFence(renderCompleteFence, nullptr);

		for (const VkSemaphore renderCompleteSemaphore :__renderCompleteSemaphores)
			__device.vkDestroySemaphore(renderCompleteSemaphore, nullptr);

		for (const VkSemaphore presentCompleteSemaphore :__presentCompleteSemaphores)
			__device.vkDestroySemaphore(presentCompleteSemaphore, nullptr);
	}

	void ScreenManager::ScreenImpl::__populatePipelineBuildParam() noexcept
	{
		__pipelineBuildParam.renderPass = __renderPass;
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
	}

	void ScreenManager::ScreenImpl::__buildPipelines(tf::Subflow &subflow)
	{
		if (__pDrawcall)
		{
			__pipelineFactory.build(
				__pDrawcall->getAttributeFlags(), __pipelineBuildParam, subflow);
		}
	}

	void ScreenManager::ScreenImpl::__resetPipelines() noexcept
	{
		__pipelineFactory.reset();
	}

	void ScreenManager::ScreenImpl::__recordMainCommand(const size_t imageIdx) noexcept
	{
		static const VkCommandBufferBeginInfo commandBufferBeginInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
		};

		const VkRenderPassAttachmentBeginInfo renderPassAttachmentInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_RENDER_PASS_ATTACHMENT_BEGIN_INFO,
			.attachmentCount = 1U,
			.pAttachments = &(__swapChainImageViews[imageIdx]->getHandle())
		};

		const VkClearValue clearColor
		{
			.color = {.float32 = { 0.004f, 0.004f, 0.004f, 1.0f } }
		};

		const VkRenderPassBeginInfo renderPassBeginInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
			.pNext = &renderPassAttachmentInfo,
			.renderPass = __renderPass,
			.framebuffer = __framebuffer,
			.renderArea =
			{
				.offset = { 0, 0 },
				.extent = __swapchainExtent
			},
			.clearValueCount = 1U,
			.pClearValues = &clearColor
		};

		const VkCommandBuffer commandBuffer{ __mainCommandBufferManagers[imageIdx]->getNextBuffer() };
		__mainCommandBuffers[imageIdx] = commandBuffer;

		__device.vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
		__device.vkCmdBeginRenderPass(
			commandBuffer, &renderPassBeginInfo, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);

		if (__pDrawcall)
		{
			for (const VertexAttributeFlag attribFlag : __pDrawcall->getAttributeFlags())
			{
				const VkPipeline pipeline{ __pipelineFactory.get(attribFlag) };

				__device.vkCmdBindPipeline(
					commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

				__pDrawcall->draw(attribFlag, commandBuffer);
			}
		}

		__device.vkCmdEndRenderPass(commandBuffer);
		__device.vkEndCommandBuffer(commandBuffer);
	}
}