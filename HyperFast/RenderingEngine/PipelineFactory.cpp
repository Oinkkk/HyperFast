#include "PipelineFactory.h"

namespace HyperFast
{
	PipelineFactory::PipelineFactory(Vulkan::Device &device, Infra::TemporalDeleter &resourceDeleter) noexcept :
		__device{ device }, __resourceDeleter{ resourceDeleter }
	{
		__createPipelineLayouts();
		__createResource();
	}

	PipelineFactory::~PipelineFactory() noexcept
	{
		__pResource = nullptr;
		__resourceDeleter.reserve(__pPipelineLayout);
	}

	void PipelineFactory::reset() noexcept
	{
		__pResource->reset();
	}

	void PipelineFactory::build(const BuildParam &buildParam, tf::Subflow &subflow)
	{
		// TODO: shading type별로 resource 생성
		subflow.emplace([this, &buildParam]
		{
			__pResource->build(buildParam);
		});
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

		__pPipelineLayout = new Vulkan::PipelineLayout{ __device, createInfo };
	}

	void PipelineFactory::__createResource() noexcept
	{
		__pResource = std::make_unique<PipelineResource>(
			__device, __resourceDeleter, __pPipelineLayout->getHandle());
	}
}