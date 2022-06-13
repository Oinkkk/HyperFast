#include "ScreenManager.h"

namespace HyperFast
{
	ScreenManager::ScreenImpl::ResourceBundle::ResourceBundle(
		const VkInstance instance, const VKL::InstanceProcedure &instanceProc,
		Win::Window &window, const VkSurfaceKHR surface,
		const VkPhysicalDevice physicalDevice, const uint32_t graphicsQueueFamilyIndex,
		const VkDevice device, const VKL::DeviceProcedure &deviceProc) noexcept :
		__instance{ instance }, __instanceProc{ instanceProc },
		__window{ window }, __surface{ surface },
		__physicalDevice{ physicalDevice }, __graphicsQueueFamilyIndex{ graphicsQueueFamilyIndex },
		__device{ device }, __deviceProc{ deviceProc }, __pipelineFactory{ device, deviceProc }
	{}
	
	ScreenManager::ScreenImpl::ResourceBundle::~ResourceBundle() noexcept
	{
		__resetPipelines();
		__destroyFramebuffer();
		__destroyRenderPasses();
		__destroySyncObjects();
		__destroyMainCommandBufferManagers();
		__destroySwapchainImageViews();
		__destroySwapchain(__swapchain);
	}

	void ScreenManager::ScreenImpl::ResourceBundle::update(tf::Subflow &subflow)
	{
		const VkSwapchainKHR oldSwapchain{ __swapchain };

		tf::Task t1
		{
			subflow.emplace([this, oldSwapchain](tf::Subflow &subflow)
			{
				__resetPipelines();
				__destroyFramebuffer();
				__destroyRenderPasses();
				__destroySwapchainImageViews();

				__checkSurfaceSupport();
				__querySurfaceCapabilities();
				__querySupportedSurfaceFormats();
				__querySupportedSurfacePresentModes();
				__createSwapchain(oldSwapchain);
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
					subflow.emplace([this]
					{
						__populatePipelineBuildParam();
						__buildPipelines();
					})
				};
				t3.succeed(t1);
			})
		};

		tf::Task t2
		{
			subflow.emplace([this](tf::Subflow &subflow)
			{
				const size_t numSwapchainImages{ __swapChainImages.size() };
				for (size_t swapchainImageIter = 0ULL; swapchainImageIter < numSwapchainImages; swapchainImageIter++)
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

		if (oldSwapchain)
		{
			subflow.emplace([this, oldSwapchain]
			{
				__destroySwapchain(oldSwapchain);
			}).succeed(t1);
		}
	}

	VkResult ScreenManager::ScreenImpl::ResourceBundle::acquireNextImage(
		const VkSemaphore semaphore, uint32_t &imageIdx) noexcept
	{
		return __deviceProc.vkAcquireNextImageKHR(
			__device, __swapchain, 0ULL, semaphore, VK_NULL_HANDLE, &imageIdx);
	}

	void ScreenManager::ScreenImpl::ResourceBundle::__checkSurfaceSupport() const
	{
		VkBool32 surfaceSupported{};
		__instanceProc.vkGetPhysicalDeviceSurfaceSupportKHR(
			__physicalDevice, __graphicsQueueFamilyIndex, __surface, &surfaceSupported);

		if (!surfaceSupported)
			throw std::exception{ "The physical device doesn't support the surface." };
	}

	void ScreenManager::ScreenImpl::ResourceBundle::__querySurfaceCapabilities() noexcept
	{
		__instanceProc.vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
			__physicalDevice, __surface, &__surfaceCapabilities);
	}

	void ScreenManager::ScreenImpl::ResourceBundle::__querySupportedSurfaceFormats() noexcept
	{
		uint32_t numFormats{};
		__instanceProc.vkGetPhysicalDeviceSurfaceFormatsKHR(
			__physicalDevice, __surface, &numFormats, nullptr);

		__supportedSurfaceFormats.resize(numFormats);
		__instanceProc.vkGetPhysicalDeviceSurfaceFormatsKHR(
			__physicalDevice, __surface, &numFormats, __supportedSurfaceFormats.data());
	}

	void ScreenManager::ScreenImpl::ResourceBundle::__querySupportedSurfacePresentModes() noexcept
	{
		uint32_t numModes{};
		__instanceProc.vkGetPhysicalDeviceSurfacePresentModesKHR(
			__physicalDevice, __surface, &numModes, nullptr);

		__supportedSurfacePresentModes.resize(numModes);
		__instanceProc.vkGetPhysicalDeviceSurfacePresentModesKHR(
			__physicalDevice, __surface, &numModes, __supportedSurfacePresentModes.data());
	}

	void ScreenManager::ScreenImpl::ResourceBundle::__createSwapchain(const VkSwapchainKHR oldSwapchain)
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
			.oldSwapchain = oldSwapchain
		};

