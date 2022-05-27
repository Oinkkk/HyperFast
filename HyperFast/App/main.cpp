#include <iostream>
#include "../Infrastructure/Looper.h"
#include "../Window/AppInstance.h"
#include "../Window/Window.h"
#include "../Window/MainLooper.h"
#include "../RenderingEngine/RenderingEngine.h"
#include "../RenderingEngine/Screen.h"

int main()
{
	Infra::Logger logger;
	logger.log(Infra::LogSeverityType::INFO, "The program starts.");

	const auto pDestroyEventListener
	{
		std::make_shared<Infra::EventListener<Win::Window &>>()
	};

	pDestroyEventListener->setCallback([] (Win::Window &window)
	{
		Win::MainLooper::postQuitMessage();
	});

	Win::AppInstance &appInstance{ Win::AppInstance::getInstance() };

	Win::WindowClass winClass{ appInstance.getHandle(), "DefaultWinClass" };
	Win::Window win1{ winClass, "win1", true };
	Win::Window win2{ winClass, "win2", true };

	win1.setSize(400, 300);
	win2.setSize(400, 300);

	win1.getDestroyEvent() += pDestroyEventListener;

	VKL::VulkanLoader &vulkanLoader{ VKL::VulkanLoader::getInstance() };
	vulkanLoader.load();

	logger.log(Infra::LogSeverityType::INFO, "Start to create the rendering engine.");

	std::unique_ptr<HyperFast::RenderingEngine> pRenderingEngine
	{
		std::make_unique<HyperFast::RenderingEngine>(logger, "HyperFastDemo", "HyperFast")
	};

	logger.log(Infra::LogSeverityType::INFO, "The rendering engine is created.");

	HyperFast::ScreenManager &screenManager{ pRenderingEngine->getScreenManager() };
	std::shared_ptr<HyperFast::Screen> pScreen1{ std::make_shared<HyperFast::Screen>(screenManager, win1) };
	std::shared_ptr<HyperFast::Screen> pScreen2{ std::make_shared<HyperFast::Screen>(screenManager, win2) };

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

	pScreen2 = nullptr;
	pScreen1 = nullptr;
	pRenderingEngine = nullptr;
	logger.log(Infra::LogSeverityType::INFO, "The rendering engine destroyed.");

	vulkanLoader.free();
	logger.log(Infra::LogSeverityType::INFO, "UpdateLooper ends.");

	logger.log(Infra::LogSeverityType::INFO, "End of the program");
	return 0;
}