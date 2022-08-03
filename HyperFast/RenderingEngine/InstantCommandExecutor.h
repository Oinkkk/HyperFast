#pragma once

#include "CommandSubmitter.h"
#include "BarrierSectionType.h"
#include "CommandBufferManager.h"
#include "../Infrastructure/Environment.h"
#include <functional>

namespace HyperFast
{
	class InstantCommandExecutor : public Infra::Unique
	{
	public:
		using CommandDelegate = std::function<void(Vulkan::CommandBuffer &)>;

		InstantCommandExecutor(
			Vulkan::Device &device, const uint32_t queueFamilyIndex,
			CommandSubmitter &commandSubmitter, Infra::TemporalDeleter &resourceDeleter) noexcept;

		void add(const BarrierSectionType sectionType, const CommandDelegate &commandDelegate) noexcept;
		void execute() noexcept;

	private:
		class BarrierFunction
		{
		public:
			CommandDelegate start;
			CommandDelegate end;
		};

		CommandSubmitter &__commandSubmitter;

		std::unique_ptr<CommandBufferManager> __pCommandBufferManager;

		std::unordered_map<BarrierSectionType, BarrierFunction> __barrierFunctionMap;
		std::unordered_map<BarrierSectionType, std::vector<CommandDelegate>> __commandReserved;
		std::list<std::pair<tf::Future<void>, Vulkan::CommandBuffer *>> __jobList;

		std::vector<VkCommandBufferSubmitInfo> __commandBufferInfos;

		void __createCommandBufferManager(
			Vulkan::Device &device, const uint32_t queueFamilyIndex,
			Infra::TemporalDeleter &resourceDeleter) noexcept;

		void __initBarrierFunctionMap() noexcept;

		bool __addJob() noexcept;
		void __enqueueFinishedCommandBuffers() noexcept;
	};
}