		__deviceProc.vkCreateSwapchainKHR(__device, &createInfo, nullptr, &__swapchain);
		if (!__swapchain)
			throw std::exception{ "Cannot create a VkSwapchainKHR." };

		__swapchainFormat = createInfo.imageFormat;
		__swapchainExtent = createInfo.imageExtent;
	}

	void ScreenManager::ScreenImpl::ResourceBundle::__destroySwapchain(const VkSwapchainKHR swapchain) noexcept
	{
		__deviceProc.vkDestroySwapchainKHR(__device, swapchain, nullptr);
	}

	void ScreenManager::ScreenImpl::ResourceBundle::__retrieveSwapchainImages() noexcept
	{
		uint32_t numImages{};
		__deviceProc.vkGetSwapchainImagesKHR(__device, __swapchain, &numImages, nullptr);

		__swapChainImages.resize(numImages);
		__deviceProc.vkGetSwapchainImagesKHR(__device, __swapchain, &numImages, __swapChainImages.data());
	}

	void ScreenManager::ScreenImpl::ResourceBundle::__reserveSwapchainImageDependencyPlaceholers() noexcept
	{
		const size_t numSwapchainImages{ __swapChainImages.size() };

		__mainCommandBufferManagers.resize(numSwapchainImages, nullptr);
		__mainCommandBuffers.resize(numSwapchainImages);
		__swapChainImageViews.resize(numSwapchainImages);
		__presentCompleteSemaphores.resize(numSwapchainImages, VK_NULL_HANDLE);
		__renderCompleteSemaphores.resize(numSwapchainImages, VK_NULL_HANDLE);
		__renderCompleteFences.resize(numSwapchainImages, VK_NULL_HANDLE);
	}

	void ScreenManager::ScreenImpl::ResourceBundle::__createMainCommandBufferManager(const size_t imageIdx)
	{
		CommandBufferManager *&pManager{ __mainCommandBufferManagers[imageIdx] };
		if (pManager)
			return;

		pManager = new CommandBufferManager{ __device, __deviceProc, __graphicsQueueFamilyIndex, 8ULL };
	}

	void ScreenManager::ScreenImpl::ResourceBundle::__destroyMainCommandBufferManagers() noexcept
	{
		for (CommandBufferManager *const pManager : __mainCommandBufferManagers)
			delete pManager;
	}

	void ScreenManager::ScreenImpl::ResourceBundle::__createSwapchainImageView(const size_t imageIdx)
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

		VkImageView swapchainImageView{};
		__deviceProc.vkCreateImageView(__device, &createInfo, nullptr, &swapchainImageView);

		if (!swapchainImageView)
			throw std::exception{ "Cannot create a VkImageView for the swapchain." };

		__swapChainImageViews[imageIdx] = swapchainImageView;
	}

	void ScreenManager::ScreenImpl::ResourceBundle::__destroySwapchainImageViews() noexcept
	{
		for (VkImageView &swapchainImageView : __swapChainImageViews)
			__deviceProc.vkDestroyImageView(__device, swapchainImageView, nullptr);
	}

	void ScreenManager::ScreenImpl::ResourceBundle::__createRenderPasses()
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

		__deviceProc.vkCreateRenderPass2(__device, &createInfo, nullptr, &__renderPass);
		if (!__renderPass)
			throw std::exception{ "Cannot create a VkRenderPass." };
	}

	void ScreenManager::ScreenImpl::ResourceBundle::__destroyRenderPasses() noexcept
	{
		if (!__renderPass)
			return;

		__deviceProc.vkDestroyRenderPass(__device, __renderPass, nullptr);
		__renderPass = VK_NULL_HANDLE;
	}

	void ScreenManager::ScreenImpl::ResourceBundle::__createFramebuffer()
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

		__deviceProc.vkCreateFramebuffer(__device, &createInfo, nullptr, &__framebuffer);
		if (!__framebuffer)
			throw std::exception{ "Cannot create a VkFramebuffer." };
	}

	void ScreenManager::ScreenImpl::ResourceBundle::__destroyFramebuffer() noexcept
	{
		if (!__framebuffer)
			return;

		__deviceProc.vkDestroyFramebuffer(__device, __framebuffer, nullptr);
		__framebuffer = VK_NULL_HANDLE;
	}

	void ScreenManager::ScreenImpl::ResourceBundle::__createSyncObject(const size_t imageIdx)
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

		__deviceProc.vkCreateSemaphore(__device, &semaphoreCreateInfo, nullptr, &presentCompleteSemaphore);
		__deviceProc.vkCreateSemaphore(__device, &semaphoreCreateInfo, nullptr, &renderCompleteSemaphore);
		__deviceProc.vkCreateFence(__device, &fenceCreateInfo, nullptr, &renderCompleteFence);

		if (!(presentCompleteSemaphore && renderCompleteSemaphore && renderCompleteFence))
			throw std::exception{ "Error occurred while create sync objects." };

		__presentCompleteSemaphores[imageIdx] = presentCompleteSemaphore;
		__renderCompleteSemaphores[imageIdx] = renderCompleteSemaphore;
		__renderCompleteFences[imageIdx] = renderCompleteFence;
	}

	void ScreenManager::ScreenImpl::ResourceBundle::__destroySyncObjects() noexcept
	{
		for (const VkFence renderCompleteFence : __renderCompleteFences)
			__deviceProc.vkDestroyFence(__device, renderCompleteFence, nullptr);

		for (const VkSemaphore renderCompleteSemaphore : __renderCompleteSemaphores)
			__deviceProc.vkDestroySemaphore(__device, renderCompleteSemaphore, nullptr);

		for (const VkSemaphore presentCompleteSemaphore : __presentCompleteSemaphores)
			__deviceProc.vkDestroySemaphore(__device, presentCompleteSemaphore, nullptr);
	}

	void ScreenManager::ScreenImpl::ResourceBundle::__populatePipelineBuildParam() noexcept
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

	void ScreenManager::ScreenImpl::ResourceBundle::__buildPipelines()
	{
		__pipelineFactory.build(__pipelineBuildParam);
	}

	void ScreenManager::ScreenImpl::ResourceBundle::__resetPipelines() noexcept
	{
		__pipelineFactory.reset();
	}

	void ScreenManager::ScreenImpl::ResourceBundle::__recordMainCommand(const size_t imageIdx) noexcept
	{
		const VkCommandBufferBeginInfo commandBufferBeginInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
		};

		const VkRenderPassAttachmentBeginInfo renderPassAttachmentInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_RENDER_PASS_ATTACHMENT_BEGIN_INFO,
			.attachmentCount = 1U,
			.pAttachments = &(__swapChainImageViews[imageIdx])
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

		// TODO: buffer manager 리셋 타임 확인 필요
		const VkCommandBuffer commandBuffer{ __mainCommandBufferManagers[imageIdx]->getNextBuffer() };
		__mainCommandBuffers[imageIdx] = commandBuffer;

		__deviceProc.vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);
		__deviceProc.vkCmdBeginRenderPass(
			commandBuffer, &renderPassBeginInfo, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);

		__deviceProc.vkCmdBindPipeline(
			commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, __pipelineFactory.get());

		__deviceProc.vkCmdDraw(commandBuffer, 3U, 1U, 0U, 0U);
		__deviceProc.vkCmdEndRenderPass(commandBuffer);
		__deviceProc.vkEndCommandBuffer(commandBuffer);
	}
}