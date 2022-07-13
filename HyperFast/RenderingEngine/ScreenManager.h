#pragma once

#include "../Window/Window.h"
#include "PipelineFactory.h"
#include "../Infrastructure/Logger.h"
#include "CommandBufferManager.h"
#include "../Infrastructure/Environment.h"
#include "Drawcall.h"
#include "../Vulkan/Queue.h"
#include "../Vulkan/Swapchain.h"
#include "../Vulkan/ImageView.h"
#include "../Vulkan/RenderPass.h"
#include "../Vulkan/Framebuffer.h"
#include "../Vulkan/Semaphore.h"
#include "../Vulkan/Fence.h"
#include "../Vulkan/Surface.h"

namespace HyperFast
{
	class RenderingEngine;

	class ScreenManager final : public Infra::Unique
	{
	public:
		class ScreenImpl final : public Infra::Unique
		{
		public:
			ScreenImpl(
				RenderingEngine &renderingEngine,
				Vulkan::Instance &instance, Vulkan::PhysicalDevice &physicalDevice,
				const uint32_t graphicsQueueFamilyIndex, Vulkan::Device &device,
				Vulkan::Queue &queue, Win::Window &window);

			~ScreenImpl() noexcept;

			void setDrawcall(Drawcall *const pDrawcall) noexcept;

			void render();
			void present() noexcept;

		private:
			RenderingEngine &__renderingEngine;
			Vulkan::Instance &__instance;
			Vulkan::PhysicalDevice &__physicalDevice;
			Vulkan::Device &__device;

			const uint32_t __graphicsQueueFamilyIndex;
			Vulkan::Queue &__queue;

			Win::Window &__window;
			std::shared_ptr<Infra::EventListener<Win::Window &, Win::Window::ResizingType>> __pResizeEventListener;
			std::shared_ptr<Infra::EventListener<Win::Window &>> __pDrawEventListener;
			std::shared_ptr<Infra::EventListener<Win::Window &>> __pDestroyEventListener;

			Drawcall *__pDrawcall{};
			std::shared_ptr<Infra::EventListener<Drawcall &>> __pAttribFlagsUpdateEventListener;
			std::shared_ptr<Infra::EventListener<Drawcall &>> __pIndirectBufferUpdateListener;
			std::shared_ptr<Infra::EventListener<Drawcall &>> __pIndirectBufferCreateListener;

			PipelineFactory::BuildParam __pipelineBuildParam;
			PipelineFactory __pipelineFactory;

			std::unique_ptr<Vulkan::Surface> __pSurface;
			std::vector<std::unique_ptr<CommandBufferManager>> __renderCommandBufferManagers;
			std::vector<Vulkan::CommandBuffer *> __renderCommandBuffers;

			VkSurfaceCapabilitiesKHR __surfaceCapabilities{};
			std::vector<VkSurfaceFormatKHR> __supportedSurfaceFormats;
			std::vector<VkPresentModeKHR> __supportedSurfacePresentModes;

			std::unique_ptr<Vulkan::Swapchain> __pSwapchain;
			VkFormat __swapchainFormat{};
			VkExtent2D __swapchainExtent{};

			std::vector<VkImage> __swapChainImages;
			std::vector<std::unique_ptr<Vulkan::ImageView>> __swapChainImageViews;

			std::unique_ptr<Vulkan::RenderPass> __pRenderPass;
			std::unique_ptr<Vulkan::Framebuffer> __pFramebuffer;

			VkSemaphoreSubmitInfo __submitWaitInfo{};
			VkCommandBufferSubmitInfo __submitCommandBufferInfo{};
			VkSemaphoreSubmitInfo __submitSignalInfos[2]{};

			std::vector<std::unique_ptr<Vulkan::Semaphore>> __imageAcquireSemaphores;
			std::vector<std::unique_ptr<Vulkan::Semaphore>> __attachmentOuputSemaphores;
			std::vector<std::unique_ptr<Vulkan::Semaphore>> __renderCompletionSemaphores;
			std::vector<uint64_t> __renderCompletionSemaphoreValues;

