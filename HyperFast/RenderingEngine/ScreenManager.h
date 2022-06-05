#pragma once

#include "../Window/Window.h"
#include "PipelineFactory.h"
#include "../Infrastructure/Logger.h"
#include "CommandBufferManager.h"

namespace HyperFast
{
	class ScreenManager final : public Infra::Unique
	{
	public:
		class ScreenImpl final : public Infra::Unique
		{
		public:
			ScreenImpl(
				const VkInstance instance, const VKL::InstanceProcedure &instanceProc,
				const VkPhysicalDevice physicalDevice, const uint32_t graphicsQueueFamilyIndex,
				const VkDevice device, const VKL::DeviceProcedure &deviceProc, Win::Window &window,
				Infra::Logger &logger);

			~ScreenImpl() noexcept;

			void draw() noexcept;

		private:
			const VkInstance __instance;
			const VKL::InstanceProcedure &__instanceProc;

			const VkPhysicalDevice __physicalDevice;
			const uint32_t __graphicsQueueFamilyIndex;

			const VkDevice __device;
			const VKL::DeviceProcedure &__deviceProc;

			Win::Window &__window;
			Infra::Logger &__logger;

			PipelineFactory::BuildParam __pipelineBuildParam;
			PipelineFactory __pipelineFactory;

			VkSurfaceKHR __surface{};
			std::unique_ptr<CommandBufferManager> __pMainCommandBufferManager;

			VkSurfaceCapabilitiesKHR __surfaceCapabilities{};
			std::vector<VkSurfaceFormatKHR> __supportedSurfaceFormats;
			std::vector<VkPresentModeKHR> __supportedSurfacePresentModes;
			
			VkSwapchainKHR __swapchain{};
			VkFormat __swapchainFormat{};
			VkExtent2D __swapchainExtent{};

			std::vector<VkImage> __swapChainImages;
			std::vector<VkImageView> __swapChainImageViews;

			VkRenderPass __renderPass{};
			VkFramebuffer __framebuffer{};

			std::vector<VkSemaphore> __presentCompleteSemaphores;
			std::vector<VkSemaphore> __renderCompleteSemaphores;
			std::vector<VkFence> __renderCompleteFences;

			std::vector<VkCommandBuffer> __mainCommandBuffers;

			void __init();
			void __reset() noexcept;

			void __createSurface();
			void __destroySurface() noexcept;
			void __createMainCommandBufferManager();
			void __destroyMainCommandBufferManager() noexcept;

			void __checkSurfaceSupport() const;
			void __querySurfaceCapabilities() noexcept;
			void __querySupportedSurfaceFormats() noexcept;
			void __querySupportedSurfacePresentModes() noexcept;
			void __createSwapchain();
			void __destroySwapchain() noexcept;
			void __retrieveSwapchainImages() noexcept;
			void __resetSwapchainImages() noexcept;
			void __createSwapchainImageViews();
			void __destroySwapchainImageViews() noexcept;
			void __createRenderPasses();
			void __destroyRenderPasses() noexcept;
			void __createFramebuffer();
			void __destroyFramebuffer() noexcept;
			void __initSyncObjects();
			void __destroySyncObjects() noexcept;

			void __populatePipelineBuildParam() noexcept;
			void __buildPipelines();
			void __recordMainCommands() noexcept;
		};

		ScreenManager(
			const VkInstance instance, const VKL::InstanceProcedure &instanceProc,
			const VkPhysicalDevice physicalDevice, const uint32_t graphicsQueueFamilyIndex,
			const VkDevice device, const VKL::DeviceProcedure &deviceProc,
			Infra::Logger &logger) noexcept;

		~ScreenManager() noexcept = default;

		[[nodiscard]]
		std::unique_ptr<ScreenImpl> create(Win::Window &window) noexcept;

	private:
		const VkInstance __instance;
		const VKL::InstanceProcedure &__instanceProc;

		const VkPhysicalDevice __physicalDevice;
		const uint32_t __graphicsQueueFamilyIndex;

		const VkDevice __device;
		const VKL::DeviceProcedure &__deviceProc;

		Infra::Logger &__logger;
	};
}
