#include "Screen.h"

namespace HyperFast
{
	Screen::Screen(SurfaceAllocator &surfaceAllocator, Win::Window &window) :
		__surfaceAllocator{ surfaceAllocator }, __window{ window }, __surface{ surfaceAllocator.allocate(__window) }
	{
		__pDestroyEventListener = std::make_shared<Infra::EventListener<Win::Window &>>();
		__pDestroyEventListener->setCallback([this] (Win::Window &)
		{
			destroy();
		});

		__window.getDestroyEvent() += __pDestroyEventListener;
	}

	Screen::~Screen() noexcept
	{
		destroy();
	}

	void Screen::destroy() noexcept
	{
		if (!__surface)
			return;

		__destroyEvent.invoke(*this);

		__surfaceAllocator.free(__surface);
		__surface = nullptr;
	}
}