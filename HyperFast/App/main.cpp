#include <iostream>
#include "../Infrastructure/Looper.h"
#include "../Window/Window.h"
#include "../Window/MainLooper.h"

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
		Win::MainLooper::postQuitMessage();
	});

	pWindow->setShow(true);
	pWindow->getCloseEvent() += pCloseEventListener;

	const Infra::Looper::InitFunc initFunc
	{
		[]
		{
			int a = 0;
		}
	};

	const Infra::Looper::MessageFunc messageFunc
	{
		[] (const uint64_t id, const std::vector<std::any> &arguments)
		{
			int a = 0;
		}
	};

	const Infra::Looper::UpdateFunc updateFunc
	{
		[] (const float deltaTime)
		{
			int a = 0;
		}
	};

	const Infra::Looper::EndFunc endFunc
	{
		[]
		{
			int a = 0;
		}
	};

	Infra::Looper updateLooper;
	updateLooper.start(initFunc, messageFunc, updateFunc, endFunc);

	Win::MainLooper::start();
	updateLooper.stop();

	return 0;
}