#include "Scene.h"

namespace Jin
{
	Scene::Scene(HyperFast::RenderingEngine &renderingEngine) noexcept :
		__renderingEngine{ renderingEngine }, __pDrawcall{ renderingEngine.createDrawcall() }
	{}

	Scene::~Scene() noexcept
	{

	}

	void Scene::process(const float deltaTime)
	{
		_onProcess(deltaTime);
		__pDrawcall->validate();
	}

	std::shared_ptr<HyperFast::Buffer> Scene::_createBuffer(
		const VkDeviceSize size, const VkBufferUsageFlags usage) const
	{
		return __renderingEngine.createBuffer(size, usage);
	}

	std::shared_ptr<HyperFast::Memory> Scene::_createMemory(
		const VkMemoryRequirements &memRequirements, const VkMemoryPropertyFlags propFlags) const
	{
		return __renderingEngine.createMemory(memRequirements, propFlags, true);
	}

	std::shared_ptr<HyperFast::Mesh> Scene::_createMesh() noexcept
	{
		return __renderingEngine.createMesh();
	}

	std::shared_ptr<HyperFast::Submesh>
		Scene::_createSubmesh(const std::shared_ptr<HyperFast::Mesh> &pMesh) noexcept
	{
		std::shared_ptr<HyperFast::Submesh> pRetVal{ __renderingEngine.createSubmesh(pMesh) };
		__pDrawcall->addSubmesh(*pRetVal);

		return pRetVal;
	}

	void Scene::_copyVertexBuffer(
		const VkBuffer dst, const void *const pSrc, const VkDeviceSize srcBufferSize,
		const uint32_t regionCount, const VkBufferCopy *const pRegions, const std::any &srcPlaceholder) noexcept
	{
		__renderingEngine.copyBuffer(
			VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE,
			VK_PIPELINE_STAGE_2_VERTEX_ATTRIBUTE_INPUT_BIT, VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT,
			dst, pSrc, srcBufferSize, regionCount, pRegions, srcPlaceholder);
	}

	void Scene::_copyIndexBuffer(
		const VkBuffer dst, const void *const pSrc, const VkDeviceSize srcBufferSize,
		const uint32_t regionCount, const VkBufferCopy *const pRegions, const std::any &srcPlaceholder) noexcept
	{
		__renderingEngine.copyBuffer(
			VK_PIPELINE_STAGE_2_NONE, VK_ACCESS_2_NONE,
			VK_PIPELINE_STAGE_2_INDEX_INPUT_BIT, VK_ACCESS_2_INDEX_READ_BIT,
			dst, pSrc, srcBufferSize, regionCount, pRegions, srcPlaceholder);
	}

	void Scene::_bindScreen(HyperFast::Screen &screen) noexcept
	{
		screen.setDrawcall(__pDrawcall.get());
	}

	void Scene::_onProcess(const float deltaTime)
	{}
}