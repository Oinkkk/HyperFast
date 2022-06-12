#pragma once

#include "../Infrastructure/Unique.h"
#include "../VulkanLoader/Procedure.h"

namespace HyperFast
{
	class Drawcall : public Infra::Unique
	{
	public:
		void bind(const VkCommandBuffer commandBuffer) noexcept;
	};
}
