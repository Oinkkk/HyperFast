#pragma once

#include "../RenderingEngine/RenderingEngine.h"
#include "../RenderingEngine/Buffer.h"

namespace Jin
{
	class Scene : public Infra::Unique
	{
	public:
		Scene(HyperFast::RenderingEngine &renderingEngine) noexcept;
		virtual ~Scene() noexcept;

	protected:
		[[nodiscard]]
		std::shared_ptr<HyperFast::Buffer> _createVertexBuffer(const VkDeviceSize memSize, const void *const pData);

	private:
		HyperFast::RenderingEngine &__renderingEngine;
	};
}
