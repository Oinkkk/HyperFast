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

	void __createMesh();

private:
	std::unique_ptr<HyperFast::Screen> __pScreen1;
	std::unique_ptr<HyperFast::Screen> __pScreen2;

	std::shared_ptr<HyperFast::Mesh> __pMesh;
};