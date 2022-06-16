#include "MemoryManager.h"

namespace HyperFast
{
	MemoryManager::MemoryManager(
		const VkPhysicalDevice physicalDevice, const VKL::InstanceProcedure &instanceProc,
		const VkDevice device, const VKL::DeviceProcedure &deviceProc,
		const VkDeviceSize bufferImageGranularity) noexcept :
		__physicalDevice{ physicalDevice }, __instanceProc{ instanceProc },
		__device{ device }, __deviceProc{ deviceProc }, __bufferImageGranularity{ bufferImageGranularity }
	{
		__setupDeviceMemProps();
	}

	MemoryManager::MemoryImpl *MemoryManager::create(
		const VkDeviceSize memSize, const VkDeviceSize alignment, const uint32_t memoryTypeBits)
	{
		__updateDeviceMemProps();

		MemoryBank *const pMemoryBank{ __getMemoryBank(memSize, alignment, memoryTypeBits) };
		if (!pMemoryBank)
			throw std::exception{ "Cannot retrieve a MemoryBank." };

		return new MemoryImpl{ *pMemoryBank, memSize, alignment };
	}

	void MemoryManager::destroy(MemoryImpl *const pImpl) noexcept
	{
		delete pImpl;
	}

	void MemoryManager::__updateDeviceMemProps() noexcept
	{
		__instanceProc.vkGetPhysicalDeviceMemoryProperties2(__physicalDevice, &__deviceMemProps2);
	}

	MemoryManager::MemoryBank *MemoryManager::__getMemoryBank(
		const VkDeviceSize memSize, const VkDeviceSize alignment, const uint32_t memoryTypeBits)
	{
		for (
			uint32_t memoryTypeIter = 0U;
			memoryTypeIter < __deviceMemProps2.memoryProperties.memoryTypeCount;
			memoryTypeIter++)
		{
			if (memoryTypeBits & (1U << memoryTypeIter));
		}

		/*
			1. memoryTypeBits가 가리키는 타입 중 이미 할당된 뱅크가 있는 경우 거기서 쪼개서 반환
			2. 뱅크는 용량으로 정렬된 메모리 세그먼트를 제공해야 함
			3. 뱅크가 없다면 memoryTypeBits가 가리키는 모든 memory type 순회
			4. 해당 memory type의 heap index를 기반으로 budget 조사 (메모리 할당 가능한지)
			5. 할당 가능하다면 새로 할당. 메모리 용량이 기본 뱅크 크기보다 큰 경우 해당 크기에 맞추어 온전히 할당(3을 만족하는지 확인)
			6. 1부터 반복
		*/

		return nullptr;
	}
}