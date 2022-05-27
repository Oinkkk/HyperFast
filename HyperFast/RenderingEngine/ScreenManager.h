#pragma once

#include "../Window/Window.h"
#include "PipelineFactory.h"

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
				const VkPhysicalDevice firstPhysicalDevice, const uint32_t graphicsQueueFamilyIndex,
				const VkDevice device, const VKL::DeviceProcedure &deviceProc, Win::Window &window);

			~ScreenImpl() noexcept;

		private:
			const VkInstance __instance;
			const VKL::InstanceProcedure &__instanceProc;

			const VkPhysicalDevice __firstPhysicalDevice;
			const uint32_t __graphicsQueueFamilyIndex;

			const VkDevice __device;
			const VKL::DeviceProcedure &__deviceProc;

			Win::Window &__window;

			VkSurfaceKHR __surface{};
			VkSurfaceCapabilitiesKHR __surfaceCapabilities{};
			std::vector<VkSurfaceFormatKHR> __supportedSurfaceFormats;

			PipelineFactory::BuildParam __pipelineFactoryBuildParam;
			PipelineFactory __pipelineFactory;

			void __destroySurface() noexcept;
			void __checkSurfaceSupport() const;
			void __initPipelineFactoryBuildParam() noexcept;
			void __querySurfaceCapabilities() noexcept;
			void __querySupportedSurfaceFormats() noexcept;

			[[nodiscard]]
			static VkSurfaceKHR __createSurface(
				const VkInstance instance, const VKL::InstanceProcedure &instanceProc, Win::Window &window);
		};

		ScreenManager(
			const VkInstance instance, const VKL::InstanceProcedure &instanceProc,
			const VkPhysicalDevice firstPhysicalDevice, const uint32_t graphicsQueueFamilyIndex,
			const VkDevice device, const VKL::DeviceProcedure &deviceProc) noexcept;

		~ScreenManager() noexcept = default;

		[[nodiscard]]
		std::unique_ptr<ScreenImpl> create(Win::Window &window) noexcept;

	private:
		const VkInstance __instance;
		const VKL::InstanceProcedure &__instanceProc;

		const VkPhysicalDevice __firstPhysicalDevice;
		const uint32_t __graphicsQueueFamilyIndex;

		const VkDevice __device;
		const VKL::DeviceProcedure &__deviceProc;
	};
}
