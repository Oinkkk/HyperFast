#pragma once

#include "CommandSubmitter.h"
#include "CommandBufferManager.h"

namespace HyperFast
{
	class InstantCommandExecutor : public Infra::Unique
	{
	public:
		InstantCommandExecutor(
			Vulkan::Device &device, const uint32_t queueFamilyIndex,
			CommandSubmitter &commandSubmitter, Infra::TemporalDeleter &resourceDeleter) noexcept;

		void execute() noexcept;

		void vkCmdPipelineBarrier2(const VkDependencyInfo *const pDependencyInfo) noexcept;

		void vkCmdCopyBuffer(
			const VkBuffer srcBuffer, const VkBuffer dstBuffer,
			const uint32_t regionCount, const VkBufferCopy *const pRegions) noexcept;

	private:
		CommandSubmitter &__commandSubmitter;

		Vulkan::CommandBuffer *__pCommandBuffer{};
		std::unique_ptr<CommandBufferManager> __pCommandBufferManager;

		void __createCommandBufferManager(
			Vulkan::Device &device, const uint32_t queueFamilyIndex,
			Infra::TemporalDeleter &resourceDeleter) noexcept;

		void __begin() noexcept;
		void __end() noexcept;
		void __advance() noexcept;
	};
}
