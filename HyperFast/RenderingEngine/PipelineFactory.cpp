#include "PipelineFactory.h"
#include <glm/glm.hpp>

namespace HyperFast
{
	PipelineFactory::PipelineFactory(Vulkan::Device &device) noexcept :
		__device{ device }
	{
		__createPipelineLayouts();
	}

	PipelineFactory::~PipelineFactory() noexcept
	{
		__attribFlag2ResourceMap.clear();
		__destroyPipelineLayouts();
	}

	void PipelineFactory::build(
		const std::vector<VertexAttributeFlag> &usedAttribFlags,
		const BuildParam &buildParam, tf::Subflow &subflow)
	{
		for (const VertexAttributeFlag attribFlag : usedAttribFlags)
		{
			PipelineResource &pipelineResource
			{
				__attribFlag2ResourceMap.try_emplace(
					attribFlag, __device, __pipelineLayout, attribFlag).first->second
			};

			subflow.emplace([&pipelineResource, &buildParam]
			{
				pipelineResource.build(buildParam);
			});
		}
	}

	void PipelineFactory::reset() noexcept
	{
		for (auto &[attribFlag, pipelineResource] : __attribFlag2ResourceMap)
			pipelineResource.reset();
	}

	VkPipeline PipelineFactory::get(const VertexAttributeFlag attribFlag) noexcept
	{
		return __attribFlag2ResourceMap.at(attribFlag).getPipeline();
	}

	void PipelineFactory::__createPipelineLayouts()
	{
		const VkPipelineLayoutCreateInfo createInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO
		};

