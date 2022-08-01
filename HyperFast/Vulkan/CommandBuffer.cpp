#include "CommandBuffer.h"

namespace Vulkan
{
	CommandBuffer::CommandBuffer(Device &device, const VkCommandBuffer handle) noexcept :
		Handle{ handle }, __device{ device }
	{}

	VkResult CommandBuffer::vkBeginCommandBuffer(
		const VkCommandBufferBeginInfo *const pBeginInfo) noexcept
	{
		return __device.vkBeginCommandBuffer(getHandle(), pBeginInfo);
	}

	VkResult CommandBuffer::vkEndCommandBuffer() noexcept
	{
		return __device.vkEndCommandBuffer(getHandle());
	}

	VkResult CommandBuffer::vkResetCommandBuffer(
		const VkCommandBufferResetFlags flags) noexcept
	{
		return __device.vkResetCommandBuffer(getHandle(), flags);
	}

	void CommandBuffer::vkCmdBeginRenderPass(
		const VkRenderPassBeginInfo *const pRenderPassBegin,
		const VkSubpassContents contents) noexcept
	{
		__device.vkCmdBeginRenderPass(getHandle(), pRenderPassBegin, contents);
	}

	void CommandBuffer::vkCmdEndRenderPass() noexcept
	{
		__device.vkCmdEndRenderPass(getHandle());
	}

	void CommandBuffer::vkCmdBindPipeline(
		const VkPipelineBindPoint pipelineBindPoint,
		const VkPipeline pipeline) noexcept
	{
		__device.vkCmdBindPipeline(getHandle(), pipelineBindPoint, pipeline);
	}

	void CommandBuffer::vkCmdBindVertexBuffers(
		const uint32_t firstBinding, const uint32_t bindingCount,
		const VkBuffer *const pBuffers, const VkDeviceSize *const pOffsets) noexcept
	{
		__device.vkCmdBindVertexBuffers(
			getHandle(), firstBinding, bindingCount, pBuffers, pOffsets);
	}

	void CommandBuffer::vkCmdBindIndexBuffer(
		const VkBuffer buffer, const VkDeviceSize offset,
		const VkIndexType indexType) noexcept
	{
		__device.vkCmdBindIndexBuffer(getHandle(), buffer, offset, indexType);
	}

	void CommandBuffer::vkCmdDrawIndexedIndirectCount(
		const VkBuffer buffer, const VkDeviceSize offset,
		const VkBuffer countBuffer, const VkDeviceSize countBufferOffset,
		const uint32_t maxDrawCount, const uint32_t stride) noexcept
	{
		__device.vkCmdDrawIndexedIndirectCount(
			getHandle(), buffer, offset, countBuffer, countBufferOffset, maxDrawCount, stride);
	}

	void CommandBuffer::vkCmdPipelineBarrier2(
		const VkDependencyInfo *const pDependencyInfo) noexcept
	{
		__device.vkCmdPipelineBarrier2(getHandle(), pDependencyInfo);
	}

	void CommandBuffer::vkCmdCopyBuffer(
		const VkBuffer srcBuffer, const VkBuffer dstBuffer,
		const uint32_t regionCount, const VkBufferCopy *const pRegions) noexcept
	{
		__device.vkCmdCopyBuffer(
			getHandle(), srcBuffer, dstBuffer, regionCount, pRegions);
	}

	void CommandBuffer::vkCmdExecuteCommands(
		const uint32_t commandBufferCount, const VkCommandBuffer *const pCommandBuffers) noexcept
	{
		__device.vkCmdExecuteCommands(getHandle(), commandBufferCount, pCommandBuffers);
	}
}