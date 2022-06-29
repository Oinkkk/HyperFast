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
	_bindScreen(*__pScreen1);
	_bindScreen(*__pScreen2);
	__createMesh();
}

BufferTestScene::~BufferTestScene() noexcept
{
	__pScreen1->setDrawcall(nullptr);
	__pScreen2->setDrawcall(nullptr);
}

void BufferTestScene::__createMesh()
{
	std::vector<glm::vec3> positions;
	positions.emplace_back(-0.5f, -0.5f, 0.5f);
	positions.emplace_back(-0.5f, 0.5f, 0.5f);
	positions.emplace_back(0.5f, 0.5f, 0.5f);
	positions.emplace_back(0.5f, -0.5f, 0.5f);

	std::vector<glm::vec4> colors;
	colors.emplace_back(1.0f, 0.0f, 0.0f, 1.0f);
	colors.emplace_back(0.0f, 1.0f, 0.0f, 1.0f);
	colors.emplace_back(0.0f, 0.0f, 1.0f, 1.0f);
	colors.emplace_back(1.0f, 1.0f, 0.0f, 1.0f);

	std::vector<uint16_t> indices;
	indices.emplace_back(0U);
	indices.emplace_back(1U);
	indices.emplace_back(2U);
	indices.emplace_back(0U);
	indices.emplace_back(2U);
	indices.emplace_back(3U);

	const size_t positionDataSize{ sizeof(glm::vec3) * positions.size() };
	const size_t colorDataSize{ sizeof(glm::vec4) * colors.size() };
	const size_t indexDataSize{ sizeof(uint16_t) * indices.size() };

	std::shared_ptr<HyperFast::Buffer> pPositionBuffer
	{
		_createBuffer(positionDataSize, VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
	};

	std::shared_ptr<HyperFast::Buffer> pColorBuffer
	{
		_createBuffer(colorDataSize, VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
	};

	std::shared_ptr<HyperFast::Buffer> pIndexBuffer
	{
		_createBuffer(indexDataSize, VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
	};

	std::shared_ptr<HyperFast::Memory> pPositionMemory{ _createMemory(pPositionBuffer->getMemoryRequirements()) };
	std::shared_ptr<HyperFast::Memory> pColorMemory{ _createMemory(pColorBuffer->getMemoryRequirements()) };
	std::shared_ptr<HyperFast::Memory> pIndexMemory{ _createMemory(pIndexBuffer->getMemoryRequirements()) };

	std::memcpy(pPositionMemory->map(), positions.data(), positionDataSize);
	std::memcpy(pColorMemory->map(), colors.data(), colorDataSize);
	std::memcpy(pIndexMemory->map(), indices.data(), indexDataSize);

	pIndexMemory->unmap();
	pPositionMemory->unmap();
	pColorMemory->unmap();

	pPositionBuffer->bindMemory(pPositionMemory, 0ULL);
	pColorBuffer->bindMemory(pColorMemory, 0ULL);
	pIndexBuffer->bindMemory(pIndexMemory, 0ULL);

	__pMesh = _createMesh();
	__pMesh->setPositionBuffer(pPositionBuffer);
	__pMesh->setColorBuffer(pColorBuffer);
	__pMesh->setIndexBuffer(pIndexBuffer, VkIndexType::VK_INDEX_TYPE_UINT16);

	__pSubmesh1 = _createSubmesh(__pMesh);
	__pSubmesh1->setDrawCommand(3U, 1U, 0U, 0, 0U);

	__pSubmesh2 = _createSubmesh(__pMesh);
	__pSubmesh2->setDrawCommand(3U, 1U, 3U, 0, 0U);
}