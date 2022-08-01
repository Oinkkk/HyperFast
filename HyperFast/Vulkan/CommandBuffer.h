#pragma once

#include "Device.h"

namespace Vulkan
{
	class CommandBuffer final : public Handle<VkCommandBuffer>
	{
	public:
		CommandBuffer(Device &device, const VkCommandBuffer handle) noexcept;
		virtual ~CommandBuffer() noexcept = default;

		VkResult vkBeginCommandBuffer(const VkCommandBufferBeginInfo *const pBeginInfo) noexcept;
		VkResult vkEndCommandBuffer() noexcept;

		VkResult vkResetCommandBuffer(const VkCommandBufferResetFlags flags) noexcept;

		void vkCmdBeginRenderPass(
			const VkRenderPassBeginInfo *const pRenderPassBegin,
			const VkSubpassContents contents) noexcept;

		void vkCmdEndRenderPass() noexcept;

		void vkCmdBindPipeline(
			const VkPipelineBindPoint pipelineBindPoint,
			const VkPipeline pipeline) noexcept;

		void vkCmdBindVertexBuffers(
			const uint32_t firstBinding, const uint32_t bindingCount,
			const VkBuffer *const pBuffers, const VkDeviceSize *const pOffsets) noexcept;

		void vkCmdBindIndexBuffer(
			const VkBuffer buffer, const VkDeviceSize offset, const VkIndexType indexType) noexcept;

		void vkCmdDrawIndexedIndirectCount(
			const VkBuffer buffer, const VkDeviceSize offset,
			const VkBuffer countBuffer, const VkDeviceSize countBufferOffset,
			const uint32_t maxDrawCount, const uint32_t stride) noexcept;

		void vkCmdPipelineBarrier2(const VkDependencyInfo *const pDependencyInfo) noexcept;

		void vkCmdCopyBuffer(
			const VkBuffer srcBuffer, const VkBuffer dstBuffer,
			const uint32_t regionCount, const VkBufferCopy *const pRegions) noexcept;

		void vkCmdExecuteCommands(
			const uint32_t commandBufferCount, const VkCommandBuffer *const pCommandBuffers) noexcept;

	private:
		Device &__device;
	};
}