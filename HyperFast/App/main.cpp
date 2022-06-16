﻿#include <iostream>
#include "../Window/AppInstance.h"
#include "../Window/Window.h"
#include "../Window/MainLooper.h"
#include "../RenderingEngine/RenderingEngine.h"
#include "../RenderingEngine/Screen.h"
#include "BufferTestScene.h"

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

	win1.setSize(800, 600);
	win2.setSize(800, 600);

	std::shared_ptr<Infra::EventListener<Win::Window &>> pDestroyEventListener
	{
		Infra::EventListener<Win::Window &>::make([&](Win::Window &window)
		{
			mainLooper.stop();
		})
	};

	std::shared_ptr<Infra::EventListener<float>> pIdleEventListener
	{
		Infra::EventListener<float>::make([&](const float deltaTime)
		{
			
		})
	};

	win1.getDestroyEvent() += pDestroyEventListener;
	// mainLooper.getIdleEvent() += pIdleEventListener;

	HyperFast::ScreenManager &screenManager{ pRenderingEngine->getScreenManager() };
	std::unique_ptr<HyperFast::Screen> pScreen1{ std::make_unique<HyperFast::Screen>(screenManager, win1) };
	std::unique_ptr<HyperFast::Screen> pScreen2{ std::make_unique<HyperFast::Screen>(screenManager, win2) };

	std::unique_ptr<BufferTestScene> pBufferTestScene
	{
		std::make_unique<BufferTestScene>(*pRenderingEngine, std::move(pScreen1), std::move(pScreen2))
	};

	mainLooper.start();

	pBufferTestScene = nullptr;
	pScreen2 = nullptr;
	pScreen1 = nullptr;
	pRenderingEngine = nullptr;

	vulkanLoader.free();
	return 0;
}