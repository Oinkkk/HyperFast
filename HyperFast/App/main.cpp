#include <iostream>
#include "../Infrastructure/Looper.h"
#include "../Window/Window.h"
#include "../Window/MainLooper.h"
#include "../VulkanLoader/VulkanLoader.h"
#include "../RenderingEngine/RenderingEngine.h"

int main()
{
	Infra::Logger logger;
	logger.log(Infra::LogSeverityType::INFO, "The program starts.");

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
		[] (Win::Window *const pWindow, Win::EventConsumptionView *const pEventConsumption,
			const Win::Window::ResizingType resizingType)
	{
		pWindow->validate();
		pEventConsumption->consume();
	});

	pDrawEventListener->setCallback([](Win::Window *const pWindow, Win::EventConsumptionView *const pEventConsumption)
	{
		pWindow->validate();
		pEventConsumption->consume();
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

	logger.log(Infra::LogSeverityType::INFO, "Start to create the rendering engine.");

	std::unique_ptr<HyperFast::RenderingEngine> pRenderingEngine
	{
		std::make_unique<HyperFast::RenderingEngine>(logger, "HyperFastDemo", "HyperFast")
	};

	logger.log(Infra::LogSeverityType::INFO, "The rendering engine is created.");

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

	logger.log(Infra::LogSeverityType::INFO, "UpdateLooper starts.");
	Infra::Looper updateLooper;
	updateLooper.start(messageFunc, updateFunc);

	logger.log(Infra::LogSeverityType::INFO, "MainLooper starts.");
	Win::MainLooper::start();
	logger.log(Infra::LogSeverityType::INFO, "MainLooper ends.");

	updateLooper.stop();
	logger.log(Infra::LogSeverityType::INFO, "UpdateLooper ends.");

	pRenderingEngine = nullptr;
	logger.log(Infra::LogSeverityType::INFO, "The rendering engine destroyed.");

	vulkanLoader.free();
	logger.log(Infra::LogSeverityType::INFO, "UpdateLooper ends.");

	logger.log(Infra::LogSeverityType::INFO, "End of the program");
	return 0;
}