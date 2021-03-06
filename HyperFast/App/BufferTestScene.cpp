#include "BufferTestScene.h"
#include <glm/glm.hpp>
#include <glm/gtc/type_aligned.hpp>

BufferTestScene::BufferTestScene(
	HyperFast::RenderingEngine &renderingEngine,
	HyperFast::Screen &screen1, HyperFast::Screen &screen2) noexcept :
	Scene{ renderingEngine }, __screen1{ screen1 }, __screen2{ screen2 }
{
	_bindScreen(__screen1);
	_bindScreen(__screen2);
	__createMesh();
}

BufferTestScene::~BufferTestScene() noexcept
{
	__screen1.setDrawcall(nullptr);
	__screen2.setDrawcall(nullptr);

	__pSubmesh2 = nullptr;
	__pSubmesh1 = nullptr;
	__pMesh = nullptr;
}

void BufferTestScene::_onProcess(const float deltaTime)
{
	__submeshTimer1 += deltaTime;
	__submeshTimer2 += deltaTime;

	if (__submeshTimer1 > 500.0f)
	{
		__pSubmesh1->setVisible(!__pSubmesh1->isVisible());
		__submeshTimer1 -= 500.0f;
	}

	if (__submeshTimer2 > 1000.0f)
	{
		__pSubmesh2->setVisible(!__pSubmesh2->isVisible());
		__submeshTimer2 -= 1000.0f;
	}
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

	std::unique_ptr<HyperFast::Buffer> pPositionBuffer
	{
		_createBuffer(
			positionDataSize,
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
	};

	std::unique_ptr<HyperFast::Buffer> pColorBuffer
	{
		_createBuffer(
			colorDataSize,
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT)
	};

	std::unique_ptr<HyperFast::Buffer> pIndexBuffer
	{
		_createBuffer(
			indexDataSize,
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT)
	};

	const VkMemoryPropertyFlags memProps
	{
		VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
		VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
	};

	std::shared_ptr<HyperFast::Memory> pPositionMemory{ _createMemory(pPositionBuffer->getMemoryRequirements(), memProps) };
	std::shared_ptr<HyperFast::Memory> pColorMemory{ _createMemory(pColorBuffer->getMemoryRequirements(), memProps) };
	std::shared_ptr<HyperFast::Memory> pIndexMemory{ _createMemory(pIndexBuffer->getMemoryRequirements(), memProps) };

	std::memcpy(pPositionMemory->map(), positions.data(), positionDataSize);
	std::memcpy(pColorMemory->map(), colors.data(), colorDataSize);
	std::memcpy(pIndexMemory->map(), indices.data(), indexDataSize);

	pPositionBuffer->bindMemory(pPositionMemory, 0ULL);
	pColorBuffer->bindMemory(pColorMemory, 0ULL);
	pIndexBuffer->bindMemory(pIndexMemory, 0ULL);

	__pMesh = _createMesh();
	__pMesh->setPositionBuffer(std::move(pPositionBuffer));
	__pMesh->setColorBuffer(std::move(pColorBuffer));
	__pMesh->setIndexBuffer(std::move(pIndexBuffer), VkIndexType::VK_INDEX_TYPE_UINT16);

	__pSubmesh1 = _createSubmesh(__pMesh);
	__pSubmesh1->setDrawCommand(3U, 1U, 0U, 0, 0U);

	__pSubmesh2 = _createSubmesh(__pMesh);
	__pSubmesh2->setDrawCommand(3U, 1U, 3U, 0, 0U);
}