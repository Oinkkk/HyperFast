#include "Screen.h"

namespace HyperFast
{
	Screen::Screen(ScreenManager &screenManager, Win::Window &window) :
		__screenManager{ screenManager }, __pImpl{ screenManager.create(window) }
	{}

	void Screen::setDrawcall(Drawcall *const pDrawcall) noexcept
	{
		__pImpl->setDrawcall(pDrawcall);
	}

	void Screen::draw()
	{
		__pImpl->draw();
	}
}