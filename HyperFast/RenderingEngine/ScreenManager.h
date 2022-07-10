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

namespace HyperFast
{
	class ScreenManager final : public Infra::Unique
	{
	public:
		class ScreenImpl final : public Infra::Unique
		{
		public:
			ScreenImpl(
				Vulkan::Instance &instance, Vulkan::PhysicalDevice &physicalDevice,
				const uint32_t graphicsQueueFamilyIndex, Vulkan::Device &device,
				Vulkan::Queue &queue, Win::Window &window);

			~ScreenImpl() noexcept;

			void setDrawcall(Drawcall *const pDrawcall) noexcept;
			void draw();

		private:
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

			VkSurfaceKHR __surface{};
			std::vector<CommandBufferManager *> __mainCommandBufferManagers;
			std::vector<VkCommandBuffer> __mainCommandBuffers;

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

			std::vector<std::unique_ptr<Vulkan::Semaphore>> __presentCompleteSemaphores;
			std::vector<std::unique_ptr<Vulkan::Semaphore>> __renderCompleteSemaphores;
			std::vector<std::unique_ptr<Vulkan::Fence>> __renderCompleteFences;

			size_t __frameCursor{};
			bool __imageAcquired{};
			uint32_t __imageIdx{};

			bool __needToDraw{};
			bool __needToUpdateSurfaceDependencies{};
			bool __needToUpdatePipelineDependencies{};
			bool __needToUpdateMainCommands{};
			bool __destroyed{};

			void __update();
			bool __draw();
			void __destroy() noexcept;

			void __initListeners() noexcept;
			void __createSurface();
			void __destroySurface() noexcept;
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
			void __createMainCommandBufferManager(const size_t imageIdx);
			void __destroyMainCommandBufferManagers() noexcept;
			void __createSwapchainImageView(const size_t imageIdx);
			void __createRenderPasses();
			void __createFramebuffer();
			void __createSyncObject(const size_t imageIdx);

			void __populatePipelineBuildParam() noexcept;
			void __buildPipelines(tf::Subflow &subflow);
			void __resetPipelines() noexcept;
			void __recordMainCommand(const size_t imageIdx) noexcept;

			constexpr void __resetFrameCursor() noexcept;
			constexpr void __advanceFrameCursor() noexcept;
		};

		ScreenManager(
			Vulkan::Instance &instance, Vulkan::PhysicalDevice &physicalDevice,
			const uint32_t graphicsQueueFamilyIndex,
			Vulkan::Device &device, Vulkan::Queue &queue) noexcept;

		~ScreenManager() noexcept = default;

		[[nodiscard]]
		std::unique_ptr<ScreenImpl> create(Win::Window &window) noexcept;

	private:
		Vulkan::Instance &__instance;
		Vulkan::PhysicalDevice &__physicalDevice;
		Vulkan::Device &__device;

		const uint32_t __graphicsQueueFamilyIndex;
		Vulkan::Queue &__queue;
	};

	constexpr void ScreenManager::ScreenImpl::__resetFrameCursor() noexcept
	{
		__frameCursor = 0ULL;
	}

	constexpr void ScreenManager::ScreenImpl::__advanceFrameCursor() noexcept
	{
		const size_t numCommandBuffers{ __mainCommandBuffers.size() };
		__frameCursor = ((__frameCursor + 1ULL) % numCommandBuffers);
	}
}