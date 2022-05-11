#include <iostream>
#include "../Infrastructure/Looper.h"
#include "../Window/Window.h"
#include "../Window/MainLooper.h"
#include "../VulkanLoader/VulkanLoader.h"
#include "../RenderingEngine/RenderingEngine.h"

int main()
{
	const auto pResizeEventListener
	{
		std::make_shared<Infra::EventListener<Win::Window *, Win::EventConsumptionView *, Win::Window::ResizingType>>()
	};

	const auto pDrawEventListener
	{
		std::make_shared<Infra::EventListener<Win::Window *, Win::EventConsumptionView *>>()
	};

	const auto pCloseEventListener
	{
		std::make_shared<Infra::EventListener<Win::Window *, Win::EventConsumptionView *>>()
	};

	pResizeEventListener->setCallback(
		[] (Win::Window *const pWindow, Win::EventConsumptionView *const pEventConsumption, const Win::Window::ResizingType resizingType)
	{
		pWindow->validate();
		pEventConsumption->consume();
		std::cout << "onResize" << std::endl;
	});

	pDrawEventListener->setCallback([](Win::Window *const pWindow, Win::EventConsumptionView *const pEventConsumption)
	{
		pWindow->validate();
		pEventConsumption->consume();
		std::cout << "onDraw" << std::endl;
	});

	pCloseEventListener->setCallback([] (Win::Window *const pWindow, Win::EventConsumptionView *const pEventConsumption)
	{
		pWindow->destroy();
		pEventConsumption->consume();
		Win::MainLooper::postQuitMessage();
	});

	Win::WindowClass winClass{ "DefaultWinClass" };
	Win::Window win1{ winClass, "win1", true };
	Win::Window win2{ winClass, "win2", true };

	win1.setSize(400, 300);
	win2.setSize(400, 300);

	win1.getResizeEvent() += pResizeEventListener;
	win1.getDrawEvent() += pDrawEventListener;
	win1.getCloseEvent() += pCloseEventListener;

	VKL::VulkanLoader &vulkanLoader{ VKL::VulkanLoader::getInstance() };
	vulkanLoader.load();

	HyperFast::RenderingEngine renderingEngine{ "HyperFastDemo", "HyperFast" };

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

	Infra::Looper updateLooper;
	updateLooper.start(messageFunc, updateFunc);

	Win::MainLooper::start();
	updateLooper.stop();

	return 0;
}