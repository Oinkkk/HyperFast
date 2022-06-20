#pragma once

#include "../Window/Window.h"
#include "PipelineFactory.h"
#include "../Infrastructure/Logger.h"
#include "CommandBufferManager.h"
#include "../Infrastructure/Environment.h"
#include "Drawcall.h"

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
				const VkDevice device, const VKL::DeviceProcedure &deviceProc, const VkQueue graphicsQueue,
				Win::Window &window);

			~ScreenImpl() noexcept;

			void setDrawcall(Drawcall *const pDrawcall) noexcept;

		private:
			const VkInstance __instance;
			const VKL::InstanceProcedure &__instanceProc;

			const VkPhysicalDevice __physicalDevice;
			const uint32_t __graphicsQueueFamilyIndex;

			const VkDevice __device;
			const VKL::DeviceProcedure &__deviceProc;
			const VkQueue __graphicsQueue;

			Win::Window &__window;
			std::shared_ptr<Infra::EventListener<Win::Window &, Win::Window::ResizingType>> __pResizeEventListener;
			std::shared_ptr<Infra::EventListener<Win::Window &>> __pDrawEventListener;
			std::shared_ptr<Infra::EventListener<Win::Window &>> __pDestroyEventListener;

			Drawcall *__pDrawcall{};

			PipelineFactory::BuildParam __pipelineBuildParam;
			PipelineFactory __pipelineFactory;

			VkSurfaceKHR __surface{};
			std::vector<CommandBufferManager *> __mainCommandBufferManagers;
			std::vector<VkCommandBuffer> __mainCommandBuffers;

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

			size_t __frameCursor{};

			bool __draw();
			void __destroy() noexcept;

			void __initListeners() noexcept;
			void __createSurface();
			void __destroySurface() noexcept;
			void __updateSurfaceDependencies();

			void __checkSurfaceSupport() const;
			void __querySurfaceCapabilities() noexcept;
			void __querySupportedSurfaceFormats() noexcept;
			void __querySupportedSurfacePresentModes() noexcept;
			void __createSwapchain(const VkSwapchainKHR oldSwapchain);
			void __destroySwapchain(const VkSwapchainKHR swapchain) noexcept;
			void __retrieveSwapchainImages() noexcept;
			void __reserveSwapchainImageDependencyPlaceholers() noexcept;
			void __createMainCommandBufferManager(const size_t imageIdx);
			void __destroyMainCommandBufferManagers() noexcept;
			void __createSwapchainImageView(const size_t imageIdx);
			void __destroySwapchainImageViews() noexcept;
			void __createRenderPasses();
			void __destroyRenderPasses() noexcept;
			void __createFramebuffer();
			void __destroyFramebuffer() noexcept;
			void __createSyncObject(const size_t imageIdx);
			void __destroySyncObjects() noexcept;

			void __populatePipelineBuildParam() noexcept;
			void __buildPipelines(tf::Subflow &subflow);
			void __resetPipelines() noexcept;
			void __recordMainCommand(const size_t imageIdx) noexcept;
			constexpr void __resetFrameCursor() noexcept;

			void __waitDeviceIdle() noexcept;
			void __onDeviceIdle() noexcept;

			VkResult __acquireNextImage(const VkSemaphore semaphore, uint32_t &imageIdx) noexcept;

			static inline constexpr uint64_t __maxTime{ std::numeric_limits<uint64_t>::max() };
		};

		ScreenManager(
			const VkInstance instance, const VKL::InstanceProcedure &instanceProc,
			const VkPhysicalDevice physicalDevice, const uint32_t graphicsQueueFamilyIndex,
			const VkDevice device, const VKL::DeviceProcedure &deviceProc, const VkQueue graphicsQueue) noexcept;

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
		const VkQueue __graphicsQueue;
	};

	constexpr void ScreenManager::ScreenImpl::__resetFrameCursor() noexcept
	{
		__frameCursor = 0ULL;
	}
}