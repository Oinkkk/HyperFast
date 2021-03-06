#include "Scene.h"

namespace Jin
{
	Scene::Scene(HyperFast::RenderingEngine &renderingEngine) noexcept :
		__renderingEngine{ renderingEngine }, __pDrawcall{ renderingEngine.createDrawcall() }
	{}

	void Scene::process(const float deltaTime)
	{
		_onProcess(deltaTime);
	}

	std::unique_ptr<HyperFast::Buffer> Scene::_createBuffer(
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
		std::shared_ptr<HyperFast::Submesh> pRetVal
		{
			std::make_shared<HyperFast::Submesh>(pMesh)
		};

		__pDrawcall->addSubmesh(*pRetVal);

		return pRetVal;
	}

	void Scene::_bindScreen(HyperFast::Screen &screen) noexcept
	{
		screen.setDrawcall(__pDrawcall.get());
	}

	void Scene::_onProcess(const float deltaTime)
	{}
}