#include "PipelineFactory.h"

namespace HyperFast
{
	PipelineFactory::PipelineFactory(Vulkan::Device &device) noexcept :
		__device{ device }
	{
		__createPipelineLayouts();
		__createResource();
	}

	PipelineFactory::~PipelineFactory() noexcept
	{
		__pResource = nullptr;
		__pPipelineLayout = nullptr;
	}

	void PipelineFactory::build(const BuildParam &buildParam, tf::Subflow &subflow)
	{
		subflow.emplace([this, &buildParam]
		{
			__pResource->build(buildParam);
		});
	}

	void PipelineFactory::reset() noexcept
	{
		__pResource->reset();
	}

	VkPipeline PipelineFactory::get() noexcept
	{
		return __pResource->getPipeline();
	}

	void PipelineFactory::__createPipelineLayouts()
	{
		const VkPipelineLayoutCreateInfo createInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO
		};

		__pPipelineLayout = std::make_unique<Vulkan::PipelineLayout>(__device, createInfo);
	}

	void PipelineFactory::__createResource() noexcept
	{
		__pResource = std::make_unique<PipelineResource>(__device, __pPipelineLayout->getHandle());
	}
}