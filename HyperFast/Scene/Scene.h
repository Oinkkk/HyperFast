#pragma once

#include "../RenderingEngine/RenderingEngine.h"

namespace Jin
{
	class Scene : public Infra::Unique
	{
	public:
		Scene(HyperFast::RenderingEngine &renderingEngine) noexcept;
		virtual ~Scene() noexcept;

	private:
		HyperFast::RenderingEngine &__renderingEngine;
	};
}
