#pragma once

#include "../Scene/Scene.h"
#include "../RenderingEngine/Screen.h"

class BufferTestScene : public Jin::Scene
{
public:
	BufferTestScene(
		HyperFast::RenderingEngine &renderingEngine,
		std::unique_ptr<HyperFast::Screen> &&pScreen1,
		std::unique_ptr<HyperFast::Screen> &&pScreen2) noexcept;

	virtual ~BufferTestScene() noexcept;

private:
	std::unique_ptr<HyperFast::Screen> __pScreen1;
	std::unique_ptr<HyperFast::Screen> __pScreen2;
};