#pragma once

#include "../Window/Window.h"
#include "PipelineFactory.h"
#include "../Infrastructure/Logger.h"

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

		private:
			const VkInstance __instance;
			const VKL::InstanceProcedure &__instanceProc;

			const VkPhysicalDevice __physicalDevice;
			const uint32_t __graphicsQueueFamilyIndex;

			const VkDevice __device;
			const VKL::DeviceProcedure &__deviceProc;

			Win::Window &__window;
			Infra::Logger &__logger;

			VkSurfaceKHR __surface{};
			VkSurfaceCapabilitiesKHR __surfaceCapabilities{};
			std::vector<VkSurfaceFormatKHR> __supportedSurfaceFormats;
			std::vector<VkPresentModeKHR> __supportedSurfacePresentModes;
			VkSwapchainKHR __swapchain{};

			PipelineFactory::BuildParam __pipelineFactoryBuildParam;
			PipelineFactory __pipelineFactory;

			void __destroySurface() noexcept;
			void __checkSurfaceSupport() const;
			void __querySurfaceCapabilities() noexcept;
			void __querySupportedSurfaceFormats() noexcept;
			void __querySupportedSurfacePresentModes() noexcept;
			void __createSwapchain();
			void __destroySwapchain() noexcept;

			void __initPipelineFactoryBuildParam() noexcept;
			void __buildPipelines();

			[[nodiscard]]
			static VkSurfaceKHR __createSurface(
				const VkInstance instance, const VKL::InstanceProcedure &instanceProc, Win::Window &window);
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