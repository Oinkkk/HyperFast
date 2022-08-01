#include "PipelineFactory.h"
#include "VertexAttribute.h"
#include <glm/glm.hpp>

namespace HyperFast
{
	PipelineFactory::PipelineResource::PipelineResource(
		Vulkan::Device &device, Infra::TemporalDeleter &resourceDeleter,
		const VkPipelineLayout pipelineLayout) noexcept :
		__device{ device }, __resourceDeleter{ resourceDeleter }, __pipelineLayout{pipelineLayout}
	{
		__createShaderModules();
		__createPipelineCache();
		__populatePipelineCreateInfos();
	}

	PipelineFactory::PipelineResource::~PipelineResource() noexcept
	{
		__resourceDeleter.reserve(__pVertexShader);
		__resourceDeleter.reserve(__pFragShader);
		__resourceDeleter.reserve(__pPipelineCache);
		__resourceDeleter.reserve(__pPipeline);
	}

	void PipelineFactory::PipelineResource::reset() noexcept
	{
		__resourceDeleter.reserve(__pPipeline);
	}

	void PipelineFactory::PipelineResource::build(const BuildParam &buildParam)
	{
		__buildPipelines(buildParam);
	}

	VkPipeline PipelineFactory::PipelineResource::getPipeline() noexcept
	{
		return __pPipeline->getHandle();
	}

	void PipelineFactory::PipelineResource::__createShaderModules()
	{
		ShaderCompiler shaderCompiler;

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

		__pVertexShader = new Vulkan::ShaderModule{ __device, vertexShaderCreateInfo };
		__pFragShader = new Vulkan::ShaderModule{ __device, fragShaderCreateInfo };
	}

	void PipelineFactory::PipelineResource::__createPipelineCache()
	{
		const VkPipelineCacheCreateInfo createInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
			.flags = VkPipelineCacheCreateFlagBits::VK_PIPELINE_CACHE_CREATE_EXTERNALLY_SYNCHRONIZED_BIT
		};

		__pPipelineCache = new Vulkan::PipelineCache{ __device, createInfo };
	}

	void PipelineFactory::PipelineResource::__populatePipelineCreateInfos() noexcept
	{
		// shader

		VkPipelineShaderStageCreateInfo &vsStageInfo{ __shaderStageInfos.emplace_back() };
		vsStageInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		vsStageInfo.stage = VkShaderStageFlagBits::VK_SHADER_STAGE_VERTEX_BIT;
		vsStageInfo.module = __pVertexShader->getHandle();
		vsStageInfo.pName = "main";

		VkPipelineShaderStageCreateInfo &fsStageInfo{ __shaderStageInfos.emplace_back() };
		fsStageInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		fsStageInfo.stage = VkShaderStageFlagBits::VK_SHADER_STAGE_FRAGMENT_BIT;
		fsStageInfo.module = __pFragShader->getHandle();
		fsStageInfo.pName = "main";


		// vertex binding description

		VkVertexInputBindingDescription &posBindingDesc{ __vertexBindingDescs.emplace_back() };
		posBindingDesc.binding = VERTEX_ATTRIB_LOCATION_POS;
		posBindingDesc.stride = sizeof(glm::vec3);
		posBindingDesc.inputRate = VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX;

		VkVertexInputBindingDescription &colorBindingDesc{ __vertexBindingDescs.emplace_back() };
		colorBindingDesc.binding = VERTEX_ATTRIB_LOCATION_COLOR;
		colorBindingDesc.stride = sizeof(glm::vec4);
		colorBindingDesc.inputRate = VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX;

		VkVertexInputBindingDescription &normalBindingDesc{ __vertexBindingDescs.emplace_back() };
		normalBindingDesc.binding = VERTEX_ATTRIB_LOCATION_NORMAL;
		normalBindingDesc.stride = sizeof(glm::vec3);
		normalBindingDesc.inputRate = VkVertexInputRate::VK_VERTEX_INPUT_RATE_VERTEX;


		// vertex attribute description

		VkVertexInputAttributeDescription &posAttribDesc{ __vertexAttribDescs.emplace_back() };
		posAttribDesc.location = VERTEX_ATTRIB_LOCATION_POS;
		posAttribDesc.binding = VERTEX_ATTRIB_LOCATION_POS;
		posAttribDesc.format = VkFormat::VK_FORMAT_R32G32B32_SFLOAT;
		posAttribDesc.offset = 0U;

		VkVertexInputAttributeDescription &colorAttribDesc{ __vertexAttribDescs.emplace_back() };
		colorAttribDesc.location = VERTEX_ATTRIB_LOCATION_COLOR;
		colorAttribDesc.binding = VERTEX_ATTRIB_LOCATION_COLOR;
		colorAttribDesc.format = VkFormat::VK_FORMAT_R32G32B32A32_SFLOAT;
		colorAttribDesc.offset = 0U;

		VkVertexInputAttributeDescription &normalAttribDesc{ __vertexAttribDescs.emplace_back() };
		normalAttribDesc.location = VERTEX_ATTRIB_LOCATION_NORMAL;
		normalAttribDesc.binding = VERTEX_ATTRIB_LOCATION_NORMAL;
		normalAttribDesc.format = VkFormat::VK_FORMAT_R32G32B32_SFLOAT;
		normalAttribDesc.offset = 0U;


		// vertex input info

		__vertexInputInfo.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
		__vertexInputInfo.vertexBindingDescriptionCount = uint32_t(__vertexBindingDescs.size());
		__vertexInputInfo.pVertexBindingDescriptions = __vertexBindingDescs.data();
		__vertexInputInfo.vertexAttributeDescriptionCount = uint32_t(__vertexAttribDescs.size());
		__vertexInputInfo.pVertexAttributeDescriptions = __vertexAttribDescs.data();
	}

	void PipelineFactory::PipelineResource::__buildPipelines(const BuildParam &buildParam)
	{
		static constexpr VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo
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

		static constexpr VkPipelineRasterizationStateCreateInfo rasterizationInfo
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

		static constexpr VkPipelineMultisampleStateCreateInfo multisampleInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
			.rasterizationSamples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT,
			.sampleShadingEnable = VK_FALSE
		};

		static constexpr VkPipelineDepthStencilStateCreateInfo depthStencilInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
			.depthTestEnable = VK_FALSE,
			.depthWriteEnable = VK_FALSE,
			.depthBoundsTestEnable = VK_FALSE,
			.stencilTestEnable = VK_FALSE
		};

		static constexpr VkPipelineColorBlendAttachmentState colorBlendAttachmentInfo
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

		static constexpr VkPipelineColorBlendStateCreateInfo colorBlendInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
			.logicOpEnable = VK_FALSE,
			.attachmentCount = 1U,
			.pAttachments = &colorBlendAttachmentInfo,
		};

		static constexpr VkPipelineDynamicStateCreateInfo dynamicState
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO
		};

		const VkGraphicsPipelineCreateInfo createInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
			.stageCount = uint32_t(__shaderStageInfos.size()),
			.pStages = __shaderStageInfos.data(),
			.pVertexInputState = &__vertexInputInfo,
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

		__pPipeline = new Vulkan::Pipeline{ __device, __pPipelineCache->getHandle(), createInfo };
	}
}