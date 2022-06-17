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
		const VkDeviceSize memSize, const VkDeviceSize alignment,
		const uint32_t memoryTypeBits, const VkMemoryPropertyFlags requiredProps,
		const bool linearity)
	{
		__updateDeviceMemProps();

		MemoryManager::MemoryBank *const pBank
		{
			__getBank(memSize, alignment, memoryTypeBits, requiredProps, linearity)
		};

		if (!pBank)
			throw std::exception{ "Cannot retrieve a MemoryBank." };

		return new MemoryImpl{ *pBank, memSize, alignment, linearity };
	}

	void MemoryManager::destroy(MemoryImpl *const pImpl) noexcept
	{
		delete pImpl;
	}

	void MemoryManager::__updateDeviceMemProps() noexcept
	{
		__instanceProc.vkGetPhysicalDeviceMemoryProperties2(__physicalDevice, &__deviceMemProps2);
	}

	MemoryManager::MemoryBank *MemoryManager::__getBank(
		const VkDeviceSize memSize, const VkDeviceSize alignment,
		const uint32_t memoryTypeBits, const VkMemoryPropertyFlags requiredProps,
		const bool linearity)
	{
		MemoryBank *pRetVal{};

		for (
			uint32_t memoryTypeIndex = 0U;
			memoryTypeIndex < __deviceMemProps2.memoryProperties.memoryTypeCount;
			memoryTypeIndex++)
		{
			if (!(memoryTypeBits & (1U << memoryTypeIndex)))
				continue;

			const VkMemoryType &memoryType
			{
				__deviceMemProps2.memoryProperties.memoryTypes[memoryTypeIndex]
			};

			if ((memoryType.propertyFlags & requiredProps) != requiredProps)
				continue;

			std::vector<std::unique_ptr<MemoryBank>> &memoryBankList{ __memoryBankListMap[memoryTypeIndex] };
			if (memoryBankList.empty())
			{
				if (__deviceMemBudget.heapBudget[memoryType.heapIndex] < memSize)
					continue;

				std::unique_ptr<MemoryBank> pNewBank
				{
					std::make_unique<MemoryBank>(
						__device, __deviceProc, memoryTypeIndex,
						std::max(__defaultBankSize, memSize))
				};

				pRetVal = pNewBank.get();
				memoryBankList.emplace_back(std::move(pNewBank));
				break;
			}

			MemoryBank *const pBank{ memoryBankList.back().get() };
			if (pBank->isAllocatable(memSize, alignment, linearity))
			{
				pRetVal = pBank;
				break;
			}
		}

		return pRetVal;
	}
}