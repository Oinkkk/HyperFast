#include "PipelineFactory.h"

namespace HyperFast
{
	PipelineFactory::PipelineFactory(const VkDevice device, const VKL::DeviceProcedure &deviceProc) noexcept :
		__device{ device }, __deviceProc{ deviceProc }
	{
		__setupShaderCompiler();
		__createShaderModules();
		__createPipelineLayouts();
		__createPipelineCaches();
	}

	PipelineFactory::~PipelineFactory() noexcept
	{
		if (__pipeline)
		{
			__destroyPipelines();
			__destroyRenderPasses();
		}

		__destroyPipelineCaches();
		__destroyPipelineLayouts();
		__destroyShaderModules();
	}

	void PipelineFactory::build(const BuildParam &param)
	{
		if (__pipeline)
		{
			__destroyPipelines();
			__destroyRenderPasses();
		}

		__createRenderPasses(param);
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

	void PipelineFactory::__createPipelineCaches()
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

	void PipelineFactory::__destroyPipelineCaches() noexcept
	{
		__deviceProc.vkDestroyPipelineCache(__device, __pipelineCache, nullptr);
		__pipelineCache = VK_NULL_HANDLE;
	}

	void PipelineFactory::__createRenderPasses(const BuildParam &buildParam)
	{
		std::vector<VkAttachmentDescription> attachments;
		std::vector<VkSubpassDescription> subpasses;
		std::vector<VkSubpassDependency> dependencies;

		VkAttachmentDescription &colorAttachment{ attachments.emplace_back() };
		colorAttachment.format = buildParam.swapchainFormat;
		colorAttachment.samples = VkSampleCountFlagBits::VK_SAMPLE_COUNT_1_BIT;
		colorAttachment.loadOp = VkAttachmentLoadOp::VK_ATTACHMENT_LOAD_OP_CLEAR;
		colorAttachment.storeOp = VkAttachmentStoreOp::VK_ATTACHMENT_STORE_OP_STORE;
		colorAttachment.initialLayout = VkImageLayout::VK_IMAGE_LAYOUT_UNDEFINED;
		colorAttachment.finalLayout = VkImageLayout::VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

		const VkAttachmentReference colorAttachmentRef
		{
			.attachment = 0U,
			.layout = VkImageLayout::VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
		};

		VkSubpassDescription &subpass{ subpasses.emplace_back() };
		subpass.pipelineBindPoint = VkPipelineBindPoint::VK_PIPELINE_BIND_POINT_GRAPHICS;
		subpass.colorAttachmentCount = 1U;
		subpass.pColorAttachments = &colorAttachmentRef;

		VkSubpassDependency &dependency{ dependencies.emplace_back() };

		// srcSubpass의 srcStageMask 파이프가 idle이 되고, 거기에 srcAccessMask가 모두 available해질 때까지 블록
		dependency.srcSubpass = VK_SUBPASS_EXTERNAL;

		// dstSubpass 진행에 대해 위 조건 + dstAccessMask가 visible 해질 때 까지 dstStageMask를 블록
		dependency.dstSubpass = 0U;
		dependency.srcStageMask = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.dstStageMask = VkPipelineStageFlagBits::VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT;
		dependency.srcAccessMask = 0U;
		dependency.dstAccessMask = 0U;
		dependency.dependencyFlags = VkDependencyFlagBits::VK_DEPENDENCY_BY_REGION_BIT;

		/*
			세마포어나 펜스는 시그널이 들어오면 해당 큐가 모든 작업을 처리했음을 보장
			또한 모든 메모리 access에 대해 available을 보장 (암묵적 메모리 디펜던시)
			vkQueueSubmit는 host visible 메모리의 모든 access에 대해 visible함을 보장 (암묵적 메모리 디펜던시)
			세마포어 대기 요청은 모든 메모리 access에 대해 visible함을 보장 (암묵적 메모리 디펜던시)
		*/

		const VkRenderPassCreateInfo createInfo
		{
			.sType = VkStructureType::VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
			.attachmentCount = uint32_t(attachments.size()),
			.pAttachments = attachments.data(),
			.subpassCount = uint32_t(subpasses.size()),
			.pSubpasses = subpasses.data(),
			.dependencyCount = uint32_t(dependencies.size()),
			.pDependencies = dependencies.data()
		};

		__deviceProc.vkCreateRenderPass(__device, &createInfo, nullptr, &__renderPass);
		if (!__renderPass)
			throw std::exception{ "Cannot create a VkRenderPass." };
	}

	void PipelineFactory::__destroyRenderPasses() noexcept
	{
		__deviceProc.vkDestroyRenderPass(__device, __renderPass, nullptr);
		__renderPass = VK_NULL_HANDLE;
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
			.renderPass = __renderPass,
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