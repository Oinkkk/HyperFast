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

	std::shared_ptr<HyperFast::Buffer> Scene::_createVertexBuffer(const VkDeviceSize dataSize)
	{
		HyperFast::BufferManager &bufferManager{ __renderingEngine.getBufferManager() };
		return std::make_shared<HyperFast::Buffer>(
			bufferManager, dataSize, VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	}

	std::shared_ptr<HyperFast::Memory> Scene::_createVertexMemory(const VkMemoryRequirements &memRequirements)
	{
		HyperFast::MemoryManager &memoryManager{ __renderingEngine.getMemoryManager() };

		const VkMemoryPropertyFlags requiredProps
		{
			VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		};
		
		return std::make_shared<HyperFast::Memory>(memoryManager, memRequirements, requiredProps, true);
	}
}