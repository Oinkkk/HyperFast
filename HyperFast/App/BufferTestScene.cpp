#include "BufferTestScene.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_aligned.hpp>

BufferTestScene::BufferTestScene(
	HyperFast::RenderingEngine &renderingEngine,
	std::unique_ptr<HyperFast::Screen> &&pScreen1,
	std::unique_ptr<HyperFast::Screen> &&pScreen2) noexcept :
	Scene{ renderingEngine },
	__pScreen1{ std::move(pScreen1) }, __pScreen2{ std::move(pScreen2) }
{
	__createVertexBuffers();
}

BufferTestScene::~BufferTestScene() noexcept
{}

void BufferTestScene::__createVertexBuffers()
{
	std::vector<glm::vec3> positions;
	positions.emplace_back(0.0f, -0.5f, 0.0f);
	positions.emplace_back(-0.5f, 0.5f, 0.0f);
	positions.emplace_back(0.5f, 0.5f, 0.0f);

	std::vector<glm::vec4> colors;
	colors.emplace_back(1.0f, 0.0f, 0.0f, 1.0f);
	colors.emplace_back(0.0f, 1.0f, 0.0f, 1.0f);
	colors.emplace_back(0.0f, 0.0f, 1.0f, 1.0f);

	__positionBuffer = _createVertexBuffer(sizeof(glm::vec3) * positions.size(), positions.data());
	__colorBuffer = _createVertexBuffer(sizeof(glm::vec4) * colors.size(), colors.data());
}