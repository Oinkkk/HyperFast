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
		__needToUpdatePipelineDependencies = true;
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
		if (!(__isValid()))
			return false;

		// TODO: 추가 로직

		return true;
	}

	void ScreenManager::ScreenImpl::__update()
	{
		if (__needToUpdateSwapchainDependencies)
			__updateSwapchainDependencies();

		if (__needToUpdatePipelineDependencies)
			__updatePipelineDependencies();

		if (__needToUpdateCommandBuffers)
			__updateCommandBuffers();
	}

	void ScreenManager::ScreenImpl::__render() noexcept
	{
		// TODO: 렌더링 로직

		__needToRender = false;
		__needToPresent = true;
	}

	void ScreenManager::ScreenImpl::__present() noexcept
	{
		// TODO: present 로직
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

		__resourceDeleter.reserve(__pFramebuffer);
		__resourceDeleter.reserve(__pRenderPass);
		__resourceDeleter.reserve(__pSwapchain);
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
		__needToUpdateSwapchainDependencies = true;
	}

	void ScreenManager::ScreenImpl::__updateSwapchainDependencies()
	{
		__checkSurfaceSupport();
		__querySurfaceCapabilities();
		__querySupportedSurfaceFormats();
		__querySupportedSurfacePresentModes();

		__resourceDeleter.reserve(__pFramebuffer);
		__resourceDeleter.reserve(__pRenderPass);
		__resourceDeleter.reserve(__pSwapchain);

		__createSwapchain();
		__createRenderPass();
		__createFramebuffer();

		__updatePipelineDependencies();
		__needToUpdateSwapchainDependencies = false;
	}

	void ScreenManager::ScreenImpl::__updatePipelineDependencies()
	{
		// TODO: 파이프라인 업데이트
		__updateCommandBuffers();
		__needToUpdatePipelineDependencies = false;
	}

	void ScreenManager::ScreenImpl::__updateCommandBuffers()
	{
		// TODO: 커맨드 버퍼 업데이트
		__needToUpdateCommandBuffers = false;
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

	void ScreenManager::ScreenImpl::__onWindowResize(
		Win::Window &window, const Win::Window::ResizingType resizingType) noexcept
	{
		if (resizingType == Win::Window::ResizingType::MINIMIZED)
			return;

		__needToUpdateSwapchainDependencies = true;
	}

	void ScreenManager::ScreenImpl::__onDrawcallMeshBufferChange(
		Drawcall &drawcall, const size_t segmentIndex) noexcept
	{
		__needToUpdateCommandBuffers = true;
	}

	void ScreenManager::ScreenImpl::__onDrawcallIndirectBufferUpdate(
		Drawcall &drawcall, const size_t segmentIndex) noexcept
	{
		__needToRender = true;
	}

	void ScreenManager::ScreenImpl::__onDrawcallIndirectBufferCreate(
		Drawcall &drawcall, const size_t segmentIndex) noexcept
	{
		__needToUpdateCommandBuffers = true;
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
		if (!(__isValid()))
			return;

		__update();
	}

	void ScreenManager::ScreenImpl::__onRender() noexcept
	{
		if (!__needToRender)
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