#include "MemoryManager.h"

namespace HyperFast
{
	MemoryManager::MemoryManager(
		const VkPhysicalDevice physicalDevice, const VKL::InstanceProcedure &instanceProc,
		const VkDevice device, const VKL::DeviceProcedure &deviceProc) noexcept :
		__physicalDevice{ physicalDevice }, __instanceProc{ instanceProc },
		__device{ device }, __deviceProc{ deviceProc }
	{
		__setupDeviceMemProps();
	}

	MemoryManager::MemoryImpl *MemoryManager::create(
		const VkMemoryRequirements &memRequirements,
		const VkMemoryPropertyFlags requiredProps, const bool linearity)
	{
		__updateDeviceMemProps();

		for (
			uint32_t memoryTypeIndex = 0U;
			memoryTypeIndex < __deviceMemProps2.memoryProperties.memoryTypeCount;
			memoryTypeIndex++)
		{
			if (!(memRequirements.memoryTypeBits & (1U << memoryTypeIndex)))
				continue;

			const VkMemoryType &memoryType
			{
				__deviceMemProps2.memoryProperties.memoryTypes[memoryTypeIndex]
			};

			if ((memoryType.propertyFlags & requiredProps) != requiredProps)
				continue;

			std::unordered_map<uint32_t, std::vector<std::unique_ptr<MemoryBank>>> &targetBankListMap
			{
				linearity ? __linearBankListMap : __tilingBankListMap
			};

			const VkDeviceSize targetBankSize{ linearity ? __linearBankSize : __tilingBankSize };

			std::vector<std::unique_ptr<MemoryBank>> &memoryBankList{ targetBankListMap[memoryTypeIndex] };
			if (memoryBankList.empty())
			{
				if (__deviceMemBudget.heapBudget[memoryType.heapIndex] < memRequirements.size)
					continue;

				std::unique_ptr<MemoryBank> pNewBank
				{
					std::make_unique<MemoryBank>(
						__device, __deviceProc, memoryTypeIndex,
						std::max(targetBankSize, memRequirements.size))
				};

				memoryBankList.emplace_back(std::move(pNewBank));
			}

			MemoryBank *const pBank{ memoryBankList.back().get() };
			const std::optional<MemoryBank::MemorySegment> segment
			{
				pBank->findSegment(memRequirements.size, memRequirements.alignment)
			};

			if (segment.has_value())
				return new MemoryImpl{ *pBank, segment.value() };
		}

		throw std::exception{ "Cannot create a memory." };
	}

	void MemoryManager::destroy(MemoryImpl *const pImpl) noexcept
	{
		delete pImpl;
	}

	void MemoryManager::__updateDeviceMemProps() noexcept
	{
		__instanceProc.vkGetPhysicalDeviceMemoryProperties2(__physicalDevice, &__deviceMemProps2);
	}
}