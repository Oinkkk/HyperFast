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
	{}

	std::shared_ptr<HyperFast::Buffer> Scene::_createBuffer(
		const VkDeviceSize size, const VkBufferUsageFlags usage) const
	{
		return __renderingEngine.createBuffer(size, usage);
	}

	std::shared_ptr<HyperFast::Memory> Scene::_createMemory(const VkMemoryRequirements &memRequirements) const
	{
		const VkMemoryPropertyFlags requiredProps
		{
			VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		};
		
		return __renderingEngine.createMemory(memRequirements, requiredProps, true);
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

	void Scene::_bindScreen(HyperFast::Screen &screen) noexcept
	{
		screen.setDrawcall(__pDrawcall.get());
	}
}