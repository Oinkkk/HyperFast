#include <iostream>
#include "../Window/Window.h"
#include "../Window/MessageLooper.h"

int main()
{
	Win::WindowClass winClass{ "DefaultWinClass" };
	
	std::unique_ptr<Win::Window> pWindow
	{ 
		std::make_unique<Win::Window>(winClass, "test")
	};

	const std::shared_ptr<Infra::EventListener<Win::Window *>> pCloseEventListener
	{
		std::make_shared<Infra::EventListener<Win::Window *>>()
	};

	pCloseEventListener->setCallback([] (Win::Window *const pWindow)
	{
		pWindow->destroy();
		Win::MessageLooper::stopLoop();
	});

	pWindow->setShow(true);
	pWindow->getCloseEvent() += pCloseEventListener;

	Win::MessageLooper::startLoop();
	return 0;
}