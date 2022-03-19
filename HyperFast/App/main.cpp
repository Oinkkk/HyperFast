#include <iostream>
#include "../Window/Window.h"

int main()
{
	Win::WindowClass winClass{ "DefaultWinClass" };
	Win::Window window{ winClass, "test" };

	std::cout << "hi" << std::endl;

	window.setShow(true);
	winClass.loop();

	return 0;
}