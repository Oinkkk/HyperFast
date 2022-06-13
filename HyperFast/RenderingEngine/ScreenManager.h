#pragma once

#include "../Window/Window.h"
#include "PipelineFactory.h"
#include "../Infrastructure/Logger.h"
#include "CommandBufferManager.h"
#include "../Infrastructure/Environment.h"

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
				Win::Window &window, Infra::Logger &logger);

			~ScreenImpl() noexcept;

			bool draw();

		private:
			class ResourceBundle final : public Infra::Unique
			{
			public:
				ResourceBundle(
					const VkInstance instance, const VKL::InstanceProcedure &instanceProc,
					Win::Window &window, const VkSurfaceKHR surface,
					const VkPhysicalDevice physicalDevice, const uint32_t graphicsQueueFamilyIndex,
					const VkDevice device, const VKL::DeviceProcedure &deviceProc) noexcept;

				~ResourceBundle() noexcept;

				void update(tf::Subflow &subflow);

				[[nodiscard]]
				constexpr VkSwapchainKHR getSwapchain() noexcept;

				[[nodiscard]]
				constexpr VkSemaphore getPresentCompleteSemaphore() noexcept;
				VkResult acquireNextImage(const VkSemaphore semaphore, uint32_t &imageIdx) noexcept;
				constexpr void nextFrame() noexcept;

				[[nodiscard]]
				constexpr VkCommandBuffer getMainCommandBuffer(const uint32_t imageIdx) noexcept;

				[[nodiscard]]
				constexpr VkSemaphore getRenderCompleteSemaphore(const uint32_t imageIdx) noexcept;

				[[nodiscard]]
				constexpr VkFence getRenderCompleteFence(const uint32_t imageIdx) noexcept;

			private:
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
				void __buildPipelines();
				void __resetPipelines() noexcept;
				void __recordMainCommand(const size_t imageIdx) noexcept;
				constexpr void __resetFrameCursor() noexcept;

			private:
				const VkInstance __instance;
				const VKL::InstanceProcedure &__instanceProc;

				Win::Window &__window;
				const VkSurfaceKHR __surface;

				const VkPhysicalDevice __physicalDevice;
				const uint32_t __graphicsQueueFamilyIndex;

				const VkDevice __device;
				const VKL::DeviceProcedure &__deviceProc;

				PipelineFactory::BuildParam __pipelineBuildParam;
				PipelineFactory __pipelineFactory;

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
			};

			const VkInstance __instance;
			const VKL::InstanceProcedure &__instanceProc;

			const VkPhysicalDevice __physicalDevice;
			const uint32_t __graphicsQueueFamilyIndex;

			const VkDevice __device;
			const VKL::DeviceProcedure &__deviceProc;
			const VkQueue __graphicsQueue;

			Win::Window &__window;
			Infra::Logger &__logger;

			tf::Future<void> __available;

			VkSurfaceKHR __surface{};

			std::unique_ptr<ResourceBundle> __pResourceBundle;

			void __createSurface();
			void __destroySurface() noexcept;
			void __updateSurfaceDependencies();

			void __createResourceBundle() noexcept;
			void __destroyResourceBundle() noexcept;

			void __waitDeviceIdle() const noexcept;

			static inline constexpr uint64_t __maxTime{ std::numeric_limits<uint64_t>::max() };
		};

		ScreenManager(
			const VkInstance instance, const VKL::InstanceProcedure &instanceProc,
			const VkPhysicalDevice physicalDevice, const uint32_t graphicsQueueFamilyIndex,
			const VkDevice device, const VKL::DeviceProcedure &deviceProc, const VkQueue graphicsQueue,
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
		const VkQueue __graphicsQueue;

		Infra::Logger &__logger;
	};

	constexpr VkSwapchainKHR ScreenManager::ScreenImpl::ResourceBundle::getSwapchain() noexcept
	{
		return __swapchain;
	}

	[[nodiscard]]
	constexpr VkSemaphore ScreenManager::ScreenImpl::ResourceBundle::getPresentCompleteSemaphore() noexcept
	{
		return __presentCompleteSemaphores[__frameCursor];
	}

	constexpr void ScreenManager::ScreenImpl::ResourceBundle::nextFrame() noexcept
	{
		const size_t numCommandBuffers{ __mainCommandBuffers.size() };
		__frameCursor = ((__frameCursor + 1ULL) % numCommandBuffers);
	}

	[[nodiscard]]
	constexpr VkCommandBuffer ScreenManager::ScreenImpl::ResourceBundle::getMainCommandBuffer(const uint32_t imageIdx) noexcept
	{
		return __mainCommandBuffers[imageIdx];
	}

	constexpr VkSemaphore ScreenManager::ScreenImpl::ResourceBundle::getRenderCompleteSemaphore(const uint32_t imageIdx) noexcept
	{
		return __renderCompleteSemaphores[imageIdx];
	}

	constexpr VkFence ScreenManager::ScreenImpl::ResourceBundle::getRenderCompleteFence(const uint32_t imageIdx) noexcept
	{
		return __renderCompleteFences[imageIdx];
	}

	constexpr void ScreenManager::ScreenImpl::ResourceBundle::__resetFrameCursor() noexcept
	{
		__frameCursor = 0ULL;
	}
}
