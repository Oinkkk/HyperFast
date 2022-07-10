#pragma once

#include "Submesh.h"
#include <unordered_set>

namespace HyperFast
{
	class IndirectBufferBuilder : public Infra::Unique
	{
	public:
		IndirectBufferBuilder(
			Vulkan::Device &device, HyperFast::BufferManager &bufferManager,
			HyperFast::MemoryManager &memoryManager) noexcept;

		void addSubmesh(Submesh &submesh) noexcept;
		void removeSubmesh(Submesh &submesh) noexcept;

		[[nodiscard]]
		constexpr bool isEmpty() const noexcept;

		void validate();
		void draw(VkCommandBuffer commandBuffer) noexcept;

		[[nodiscard]]
		constexpr Infra::EventView<IndirectBufferBuilder &> &getIndirectBufferUpdateEvent() noexcept;

		[[nodiscard]]
		constexpr Infra::EventView<IndirectBufferBuilder &> &getIndirectBufferCreateEvent() noexcept;

	private:
		Vulkan::Device &__device;
		HyperFast::BufferManager &__bufferManager;
		HyperFast::MemoryManager &__memoryManager;

		std::unordered_set<Submesh *> __submeshes;
		std::vector<VkDrawIndexedIndirectCommand> __hostBuffer;

		std::unique_ptr<Buffer> __pIndirectBuffer;
		std::shared_ptr<Memory> __pIndirectMemory;
		uint32_t __drawCount{};
		uint32_t __maxDrawCount{};

		std::unique_ptr<Buffer> __pCountBuffer;
		std::shared_ptr<Memory> __pCountMemory;

		bool __needToUpdate{};

		std::shared_ptr<Infra::EventListener<Submesh &>> __pSubmeshDrawCommandChangeEventListener;
		std::shared_ptr<Infra::EventListener<Submesh &>> __pSubmeshVisibleChangeEventListener;
		std::shared_ptr<Infra::EventListener<Submesh &>> __pSubmeshDestroyEventListener;

		Infra::Event<IndirectBufferBuilder &> __indirectBufferUpdateEvent;
		Infra::Event<IndirectBufferBuilder &> __indirectBufferCreateEvent;

		void __initEventListeners() noexcept;
		void __update();

		void __createCountResources();
		void __createIndirectResources();

		void __updateHostBuffer() noexcept;

		void __onSubmeshDrawCommandChange(Submesh &submesh) noexcept;
		void __onSubmeshVisibleChange(Submesh &submesh) noexcept;
		void __onSubmeshDestroy(Submesh &submesh) noexcept;
	};

	constexpr bool IndirectBufferBuilder::isEmpty() const noexcept
	{
		return !__drawCount;
	}

	constexpr Infra::EventView<IndirectBufferBuilder &> &IndirectBufferBuilder::getIndirectBufferUpdateEvent() noexcept
	{
		return __indirectBufferUpdateEvent;
	}

	constexpr Infra::EventView<IndirectBufferBuilder &> &IndirectBufferBuilder::getIndirectBufferCreateEvent() noexcept
	{
		return __indirectBufferCreateEvent;
	}
}