		__device.vkCreatePipelineLayout(&createInfo, nullptr, &__pipelineLayout);
		if (!__pipelineLayout)
			throw std::exception{ "Cannot create a VkPipelineLayout." };
	}

	void PipelineFactory::__destroyPipelineLayouts() noexcept
	{
		__device.vkDestroyPipelineLayout(__pipelineLayout, nullptr);
	}

	PipelineFactory::PipelineResource::PipelineResource(
		Vulkan::Device &device, const VkPipelineLayout pipelineLayout,
		const VertexAttributeFlag attribFlag) noexcept :
		__device{ device }, __pipelineLayout{ pipelineLayout }, __attribFlag{ attribFlag }
	{
		__createShaderModules();
		__createPipelineCache();
	}

	PipelineFactory::PipelineResource::~PipelineResource() noexcept
	{
		reset();
		__destroyPipelineCache();
		__destroyShaderModules();
	}

	void PipelineFactory::PipelineResource::build(const BuildParam &buildParam)
	{
		__createPipeline(buildParam);
	}

	void PipelineFactory::PipelineResource::reset() noexcept
	{
		if (!__pipeline)
			return;

		__destroyPipeline();
	}

	void PipelineFactory::PipelineResource::__createShaderModules()
	{
		ShaderCompiler shaderCompiler;
		shaderCompiler.setVertexAttributeFlag(__attribFlag);

#ifndef NDEBUG
		shaderCompiler.setOptimizationLevel(shaderc_optimization_level::shaderc_optimization_level_zero);
#else
		shaderCompiler.setOptimizationLevel(shaderc_optimization_level::shaderc_optimization_level_performance);
#endif

		const std::vector<uint32_t> &vertexShader
		{
			shaderCompiler.compile("shader/triangle.vert", shaderc_shader_kind::shaderc_vertex_shader)
		};

		const std::vector<uint32_t> &fragShader
		{
			shaderCompiler.compile("shader/triangle.frag", shaderc_shader_kind::shaderc_fragment_shader)
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

		__device.vkCreateShaderModule(&vertexShaderCreateInfo, nullptr, &__vertexShader);
		__device.vkCreateShaderModule(&fragShaderCreateInfo, nullptr, &__fragShader);

		if (!__vertexShader)
			throw std::exception{ "Cannot create the vertex shader." };

		if (!__fragShader)
			throw std::exception{ "Cannot create the fragment shader." };
	}

	void PipelineFactory::PipelineResource::__destroyShaderModules() noexcept
	{
		__device.vkDestroyShaderModule(__fragShader, nullptr);
		__device.vkDestroyShaderModule(__vertexShader, nullptr);
	}

	void PipelineFactory::PipelineResource::__createPipelineCache()
	{
		const VkPipelineCacheCreateInfo createInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
			.flags = VkPipelineCacheCreateFlagBits::VK_PIPELINE_CACHE_CREATE_EXTERNALLY_SYNCHRONIZED_BIT
		};

		__device.vkCreatePipelineCache(&createInfo, nullptr, &__pipelineCache);
		if (!__pipelineCache)
			throw std::exception{ "Cannot create a VkPipelineCache." };
	}

	void PipelineFactory::PipelineResource::__destroyPipelineCache() noexcept
	{
		__device.vkDestroyPipelineCache(__pipelineCache, nullptr);
	}

	void PipelineFactory::PipelineResource::__createPipeline(const BuildParam &buildParam)
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

		std::vector<VkVertexInputBindingDescription> vertexBindingDescs;
		std::vector<VkVertexInputAttributeDescription> vertexAttribDescs;

		if (__attribFlag & VertexAttributeFlagBit::POS3)
		{
			VkVertexInputBindingDescription &posBindingDesc{ vertexBindingDescs.emplace_back() };
			posBindingDesc.binding = VERTEX_ATTRIB_LOCATION_POS;
			posBindingDesc.stride = sizeof(glm::vec3);
			posBindingDesc.inputRate = VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX;

			VkVertexInputAttributeDescription &posAttribDesc{ vertexAttribDescs.emplace_back() };
			posAttribDesc.location = VERTEX_ATTRIB_LOCATION_POS;
			posAttribDesc.binding = VERTEX_ATTRIB_LOCATION_POS;
			posAttribDesc.format = VkFormat::VK_FORMAT_R32G32B32_SFLOAT;
			posAttribDesc.offset = 0U;
		}

		if (__attribFlag & VertexAttributeFlagBit::COLOR4)
		{
			VkVertexInputBindingDescription &colorBindingDesc{ vertexBindingDescs.emplace_back() };
			colorBindingDesc.binding = VERTEX_ATTRIB_LOCATION_COLOR;
			colorBindingDesc.stride = sizeof(glm::vec4);
			colorBindingDesc.inputRate = VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX;

			VkVertexInputAttributeDescription &colorAttribDesc{ vertexAttribDescs.emplace_back() };
			colorAttribDesc.location = VERTEX_ATTRIB_LOCATION_COLOR;
			colorAttribDesc.binding = VERTEX_ATTRIB_LOCATION_COLOR;
			colorAttribDesc.format = VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT;
			colorAttribDesc.offset = 0U;
		}

		const VkPipelineVertexInputStateCreateInfo vertexInputInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
			.vertexBindingDescriptionCount = uint32_t(vertexBindingDescs.size()),
			.pVertexBindingDescriptions = vertexBindingDescs.data(),
			.vertexAttributeDescriptionCount = uint32_t(vertexAttribDescs.size()),
			.pVertexAttributeDescriptions = vertexAttribDescs.data()
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
			.blendEnable = VK_FALSE,
			.colorWriteMask =
			(
				VkColorComponentFlagBits::VK_COLOR_COMPONENT_R_BIT |
				VkColorComponentFlagBits::VK_COLOR_COMPONENT_G_BIT |
				VkColorComponentFlagBits::VK_COLOR_COMPONENT_B_BIT |
				VkColorComponentFlagBits::VK_COLOR_COMPONENT_A_BIT
			)
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

		__device.vkCreateGraphicsPipelines(__pipelineCache, 1U, &createInfo, nullptr, &__pipeline);
		if (!__pipeline)
			throw std::exception{ "Cannot create a graphics pipeline." };
	}

	void PipelineFactory::PipelineResource::__destroyPipeline() noexcept
	{
		__device.vkDestroyPipeline(__pipeline, nullptr);
		__pipeline = VK_NULL_HANDLE;
	}
}