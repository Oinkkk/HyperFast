#pragma once

#include "../RenderingEngine/RenderingEngine.h"
#include "../RenderingEngine/Drawcall.h"
#include "../RenderingEngine/Screen.h"

namespace Jin
{
	class Scene : public Infra::Unique
	{
	public:
		Scene(HyperFast::RenderingEngine &renderingEngine) noexcept;
		virtual ~Scene() noexcept;

	protected:
		[[nodiscard]]
		std::shared_ptr<HyperFast::Buffer> _createBuffer(
			const VkDeviceSize dataSize, const VkBufferUsageFlags usage) const;

		[[nodiscard]]
		std::shared_ptr<HyperFast::Memory> _createMemory(const VkMemoryRequirements &memRequirements) const;

		[[nodiscard]]
		std::shared_ptr<HyperFast::Mesh> _createMesh() noexcept;

		[[nodiscard]]
		std::shared_ptr<HyperFast::Submesh> _createSubmesh(const std::shared_ptr<HyperFast::Mesh> &pMesh) noexcept;

		void _bindScreen(HyperFast::Screen &screen) noexcept;

	private:
		HyperFast::RenderingEngine &__renderingEngine;
		HyperFast::Drawcall __drawcall;
	};
}
