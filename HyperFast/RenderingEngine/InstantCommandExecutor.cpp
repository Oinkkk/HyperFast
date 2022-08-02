#include "InstantCommandExecutor.h"

namespace HyperFast
{
	InstantCommandExecutor::InstantCommandExecutor(
		Vulkan::Device &device, const uint32_t queueFamilyIndex, LifeCycle &lifeCycle,
		CommandSubmitter &commandSubmitter, Infra::TemporalDeleter &resourceDeleter) noexcept :
		__commandSubmitter{ commandSubmitter }
	{
		__createCommandBufferManager(device, queueFamilyIndex, resourceDeleter);
		__begin();
	}

	void InstantCommandExecutor::execute() noexcept
	{
		__end();
		// __commandSubmitter.enqueue();
	}

	void InstantCommandExecutor::vkCmdBeginRenderPass(
		const VkRenderPassBeginInfo *const pRenderPassBegin,
		const VkSubpassContents contents) noexcept
	{

	}

	void InstantCommandExecutor::vkCmdEndRenderPass() noexcept
	{

	}

	void InstantCommandExecutor::vkCmdBindPipeline(
		const VkPipelineBindPoint pipelineBindPoint,
		const VkPipeline pipeline) noexcept
	{

	}

	void InstantCommandExecutor::vkCmdBindVertexBuffers(
		const uint32_t firstBinding, const uint32_t bindingCount,
		const VkBuffer *const pBuffers, const VkDeviceSize *const pOffsets) noexcept
	{

	}

	void InstantCommandExecutor::vkCmdBindIndexBuffer(
		const VkBuffer buffer, const VkDeviceSize offset, const VkIndexType indexType) noexcept
	{

	}

	void InstantCommandExecutor::vkCmdDrawIndexedIndirectCount(
		const VkBuffer buffer, const VkDeviceSize offset,
		const VkBuffer countBuffer, const VkDeviceSize countBufferOffset,
		const uint32_t maxDrawCount, const uint32_t stride) noexcept
	{

	}

	void InstantCommandExecutor::vkCmdPipelineBarrier2(const VkDependencyInfo *const pDependencyInfo) noexcept
	{

	}

	void InstantCommandExecutor::vkCmdCopyBuffer(
		const VkBuffer srcBuffer, const VkBuffer dstBuffer,
		const uint32_t regionCount, const VkBufferCopy *const pRegions) noexcept
	{

	}

	void InstantCommandExecutor::vkCmdExecuteCommands(
		const uint32_t commandBufferCount, const VkCommandBuffer *const pCommandBuffers) noexcept
	{

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

	}

	void InstantCommandExecutor::__end() noexcept
	{

	}
}