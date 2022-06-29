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

		void process(const float deltaTime);

	protected:
		[[nodiscard]]
		std::shared_ptr<HyperFast::Buffer> _createBuffer(
			const VkDeviceSize size, const VkBufferUsageFlags usage) const;

		[[nodiscard]]
		std::shared_ptr<HyperFast::Memory> _createMemory(const VkMemoryRequirements &memRequirements) const;

		[[nodiscard]]
		std::shared_ptr<HyperFast::Mesh> _createMesh() noexcept;

		[[nodiscard]]
		std::shared_ptr<HyperFast::Submesh> _createSubmesh(const std::shared_ptr<HyperFast::Mesh> &pMesh) noexcept;

		void _bindScreen(HyperFast::Screen &screen) noexcept;

		virtual void _onProcess(const float deltaTime);

	private:
		HyperFast::RenderingEngine &__renderingEngine;
		std::unique_ptr<HyperFast::Drawcall> __pDrawcall;
	};
}
