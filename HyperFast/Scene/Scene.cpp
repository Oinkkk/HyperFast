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

	std::shared_ptr<HyperFast::Buffer> Scene::_createVertexBuffer(const void *const pData, const VkDeviceSize size) const
	{
		const VkBufferUsageFlags bufferUsage
		{
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT |
			VkBufferUsageFlagBits::VK_BUFFER_USAGE_TRANSFER_DST_BIT
		};

		const VkMemoryPropertyFlags memProps
		{
			VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT
		};

		const std::shared_ptr<HyperFast::Buffer> pBuffer
		{
			__renderingEngine.createBuffer(size, bufferUsage)
		};

		const std::shared_ptr<HyperFast::Memory> pMemory
		{
			__renderingEngine.createMemory(
				pBuffer->getMemoryRequirements(), memProps, true)
		};

		pBuffer->bindMemory(pMemory, 0ULL);
		//__renderingEngine.copyBuffer(VK_PIPELINE_STAGE_2_VERTEX_SHADER_BIT, VK_ACCESS_2_VERTEX_ATTRIBUTE_READ_BIT)

		return pBuffer;
	}

	std::shared_ptr<HyperFast::Buffer> Scene::_createIndexBuffer(const void *const pData, const VkDeviceSize size) const
	{

	}

	std::shared_ptr<HyperFast::Buffer> Scene::_createBuffer(
		const VkDeviceSize size, const VkBufferUsageFlags usage) const
	{
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

	void Scene::_onProcess(const float deltaTime)
	{}
}