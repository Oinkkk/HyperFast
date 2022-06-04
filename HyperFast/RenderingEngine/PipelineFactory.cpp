#include "PipelineFactory.h"

namespace HyperFast
{
	PipelineFactory::PipelineFactory(const VkDevice device, const VKL::DeviceProcedure &deviceProc) noexcept :
		__device{ device }, __deviceProc{ deviceProc }
	{
		__setupShaderCompiler();
		__createShaderModules();
		__createPipelineLayouts();
		__createPipelineCache();
	}

	PipelineFactory::~PipelineFactory() noexcept
	{
		if (__pipeline)
			__destroyPipelines();

		__destroyPipelineCache();
		__destroyPipelineLayouts();
		__destroyShaderModules();
	}

	void PipelineFactory::build(const BuildParam &param)
	{
		if (__pipeline)
			__destroyPipelines();

		__createPipelines(param);
	}

	VkPipeline PipelineFactory::get() const noexcept
	{
		return nullptr;
	}

	void PipelineFactory::__setupShaderCompiler() noexcept
	{
#ifndef NDEBUG
		__shaderCompiler.setOptimizationLevel(shaderc_optimization_level::shaderc_optimization_level_zero);
#elif
		__shaderCompiler.setOptimizationLevel(shaderc_optimization_level::shaderc_optimization_level_performance);
#endif
	}

	void PipelineFactory::__createShaderModules()
	{
		const std::vector<uint32_t> &vertexShader
		{
			__shaderCompiler.compile(
				"shader/triangle.vert", shaderc_shader_kind::shaderc_vertex_shader)
		};

		const std::vector<uint32_t> &fragShader
		{
			__shaderCompiler.compile(
				"shader/triangle.frag", shaderc_shader_kind::shaderc_fragment_shader)
		};

		const VkShaderModuleCreateInfo vertexShaderCreateInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.codeSize = (sizeof(uint32_t) * vertexShader.size()),
			.pCode = vertexShader.data()
		};

