#include "InstantCommandExecutor.h"

namespace HyperFast
{
	InstantCommandExecutor::InstantCommandExecutor(
		Vulkan::Device &device, const uint32_t queueFamilyIndex,
		CommandSubmitter &commandSubmitter, Infra::TemporalDeleter &resourceDeleter) noexcept :
		__commandSubmitter{ commandSubmitter }
	{
		__createCommandBufferManager(device, queueFamilyIndex, resourceDeleter);
		__initBarrierFunctionMap();
	}

	void InstantCommandExecutor::add(
		const BarrierSectionType sectionType, const CommandDelegate &commandDelegate) noexcept
	{
		__commandReserved[sectionType].emplace_back(commandDelegate);
	}

	void InstantCommandExecutor::execute() noexcept
	{
 		const bool executed{ __addJob() };
		if (executed)
			__pCommandBufferManager->advance();

		__enqueueFinishedCommandBuffers();
	}

	void InstantCommandExecutor::__createCommandBufferManager(
		Vulkan::Device &device, const uint32_t queueFamilyIndex,
		Infra::TemporalDeleter &resourceDeleter) noexcept
	{
		__pCommandBufferManager = std::make_unique<CommandBufferManager>(
			device, queueFamilyIndex,
			VkCommandBufferLevel::VK_COMMAND_BUFFER_LEVEL_PRIMARY, resourceDeleter);
	}

	void InstantCommandExecutor::__initBarrierFunctionMap() noexcept
	{
		BarrierFunction &updateIndirectCommandBarrierFunction
		{
			__barrierFunctionMap[BarrierSectionType::UPDATE_INDIRECT_COMMAND]
		};

		updateIndirectCommandBarrierFunction.start = [](Vulkan::CommandBuffer &commandBuffer)
		{
			const VkMemoryBarrier2 memoryBarrier
			{
				.sType = VkStructureType::VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
				.srcStageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT,
				.srcAccessMask = 0ULL,
				.dstStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
				.dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT
			};

			const VkDependencyInfo dependencyInfo
			{
				.sType = VkStructureType::VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
				.memoryBarrierCount = 1U,
				.pMemoryBarriers = &memoryBarrier
			};

			commandBuffer.vkCmdPipelineBarrier2(&dependencyInfo);
		};

		updateIndirectCommandBarrierFunction.end = [](Vulkan::CommandBuffer &commandBuffer)
		{
			const VkMemoryBarrier2 memoryBarrier
			{
				.sType = VkStructureType::VK_STRUCTURE_TYPE_MEMORY_BARRIER_2,
				.srcStageMask = VK_PIPELINE_STAGE_2_TRANSFER_BIT,
				.srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
				.dstStageMask = VK_PIPELINE_STAGE_2_DRAW_INDIRECT_BIT,
				.dstAccessMask = VK_ACCESS_2_INDIRECT_COMMAND_READ_BIT
			};

			const VkDependencyInfo dependencyInfo
			{
				.sType = VkStructureType::VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
				.memoryBarrierCount = 1U,
				.pMemoryBarriers = &memoryBarrier
			};

			commandBuffer.vkCmdPipelineBarrier2(&dependencyInfo);
		};
	}

	bool InstantCommandExecutor::__addJob() noexcept
	{
		if (__commandReserved.empty())
			return false;

		Vulkan::CommandBuffer *const pCommandBuffer{ &(__pCommandBufferManager->get()) };

		tf::Taskflow taskflow;
		taskflow.emplace([this, pCommandBuffer, commandReserved = std::move(__commandReserved)]
		{
			static constexpr VkCommandBufferBeginInfo beginInfo
			{
				.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
				.flags = VkCommandBufferUsageFlagBits::VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
			};

			pCommandBuffer->vkBeginCommandBuffer(&beginInfo);

			for (const auto &[barrierSectionType, commandDelegates] : commandReserved)
			{
				const auto &[barrierStart, barrierEnd] {__barrierFunctionMap[barrierSectionType] };

				barrierStart(*pCommandBuffer);

				for (const CommandDelegate &commandDelegate : commandDelegates)
					commandDelegate(*pCommandBuffer);

				barrierEnd(*pCommandBuffer);
			}

			pCommandBuffer->vkEndCommandBuffer();
		});

		tf::Executor &executor{ Infra::Environment::getInstance().getTaskflowExecutor() };
		__jobList.emplace_back(executor.run(std::move(taskflow)), pCommandBuffer);

		return true;
	}

	void InstantCommandExecutor::__enqueueFinishedCommandBuffers() noexcept
	{
		using namespace std;

		if (__jobList.empty())
			return;

		__commandBufferInfos.clear();

		for (auto jobIter = __jobList.begin(); jobIter != __jobList.end(); )
		{
			auto &[job, pCommandBuffer]{ *jobIter };

			if (job.wait_for(0s) != future_status::ready)
			{
				jobIter++;
				continue;
			}

			VkCommandBufferSubmitInfo &commandBufferInfo{ __commandBufferInfos.emplace_back() };
			commandBufferInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;
			commandBufferInfo.commandBuffer = pCommandBuffer->getHandle();

			jobIter = __jobList.erase(jobIter);
		}

		__commandSubmitter.enqueue(
			0U, nullptr,
			uint32_t(__commandBufferInfos.size()), __commandBufferInfos.data(),
			0U, nullptr);
	}
}