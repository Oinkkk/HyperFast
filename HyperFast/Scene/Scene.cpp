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

	[[nodiscard]]
	std::shared_ptr<HyperFast::Buffer> Scene::_createVertexBuffer(const VkDeviceSize memSize, const void *const pData)
	{
		HyperFast::BufferManager &bufferManager{ __renderingEngine.getBufferManager() };
		return std::make_shared<HyperFast::Buffer>(
			bufferManager, memSize, VkBufferUsageFlagBits::VK_BUFFER_USAGE_VERTEX_BUFFER_BIT);
	}
}