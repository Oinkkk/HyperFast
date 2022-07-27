#include "ScreenResource.h"

namespace HyperFast
{
	ScreenResource::ScreenResource(Vulkan::Device &device, const uint32_t queueFamilyIndex) noexcept :
		__device{ device }, __queueFamilyIndex{ queueFamilyIndex }, __pipelineFactory{ device }
	{
		__createSecondaryCommandBuffers();
	}

	ScreenResource::~ScreenResource() noexcept
	{
		waitIdle();
		__pipelineFactory.reset();
		__pFramebuffer = nullptr;
		__pRenderPass = nullptr;
		__swapChainImageViews.clear();
		__primaryCommandBufferManagers.clear();
	}

	Vulkan::CommandBuffer &ScreenResource::getRenderCommandBuffer(const size_t imageIdx) noexcept
	{
		return __primaryCommandBufferManagers[imageIdx]->get();
	}

	void ScreenResource::addSemaphoreDependency(
		const std::shared_ptr<SemaphoreDependency> &pDependency) noexcept
	{
		__semaphoreDependencyCluster.add(pDependency);
	}

	bool ScreenResource::isIdle() noexcept
	{
		using namespace std;

		if (!(__semaphoreDependencyCluster.isIdle()))
			return false;

		if (__job.valid())
		{
			if (__job.wait_for(0s) != std::future_status::ready)
				return false;
			else
			{
				__job.get();
				return true;
			}
		}

		return true;
	}

	void ScreenResource::waitIdle() noexcept
	{
		if (!(__job.valid()))
			return;

		__job.wait();
	}

	void ScreenResource::update(const SwapchainParam &swapchainParam, Drawcall *const pDrawcall)
	{
		if (__needToUpdateSwapchainDependencies)
			__updateSwapchainDependencies(swapchainParam, pDrawcall);

		if (__needToUpdatePipelineDependencies)
			__updatePipelineDependencies(swapchainParam, pDrawcall);

		if (__needToPrimaryCommandBuffer)
			__updateCommandBuffers(swapchainParam, pDrawcall);
	}

	void ScreenResource::__createSecondaryCommandBuffers() noexcept
	{
		const uint32_t numCores{ std::thread::hardware_concurrency() };
		for (uint32_t iter = 0U; iter < numCores; iter++)
		{
			__secondaryCommandBufferManagers.emplace_back(
				std::make_unique<CommandBufferManager>(
					__device, __queueFamilyIndex,
					VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_SECONDARY));
		}
	}

	void ScreenResource::__reserveSwapchainImageDependencyPlaceholders(
		const SwapchainParam &swapchainParam) noexcept
	{
		const size_t numSwapchainImages{ swapchainParam.swapChainImages.size() };

		__primaryCommandBufferManagers.resize(numSwapchainImages);
		__swapChainImageViews.resize(numSwapchainImages);
	}

	void ScreenResource::__createRenderPasses(const SwapchainParam &swapchainParam)
	{
		std::vector<VkAttachmentDescription2> attachments;
		std::vector<VkSubpassDescription2> subpasses;
		std::vector<VkSubpassDependency2> dependencies;

		VkAttachmentDescription2 &colorAttachment{ attachments.emplace_back() };
		colorAttachment.sType = VkStructureType::VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
		colorAttachment.format = swapchainParam.swapchainFormat;
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

		__pRenderPass = std::make_unique<Vulkan::RenderPass>(__device, createInfo);
	}

	void ScreenResource::__createFramebuffer(const SwapchainParam &swapchainParam)
	{
		std::vector<VkFramebufferAttachmentImageInfo> attachmentImageInfos;

		VkFramebufferAttachmentImageInfo &colorAttachmentImageInfo{ attachmentImageInfos.emplace_back() };
		colorAttachmentImageInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO;
		colorAttachmentImageInfo.usage = VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		colorAttachmentImageInfo.width = swapchainParam.swapchainExtent.width;
		colorAttachmentImageInfo.height = swapchainParam.swapchainExtent.height;
		colorAttachmentImageInfo.layerCount = 1U;
		colorAttachmentImageInfo.viewFormatCount = 1U;
		colorAttachmentImageInfo.pViewFormats = &(swapchainParam.swapchainFormat);

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
			.width = swapchainParam.swapchainExtent.width,
			.height = swapchainParam.swapchainExtent.height,
			.layers = 1U
		};

