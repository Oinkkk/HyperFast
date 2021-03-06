#pragma once

#include "Mesh.h"
#include "../Infrastructure/Event.h"

namespace HyperFast
{
	class Submesh : public Infra::Unique
	{
	public:
		Submesh(const std::shared_ptr<Mesh> &pMesh) noexcept;
		~Submesh() noexcept;

		[[nodiscard]]
		constexpr const VkDrawIndexedIndirectCommand &getDrawCommand() const noexcept;

		void setIndexCount(const uint32_t indexCount) noexcept;
		void setInstanceCount(const uint32_t instanceCount) noexcept;
		void setFirstIndex(const uint32_t firstIndex) noexcept;
		void setVertexOffset(const int32_t vertexOffset) noexcept;
		void setFirstInstance(const uint32_t firstInstance) noexcept;
		void setDrawCommand(
			const uint32_t indexCount, const uint32_t instanceCount, const uint32_t firstIndex,
			const int32_t vertexOffset, const uint32_t firstInstance) noexcept;

		[[nodiscard]]
		constexpr bool isVisible() const noexcept;
		void setVisible(const bool visible) noexcept;

		[[nodiscard]]
		Mesh &getMesh() const noexcept;

		[[nodiscard]]
		constexpr Infra::EventView<Submesh &> &getDrawCommandChangeEvent() noexcept;

		[[nodiscard]]
		constexpr Infra::EventView<Submesh &> &getVisibleChangeEvent() noexcept;

		[[nodiscard]]
		constexpr Infra::EventView<Submesh &> &getDestroyEvent() noexcept;

	private:
		const std::shared_ptr<Mesh> __pMesh;

		VkDrawIndexedIndirectCommand __drawCommand{};
		bool __visible{ true };

		Infra::Event<Submesh &> __drawCommandChangeEvent;
		Infra::Event<Submesh &> __visibleChangeEvent;
		Infra::Event<Submesh &> __destroyEvent;
	};

	constexpr const VkDrawIndexedIndirectCommand &Submesh::getDrawCommand() const noexcept
	{
		return __drawCommand;
	}

	constexpr bool Submesh::isVisible() const noexcept
	{
		return __visible;
	}

	constexpr Infra::EventView<Submesh &> &Submesh::getDrawCommandChangeEvent() noexcept
	{
		return __drawCommandChangeEvent;
	}

	constexpr Infra::EventView<Submesh &> &Submesh::getVisibleChangeEvent() noexcept
	{
		return __visibleChangeEvent;
	}

	constexpr Infra::EventView<Submesh &> &Submesh::getDestroyEvent() noexcept
	{
		return __destroyEvent;
	}
}