			size_t __frameCursor{};
			bool __imageAcquired{};
			uint32_t __imageIdx{};

			bool __needToRender{};
			bool __needToPresent{};

			bool __needToUpdateSurfaceDependencies{};
			bool __needToUpdatePipelineDependencies{};
			bool __needToUpdateMainCommands{};
			bool __destroyed{};

			void __update();
			void __render() noexcept;
			void __present() noexcept;

			void __destroy() noexcept;

			void __initListeners() noexcept;
			void __createSurface();
			constexpr void __initSubmitInfo() noexcept;

			void __updateSurfaceDependencies();
			void __updatePipelineDependencies();
			void __updateMainCommands() noexcept;

			void __checkSurfaceSupport() const;
			void __querySurfaceCapabilities() noexcept;
			void __querySupportedSurfaceFormats() noexcept;
			void __querySupportedSurfacePresentModes() noexcept;
			void __createSwapchain(Vulkan::Swapchain *const pOldSwapchain);
			void __retrieveSwapchainImages() noexcept;
			void __reserveSwapchainImageDependencyPlaceholers() noexcept;
			void __createRenderCommandBufferManager(const size_t imageIdx);
			void __createSwapchainImageView(const size_t imageIdx);
			void __createRenderPasses();
			void __createFramebuffer();
			void __createRenderSemaphores(const size_t imageIdx);

			void __populatePipelineBuildParam() noexcept;
			void __buildPipelines(tf::Subflow &subflow);
			void __resetPipelines() noexcept;
			void __recordRenderCommand(const size_t imageIdx) noexcept;

			[[nodiscard]]
			bool __isValid() const noexcept;

			constexpr void __resetFrameCursor() noexcept;
			constexpr void __advanceFrameCursor() noexcept;

			[[nodiscard]]
			Vulkan::Semaphore &__getCurrentImageAcquireSemaphore() noexcept;

			[[nodiscard]]
			Vulkan::CommandBuffer &__getCurrentRenderCommandBuffer() noexcept;

			[[nodiscard]]
			Vulkan::Semaphore &__getCurrentAttachmentOuputSemaphore() noexcept;

			[[nodiscard]]
			Vulkan::Semaphore &__getCurrentRenderCompletionSemaphore() noexcept;

			[[nodiscard]]
			uint64_t &__getCurrentRenderCompletionSemaphoreValue() noexcept;

			[[nodiscard]]
			bool __acquireNextSwapchainImageIdx(Vulkan::Semaphore &semaphore) noexcept;
		};

		ScreenManager(
			RenderingEngine &renderingEngine,
			Vulkan::Instance &instance, Vulkan::PhysicalDevice &physicalDevice,
			const uint32_t graphicsQueueFamilyIndex,
			Vulkan::Device &device, Vulkan::Queue &queue) noexcept;

		~ScreenManager() noexcept = default;

		[[nodiscard]]
		std::unique_ptr<ScreenImpl> create(Win::Window &window) noexcept;

	private:
		RenderingEngine &__renderingEngine;
		Vulkan::Instance &__instance;
		Vulkan::PhysicalDevice &__physicalDevice;
		Vulkan::Device &__device;

		const uint32_t __graphicsQueueFamilyIndex;
		Vulkan::Queue &__queue;
	};

	constexpr void ScreenManager::ScreenImpl::__initSubmitInfo() noexcept
	{
		__submitWaitInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
		__submitWaitInfo.stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT;

		__submitCommandBufferInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO;

		// binary for signal color attachment output
		__submitSignalInfos[0].sType = VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;

		// timeline for signal render completion
		__submitSignalInfos[1].sType = VkStructureType::VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO;
	}

	constexpr void ScreenManager::ScreenImpl::__resetFrameCursor() noexcept
	{
		__frameCursor = 0ULL;
	}

	constexpr void ScreenManager::ScreenImpl::__advanceFrameCursor() noexcept
	{
		const size_t numCommandBuffers{ __renderCommandBuffers.size() };
		__frameCursor = ((__frameCursor + 1ULL) % numCommandBuffers);
	}
}