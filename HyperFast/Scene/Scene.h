#pragma once

#include "../RenderingEngine/RenderingEngine.h"
#include "../RenderingEngine/Drawcall.h"

namespace Jin
{
	class Scene : public Infra::Unique
	{
	public:
		Scene(HyperFast::RenderingEngine &renderingEngine) noexcept;
		virtual ~Scene() noexcept;

	protected:
		[[nodiscard]]
		std::shared_ptr<HyperFast::Buffer> _createVertexBuffer(const VkDeviceSize dataSize) const;

		[[nodiscard]]
		std::shared_ptr<HyperFast::Memory> _createVertexMemory(const VkMemoryRequirements &memRequirements) const;

		[[nodiscard]]
		std::shared_ptr<HyperFast::Mesh> _createMesh() noexcept;

		[[nodiscard]]
		std::shared_ptr<HyperFast::Submesh> _createSubmesh(const std::shared_ptr<HyperFast::Mesh> &pMesh) noexcept;

	private:
		HyperFast::RenderingEngine &__renderingEngine;
		HyperFast::Drawcall __drawcall;
	};
}
