#pragma once

#include <memory>
#include <vector>
#include "../Vulkan/Surface.h"
#include "../Vulkan/Swapchain.h"
#include "../Vulkan/ImageView.h"
#include "../Vulkan/RenderPass.h"
#include "../Vulkan/Framebuffer.h"
#include "PipelineFactory.h"
#include "CommandBufferManager.h"
#include "Drawcall.h"
#include "../Infrastructure/Environment.h"

namespace HyperFast
{
	class ScreenResource final : public Infra::Unique
	{
	public:
		class ExternalParam
		{
		public:
			VkFormat swapchainFormat{};
			VkExtent2D swapchainExtent{};
			std::vector<VkImage> swapChainImages;
			Drawcall *pDrawcall{};
		};

		ScreenResource(
			Vulkan::Device &device, const ExternalParam &externalParam,
			const uint32_t queueFamilyIndex) noexcept;
		
		~ScreenResource() noexcept;

		[[nodiscard]]
		bool isIdle() noexcept;
		void waitIdle() noexcept;

		[[nodiscard]]
		constexpr Vulkan::CommandBuffer &getRenderCommandBuffer(const size_t imageIdx) noexcept;

		constexpr void needToUpdateSwapchainDependencies() noexcept;
		constexpr void needToUpdatePipelineDependencies() noexcept;
		constexpr void needToUpdateMainCommands() noexcept;

		void update();

	private:
		Vulkan::Device &__device;
		const ExternalParam &__externalParam;
		const uint32_t __queueFamilyIndex;

		PipelineFactory::BuildParam __pipelineBuildParam;
		PipelineFactory __pipelineFactory;

		std::vector<std::unique_ptr<Vulkan::ImageView>> __swapChainImageViews;
		std::unique_ptr<Vulkan::RenderPass> __pRenderPass;
		std::unique_ptr<Vulkan::Framebuffer> __pFramebuffer;

		std::vector<std::unique_ptr<CommandBufferManager>> __renderCommandBufferManagers;
		std::vector<Vulkan::CommandBuffer *> __renderCommandBuffers;

		bool __needToUpdateSwapchainDependencies{};
		bool __needToUpdatePipelineDependencies{};
		bool __needToUpdateMainCommands{};

		tf::Future<void> __job;

		void __reserveSwapchainImageDependencyPlaceholers() noexcept;
		void __createRenderPasses();
		void __createFramebuffer();

		void __buildPipelines(tf::Subflow &subflow);
		void __createSwapchainImageView(const size_t imageIdx);
		void __createRenderCommandBufferManager(const size_t imageIdx);
		void __recordRenderCommand(const size_t imageIdx) noexcept;

		void __updateSwapchainDependencies();
		void __updatePipelineDependencies();
		void __updateMainCommands() noexcept;
	};

	constexpr Vulkan::CommandBuffer &ScreenResource::getRenderCommandBuffer(const size_t imageIdx) noexcept
	{
		return *__renderCommandBuffers[imageIdx];
	}

	constexpr void ScreenResource::needToUpdateSwapchainDependencies() noexcept
	{
		__needToUpdateSwapchainDependencies = true;
	}

	constexpr void ScreenResource::needToUpdatePipelineDependencies() noexcept
	{
		__needToUpdatePipelineDependencies = true;
	}

	constexpr void ScreenResource::needToUpdateMainCommands() noexcept
	{
		__needToUpdateMainCommands = true;
	}
}