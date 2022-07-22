#pragma once

#include "IndirectBufferBuilder.h"
#include <unordered_map>
#include <unordered_set>
#include "../Vulkan/CommandBuffer.h"

namespace HyperFast
{
	// Per shading type
	class Drawcall : public Infra::Unique
	{
	public:
		Drawcall(
			Vulkan::Device &device, BufferManager &bufferManager,
			MemoryManager &memoryManager) noexcept;

		void addSubmesh(Submesh &submesh) noexcept;
		void removeSubmesh(Submesh &submesh) noexcept;

		void validate();

		[[nodiscard]]
		constexpr const std::vector<VertexAttributeFlag> &getAttributeFlags() const noexcept;

		// TODO: secondary buffer recording¿∫ attribFlag ∫∞∑Œ
		void render(const VertexAttributeFlag attribFlag, Vulkan::CommandBuffer &commandBuffer) noexcept;

		void addSemaphoreDependency(const std::shared_ptr<SemaphoreDependency> &pDependency) noexcept;

		[[nodiscard]]
		constexpr Infra::EventView<Drawcall &> &getAttributeFlagsUpdateEvent() noexcept;

		[[nodiscard]]
		constexpr Infra::EventView<Drawcall &> &getMeshBufferChangeEvent() noexcept;

		[[nodiscard]]
		constexpr Infra::EventView<Drawcall &> &getIndirectBufferUpdateEvent() noexcept;

		[[nodiscard]]
		constexpr Infra::EventView<Drawcall &> &getIndirectBufferCreateEvent() noexcept;

	private:
		using IndirectBufferBuilderMap = std::unordered_map<Mesh *, std::unique_ptr<IndirectBufferBuilder>>;
		using MeshAttributeFlagChangeEventListener = Infra::EventListener<Mesh &, VertexAttributeFlag, VertexAttributeFlag>;

		Vulkan::Device &__device;
		BufferManager &__bufferManager;
		MemoryManager &__memoryManager;

		bool __attribFlagsUpdated{};
		bool __meshBufferChanged{};
		bool __indirectBufferUpdated{};
		bool __indirectBufferCreated{};

		std::unordered_map<VertexAttributeFlag, IndirectBufferBuilderMap> __attribFlag2IndirectBufferMap;
		std::vector<VertexAttributeFlag> __attribFlags;

		std::shared_ptr<MeshAttributeFlagChangeEventListener> __pMeshAttributeFlagChangeEventListener;
		std::shared_ptr<Infra::EventListener<Mesh &>> __pMeshBufferChangeEventListener;

		std::shared_ptr<Infra::EventListener<IndirectBufferBuilder &>> __pIndirectBufferUpdateEventListener;
		std::shared_ptr<Infra::EventListener<IndirectBufferBuilder &>> __pIndirectBufferCreateEventListener;

		Infra::Event<Drawcall &> __attributeFlagsUpdateEvent;
		Infra::Event<Drawcall &> __meshBufferChangeEvent;
		Infra::Event<Drawcall &> __indirectBufferUpdateEvent;
		Infra::Event<Drawcall &> __indirectBufferCreateEvent;

		void __initEventListeners() noexcept;
		
		void __onMeshAttributeFlagChange(
			Mesh &mesh, const VertexAttributeFlag oldFlag, VertexAttributeFlag newFlag) noexcept;

		void __onMeshBufferChange(Mesh &mesh) noexcept;

		void __onIndirectBufferUpdate(IndirectBufferBuilder &builder) noexcept;
		void __onIndirectBufferCreate(IndirectBufferBuilder &builder) noexcept;
	};

	constexpr const std::vector<VertexAttributeFlag> &Drawcall::getAttributeFlags() const noexcept
	{
		return __attribFlags;
	}

	constexpr Infra::EventView<Drawcall &> &Drawcall::getAttributeFlagsUpdateEvent() noexcept
	{
		return __attributeFlagsUpdateEvent;
	}

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