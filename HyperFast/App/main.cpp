#include <iostream>
#include "../Window/Window.h"

int main()
{
	Win::WindowClass winClass{ "DefaultWinClass" };
	Win::Window window{ winClass, "test" };

	window.setEnabled(true);

	std::cout << "hi" << std::endl;
	return 0;
}