#pragma once

#include "Device.h"

namespace Vulkan
{
	class ShaderModule final : public Handle<VkShaderModule>
	{
	public:
		ShaderModule(Device &device, const VkShaderModuleCreateInfo &createInfo);
		~ShaderModule() noexcept;

	private:
		Device &__device;

		void __destroy() noexcept;

		[[nodiscard]]
		static VkShaderModule __create(
			Device &device, const VkShaderModuleCreateInfo &createInfo);
	};
}