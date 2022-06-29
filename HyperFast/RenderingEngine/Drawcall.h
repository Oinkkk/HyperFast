#pragma once

#include "Submesh.h"
#include <unordered_map>
#include <unordered_set>

namespace HyperFast
{
	// Per shading type
	class Drawcall : public Infra::Unique
	{
	public:
		Drawcall() noexcept;

		void addSubmesh(Submesh &submesh) noexcept;
		void removeSubmesh(Submesh &submesh) noexcept;

		void update() noexcept;

		[[nodiscard]]
		constexpr const std::vector<VertexAttributeFlag> &getUsedAttributeFlags() const noexcept;

		// TODO: mesh별로 submesh 묶은 다음 indirect command buffer 구성
		// TODO: secondary buffer recording은 attribFlag 별로
		void draw(const VertexAttributeFlag attribFlag, VkCommandBuffer commandBuffer) noexcept;

		[[nodiscard]]
		constexpr Infra::EventView<Drawcall &> &getUsedAttributeFlagsChangeEvent() noexcept;

		[[nodiscard]]
		constexpr Infra::EventView<Drawcall &> &getDrawcallChangeEvent() noexcept;

	private:
		using SubmeshGroup = std::unordered_map<Mesh *, std::unordered_set<Submesh *>>;
		using AttributeFlagChangeEventListener = Infra::EventListener<Mesh &, VertexAttributeFlag, VertexAttributeFlag>;

		bool __attribFlagsChanged{};
		bool __drawcallChanged{};

		std::unordered_map<VertexAttributeFlag, SubmeshGroup> __attribFlag2SubmeshGroup;
		std::vector<VertexAttributeFlag> __usedAttribFlags;

		const std::shared_ptr<AttributeFlagChangeEventListener> __pAttributeFlagChangeEventListener;
		const std::shared_ptr<Infra::EventListener<Submesh &>> __pSubmeshDestroyEventListener;

		Infra::Event<Drawcall &> __usedAttributeFlagsChangeEvent;
		Infra::Event<Drawcall &> __drawcallChangeEvent;

		void __onAttributeFlagChange(
			Mesh &mesh, const VertexAttributeFlag oldFlag, VertexAttributeFlag newFlag) noexcept;

		void __onSubmeshDestroy(Submesh &submesh) noexcept;
	};

	constexpr const std::vector<VertexAttributeFlag> &Drawcall::getUsedAttributeFlags() const noexcept
	{
		return __usedAttribFlags;
	}

	constexpr Infra::EventView<Drawcall &> &Drawcall::getUsedAttributeFlagsChangeEvent() noexcept
	{
		return __usedAttributeFlagsChangeEvent;
	}

	constexpr Infra::EventView<Drawcall &> &Drawcall::getDrawcallChangeEvent() noexcept
	{
		return __drawcallChangeEvent;
	}
}