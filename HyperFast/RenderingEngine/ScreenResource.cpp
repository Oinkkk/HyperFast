#include "ScreenResource.h"

namespace HyperFast
{
	ScreenResource::ScreenResource(
		Vulkan::Device &device, const ExternalParam &externalParam,
		const uint32_t queueFamilyIndex) noexcept :
		__device{ device }, __externalParam{ externalParam },
		__queueFamilyIndex{ queueFamilyIndex }, __pipelineFactory{ device }
	{}

	ScreenResource::~ScreenResource() noexcept
	{
		waitIdle();
		__pipelineFactory.reset();
		__pFramebuffer = nullptr;
		__pRenderPass = nullptr;
		__swapChainImageViews.clear();
		__renderCommandBufferManagers.clear();
	}

	bool ScreenResource::isIdle() noexcept
	{
		using namespace std;

		if (!(__job.valid()))
			return true;

		if (__job.wait_for(0s) == std::future_status::ready)
		{
			__job.get();
			return true;
		}

		return false;
	}

	void ScreenResource::waitIdle() noexcept
	{
		if (!(__job.valid()))
			return;

		__job.get();
	}

	void ScreenResource::updateSwapchainDependencies()
	{
		tf::Taskflow taskflow;

		__pipelineFactory.reset();
		__pFramebuffer = nullptr;
		__pRenderPass = nullptr;
		__swapChainImageViews.clear();

		tf::Task t1
		{
			taskflow.emplace([this](tf::Subflow &subflow)
			{
				__reserveSwapchainImageDependencyPlaceholers();

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
				const size_t numSwapchainImages{ __externalParam.swapChainImages.size() };
				for (
					size_t swapchainImageIter = 0ULL;
					swapchainImageIter < numSwapchainImages;
					swapchainImageIter++)
				{
					subflow.emplace([this, imageIdx = swapchainImageIter]
					{
						__createRenderCommandBufferManager(imageIdx);
						__createSwapchainImageView(imageIdx);
						__recordRenderCommand(imageIdx);
					});
				}
			})
		};
		t2.succeed(t1);

		tf::Executor &executor{ Infra::Environment::getInstance().getTaskflowExecutor() };
		__job = executor.run(taskflow);
	}

	void ScreenResource::updatePipelineDependencies()
	{
		tf::Taskflow taskflow;
		
		taskflow.emplace([this](tf::Subflow &subflow)
		{
			__pipelineFactory.reset();
			__buildPipelines(subflow);

			const size_t numSwapchainImages{ __externalParam.swapChainImages.size() };
			for (
				size_t swapchainImageIter = 0ULL;
				swapchainImageIter < numSwapchainImages;
				swapchainImageIter++)
			{
				subflow.emplace([this, imageIdx = swapchainImageIter]
				{
					__recordRenderCommand(imageIdx);
				});
			}
		});

		tf::Executor &executor{ Infra::Environment::getInstance().getTaskflowExecutor() };
		__job = executor.run(taskflow);
	}

	void ScreenResource::updateMainCommands() noexcept
	{
		tf::Taskflow taskflow;
		
		taskflow.emplace([this](tf::Subflow &subflow)
		{
			const size_t numSwapchainImages{ __externalParam.swapChainImages.size() };
			for (
				size_t swapchainImageIter = 0ULL;
				swapchainImageIter < numSwapchainImages;
				swapchainImageIter++)
			{
				subflow.emplace([this, imageIdx = swapchainImageIter]
				{
					__recordRenderCommand(imageIdx);
				});
			}
		});

		tf::Executor &executor{ Infra::Environment::getInstance().getTaskflowExecutor() };
		__job = executor.run(taskflow);
	}

	void ScreenResource::__reserveSwapchainImageDependencyPlaceholers() noexcept
	{
		const size_t numSwapchainImages{ __externalParam.swapChainImages.size() };

		__renderCommandBufferManagers.resize(numSwapchainImages);
		__renderCommandBuffers.resize(numSwapchainImages);
		__swapChainImageViews.resize(numSwapchainImages);
	}

