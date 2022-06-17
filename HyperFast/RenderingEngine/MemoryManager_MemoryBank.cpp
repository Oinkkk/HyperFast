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
			1. memoryTypeBits�� ����Ű�� Ÿ�� �� �̹� �Ҵ�� ��ũ�� �ִ� ��� �ű⼭ �ɰ��� ��ȯ
			2. ��ũ�� �뷮���� ���ĵ� �޸� ���׸�Ʈ�� �����ؾ� ��
			3. ��ũ�� ���ٸ� memoryTypeBits�� ����Ű�� ��� memory type ��ȸ
			4. �ش� memory type�� heap index�� ������� budget ���� (�޸� �Ҵ� ��������)
			5. �Ҵ� �����ϴٸ� ���� �Ҵ�. �޸� �뷮�� �⺻ ��ũ ũ�⺸�� ū ��� �ش� ũ�⿡ ���߾� ������ �Ҵ�(3�� �����ϴ��� Ȯ��)
			6. 1���� �ݺ�
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

