#pragma once

#include "CommandBufferManager.h"
#include "IndirectBufferBuilder.h"
#include "LifeCycle.h"
#include <unordered_map>
#include <unordered_set>

namespace HyperFast
{
	class Drawcall : public Infra::Unique
	{
	public:
		Drawcall(
			Vulkan::Device &device, const uint32_t queueFamilyIndex,
			BufferManager &bufferManager, MemoryManager &memoryManager, LifeCycle &lifeCycle) noexcept;

		void addSubmesh(Submesh &submesh) noexcept;
		void removeSubmesh(Submesh &submesh) noexcept;

		void draw(const size_t segmentIndex, Vulkan::CommandBuffer &commandBuffer) noexcept;

		[[nodiscard]]
		constexpr size_t getNumSegments() const noexcept;

		[[nodiscard]]
		constexpr Infra::EventView<Drawcall &, size_t> &getMeshBufferChangeEvent() noexcept;

		[[nodiscard]]
		constexpr Infra::EventView<Drawcall &, size_t> &getIndirectBufferUpdateEvent() noexcept;

		[[nodiscard]]
		constexpr Infra::EventView<Drawcall &, size_t> &getIndirectBufferCreateEvent() noexcept;
	
	private:
		class DrawcallSegment : public Infra::Unique
		{
		public:
			DrawcallSegment(
				const size_t segmentIndex, Vulkan::Device &device, const uint32_t queueFamilyIndex,
				BufferManager &bufferManager, MemoryManager &memoryManager) noexcept;

			[[nodiscard]]
			constexpr size_t getSegmentIndex() const noexcept;

			void addSubmesh(Submesh &submesh) noexcept;
			void removeSubmesh(Submesh &submesh) noexcept;

			void update();
			void draw(Vulkan::CommandBuffer &commandBuffer) noexcept;

			[[nodiscard]]
			constexpr Infra::EventView<DrawcallSegment &> &getMeshBufferChangeEvent() noexcept;

			[[nodiscard]]
			constexpr Infra::EventView<DrawcallSegment &> &getIndirectBufferUpdateEvent() noexcept;

			[[nodiscard]]
			constexpr Infra::EventView<DrawcallSegment &> &getIndirectBufferCreateEvent() noexcept;

		private:
			const size_t __segmentIndex;
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

			Infra::Event<DrawcallSegment &> __meshBufferChangeEvent;
			Infra::Event<DrawcallSegment &> __indirectBufferUpdateEvent;
			Infra::Event<DrawcallSegment &> __indirectBufferCreateEvent;

			void __initEventListeners() noexcept;
			void __updateIndirectBufferBuilders() noexcept;

			void __onMeshBufferChange(Mesh &mesh) noexcept;
			void __onMeshDestroy(Mesh &mesh) noexcept;
			void __onSubmeshDestroy(Submesh &submesh) noexcept;

			void __onIndirectBufferUpdate(IndirectBufferBuilder &builder) noexcept;
			void __onIndirectBufferCreate(IndirectBufferBuilder &builder) noexcept;
		};

		size_t __segmentCursor{};
		std::vector<std::unique_ptr<DrawcallSegment>> __segments;
		std::unordered_map<Mesh *, DrawcallSegment *> __mesh2SegmentMap;

		std::shared_ptr<Infra::EventListener<Mesh &>> __pMeshDestroyEventListener;
		std::shared_ptr<Infra::EventListener<DrawcallSegment &>> __pSegmentMeshBufferChangeEventListener;
		std::shared_ptr<Infra::EventListener<DrawcallSegment &>> __pSegmentIndirectBufferUpdateEventListener;
		std::shared_ptr<Infra::EventListener<DrawcallSegment &>> __pSegmentIndirectBufferCreateEventListener;
		std::shared_ptr<Infra::EventListener<>> __pUpdateEventListener;

		Infra::Event<Drawcall &, size_t> __segmentMeshBufferChangeEvent;
		Infra::Event<Drawcall &, size_t> __segmentIndirectBufferUpdateEvent;
		Infra::Event<Drawcall &, size_t> __segmentIndirectBufferCreateEvent;

		void __initEventListeners() noexcept;

		void __createSegments(
			Vulkan::Device &device, const uint32_t queueFamilyIndex,
			BufferManager &bufferManager, MemoryManager &memoryManager) noexcept;

		constexpr void __advanceSegment() noexcept;

		[[nodiscard]]
		DrawcallSegment *__getCurrentSegment() noexcept;

		void __onMeshDestroy(Mesh &mesh) noexcept;
		void __onSegmentMeshBufferChange(DrawcallSegment &segment) noexcept;
		void __onSegmentIndirectBufferUpdate(DrawcallSegment &segment) noexcept;
		void __onSegmentIndirectBufferCreate(DrawcallSegment &segment) noexcept;
		void __onUpdate();
	};

	constexpr size_t  Drawcall::getNumSegments() const noexcept
	{
		return __segments.size();
	}

	constexpr Infra::EventView<Drawcall &, size_t> &Drawcall::getMeshBufferChangeEvent() noexcept
	{
		return __segmentMeshBufferChangeEvent;
	}

	constexpr Infra::EventView<Drawcall &, size_t> &Drawcall::getIndirectBufferUpdateEvent() noexcept
	{
		return __segmentIndirectBufferUpdateEvent;
	}

	constexpr Infra::EventView<Drawcall &, size_t> &Drawcall::getIndirectBufferCreateEvent() noexcept
	{
		return __segmentIndirectBufferCreateEvent;
	}

	constexpr void Drawcall::__advanceSegment() noexcept
	{
		__segmentCursor = ((__segmentCursor + 1ULL) % __segments.size());
	}

	constexpr size_t Drawcall::DrawcallSegment::getSegmentIndex() const noexcept
	{
		return __segmentIndex;
	}

	constexpr Infra::EventView<Drawcall::DrawcallSegment &> &
		Drawcall::DrawcallSegment::getMeshBufferChangeEvent() noexcept
	{
		return __meshBufferChangeEvent;
	}

	constexpr Infra::EventView<Drawcall::DrawcallSegment &> &
		Drawcall::DrawcallSegment::getIndirectBufferUpdateEvent() noexcept
	{
		return __indirectBufferUpdateEvent;
	}

	constexpr Infra::EventView<Drawcall::DrawcallSegment &> &
		Drawcall::DrawcallSegment::getIndirectBufferCreateEvent() noexcept
	{
		return __indirectBufferCreateEvent;
	}
}