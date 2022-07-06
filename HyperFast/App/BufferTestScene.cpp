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

	if (__submeshTimer1 > 1000.0f)
	{
		__pSubmesh1->setVisible(!__pSubmesh1->isVisible());
		__submeshTimer1 -= 1000.0f;
	}

	if (__submeshTimer2 > 2000.0f)
	{
		__pSubmesh2->setVisible(!__pSubmesh2->isVisible());
		__submeshTimer2 -= 2000.0f;
	}
}

void BufferTestScene::__createMesh()
{
	std::shared_ptr<std::vector<glm::vec3>> pPositions{ std::make_shared<std::vector<glm::vec3>>() };
	pPositions->emplace_back(-0.5f, -0.5f, 0.5f);
	pPositions->emplace_back(-0.5f, 0.5f, 0.5f);
	pPositions->emplace_back(0.5f, 0.5f, 0.5f);
	pPositions->emplace_back(0.5f, -0.5f, 0.5f);

	std::shared_ptr<std::vector<glm::vec4>> pColors{ std::make_shared<std::vector<glm::vec4>>() };
	pColors->emplace_back(1.0f, 0.0f, 0.0f, 1.0f);
	pColors->emplace_back(0.0f, 1.0f, 0.0f, 1.0f);
	pColors->emplace_back(0.0f, 0.0f, 1.0f, 1.0f);
	pColors->emplace_back(1.0f, 1.0f, 0.0f, 1.0f);

	std::shared_ptr<std::vector<uint16_t>> pIndices{ std::make_shared<std::vector<uint16_t>>() };
	pIndices->emplace_back(0U);
	pIndices->emplace_back(1U);
	pIndices->emplace_back(2U);
	pIndices->emplace_back(0U);
	pIndices->emplace_back(2U);
	pIndices->emplace_back(3U);

	const size_t positionDataSize{ sizeof(glm::vec3) * pPositions->size() };
	const size_t colorDataSize{ sizeof(glm::vec4) * pColors->size() };
	const size_t indexDataSize{ sizeof(uint16_t) * pIndices->size() };

	const VkBufferCopy positionCopy
	{
		.size = positionDataSize
	};

	const VkBufferCopy colorCopy
	{
		.size = colorDataSize
	};

	const VkBufferCopy indexCopy
	{
		.size = indexDataSize
	};

	std::shared_ptr<HyperFast::Buffer> pPositionBuffer
	{
		_createBuffer(
			positionDataSize,
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT)
	};

	std::shared_ptr<HyperFast::Buffer> pColorBuffer
	{
		_createBuffer(
			colorDataSize,
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT)
	};

	std::shared_ptr<HyperFast::Buffer> pIndexBuffer
	{
		_createBuffer(
			indexDataSize,
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDEX_BUFFER_BIT |
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT)
	};

	const std::shared_ptr<HyperFast::Memory> pPositionMemory
	{
		_createMemory(
			pPositionBuffer->getMemoryRequirements(),
			VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	};

	const std::shared_ptr<HyperFast::Memory> pColorMemory
	{
		_createMemory(
			pColorBuffer->getMemoryRequirements(),
			VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	};

	const std::shared_ptr<HyperFast::Memory> pIndexMemory
	{
		_createMemory(
			pIndexBuffer->getMemoryRequirements(),
			VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT)
	};

	pPositionBuffer->bindMemory(pPositionMemory, 0ULL);
	pColorBuffer->bindMemory(pColorMemory, 0ULL);
	pIndexBuffer->bindMemory(pIndexMemory, 0ULL);

	_copyVertexBuffer(
		pPositionBuffer->getHandle(), pPositions->data(),
		positionDataSize, 1U, &positionCopy, pPositions);

	_copyVertexBuffer(
		pColorBuffer->getHandle(), pColors->data(),
		colorDataSize, 1U, &colorCopy, pColors);

	_copyIndexBuffer(
		pIndexBuffer->getHandle(), pIndices->data(),
		indexDataSize, 1U, &indexCopy, pIndices);

	__pMesh = _createMesh();
	__pMesh->setPositionBuffer(pPositionBuffer);
	__pMesh->setColorBuffer(pColorBuffer);
	__pMesh->setIndexBuffer(pIndexBuffer, VkIndexType::VK_INDEX_TYPE_UINT16);

	__pSubmesh1 = _createSubmesh(__pMesh);
	__pSubmesh1->setDrawCommand(3U, 1U, 0U, 0, 0U);

	__pSubmesh2 = _createSubmesh(__pMesh);
	__pSubmesh2->setDrawCommand(3U, 1U, 3U, 0, 0U);

	// TODO: indirect buffer도 스테이징 버퍼 사용
	// TODO: buffer delete 지연되어 처리되도록
}