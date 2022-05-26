#include "Screen.h"

namespace HyperFast
{
	Screen::Screen(
		SurfaceCreater &surfaceAllocator, Win::Window &window,
		const VkDevice device, const VKL::DeviceProcedure &deviceProc) :
		__surfaceAllocator{ surfaceAllocator }, __window{ window },
		__surface{ surfaceAllocator.create(__window) }, __pipelineFactory{ device, deviceProc }
	{
		if (!__surface)
			throw std::exception{ "Cannot create a surface." };

		__initPipelineFactoryBuildParam();
		__pipelineFactory.build(__pipelineFactoryBuildParam);
	}

	Screen::~Screen() noexcept
	{
		destroy();
	}

	void Screen::setRenderCommand(RenderCommand *const pRenderCommand) noexcept
	{
		if (__pRenderCommand == pRenderCommand)
			return;

		__pRenderCommand = pRenderCommand;
	}

	void Screen::destroy() noexcept
	{
		if (!__surface)
			return;

		__surfaceAllocator.destroy(__surface);
		__surface = nullptr;
	}

	void Screen::__initPipelineFactoryBuildParam() noexcept
	{
		__pipelineFactoryBuildParam.viewportWidth = float(__window.getWidth());
		__pipelineFactoryBuildParam.viewportHeight = float(__window.getHeight());
	}
}