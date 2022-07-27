#pragma once

#include "../Vulkan/Surface.h"
#include "../Vulkan/Swapchain.h"
#include "../Vulkan/ImageView.h"
#include "../Vulkan/RenderPass.h"
#include "../Vulkan/Framebuffer.h"
#include "PipelineFactory.h"
#include "CommandBufferManager.h"
#include "Drawcall.h"
#include "../Infrastructure/Environment.h"
#include "SemaphoreDependencyCluster.h"

namespace HyperFast
{
	class ScreenResource final : public Infra::Unique
	{
	public:
		class SwapchainParam
		{
		public:
			VkFormat swapchainFormat{};
			VkExtent2D swapchainExtent{};
			std::vector<VkImage> swapChainImages;
		};

		ScreenResource(Vulkan::Device &device, const uint32_t queueFamilyIndex) noexcept;
		~ScreenResource() noexcept;

		[[nodiscard]]
		Vulkan::CommandBuffer &getPrimaryCommandBuffer(const size_t imageIdx) noexcept;

		void addSemaphoreDependency(const std::shared_ptr<SemaphoreDependency> &pDependency) noexcept;

		[[nodiscard]]
		bool isIdle() noexcept;
		void waitIdle() noexcept;

		constexpr void needToUpdateSwapchainDependencies() noexcept;
		constexpr void needToUpdatePipelineDependencies() noexcept;
		constexpr void needToUpdatePrimaryCommandBuffer() noexcept;
		void needToUpdateSecondaryCommandBuffer(const size_t commandBufferIndex) noexcept;

		void update(const SwapchainParam &swapchainParam, Drawcall *const pDrawcall);

	private:
		Vulkan::Device &__device;
		const uint32_t __queueFamilyIndex;

		PipelineFactory::BuildParam __pipelineBuildParam;
		PipelineFactory __pipelineFactory;

		std::vector<std::unique_ptr<Vulkan::ImageView>> __swapChainImageViews;
		std::unique_ptr<Vulkan::RenderPass> __pRenderPass;
		std::unique_ptr<Vulkan::Framebuffer> __pFramebuffer;

		std::vector<std::unique_ptr<CommandBufferManager>> __primaryCommandBufferManagers;
		std::vector<std::unique_ptr<CommandBufferManager>> __secondaryCommandBufferManagers;

		bool __needToUpdateSwapchainDependencies{};
		bool __needToUpdatePipelineDependencies{};
		bool __needToPrimaryCommandBuffer{};
		std::unordered_set<size_t> __updateNeededSecondaryCommandBufferIndices;

		tf::Future<void> __job;
		SemaphoreDependencyCluster __semaphoreDependencyCluster;

		void __createSecondaryCommandBuffers() noexcept;

		void __reserveSwapchainImageDependencyPlaceholders(const SwapchainParam &swapchainParam) noexcept;
		void __createRenderPasses(const SwapchainParam &swapchainParam);
		void __createFramebuffer(const SwapchainParam &swapchainParam);

		void __buildPipelines(const SwapchainParam &swapchainParam, tf::Subflow &subflow);
		void __createSwapchainImageView(const SwapchainParam &swapchainParam, const size_t imageIdx);
		void __createPrimaryCommandBufferManager(const size_t imageIdx);
		void __updateSecondaryCommandBuffers(tf::Subflow &subflow) noexcept;
		void __updatePrimaryCommandBuffer(
			const SwapchainParam &swapchainParam,
			Drawcall *const pDrawcall, const size_t imageIdx) noexcept;

		void __updateSwapchainDependencies(const SwapchainParam &swapchainParam, Drawcall *const pDrawcall);
		void __updatePipelineDependencies(const SwapchainParam &swapchainParam, Drawcall *const pDrawcall);
		void __updateCommandBuffers(const SwapchainParam &swapchainParam, Drawcall *const pDrawcall) noexcept;
	};

	constexpr void ScreenResource::needToUpdateSwapchainDependencies() noexcept
	{
		__needToUpdateSwapchainDependencies = true;
	}

	constexpr void ScreenResource::needToUpdatePipelineDependencies() noexcept
	{
		__needToUpdatePipelineDependencies = true;
	}

	constexpr void ScreenResource::needToUpdatePrimaryCommandBuffer() noexcept
	{
		__needToPrimaryCommandBuffer = true;
	}
}