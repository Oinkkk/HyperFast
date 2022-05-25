#include "PipelineFactory.h"

namespace HyperFast
{
	PipelineFactory::PipelineFactory(const VkDevice device, const VKL::DeviceProcedure &deviceProc) noexcept :
		__device{ device }, __deviceProc{ deviceProc }
	{}

	void PipelineFactory::build(const BuildParam &param)
	{

	}

	VkPipeline PipelineFactory::get() const noexcept
	{
		return nullptr;
	}
}