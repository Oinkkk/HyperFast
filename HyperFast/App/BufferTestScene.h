#pragma once

#include "../Scene/Scene.h"

class BufferTestScene : public Jin::Scene
{
public:
	BufferTestScene(
		HyperFast::RenderingEngine &renderingEngine,
		HyperFast::Screen &screen1, HyperFast::Screen &screen2) noexcept;

	virtual ~BufferTestScene() noexcept;

protected:
	virtual void _onProcess(const float deltaTime) override;

private:
	HyperFast::Screen &__screen1;
	HyperFast::Screen &__screen2;

	std::shared_ptr<HyperFast::Mesh> __pMesh;
	std::shared_ptr<HyperFast::Submesh> __pSubmesh1;
	std::shared_ptr<HyperFast::Submesh> __pSubmesh2;

	float __submeshTimer1{};
	float __submeshTimer2{};

	void __createMesh();
};