#include "IndirectBufferBuilder.h"
#include <iostream>

namespace HyperFast
{
	IndirectBufferBuilder::IndirectBufferBuilder(
		Vulkan::Device &device, HyperFast::BufferManager &bufferManager,
		HyperFast::MemoryManager &memoryManager) noexcept :
		__device{ device }, __bufferManager{ bufferManager },
		__memoryManager{ memoryManager }
	{
		__initEventListeners();
		__createCountResources();
	}

	void IndirectBufferBuilder::addSubmesh(Submesh &submesh) noexcept
	{
		submesh.getDrawCommandChangeEvent() += __pSubmeshDrawCommandChangeEventListener;
		submesh.getVisibleChangeEvent() += __pSubmeshVisibleChangeEventListener;

		__submeshes.emplace(&submesh);

		if (submesh.isVisible())
			__needToUpdate = true;
	}

	void IndirectBufferBuilder::removeSubmesh(Submesh &submesh) noexcept
	{
		submesh.getDrawCommandChangeEvent() -= __pSubmeshDrawCommandChangeEventListener;
		submesh.getVisibleChangeEvent() -= __pSubmeshVisibleChangeEventListener;

		__submeshes.erase(&submesh);

		if (submesh.isVisible())
			__needToUpdate = true;
	}

	void IndirectBufferBuilder::update()
	{
		if (__needToUpdate)
		{
			__update();
			__needToUpdate = false;
		}
	}

	void IndirectBufferBuilder::draw(Vulkan::CommandBuffer &commandBuffer) noexcept
	{
		if (!__pIndirectBuffer)
			return;

		commandBuffer.vkCmdDrawIndexedIndirectCount(
			__pIndirectBuffer->getHandle(), 0ULL,
			__pCountBuffer->getHandle(), 0ULL,
			__maxDrawCount, sizeof(VkDrawIndexedIndirectCommand));
	}

	void IndirectBufferBuilder::__initEventListeners() noexcept
	{
		__pSubmeshDrawCommandChangeEventListener =
			Infra::EventListener<Submesh &>::bind(
				&IndirectBufferBuilder::__onSubmeshDrawCommandChange, this, std::placeholders::_1);

		__pSubmeshVisibleChangeEventListener =
			Infra::EventListener<Submesh &>::bind(
				&IndirectBufferBuilder::__onSubmeshVisibleChange, this, std::placeholders::_1);
	}

	void IndirectBufferBuilder::__update()
	{
		__updateHostBuffer();

		std::memcpy(__pCountMemory->map(), &__drawCount, sizeof(uint32_t));

		if (__drawCount)
		{
			const bool needToCreateBuffer{ __drawCount > __maxDrawCount };
			if (needToCreateBuffer)
				__createIndirectResources();

			std::memcpy(
				__pIndirectMemory->map(), __hostBuffer.data(),
				sizeof(VkDrawIndexedIndirectCommand) * __drawCount);

			if (needToCreateBuffer)
				__indirectBufferCreateEvent.invoke(*this);
		}

		__indirectBufferUpdateEvent.invoke(*this);
	}

	void IndirectBufferBuilder::__createCountResources()
	{
		__pCountBuffer = std::make_unique<Buffer>(
			__bufferManager, sizeof(uint32_t), VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);

		const VkMemoryPropertyFlags requiredMemProps
		{
			VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		};

		__pCountMemory = std::make_shared<Memory>(
			__memoryManager, __pCountBuffer->getMemoryRequirements(),
			requiredMemProps, true);

		__pCountBuffer->bindMemory(__pCountMemory, 0ULL);
	}

	void IndirectBufferBuilder::__createIndirectResources()
	{
		const VkDeviceSize bufferSize{ sizeof(VkDrawIndexedIndirectCommand) * __drawCount };

		__pIndirectBuffer = std::make_unique<Buffer>(
			__bufferManager, bufferSize, VkBufferUsageFlagBits::VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT);

		const VkMemoryPropertyFlags requiredMemProps
		{
			VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
			VkMemoryPropertyFlagBits::VK_MEMORY_PROPERTY_HOST_COHERENT_BIT
		};

		__pIndirectMemory = std::make_shared<Memory>(
			__memoryManager, __pIndirectBuffer->getMemoryRequirements(),
			requiredMemProps, true);

		__pIndirectBuffer->bindMemory(__pIndirectMemory, 0ULL);
		__maxDrawCount = __drawCount;
	}

	void IndirectBufferBuilder::__updateHostBuffer() noexcept
	{
		// TODO: 스테이징 버퍼 사용 시 host buffer race condition 해결 필요.
		__drawCount = 0U;
		__hostBuffer.clear();

		for (Submesh *const pSubmesh : __submeshes)
		{
			if (!(pSubmesh->isVisible()))
				continue;
				
			__hostBuffer.emplace_back(pSubmesh->getDrawCommand());
			__drawCount++;
		}
	}

	void IndirectBufferBuilder::__onSubmeshDrawCommandChange(Submesh &submesh) noexcept
	{
		__needToUpdate = true;
	}

	void IndirectBufferBuilder::__onSubmeshVisibleChange(Submesh &submesh) noexcept
	{
		__needToUpdate = true;
	}
}