		const VkShaderModuleCreateInfo fragShaderCreateInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
			.codeSize = (sizeof(uint32_t) * fragShader.size()),
			.pCode = fragShader.data()
		};

		__deviceProc.vkCreateShaderModule(__device, &vertexShaderCreateInfo, nullptr, &__vertexShader);
		__deviceProc.vkCreateShaderModule(__device, &fragShaderCreateInfo, nullptr, &__fragShader);

		if (!__vertexShader)
			throw std::exception{ "Cannot create the vertex shader." };

		if (!__fragShader)
			throw std::exception{ "Cannot create the fragment shader." };
	}

	void PipelineFactory::__destroyShaderModules() noexcept
	{
		__deviceProc.vkDestroyShaderModule(__device, __fragShader, nullptr);
		__deviceProc.vkDestroyShaderModule(__device, __vertexShader, nullptr);

		__vertexShader = VK_NULL_HANDLE;
		__fragShader = VK_NULL_HANDLE;
	}

	void PipelineFactory::__createPipelineLayouts()
	{
		const VkPipelineLayoutCreateInfo createInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO
		};

		__deviceProc.vkCreatePipelineLayout(__device, &createInfo, nullptr, &__pipelineLayout);
		if (!__pipelineLayout)
			throw std::exception{ "Cannot create a VkPipelineLayout." };
	}

	void PipelineFactory::__destroyPipelineLayouts() noexcept
	{
		__deviceProc.vkDestroyPipelineLayout(__device, __pipelineLayout, nullptr);
		__pipelineLayout = VK_NULL_HANDLE;
	}

	void PipelineFactory::__createPipelineCache()
	{
		const VkPipelineCacheCreateInfo createInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
			.flags = VkPipelineCacheCreateFlagBits::VK_PIPELINE_CACHE_CREATE_EXTERNALLY_SYNCHRONIZED_BIT
		};

		__deviceProc.vkCreatePipelineCache(__device, &createInfo, nullptr, &__pipelineCache);
		if (!__pipelineCache)
			throw std::exception{ "Cannot create a VkPipelineCache." };
	}

	void PipelineFactory::__destroyPipelineCache() noexcept
	{
		__deviceProc.vkDestroyPipelineCache(__device, __pipelineCache, nullptr);
		__pipelineCache = VK_NULL_HANDLE;
	}

	void PipelineFactory::__createPipelines(const BuildParam &buildParam)
	{
		std::vector<VkPipelineShaderStageCreateInfo> shaderStageInfos;

		VkPipelineShaderStageCreateInfo &vsStageInfo{ shaderStageInfos.emplace_back() };
		vsStageInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vsStageInfo.stage = VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
		vsStageInfo.module = __vertexShader;
		vsStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo &fsStageInfo{ shaderStageInfos.emplace_back() };
		fsStageInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fsStageInfo.stage = VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT;
		fsStageInfo.module = __fragShader;
		fsStageInfo.pName = "main";

		const VkPipelineVertexInputStateCreateInfo vertexInputInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO
		};

		const VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
			.topology = VkPrimitiveTopology::VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST,
			.primitiveRestartEnable = VK_FALSE
		};

		const VkPipelineViewportStateCreateInfo viewportInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
			.viewportCount = 1U,
			.pViewports = &(buildParam.viewport),
			.scissorCount = 1U,
			.pScissors = &(buildParam.scissor)
		};

		const VkPipelineRasterizationStateCreateInfo rasterizationInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
			.depthClampEnable = VK_FALSE,
			.rasterizerDiscardEnable = VK_FALSE,
			.polygonMode = VkPolygonMode::VK_POLYGON_MODE_FILL,
			.cullMode = VkCullModeFlagBits::VK_CULL_MODE_BACK_BIT,
			.frontFace = VkFrontFace::VK_FRONT_FACE_COUNTER_CLOCKWISE,
			.depthBiasEnable = VK_FALSE,
			.lineWidth = 1.0f
		};

		const VkPipelineMultisampleStateCreateInfo multisampleInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			.rasterizationSamples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT,
			.sampleShadingEnable = VK_FALSE
		};

		const VkPipelineDepthStencilStateCreateInfo depthStencilInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			.depthTestEnable = VK_FALSE,
			.depthWriteEnable = VK_FALSE,
			.depthBoundsTestEnable = VK_FALSE,
			.stencilTestEnable = VK_FALSE
		};

		const VkPipelineColorBlendAttachmentState colorBlendAttachmentInfo
		{
			.blendEnable = VK_FALSE
		};

		const VkPipelineColorBlendStateCreateInfo colorBlendInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.logicOpEnable = VK_FALSE,
			.attachmentCount = 1U,
			.pAttachments = &colorBlendAttachmentInfo,
		};

		const VkPipelineDynamicStateCreateInfo dynamicState
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO
		};

		const VkGraphicsPipelineCreateInfo createInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.stageCount = uint32_t(shaderStageInfos.size()),
			.pStages = shaderStageInfos.data(),
			.pVertexInputState = &vertexInputInfo,
			.pInputAssemblyState = &inputAssemblyInfo,
			.pTessellationState = nullptr,
			.pViewportState = &viewportInfo,
			.pRasterizationState = &rasterizationInfo,
			.pMultisampleState = &multisampleInfo,
			.pDepthStencilState = &depthStencilInfo,
			.pColorBlendState = &colorBlendInfo,
			.pDynamicState = &dynamicState,
			.layout = __pipelineLayout,
			.renderPass = buildParam.renderPass,
			.subpass = 0,
			.basePipelineHandle = VK_NULL_HANDLE,
			.basePipelineIndex = -1
		};

		__deviceProc.vkCreateGraphicsPipelines(
			__device, __pipelineCache, 1U, &createInfo, nullptr, &__pipeline);

		if (!__pipeline)
			throw std::exception{ "Cannot create a graphics pipeline." };
	}

	void PipelineFactory::__destroyPipelines() noexcept
	{
		__deviceProc.vkDestroyPipeline(__device, __pipeline, nullptr);
		__pipeline = VK_NULL_HANDLE;
	}
}