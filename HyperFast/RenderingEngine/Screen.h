#pragma once

#include "../Infrastructure/ResourceCreater.h"
#include "../Window/Window.h"
#include "PipelineFactory.h"
#include "RenderCommand.h"

namespace HyperFast
{
	class SurfaceCreater : public Infra::ResourceCreater<VkSurfaceKHR>
	{
	public:
		virtual VkSurfaceKHR create(Win::Window &window) noexcept = 0;
	};

	class Screen : public Infra::Unique
	{
	public:
		Screen(
			SurfaceCreater &surfaceAllocator, Win::Window &window,
			const VkDevice device, const VKL::DeviceProcedure &deviceProc);
		
		virtual ~Screen() noexcept;

		void setRenderCommand(RenderCommand *const pRenderCommand) noexcept;

		void destroy() noexcept;

		[[nodiscard]]
		constexpr bool isDestroyed() const noexcept;

	private:
		SurfaceCreater &__surfaceAllocator;
		Win::Window &__window;

		VkSurfaceKHR __surface{};

		PipelineFactory::BuildParam __pipelineFactoryBuildParam;
		PipelineFactory __pipelineFactory;
		RenderCommand *__pRenderCommand{};

		void __initPipelineFactoryBuildParam() noexcept;
	};

	constexpr bool Screen::isDestroyed() const noexcept
	{
		return __surface;
	}
}
