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
				const VkDevice device, const VKL::DeviceProcedure &deviceProc, Win::Window &window);

			~ScreenImpl() noexcept;

		private:
			const VkInstance __instance;
			const VKL::InstanceProcedure &__instanceProc;

			const VkDevice __device;
			const VKL::DeviceProcedure &__deviceProc;

			Win::Window &__window;
			VkSurfaceKHR __surface{};

			PipelineFactory::BuildParam __pipelineFactoryBuildParam;
			PipelineFactory __pipelineFactory;

			[[nodiscard]]
			static VkSurfaceKHR __createSurface(
				const VkInstance instance, const VKL::InstanceProcedure &instanceProc, Win::Window &window);

			void __initPipelineFactoryBuildParam() noexcept;
		};

		ScreenManager(
			const VkInstance instance, const VKL::InstanceProcedure &instanceProc,
			const VkDevice device, const VKL::DeviceProcedure &deviceProc) noexcept;

		~ScreenManager() noexcept = default;

		[[nodiscard]]
		std::unique_ptr<ScreenImpl> create(Win::Window &window) noexcept;

	private:
		const VkInstance __instance;
		const VKL::InstanceProcedure &__instanceProc;

		const VkDevice __device;
		const VKL::DeviceProcedure &__deviceProc;
	};
}
