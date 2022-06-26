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
	__createMesh();
}

BufferTestScene::~BufferTestScene() noexcept
{}

void BufferTestScene::__createMesh()
{
	std::vector<glm::vec3> positions;
	positions.emplace_back(0.0f, -0.5f, 0.0f);
	positions.emplace_back(-0.5f, 0.5f, 0.0f);
	positions.emplace_back(0.5f, 0.5f, 0.0f);

	std::vector<glm::vec4> colors;
	colors.emplace_back(1.0f, 0.0f, 0.0f, 1.0f);
	colors.emplace_back(0.0f, 1.0f, 0.0f, 1.0f);
	colors.emplace_back(0.0f, 0.0f, 1.0f, 1.0f);

	std::vector<glm::uint16_t> indices;
	indices.emplace_back(0U);
	indices.emplace_back(1U);
	indices.emplace_back(2U);

	const size_t positionDataSize{ sizeof(glm::vec3) * positions.size() };
	const size_t colorDataSize{ sizeof(glm::vec3) * positions.size() };

	std::shared_ptr<HyperFast::Buffer> pPositionBuffer{ _createVertexBuffer(positionDataSize) };
	std::shared_ptr<HyperFast::Buffer> pColorBuffer{ _createVertexBuffer(colorDataSize) };

	std::shared_ptr<HyperFast::Memory> pPositionMemory{ _createVertexMemory(pPositionBuffer->getMemoryRequirements()) };
	std::shared_ptr<HyperFast::Memory> pColorMemory{ _createVertexMemory(pColorBuffer->getMemoryRequirements()) };

	std::memcpy(pPositionMemory->map(), positions.data(), positionDataSize);
	std::memcpy(pColorMemory->map(), colors.data(), colorDataSize);

	pPositionMemory->unmap();
	pColorMemory->unmap();

	pPositionBuffer->bindMemory(pPositionMemory, 0ULL);
	pColorBuffer->bindMemory(pColorMemory, 0ULL);

	__pMesh = _createMesh();
	__pMesh->setPositionBuffer(pPositionBuffer);
	__pMesh->setColorBuffer(pColorBuffer);

	__pSubmesh = _createSubmesh(__pMesh);
	__pSubmesh->setDrawCommand();
}