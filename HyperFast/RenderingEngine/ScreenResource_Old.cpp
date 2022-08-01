#include "ScreenResource_Old.h"

namespace HyperFast
{
	ScreenResource::ScreenResource(Vulkan::Device &device, const uint32_t queueFamilyIndex) noexcept :
		__device{ device }, __queueFamilyIndex{ queueFamilyIndex }, __pipelineFactory{ device }
	{
		__initSecondaryCommandBufferBeginInfos();
	}

	ScreenResource::~ScreenResource() noexcept
	{
		waitIdle();
	}

	Vulkan::CommandBuffer &ScreenResource::getPrimaryCommandBuffer(const size_t imageIdx) noexcept
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
			return (__job.wait_for(0s) == std::future_status::ready);

		return true;
	}

	void ScreenResource::waitIdle() noexcept
	{
		if (!(__job.valid()))
			return;

		__job.wait();
	}

	void ScreenResource::needToUpdateSecondaryCommandBuffer(const size_t drawcallSegmentIndex) noexcept
	{
		__updateNeededDrawcallSegmentIndices.emplace(drawcallSegmentIndex);
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

	void ScreenResource::__updateSecondaryCommandBuffers(
		Drawcall *const pDrawcall, const size_t imageIdx, tf::Subflow &subflow) noexcept
	{
		if (__updateNeededDrawcallSegmentIndices.empty())
			return;

		__secondaryCommandBufferInheritanceInfo.renderPass = __pRenderPass->getHandle();
		__secondaryCommandBufferInheritanceInfo.framebuffer = __pFramebuffer->getHandle();

		for (const size_t drawcallSegmentIdx : __updateNeededDrawcallSegmentIndices)
		{
			subflow.emplace([this, pDrawcall, imageIdx, drawcallSegmentIdx](tf::Subflow &subflow)
			{
				Vulkan::CommandBuffer &commandBuffer{ __nextSecondaryCommandBuffer(imageIdx, drawcallSegmentIdx) };
				commandBuffer.vkBeginCommandBuffer(&__secondaryCommandBufferBeginInfo);

				if (pDrawcall)
				{
					commandBuffer.vkCmdBindPipeline(
						VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, __pipelineFactory.get());

					pDrawcall->draw(drawcallSegmentIdx, commandBuffer);
				}

				commandBuffer.vkEndCommandBuffer();
			});
		}
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

		Vulkan::CommandBuffer &commandBuffer{ __nextPrimaryCommandBuffer(imageIdx) };
		const std::vector<VkCommandBuffer> &secondaryCommandBufferHandles{ __getSecondaryCommandBufferHandles(imageIdx) };

		commandBuffer.vkBeginCommandBuffer(&commandBufferBeginInfo);
		commandBuffer.vkCmdBeginRenderPass(
			&renderPassBeginInfo, VkSubpassContents::VK_SUBPASS_CONTENTS_SECONDARY_COMMAND_BUFFERS);

		commandBuffer.vkCmdExecuteCommands(
			uint32_t(secondaryCommandBufferHandles.size()),
			secondaryCommandBufferHandles.data());

		commandBuffer.vkCmdEndRenderPass();
		commandBuffer.vkEndCommandBuffer();
	}

	void ScreenResource::__updateSwapchainDependencies(
		const SwapchainParam &swapchainParam, Drawcall *const pDrawcall)
	{
		const size_t numSwapchainImages{ swapchainParam.swapChainImages.size() };

		tf::Taskflow taskflow;
		tf::Task t1
		{
			taskflow.emplace([this, &swapchainParam, numSwapchainImages](tf::Subflow &subflow)
			{
				__swapChainImageViews.clear();
				__secondaryCommandBufferResources.resize(numSwapchainImages);
				__primaryCommandBufferManagers.resize(numSwapchainImages);
				__swapChainImageViews.resize(numSwapchainImages);

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
			taskflow.emplace([this, &swapchainParam, numSwapchainImages, pDrawcall](tf::Subflow &subflow)
			{
				for (size_t imageIter = 0ULL; imageIter < numSwapchainImages; imageIter++)
				{
					tf::Task t1
					{
						subflow.emplace([this, pDrawcall, imageIter](tf::Subflow &subflow)
						{
							__updateSecondaryCommandBuffers(pDrawcall, imageIter, subflow);
						})
					};

					subflow.emplace([this, &swapchainParam, pDrawcall, imageIter](tf::Subflow &subflow)
					{
						__createSwapchainImageView(swapchainParam, imageIter);
						__updateSecondaryCommandBufferHandles(imageIter);
						__updatePrimaryCommandBuffer(swapchainParam, pDrawcall, imageIter);
					}).succeed(t1);
				}
			})
		};
		t2.succeed(t1);

		taskflow.emplace([this]
		{
			__updateNeededDrawcallSegmentIndices.clear();
		}).succeed(t2);

		tf::Executor &executor{ Infra::Environment::getInstance().getTaskflowExecutor() };
		__job = executor.run(std::move(taskflow));

		__needToUpdateSwapchainDependencies = false;
		__needToUpdatePipelineDependencies = false;
		__needToPrimaryCommandBuffer = false;
	}

	void ScreenResource::__updatePipelineDependencies(const SwapchainParam &swapchainParam, Drawcall *const pDrawcall)
	{
		const size_t numSwapchainImages{ swapchainParam.swapChainImages.size() };

		tf::Taskflow taskflow;
		tf::Task t1
		{
			taskflow.emplace([this, &swapchainParam, numSwapchainImages, pDrawcall](tf::Subflow &subflow)
			{
				__buildPipelines(swapchainParam, subflow);

				for (size_t imageIter = 0ULL; imageIter < numSwapchainImages; imageIter++)
				{
					tf::Task t1
					{
						subflow.emplace([this, pDrawcall, imageIter](tf::Subflow &subflow)
						{
							__updateSecondaryCommandBuffers(pDrawcall, imageIter, subflow);
						})
					};

					subflow.emplace([this, &swapchainParam, pDrawcall, imageIter](tf::Subflow &subflow)
					{
						__updateSecondaryCommandBufferHandles(imageIter);
						__updatePrimaryCommandBuffer(swapchainParam, pDrawcall, imageIter);
					}).succeed(t1);
				}
			})
		};

		taskflow.emplace([this]
		{
			__updateNeededDrawcallSegmentIndices.clear();
		}).succeed(t1);

		tf::Executor &executor{ Infra::Environment::getInstance().getTaskflowExecutor() };
		__job = executor.run(std::move(taskflow));

		__needToUpdatePipelineDependencies = false;
		__needToPrimaryCommandBuffer = false;
	}

	void ScreenResource::__updateCommandBuffers(
		const SwapchainParam &swapchainParam, Drawcall *const pDrawcall) noexcept
	{
		const size_t numSwapchainImages{ swapchainParam.swapChainImages.size() };

		tf::Taskflow taskflow;
		tf::Task t1
		{
			taskflow.emplace([this, &swapchainParam, numSwapchainImages, pDrawcall](tf::Subflow &subflow)
			{
				for (size_t imageIter = 0ULL; imageIter < numSwapchainImages; imageIter++)
				{
					tf::Task t1
					{
						subflow.emplace([this, pDrawcall, imageIter](tf::Subflow &subflow)
						{
							__updateSecondaryCommandBuffers(pDrawcall, imageIter, subflow);
						})
					};

					subflow.emplace([this, &swapchainParam, pDrawcall, imageIter](tf::Subflow &subflow)
					{
						__updateSecondaryCommandBufferHandles(imageIter);
						__updatePrimaryCommandBuffer(swapchainParam, pDrawcall, imageIter);
					}).succeed(t1);
				}
			})
		};

		taskflow.emplace([this]
		{
			__updateNeededDrawcallSegmentIndices.clear();
		}).succeed(t1);

		tf::Executor &executor{ Infra::Environment::getInstance().getTaskflowExecutor() };
		__job = executor.run(std::move(taskflow));

		__needToPrimaryCommandBuffer = false;
	}

	Vulkan::CommandBuffer &ScreenResource::__nextPrimaryCommandBuffer(const size_t imageIdx) noexcept
	{
		auto &pCommandBufferManager{ __primaryCommandBufferManagers[imageIdx] };
		if (!pCommandBufferManager)
		{
			pCommandBufferManager = std::make_unique<CommandBufferManager>(
				__device, __queueFamilyIndex, VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY);
		}
		else
			pCommandBufferManager->advance();

		return pCommandBufferManager->get();
	}

	ScreenResource::SecondaryCommandBufferResource &
		ScreenResource::__getSecondaryCommandBufferResource(const size_t imageIdx) noexcept
	{
		auto &pResource{ __secondaryCommandBufferResources[imageIdx] };
		if (!pResource)
			pResource = std::make_unique<SecondaryCommandBufferResource>();

		return *pResource;
	}

	[[nodiscard]]
	Vulkan::CommandBuffer &
		ScreenResource::__nextSecondaryCommandBuffer(
		const size_t imageIdx, const size_t drawcallSegmentIdx) noexcept
	{
		SecondaryCommandBufferResource &resource{ __getSecondaryCommandBufferResource(imageIdx) };
		auto &pCommandBufferManager{ resource.managerMap[drawcallSegmentIdx] };
		if (!pCommandBufferManager)
		{
			pCommandBufferManager =
				std::make_unique<CommandBufferManager>(
					__device, __queueFamilyIndex,
					VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_SECONDARY);
		}
		else
			pCommandBufferManager->advance();

		return pCommandBufferManager->get();
	}

	void ScreenResource::__updateSecondaryCommandBufferHandles(const size_t imageIdx) noexcept
	{
		SecondaryCommandBufferResource &resource{ __getSecondaryCommandBufferResource(imageIdx) };
		resource.handles.clear();

		for (const auto &[_, pManager] : resource.managerMap)
		{
			const VkCommandBuffer handle{ pManager->get().getHandle() };
			resource.handles.emplace_back(handle);
		}
	}

	const std::vector<VkCommandBuffer> &
		ScreenResource::__getSecondaryCommandBufferHandles(const size_t imageIdx) noexcept
	{
		SecondaryCommandBufferResource &resource{ __getSecondaryCommandBufferResource(imageIdx) };
		return resource.handles;
	}
}