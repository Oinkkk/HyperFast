#include "MemoryManager.h"

namespace HyperFast
{
	MemoryManager::MemoryBank::MemoryBank(
		const VkDevice device, const VKL::DeviceProcedure &deviceProc,
		const uint32_t memoryTypeIndex, const VkDeviceSize size) :
		__device{ device }, __deviceProc{ deviceProc },
		__memoryTypeIndex{ memoryTypeIndex }, __size{ size }
	{
		__allocateBank();
	}

	MemoryManager::MemoryBank::~MemoryBank() noexcept
	{
		__freeBank();
	}

	bool MemoryManager::MemoryBank::isAllocatable(
		const VkDeviceSize memSize, const VkDeviceSize alignment, const bool linearity) const noexcept
	{

	}

	VkDeviceAddress MemoryManager::MemoryBank::allocate(
		const VkDeviceSize memSize, const VkDeviceSize alignment, const bool linearity)
	{
		/*
			1. memoryTypeBits가 가리키는 타입 중 이미 할당된 뱅크가 있는 경우 거기서 쪼개서 반환
			2. 뱅크는 용량으로 정렬된 메모리 세그먼트를 제공해야 함
			3. 뱅크가 없다면 memoryTypeBits가 가리키는 모든 memory type 순회
			4. 해당 memory type의 heap index를 기반으로 budget 조사 (메모리 할당 가능한지)
			5. 할당 가능하다면 새로 할당. 메모리 용량이 기본 뱅크 크기보다 큰 경우 해당 크기에 맞추어 온전히 할당(3을 만족하는지 확인)
			6. 1부터 반복
		*/
	}

	void MemoryManager::MemoryBank::free(const VkDeviceAddress offset) noexcept
	{
		
	}

	void MemoryManager::MemoryBank::__allocateBank()
	{
		const VkMemoryAllocateInfo allocInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
			.allocationSize = __size,
			.memoryTypeIndex = __memoryTypeIndex
		};

		__deviceProc.vkAllocateMemory(__device, &allocInfo, nullptr, &__handle);
		if (!__handle)
			throw std::exception{ "Cannot create a VkDeviceMemory." };
	}

	void MemoryManager::MemoryBank::__freeBank() noexcept
	{
		__deviceProc.vkFreeMemory(__device, __handle, nullptr);
	}
}

