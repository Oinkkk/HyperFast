﻿#include <iostream>
#include "../Window/AppInstance.h"
#include "../Window/Window.h"
#include "../Window/MainLooper.h"
#include "../RenderingEngine/RenderingEngine.h"
#include "../RenderingEngine/Screen.h"
#include "RenderLooper.h"

int main()
{
	Win::AppInstance &appInstance{ Win::AppInstance::getInstance() };

	VKL::VulkanLoader &vulkanLoader{ VKL::VulkanLoader::getInstance() };
	vulkanLoader.load();

	Infra::Logger logger;

	std::unique_ptr<HyperFast::RenderingEngine> pRenderingEngine
	{
		std::make_unique<HyperFast::RenderingEngine>(logger, "HyperFastDemo", "HyperFast")
	};

	HyperFast::ScreenManager &screenManager{ pRenderingEngine->getScreenManager() };

	Win::WindowClass winClass{ appInstance.getHandle(), "DefaultWinClass" };
	Win::Window win1{ winClass, "win1", true };
	Win::Window win2{ winClass, "win2", true };

	std::shared_ptr<HyperFast::Screen> pScreen1{ std::make_shared<HyperFast::Screen>(screenManager, win1) };
	std::shared_ptr<HyperFast::Screen> pScreen2{ std::make_shared<HyperFast::Screen>(screenManager, win2) };

	std::unordered_map<Win::Window *, HyperFast::Screen *> window2ScreenMap;
	window2ScreenMap.emplace(&win1, pScreen1.get());
	window2ScreenMap.emplace(&win2, pScreen2.get());

	RenderLooper renderLooper;
	renderLooper.start();

	const auto pDrawEventListener
	{
		Infra::EventListener<Win::Window &>::make([&] (Win::Window &window)
		{
			renderLooper.requestDraw(*window2ScreenMap[&window]);
			window.validate();
		})
	};

	const auto pDestroyEventListener
	{
		Infra::EventListener<Win::Window &>::make([&] (Win::Window &window)
		{
			renderLooper.stop();
			Win::MainLooper::postQuitMessage();
		})
	};

	win1.getDrawEvent() += pDrawEventListener;
	win2.getDrawEvent() += pDrawEventListener;
	win1.getDestroyEvent() += pDestroyEventListener;
	win2.getDestroyEvent() += pDestroyEventListener;

	win1.setSize(800, 600);
	win2.setSize(800, 600);

	Win::MainLooper::start();

	pScreen2 = nullptr;
	pScreen1 = nullptr;
	pRenderingEngine = nullptr;

	vulkanLoader.free();
	return 0;
}