		__pFramebuffer = std::make_unique<Vulkan::Framebuffer>(__device, createInfo);
	}

	void ScreenResource::__buildPipelines(const SwapchainParam &swapchainParam, tf::Subflow &subflow)
	{
		__pipelineBuildParam.renderPass = __pRenderPass->getHandle();
		__pipelineBuildParam.viewport =
		{
			.x = 0.0f,
			.y = 0.0f,
			.width = float(swapchainParam.swapchainExtent.width),
			.height = float(swapchainParam.swapchainExtent.height),
			.minDepth = 0.0f,
			.maxDepth = 1.0f
		};

		__pipelineBuildParam.scissor =
		{
			.offset = { 0, 0 },
			.extent = swapchainParam.swapchainExtent
		};

		__pipelineFactory.build(__pipelineBuildParam, subflow);
	}

	void ScreenResource::__createSwapchainImageView(
		const SwapchainParam &swapchainParam, const size_t imageIdx)
	{
		VkImageViewCreateInfo createInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = swapchainParam.swapChainImages[imageIdx],
			.viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D,
			.format = swapchainParam.swapchainFormat,
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

	void ScreenResource::__createPrimaryCommandBufferManager(const size_t imageIdx)
	{
		auto &pManager{ __primaryCommandBufferManagers[imageIdx] };
		if (pManager)
			return;

		pManager = std::make_unique<CommandBufferManager>(
			__device, __queueFamilyIndex, VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY);
	}

	void ScreenResource::__updateSecondaryCommandBuffers(tf::Subflow &subflow) noexcept
	{
		/*
			- Secondary command buffer manager 별도 관리
			- secondary command buffer 단위는 크게 잡아야 primary command buffer recoding overhead가 적음
			- DrawcallGroup 만들기 (mesh 단위로 그룹에 편재되도록, 그룹 크기는 hardware_concurrency 크기와 동일)
			- group resource update event 제공
			- renderpass, framebuffer, pipeline 등 업데이트 시 secondary command buffer 갱신
			- external param들은 명시적으로 setter 제공 (업데이트 타이밍 알아야됨)
		*/

		// 1. Secondary command buffer recoding
		// 2. Primary command buffer에서 사용
	}

	void ScreenResource::__updatePrimaryCommandBuffer(
		const SwapchainParam &swapchainParam,
		Drawcall *const pDrawcall, const size_t imageIdx) noexcept
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
				.extent = swapchainParam.swapchainExtent
			},
			.clearValueCount = 1U,
			.pClearValues = &clearColor
		};

		CommandBufferManager &commandBufferManager{ *(__primaryCommandBufferManagers[imageIdx]) };
		commandBufferManager.advance();

		Vulkan::CommandBuffer &commandBuffer{ commandBufferManager.get() };

		commandBuffer.vkBeginCommandBuffer(&commandBufferBeginInfo);
		commandBuffer.vkCmdBeginRenderPass(
			&renderPassBeginInfo, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);

		if (pDrawcall)
		{
			commandBuffer.vkCmdBindPipeline(
				VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, __pipelineFactory.get());

			const size_t numSegments{ pDrawcall->getNumSegments() };

			for (size_t segmentIter = 0ULL; segmentIter < numSegments; segmentIter++)
				pDrawcall->draw(segmentIter, commandBuffer);
		}

		commandBuffer.vkCmdEndRenderPass();
		commandBuffer.vkEndCommandBuffer();
	}

	void ScreenResource::__updateSwapchainDependencies(
		const SwapchainParam &swapchainParam, Drawcall *const pDrawcall)
	{
		tf::Taskflow taskflow;

		__pipelineFactory.reset();
		__pFramebuffer = nullptr;
		__pRenderPass = nullptr;
		__swapChainImageViews.clear();

		tf::Task t1
		{
			taskflow.emplace([this, &swapchainParam](tf::Subflow &subflow)
			{
				__reserveSwapchainImageDependencyPlaceholders(swapchainParam);

				tf::Task t1
				{
					subflow.emplace([this, &swapchainParam]
					{
						__createRenderPasses(swapchainParam);
					})
				};

				tf::Task t2
				{
					subflow.emplace([this, &swapchainParam]
					{
						__createFramebuffer(swapchainParam);
					})
				};
				t2.succeed(t1);

				tf::Task t3
				{
					subflow.emplace([this, &swapchainParam](tf::Subflow &subflow)
					{
						__buildPipelines(swapchainParam, subflow);
					})
				};
				t3.succeed(t1);
			})
		};

		tf::Task t2
		{
			taskflow.emplace([this, &swapchainParam, pDrawcall](tf::Subflow &subflow)
			{
				const size_t numSwapchainImages{ swapchainParam.swapChainImages.size() };
				for (size_t imageIter = 0ULL; imageIter < numSwapchainImages; imageIter++)
				{
					subflow.emplace([this, imageIter, &swapchainParam, pDrawcall]
					{
						__createPrimaryCommandBufferManager(imageIter);
						__createSwapchainImageView(swapchainParam, imageIter);
						__updatePrimaryCommandBuffer(swapchainParam, pDrawcall, imageIter);
					});
				}
			})
		};
		t2.succeed(t1);

		tf::Executor &executor{ Infra::Environment::getInstance().getTaskflowExecutor() };
		__job = executor.run(std::move(taskflow));

		__needToUpdateSwapchainDependencies = false;
		__needToUpdatePipelineDependencies = false;
		__needToPrimaryCommandBuffer = false;
	}

	void ScreenResource::__updatePipelineDependencies(const SwapchainParam &swapchainParam, Drawcall *const pDrawcall)
	{
		tf::Taskflow taskflow;
		
		taskflow.emplace([this, &swapchainParam, pDrawcall](tf::Subflow &subflow)
		{
			__pipelineFactory.reset();
			__buildPipelines(swapchainParam, subflow);

			const size_t numSwapchainImages{ swapchainParam.swapChainImages.size() };
			for (size_t imageIter = 0ULL; imageIter < numSwapchainImages; imageIter++)
			{
				subflow.emplace([this, imageIter, &swapchainParam, pDrawcall]
				{
					__updatePrimaryCommandBuffer(swapchainParam, pDrawcall, imageIter);
				});
			}
		});

		tf::Executor &executor{ Infra::Environment::getInstance().getTaskflowExecutor() };
		__job = executor.run(std::move(taskflow));

		__needToUpdatePipelineDependencies = false;
		__needToPrimaryCommandBuffer = false;
	}

	void ScreenResource::__updateCommandBuffers(
		const SwapchainParam &swapchainParam, Drawcall *const pDrawcall) noexcept
	{
		tf::Taskflow taskflow;
		
		taskflow.emplace([this, &swapchainParam, pDrawcall](tf::Subflow &subflow)
		{
			tf::Task t1
			{
				subflow.emplace([this](tf::Subflow &subflow)
				{
					__updateSecondaryCommandBuffers(subflow);
				})
			};

			const size_t numSwapchainImages{ swapchainParam.swapChainImages.size() };
			for (size_t imageIter = 0ULL; imageIter < numSwapchainImages; imageIter++)
			{
				subflow.emplace([this, imageIter, &swapchainParam, pDrawcall]
				{
					__updatePrimaryCommandBuffer(swapchainParam, pDrawcall, imageIter);
				}).succeed(t1);
			}
		});

		tf::Executor &executor{ Infra::Environment::getInstance().getTaskflowExecutor() };
		__job = executor.run(std::move(taskflow));

		__needToPrimaryCommandBuffer = false;
	}
}