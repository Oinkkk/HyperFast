#include <iostream>
#include "../Infrastructure/Looper.h"
#include "../Window/AppInstance.h"
#include "../Window/Window.h"
#include "../Window/MainLooper.h"
#include "../RenderingEngine/RenderingEngine.h"

int main()
{
	Infra::Logger logger;
	logger.log(Infra::LogSeverityType::INFO, "The program starts.");

	std::shared_ptr<HyperFast::Screen> pScreen1;
	std::shared_ptr<HyperFast::Screen> pScreen2;

	const auto pDestroyEventListener1
	{
		std::make_shared<Infra::EventListener<Win::Window &>>()
	};

	const auto pDestroyEventListener2
	{
		std::make_shared<Infra::EventListener<Win::Window &>>()
	};

	pDestroyEventListener1->setCallback([&pScreen1] (Win::Window &window)
	{
		pScreen1 = nullptr;
		Win::MainLooper::postQuitMessage();
	});

	pDestroyEventListener2->setCallback([&pScreen2] (Win::Window &window)
	{
		pScreen2 = nullptr;
	});

	Win::AppInstance &appInstance{ Win::AppInstance::getInstance() };

	Win::WindowClass winClass{ appInstance.getHandle(), "DefaultWinClass" };
	Win::Window win1{ winClass, "win1", true };
	Win::Window win2{ winClass, "win2", true };

	win1.setSize(400, 300);
	win2.setSize(400, 300);

	win1.getDestroyEvent() += pDestroyEventListener1;
	win2.getDestroyEvent() += pDestroyEventListener2;

	VKL::VulkanLoader &vulkanLoader{ VKL::VulkanLoader::getInstance() };
	vulkanLoader.load();

	logger.log(Infra::LogSeverityType::INFO, "Start to create the rendering engine.");

	std::unique_ptr<HyperFast::RenderingEngine> pRenderingEngine
	{
		std::make_unique<HyperFast::RenderingEngine>(logger, "HyperFastDemo", "HyperFast")
	};

	logger.log(Infra::LogSeverityType::INFO, "The rendering engine is created.");

	pScreen1 = pRenderingEngine->createScreen(win1);
	pScreen2 = pRenderingEngine->createScreen(win2);

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