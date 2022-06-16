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
			1. memoryTypeBits�� ����Ű�� Ÿ�� �� �̹� �Ҵ�� ��ũ�� �ִ� ��� �ű⼭ �ɰ��� ��ȯ
			2. ��ũ�� �뷮���� ���ĵ� �޸� ���׸�Ʈ�� �����ؾ� ��
			3. ��ũ�� ���ٸ� memoryTypeBits�� ����Ű�� ��� memory type ��ȸ
			4. �ش� memory type�� heap index�� ������� budget ���� (�޸� �Ҵ� ��������)
			5. �Ҵ� �����ϴٸ� ���� �Ҵ�. �޸� �뷮�� �⺻ ��ũ ũ�⺸�� ū ��� �ش� ũ�⿡ ���߾� ������ �Ҵ�(3�� �����ϴ��� Ȯ��)
			6. 1���� �ݺ�
		*/

		return nullptr;
	}
}