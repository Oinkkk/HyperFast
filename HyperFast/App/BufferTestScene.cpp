#include "BufferTestScene.h"

BufferTestScene::BufferTestScene(
	HyperFast::RenderingEngine &renderingEngine,
	std::unique_ptr<HyperFast::Screen> &&pScreen1,
	std::unique_ptr<HyperFast::Screen> &&pScreen2) noexcept :
	Scene{ renderingEngine },
	__pScreen1{ std::move(pScreen1) }, __pScreen2{ std::move(pScreen2) }
{}

BufferTestScene::~BufferTestScene() noexcept
{}