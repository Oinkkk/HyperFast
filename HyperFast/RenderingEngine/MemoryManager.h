#pragma once

#include "../Infrastructure/Unique.h"
#include "../VulkanLoader/VulkanLoader.h"
#include <unordered_map>
#include <memory>
#include <optional>
#include <map>

namespace HyperFast
{
	class MemoryManager final : public Infra::Unique
	{
	public:
		class MemoryBank
		{
		public:
			class MemorySegment
			{
			public:
				const VkDeviceAddress offset;
				const VkDeviceSize size;

				constexpr MemorySegment(const VkDeviceAddress offset, const VkDeviceSize size) noexcept;
			};

			MemoryBank(
				const VkDevice device, const VKL::DeviceProcedure &deviceProc,
				const uint32_t memoryTypeIndex, const VkDeviceSize size);

			~MemoryBank() noexcept;

			[[nodiscard]]
			constexpr VkDeviceMemory getHandle() const noexcept;

			[[nodiscard]]
			std::optional<MemorySegment> findSegment(
				const VkDeviceSize memSize, const VkDeviceSize alignment) const noexcept;

			// offset
			[[nodiscard]]
			VkDeviceAddress allocate(const MemorySegment &segment) noexcept;
			void free(const VkDeviceAddress offset) noexcept;

			[[nodiscard]]
			void *map();

		private:
			const VkDevice __device;
			const VKL::DeviceProcedure &__deviceProc;
			const uint32_t __memoryTypeIndex;
			const VkDeviceSize __size;

			VkDeviceMemory __handle{};
			std::map<VkDeviceAddress, VkDeviceSize> __segmentMap;

			void *__mapped{};

			void __allocateBank();
			void __freeBank() noexcept;
			void __unmap() noexcept;
		};

		class MemoryImpl final : public Infra::Unique
		{
		public:
			MemoryImpl(MemoryBank &bank, const MemoryBank::MemorySegment &segment) noexcept;
			~MemoryImpl() noexcept;

			[[nodiscard]]
			constexpr VkDeviceMemory getBank() const noexcept;

			[[nodiscard]]
			constexpr VkDeviceSize getSize() const noexcept;

			[[nodiscard]]
			constexpr VkDeviceAddress getOffset() const noexcept;

			[[nodiscard]]
			void *map();
			void unmap() noexcept;

		private:
			MemoryBank &__bank;
			const VkDeviceSize __size;
			const VkDeviceAddress __offset;
		};

		MemoryManager(
			const VkPhysicalDevice physicalDevice, const VKL::InstanceProcedure &instanceProc,
			const VkDevice device, const VKL::DeviceProcedure &deviceProc) noexcept;

		[[nodiscard]]
		MemoryImpl *create(
			const VkMemoryRequirements &memRequirements,
			const VkMemoryPropertyFlags requiredProps, const bool linearity);
		
		void destroy(MemoryImpl *const pImpl) noexcept;

	private:
		const VkPhysicalDevice __physicalDevice;
		const VKL::InstanceProcedure &__instanceProc;

		const VkDevice __device;
		const VKL::DeviceProcedure &__deviceProc;

		VkPhysicalDeviceMemoryProperties2 __deviceMemProps2{};
		VkPhysicalDeviceMemoryBudgetPropertiesEXT __deviceMemBudget{};

		// key: memoryTypeIndex
		std::unordered_map<uint32_t, std::vector<std::unique_ptr<MemoryBank>>> __linearBankListMap;
		std::unordered_map<uint32_t, std::vector<std::unique_ptr<MemoryBank>>> __tilingBankListMap;

		static constexpr inline VkDeviceSize __linearBankSize{ 16ULL << 20ULL };
		static constexpr inline VkDeviceSize __tilingBankSize{ 64ULL << 20ULL };

		constexpr void __setupDeviceMemProps() noexcept;
		void __updateDeviceMemProps() noexcept;
	};

	constexpr MemoryManager::MemoryBank::MemorySegment::MemorySegment(
		const VkDeviceAddress offset, const VkDeviceSize size) noexcept :
		offset{ offset }, size{ size }
	{}

	constexpr void MemoryManager::__setupDeviceMemProps() noexcept
	{
		__deviceMemProps2.sType = VkStructureType::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_PROPERTIES_2;
		__deviceMemProps2.pNext = &__deviceMemBudget;

		__deviceMemBudget.sType = VkStructureType::VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_MEMORY_BUDGET_PROPERTIES_EXT;
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