#include "ShaderModule.h"
#include <exception>

namespace Vulkan
{
	ShaderModule::ShaderModule(Device &device, const VkShaderModuleCreateInfo &createInfo) :
		Handle{ __create(device, createInfo) }, __device{ device }
	{}

	ShaderModule::~ShaderModule() noexcept
	{
		__destroy();
	}

	void ShaderModule::__destroy() noexcept
	{
		__device.vkDestroyShaderModule(getHandle(), nullptr);
	}

	VkShaderModule ShaderModule::__create(Device &device, const VkShaderModuleCreateInfo &createInfo)
	{
		VkShaderModule retVal{};
		device.vkCreateShaderModule(&createInfo, nullptr, &retVal);

		if (!retVal)
			throw std::exception{ "Cannot create a VkShaderModule." };

		return retVal;
	}
}