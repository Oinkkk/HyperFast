#include "Screen.h"

namespace HyperFast
{
	Screen::Screen(ScreenManager &screenManager, Win::Window &window) :
		__screenManager{ screenManager }, __pImpl{ screenManager.create(window) }
	{}

	bool Screen::draw()
	{
		return __pImpl->draw();
	}
}