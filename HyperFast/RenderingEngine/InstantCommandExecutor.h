#pragma once

#include "LifeCycle.h"
#include "CommandSubmitter.h"
#include "CommandBufferManager.h"

namespace HyperFast
{
	class InstantCommandExecutor : public Infra::Unique
	{
	public:
		InstantCommandExecutor(
			Vulkan::Device &device, const uint32_t queueFamilyIndex, LifeCycle &lifeCycle,
			CommandSubmitter &commandSubmitter, Infra::TemporalDeleter &resourceDeleter) noexcept;

		void execute() noexcept;

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
		CommandSubmitter &__commandSubmitter;
		std::unique_ptr<CommandBufferManager> __pCommandBufferManager;

		void __createCommandBufferManager(
			Vulkan::Device &device, const uint32_t queueFamilyIndex,
			Infra::TemporalDeleter &resourceDeleter) noexcept;

		void __begin() noexcept;
		void __end() noexcept;
	};
}