	void ScreenResource::__createRenderPasses()
	{
		std::vector<VkAttachmentDescription2> attachments;
		std::vector<VkSubpassDescription2> subpasses;
		std::vector<VkSubpassDependency2> dependencies;

		VkAttachmentDescription2 &colorAttachment{ attachments.emplace_back() };
		colorAttachment.sType = VkStructureType::VK_STRUCTURE_TYPE_ATTACHMENT_DESCRIPTION_2;
		colorAttachment.format = __externalParam.swapchainFormat;
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

			// �Ϲ��� �޸� �������
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

	void ScreenResource::__createFramebuffer()
	{
		std::vector<VkFramebufferAttachmentImageInfo> attachmentImageInfos;

		VkFramebufferAttachmentImageInfo &colorAttachmentImageInfo{ attachmentImageInfos.emplace_back() };
		colorAttachmentImageInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_FRAMEBUFFER_ATTACHMENT_IMAGE_INFO;
		colorAttachmentImageInfo.usage = VkImageUsageFlagBits::VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		colorAttachmentImageInfo.width = __externalParam.swapchainExtent.width;
		colorAttachmentImageInfo.height = __externalParam.swapchainExtent.height;
		colorAttachmentImageInfo.layerCount = 1U;
		colorAttachmentImageInfo.viewFormatCount = 1U;
		colorAttachmentImageInfo.pViewFormats = &(__externalParam.swapchainFormat);

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
			.width = __externalParam.swapchainExtent.width,
			.height = __externalParam.swapchainExtent.height,
			.layers = 1U
		};

		__pFramebuffer = std::make_unique<Vulkan::Framebuffer>(__device, createInfo);
	}

	void ScreenResource::__buildPipelines(tf::Subflow &subflow)
	{
		if (!(__externalParam.pDrawcall))
			return;

		PipelineFactory::BuildParam buildParam;

		buildParam.renderPass = __pRenderPass->getHandle();
		buildParam.viewport =
		{
			.x = 0.0f,
			.y = 0.0f,
			.width = float(__externalParam.swapchainExtent.width),
			.height = float(__externalParam.swapchainExtent.height),
			.minDepth = 0.0f,
			.maxDepth = 1.0f
		};

		buildParam.scissor =
		{
			.offset = { 0, 0 },
			.extent = __externalParam.swapchainExtent
		};

		__pipelineFactory.build(
			__externalParam.pDrawcall->getAttributeFlags(), buildParam, subflow);
	}

	void ScreenResource::__createSwapchainImageView(const size_t imageIdx)
	{
		VkImageViewCreateInfo createInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			.image = __externalParam.swapChainImages[imageIdx],
			.viewType = VkImageViewType::VK_IMAGE_VIEW_TYPE_2D,
			.format = __externalParam.swapchainFormat,
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

	void ScreenResource::__createRenderCommandBufferManager(const size_t imageIdx)
	{
		auto &pManager{ __renderCommandBufferManagers[imageIdx] };
		if (pManager)
			return;

		pManager = std::make_unique<CommandBufferManager>(__device, __queueFamilyIndex, 8ULL);
	}

	void ScreenResource::__recordRenderCommand(const size_t imageIdx) noexcept
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
			.renderPass = __pRenderPass->getHandle(),
			.framebuffer = __pFramebuffer->getHandle(),
			.renderArea =
			{
				.offset = { 0, 0 },
				.extent = __externalParam.swapchainExtent
			},
			.clearValueCount = 1U,
			.pClearValues = &clearColor
		};

		Vulkan::CommandBuffer &commandBuffer{ __renderCommandBufferManagers[imageIdx]->getNextBuffer() };
		__renderCommandBuffers[imageIdx] = &commandBuffer;

		commandBuffer.vkBeginCommandBuffer(&commandBufferBeginInfo);
		commandBuffer.vkCmdBeginRenderPass(
			&renderPassBeginInfo, VkSubpassContents::VK_SUBPASS_CONTENTS_INLINE);

		if (__externalParam.pDrawcall)
		{
			for (const VertexAttributeFlag attribFlag : __externalParam.pDrawcall->getAttributeFlags())
			{
				const VkPipeline pipeline{ __pipelineFactory.get(attribFlag) };

				commandBuffer.vkCmdBindPipeline(
					VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);

				__externalParam.pDrawcall->render(attribFlag, commandBuffer);
			}
		}

		commandBuffer.vkCmdEndRenderPass();
		commandBuffer.vkEndCommandBuffer();
	}
}