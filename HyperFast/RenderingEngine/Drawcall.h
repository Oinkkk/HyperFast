#pragma once

#include "CommandBufferManager.h"
#include "IndirectBufferBuilder.h"
#include <unordered_map>
#include <unordered_set>

namespace HyperFast
{
	class Drawcall : public Infra::Unique
	{
	public:
		Drawcall(
			Vulkan::Device &device, const uint32_t queueFamilyIndex,
			BufferManager &bufferManager, MemoryManager &memoryManager) noexcept;

		void addSubmesh(Submesh &submesh) noexcept;
		void removeSubmesh(Submesh &submesh) noexcept;

		void update();
		void draw(Vulkan::CommandBuffer &commandBuffer) noexcept;

		void addSemaphoreDependency(const std::shared_ptr<SemaphoreDependency> &pDependency) noexcept;

		[[nodiscard]]
		constexpr Infra::EventView<Drawcall &> &getMeshBufferChangeEvent() noexcept;

		[[nodiscard]]
		constexpr Infra::EventView<Drawcall &> &getIndirectBufferUpdateEvent() noexcept;

		[[nodiscard]]
		constexpr Infra::EventView<Drawcall &> &getIndirectBufferCreateEvent() noexcept;

	private:
		Vulkan::Device &__device;
		const uint32_t __queueFamilyIndex;
		BufferManager &__bufferManager;
		MemoryManager &__memoryManager;

		bool __meshBufferChanged{};
		bool __indirectBufferCreated{};
		bool __indirectBufferUpdated{};

		std::unordered_map<Mesh *, std::unique_ptr<IndirectBufferBuilder>> __mesh2BuilderMap;

		std::shared_ptr<Infra::EventListener<Mesh &>> __pMeshBufferChangeEventListener;
		std::shared_ptr<Infra::EventListener<Mesh &>> __pMeshDestroyEventListener;
		std::shared_ptr<Infra::EventListener<Submesh &>> __pSubmeshDestroyEventListener;
		std::shared_ptr<Infra::EventListener<IndirectBufferBuilder &>> __pIndirectBufferUpdateEventListener;
		std::shared_ptr<Infra::EventListener<IndirectBufferBuilder &>> __pIndirectBufferCreateEventListener;

		Infra::Event<Drawcall &> __meshBufferChangeEvent;
		Infra::Event<Drawcall &> __indirectBufferCreateEvent;
		Infra::Event<Drawcall &> __indirectBufferUpdateEvent;

		void __initEventListeners() noexcept;
		void __updateIndirectBufferBuilders() noexcept;

		void __onMeshBufferChange(Mesh &mesh) noexcept;
		void __onMeshDestroy(Mesh &submesh) noexcept;
		void __onSubmeshDestroy(Submesh &submesh) noexcept;

		void __onIndirectBufferUpdate(IndirectBufferBuilder &builder) noexcept;
		void __onIndirectBufferCreate(IndirectBufferBuilder &builder) noexcept;
	};

	constexpr Infra::EventView<Drawcall &> &Drawcall::getMeshBufferChangeEvent() noexcept
	{
		return __meshBufferChangeEvent;
	}

	constexpr Infra::EventView<Drawcall &> &Drawcall::getIndirectBufferUpdateEvent() noexcept
	{
		return __indirectBufferUpdateEvent;
	}

	constexpr Infra::EventView<Drawcall &> &Drawcall::getIndirectBufferCreateEvent() noexcept
	{
		return __indirectBufferCreateEvent;
	}
}