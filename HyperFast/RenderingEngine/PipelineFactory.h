#pragma once

#include "../Infrastructure/Unique.h"
#include "../VulkanLoader/Procedure.h"

namespace HyperFast
{
	class PipelineFactory : public Infra::Unique
	{
	public:
		class BuildParam
		{
		public:
			float viewportWidth{};
			float viewportHeight{};
		};

		PipelineFactory(const VkDevice device, const VKL::DeviceProcedure &deviceProc) noexcept;

		void build(const BuildParam &param);

		[[nodiscard]]
		VkPipeline get() const noexcept;

	private:
		const VkDevice __device;
		const VKL::DeviceProcedure &__deviceProc;
	};
}
