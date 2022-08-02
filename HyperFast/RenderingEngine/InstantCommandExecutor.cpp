#include "InstantCommandExecutor.h"

namespace HyperFast
{
	InstantCommandExecutor::InstantCommandExecutor(
		Vulkan::Device &device, const uint32_t queueFamilyIndex,
		CommandSubmitter &commandSubmitter, Infra::TemporalDeleter &resourceDeleter) noexcept :
		__commandSubmitter{ commandSubmitter }
	{
		__createCommandBufferManager(device, queueFamilyIndex, resourceDeleter);
		__begin();
	}

	void InstantCommandExecutor::execute() noexcept
	{
		const VkCommandBufferSubmitInfo commandBufferInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
			.commandBuffer = __pCommandBuffer->getHandle()
		};

		__end();
		__commandSubmitter.enqueue(0U, nullptr, 1U, &commandBufferInfo, 0U, nullptr);
		__advance();
		__begin();
	}

	void InstantCommandExecutor::vkCmdPipelineBarrier2(const VkDependencyInfo *const pDependencyInfo) noexcept
	{
		__pCommandBuffer->vkCmdPipelineBarrier2(pDependencyInfo);
	}

	void InstantCommandExecutor::vkCmdCopyBuffer(
		const VkBuffer srcBuffer, const VkBuffer dstBuffer,
		const uint32_t regionCount, const VkBufferCopy *const pRegions) noexcept
	{
		__pCommandBuffer->vkCmdCopyBuffer(srcBuffer, dstBuffer, regionCount, pRegions);
	}

	void InstantCommandExecutor::__createCommandBufferManager(
		Vulkan::Device &device, const uint32_t queueFamilyIndex,
		Infra::TemporalDeleter &resourceDeleter) noexcept
	{
		__pCommandBufferManager = std::make_unique<CommandBufferManager>(
			device, queueFamilyIndex,
			VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY, resourceDeleter);
	}

	void InstantCommandExecutor::__begin() noexcept
	{
		static constexpr VkCommandBufferBeginInfo beginInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			.flags = VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
		};

		__pCommandBuffer = &(__pCommandBufferManager->get());
		__pCommandBuffer->vkBeginCommandBuffer(&beginInfo);
	}

	void InstantCommandExecutor::__end() noexcept
	{
		__pCommandBuffer->vkEndCommandBuffer();
	}

	void InstantCommandExecutor::__advance() noexcept
	{
		__pCommandBufferManager->advance();
	}
}