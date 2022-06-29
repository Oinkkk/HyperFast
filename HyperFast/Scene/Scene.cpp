#include "Scene.h"

namespace Jin
{
	Scene::Scene(HyperFast::RenderingEngine &renderingEngine) noexcept :
		__renderingEngine{ renderingEngine }
	{

	}

	Scene::~Scene() noexcept
	{

	}

	std::shared_ptr<HyperFast::Buffer> Scene::_createBuffer(
		const VkDeviceSize dataSize, const VkBufferUsageFlags usage) const
	{
		HyperFast::BufferManager &bufferManager{ __renderingEngine.getBufferManager() };
		return std::make_shared<HyperFast::Buffer>(bufferManager, dataSize, usage);
	}

	std::shared_ptr<HyperFast::Memory> Scene::_createMemory(const VkMemoryRequirements &memRequirements) const
	{
		HyperFast::MemoryManager &memoryManager{ __renderingEngine.getMemoryManager() };

		const VkMemoryPropertyFlags requiredProps
		{
			VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		};
		
		return std::make_shared<HyperFast::Memory>(memoryManager, memRequirements, requiredProps, true);
	}

	std::shared_ptr<HyperFast::Mesh> Scene::_createMesh() noexcept
	{
		return __renderingEngine.createMesh();
	}

	std::shared_ptr<HyperFast::Submesh>
		Scene::_createSubmesh(const std::shared_ptr<HyperFast::Mesh> &pMesh) noexcept
	{
		std::shared_ptr<HyperFast::Submesh> pRetVal{ __renderingEngine.createSubmesh(pMesh) };
		__drawcall.addSubmesh(*pRetVal);

		return pRetVal;
	}

	void Scene::_bindScreen(HyperFast::Screen &screen) noexcept
	{
		screen.setDrawcall(&__drawcall);
	}
}