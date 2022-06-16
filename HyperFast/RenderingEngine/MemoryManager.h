#pragma once

#include "../Infrastructure/Unique.h"
#include "../VulkanLoader/VulkanLoader.h"
#include <unordered_map>
#include <memory>

namespace HyperFast
{
	class MemoryManager final : public Infra::Unique
	{
	public:
		class MemoryBank;
		class MemoryImpl final : public Infra::Unique
		{
		public:
			MemoryImpl(MemoryBank &bank, const VkDeviceSize size, const VkDeviceSize alignment) noexcept;
			~MemoryImpl() noexcept;

			[[nodiscard]]
			constexpr VkDeviceMemory getBank() const noexcept;

			[[nodiscard]]
			constexpr VkDeviceSize getSize() const noexcept;

			[[nodiscard]]
			constexpr VkDeviceAddress getOffset() const noexcept;

		private:
			MemoryBank &__bank;
			const VkDeviceSize __size;
			const VkDeviceAddress __offset;
		};

		MemoryManager(
			const VkPhysicalDevice physicalDevice, const VKL::InstanceProcedure &instanceProc,
			const VkDevice device, const VKL::DeviceProcedure &deviceProc,
			const VkDeviceSize bufferImageGranularity) noexcept;

		[[nodiscard]]
		MemoryImpl *create(const VkDeviceSize memSize, const VkDeviceSize alignment, const uint32_t memoryTypeBits);
		void destroy(MemoryImpl *const pImpl) noexcept;

	private:
		class MemoryBank
		{
		public:
			MemoryBank(const VkDevice device, const VKL::DeviceProcedure &deviceProc, const VkDeviceSize size);
			~MemoryBank() noexcept;

			[[nodiscard]]
			constexpr VkDeviceMemory getHandle() const noexcept;

			// offset
			[[nodiscard]]
			VkDeviceAddress allocate(const VkDeviceSize memSize, const VkDeviceSize alignment);
			void free(const VkDeviceAddress offset) noexcept;

		private:
			const VkDevice __device;
			const VKL::DeviceProcedure &__deviceProc;
			const VkDeviceSize __size;

			VkDeviceMemory __handle{};

			void __allocateBank();
			void __freeBank() noexcept;
		};

		const VkPhysicalDevice __physicalDevice;
		const VKL::InstanceProcedure &__instanceProc;

		const VkDevice __device;
		const VKL::DeviceProcedure &__deviceProc;

		const VkDeviceSize __bufferImageGranularity;

		VkPhysicalDeviceMemoryProperties2 __deviceMemProps2{};
		VkPhysicalDeviceMemoryBudgetPropertiesEXT __deviceMemBudget{};

		std::unordered_map<uint32_t, std::vector<std::unique_ptr<MemoryBank>>> __memoryBankListMap;

		constexpr void __setupDeviceMemProps() noexcept;
		void __updateDeviceMemProps() noexcept;

		[[nodiscard]]
		MemoryBank *__getMemoryBank(
			const VkDeviceSize memSize, const VkDeviceSize alignment, const uint32_t memoryTypeBits);

		[[nodiscard]]
		constexpr bool __checkMemoryCapacity(const uint32_t heapIdx, const VkDeviceSize memSize) const noexcept;
	};

	constexpr void MemoryManager::__setupDeviceMemProps() noexcept
	{
		__deviceMemProps2.sType = VkStructureType::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
		__deviceMemProps2.pNext = &__deviceMemBudget;

		__deviceMemBudget.sType = VkStructureType::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT;
	}

	constexpr bool MemoryManager::__checkMemoryCapacity(const uint32_t heapIdx, const VkDeviceSize memSize) const noexcept
	{
		return (__deviceMemBudget.heapBudget[heapIdx] >= memSize);
	}

	constexpr VkDeviceMemory MemoryManager::MemoryImpl::getBank() const noexcept
	{
		return __bank.getHandle();
	}

	constexpr VkDeviceSize MemoryManager::MemoryImpl::getSize() const noexcept
	{
		return __size;
	}

	constexpr VkDeviceAddress MemoryManager::MemoryImpl::getOffset() const noexcept
	{
		return __offset;
	}

	constexpr VkDeviceMemory MemoryManager::MemoryBank::getHandle() const noexcept
	{
		return __handle;
	}
}