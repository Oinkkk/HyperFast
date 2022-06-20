#pragma once

#include "Mesh.h"
#include <unordered_map>
#include <unordered_set>

namespace HyperFast
{
	class Drawcall : public Infra::Unique
	{
	public:
		void addMesh(const std::shared_ptr<Mesh> &mesh) noexcept;
		void validate() noexcept;

		[[nodiscard]]
		constexpr const std::unordered_set<VertexAttributeFlag> &getUsedAttribFlags() const noexcept;
		void bind(const VertexAttributeFlag attribFlag, VkCommandBuffer commandBuffer) noexcept;

	private:
		std::unordered_set<VertexAttributeFlag> __usedAttribFlags;
		std::unordered_map<VertexAttributeFlag, std::unordered_set<std::weak_ptr<Mesh>>> __attribFlag2MeshesMap;
	};

	constexpr const std::unordered_set<VertexAttributeFlag> &Drawcall::getUsedAttribFlags() const noexcept
	{
		return __usedAttribFlags;
	}
}