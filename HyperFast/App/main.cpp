#include <iostream>
#include "../Window/AppInstance.h"
#include "../Window/Window.h"
#include "../Window/MainLooper.h"
#include "../RenderingEngine/RenderingEngine.h"
#include "../RenderingEngine/Screen.h"

int main()
{
	Infra::Logger logger;

	Win::MainLooper &mainLooper{ Win::MainLooper::getInstance() };
	VKL::VulkanLoader &vulkanLoader{ VKL::VulkanLoader::getInstance() };
	vulkanLoader.load();

	std::unique_ptr<HyperFast::RenderingEngine> pRenderingEngine
	{
		std::make_unique<HyperFast::RenderingEngine>(logger, "HyperFastDemo", "HyperFast")
	};

	Win::AppInstance &appInstance{ Win::AppInstance::getInstance() };
	Win::WindowClass winClass{ appInstance.getHandle(), "DefaultWinClass" };
	Win::Window win1{ winClass, "win1", true };
	Win::Window win2{ winClass, "win2", true };

	HyperFast::ScreenManager &screenManager{ pRenderingEngine->getScreenManager() };
	std::shared_ptr<HyperFast::Screen> pScreen1{ std::make_shared<HyperFast::Screen>(screenManager, win1) };
	std::shared_ptr<HyperFast::Screen> pScreen2{ std::make_shared<HyperFast::Screen>(screenManager, win2) };

	win1.setSize(800, 600);
	win2.setSize(800, 600);

	mainLooper.start();

	pScreen2 = nullptr;
	pScreen1 = nullptr;
	pRenderingEngine = nullptr;

	vulkanLoader.free();
	return 0;
}