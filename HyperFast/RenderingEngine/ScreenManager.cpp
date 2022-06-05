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
		__createMainCommandBufferManager();
	}

	ScreenManager::ScreenImpl::~ScreenImpl() noexcept
	{
		__reset();
		__destroySyncObjects();
		__destroyMainCommandBufferManager();
		__destroySurface();
	}

	bool ScreenManager::ScreenImpl::draw() noexcept
	{
		if (!__swapchain)
			__init();

		const VkSemaphore presentCompleteSemaphore{ __presentCompleteSemaphores[__mainCommandBufferCursor] };

		uint32_t imageIdx{};

		const VkResult acquirementResult
		{
			__deviceProc.vkAcquireNextImageKHR(
				__device, __swapchain, 0ULL, presentCompleteSemaphore, VK_NULL_HANDLE, &imageIdx)
		};

		if (acquirementResult )

		const size_t numCommandBuffers{ __mainCommandBuffers.size() };
		__mainCommandBufferCursor = ((__mainCommandBufferCursor + 1ULL) % numCommandBuffers);
	}

	void ScreenManager::ScreenImpl::__init()
	{
		__checkSurfaceSupport();
		__querySurfaceCapabilities();
		__querySupportedSurfaceFormats();
		__querySupportedSurfacePresentModes();
		__createSwapchain();
		__retrieveSwapchainImages();
		__createSwapchainImageViews();
		__createRenderPasses();
		__createFramebuffer();
		__initSyncObjects();

		__populatePipelineBuildParam();
		__buildPipelines();
		__recordMainCommands();
	}

	void ScreenManager::ScreenImpl::__reset() noexcept
	{
		__destroyFramebuffer();
		__destroyRenderPasses();
		__destroySwapchainImageViews();
		__resetSwapchainImages();
		__destroySwapchain();
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

	void ScreenManager::ScreenImpl::__createMainCommandBufferManager()
	{
		__pMainCommandBufferManager = std::make_unique<CommandBufferManager>(
			__device, __deviceProc, __graphicsQueueFamilyIndex, 20ULL);
	}

	void ScreenManager::ScreenImpl::__destroyMainCommandBufferManager() noexcept
	{
		__pMainCommandBufferManager = nullptr;
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
			.oldSwapchain = __swapchain
		};

		__deviceProc.vkCreateSwapchainKHR(__device, &createInfo, nullptr, &__swapchain);
		if (!__swapchain)
			throw std::exception{ "Cannot create a VkSwapchainKHR." };

		__swapchainFormat = createInfo.imageFormat;
		__swapchainExtent = createInfo.imageExtent;
	}

	void ScreenManager::ScreenImpl::__destroySwapchain() noexcept
	{
		__deviceProc.vkDestroySwapchainKHR(__device, __swapchain, nullptr);
		__swapchain = VK_NULL_HANDLE;
	}

	void ScreenManager::ScreenImpl::__retrieveSwapchainImages() noexcept
	{
		uint32_t numImages{};
		__deviceProc.vkGetSwapchainImagesKHR(__device, __swapchain, &numImages, nullptr);

		__swapChainImages.resize(numImages);
		__deviceProc.vkGetSwapchainImagesKHR(__device, __swapchain, &numImages, __swapChainImages.data());
	}

	void ScreenManager::ScreenImpl::__resetSwapchainImages() noexcept
	{
		__swapChainImages.clear();
	}

	void ScreenManager::ScreenImpl::__createSwapchainImageViews()
	{
		VkImageViewCreateInfo createInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
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

		for (const VkImage swapchainImage : __swapChainImages)
		{
			createInfo.image = swapchainImage;

			VkImageView swapchainImageView{};
			__deviceProc.vkCreateImageView(__device, &createInfo, nullptr, &swapchainImageView);

			if (!swapchainImageView)
				throw std::exception{ "Cannot create a VkImageView for the swapchain." };

			__swapChainImageViews.emplace_back(swapchainImageView);
		}
	}

	void ScreenManager::ScreenImpl::__destroySwapchainImageViews() noexcept
	{
		for (const VkImageView swapchainImageView : __swapChainImageViews)
			__deviceProc.vkDestroyImageView(__device, swapchainImageView, nullptr);

		__swapChainImageViews.clear();
	}

	void ScreenManager::ScreenImpl::__createRenderPasses()
	{
		std::vector<VkAttachmentDescription> attachments;
		std::vector<VkSubpassDescription> subpasses;
		std::vector<VkSubpassDependency> dependencies;

		VkAttachmentDescription &colorAttachment{ attachments.emplace_back() };
		colorAttachment.format = __swapchainFormat;
		colorAttachment.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		const VkAttachmentReference colorAttachmentRef
		{
			.attachment = 0U,
			.layout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		};

		VkSubpassDescription &subpass{ subpasses.emplace_back() };
		subpass.pipelineBindPoint = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1U;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkSubpassDependency &dependency{ dependencies.emplace_back() };

		// srcSubpass의 srcStageMask 파이프가 idle이 되고, 거기에 srcAccessMask가 모두 available해질 때까지 블록
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;

		// dstSubpass 진행에 대해 위 조건 + dstAccessMask가 visible 해질 때 까지 dstStageMask를 블록
		dependency.dstSubpass = 0U;
		dependency.srcStageMask = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstStageMask = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0U;
		dependency.dstAccessMask = VkAccessFlagBits::VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT;
		dependency.dependencyFlags = VkDependencyFlagBits::VK_DEPENDENCY_BY_REGION_BIT;

		/*
			세마포어나 펜스는 시그널이 들어오면 해당 큐가 모든 작업을 처리했음을 보장
			또한 모든 메모리 access에 대해 available을 보장 (암묵적 메모리 디펜던시)
			vkQueueSubmit는 host visible 메모리의 모든 access에 대해 visible함을 보장 (암묵적 메모리 디펜던시)
			세마포어 대기 요청은 모든 메모리 access에 대해 visible함을 보장 (암묵적 메모리 디펜던시)
		*/

		const VkRenderPassCreateInfo createInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.attachmentCount = uint32_t(attachments.size()),
			.pAttachments = attachments.data(),
			.subpassCount = uint32_t(subpasses.size()),
			.pSubpasses = subpasses.data(),
			.dependencyCount = uint32_t(dependencies.size()),
			.pDependencies = dependencies.data()
		};

		__deviceProc.vkCreateRenderPass(__device, &createInfo, nullptr, &__renderPass);
		if (!__renderPass)
			throw std::exception{ "Cannot create a VkRenderPass." };
	}

	void ScreenManager::ScreenImpl::__destroyRenderPasses() noexcept
	{
		__deviceProc.vkDestroyRenderPass(__device, __renderPass, nullptr);
		__renderPass = VK_NULL_HANDLE;
	}

	void ScreenManager::ScreenImpl::__createFramebuffer()
	{
		/*
			When beginning the render pass, pass in a VkRenderPassAttachmentBeginInfo structure
			into VkRenderPassBeginInfo::pNext with the compatible attachments
		*/

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

	void ScreenManager::ScreenImpl::__destroyFramebuffer() noexcept
	{
		__deviceProc.vkDestroyFramebuffer(__device, __framebuffer, nullptr);
		__framebuffer = VK_NULL_HANDLE;
	}

	void ScreenManager::ScreenImpl::__initSyncObjects()
	{
		const size_t numSwapchainImageViews{ __swapChainImageViews.size() };
		const size_t currentNumSyncObjects{ __presentCompleteSemaphores.size() };

		if (numSwapchainImageViews <= currentNumSyncObjects)
			return;

		const VkSemaphoreCreateInfo semaphoreCreateInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
		};

		const VkFenceCreateInfo fenceCreateInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_FENCE_CREATE_INFO
		};

		for (size_t imageViewIter = currentNumSyncObjects; imageViewIter < numSwapchainImageViews; imageViewIter++)
		{
			VkSemaphore presentCompleteSemaphore{};
			VkSemaphore renderCompleteSemaphore{};
			VkFence renderCompleteFence{};

			__deviceProc.vkCreateSemaphore(__device, &semaphoreCreateInfo, nullptr, &presentCompleteSemaphore);
			__deviceProc.vkCreateSemaphore(__device, &semaphoreCreateInfo, nullptr, &renderCompleteSemaphore);
			__deviceProc.vkCreateFence(__device, &fenceCreateInfo, nullptr, &renderCompleteFence);

			if (!(presentCompleteSemaphore && renderCompleteSemaphore && renderCompleteFence))
				throw std::exception{ "Error occurred while create sync objects." };

			__presentCompleteSemaphores.emplace_back(presentCompleteSemaphore);
			__renderCompleteSemaphores.emplace_back(renderCompleteSemaphore);
			__renderCompleteFences.emplace_back(renderCompleteFence);
		}
	}

	void ScreenManager::ScreenImpl::__destroySyncObjects() noexcept
	{
		for (const VkFence renderCompleteFence : __renderCompleteFences)
			__deviceProc.vkDestroyFence(__device, renderCompleteFence, nullptr);

		for (const VkSemaphore renderCompleteSemaphore : __renderCompleteSemaphores)
			__deviceProc.vkDestroySemaphore(__device, renderCompleteSemaphore, nullptr);

		for (const VkSemaphore presentCompleteSemaphore : __presentCompleteSemaphores)
			__deviceProc.vkDestroySemaphore(__device, presentCompleteSemaphore, nullptr);

		__renderCompleteFences.clear();
		__renderCompleteSemaphores.clear();
		__presentCompleteSemaphores.clear();
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

	void ScreenManager::ScreenImpl::__buildPipelines()
	{
		__pipelineFactory.build(__pipelineBuildParam);
	}

	void ScreenManager::ScreenImpl::__recordMainCommands() noexcept
	{
		const size_t numBuffers{ __swapChainImageViews.size() };
		__pMainCommandBufferManager->getNextBuffers(numBuffers, __mainCommandBuffers);
		__mainCommandBufferCursor = 0ULL;

		const VkCommandBufferBeginInfo commandBufferBeginInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO
		};

		VkRenderPassAttachmentBeginInfo renderPassAttachmentInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_RENDER_PASS_ATTACHMENT_BEGIN_INFO,
			.attachmentCount = 1U
		};

		const VkClearValue clearColor
		{
			.color = { .float32 = { 0.0f, 0.0f, 0.0f, 1.0f } }
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

		for (size_t bufferIter = 0ULL; bufferIter < numBuffers; bufferIter++)
		{
			const VkCommandBuffer commandBuffer{ __mainCommandBuffers[bufferIter] };
			__deviceProc.vkBeginCommandBuffer(commandBuffer, &commandBufferBeginInfo);

			const VkImageView colorAttachment{ __swapChainImageViews[bufferIter] };
			renderPassAttachmentInfo.pAttachments = &colorAttachment;

			__deviceProc.vkCmdBeginRenderPass(
				commandBuffer, &renderPassBeginInfo, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);

			__deviceProc.vkCmdBindPipeline(
				commandBuffer, VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, __pipelineFactory.get());

			__deviceProc.vkCmdDraw(commandBuffer, 3U, 1U, 0U, 0U);
			__deviceProc.vkCmdEndRenderPass(commandBuffer);
			__deviceProc.vkEndCommandBuffer(commandBuffer);
		}
	}
}