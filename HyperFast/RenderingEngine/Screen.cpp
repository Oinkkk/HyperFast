#include "Screen.h"

namespace HyperFast
{
	Screen::Screen(SurfaceAllocator &surfaceAllocator, Win::Window &window) :
		__surfaceAllocator{ surfaceAllocator }, __window{ window }, __surface{ surfaceAllocator.allocate(__window) }
	{

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

		__surfaceAllocator.free(__surface);
		__surface = nullptr;
	}
}