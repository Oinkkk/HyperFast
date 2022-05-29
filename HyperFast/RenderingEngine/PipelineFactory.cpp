#include "PipelineFactory.h"

namespace HyperFast
{
	PipelineFactory::PipelineFactory(const VkDevice device, const VKL::DeviceProcedure &deviceProc) noexcept :
		__device{ device }, __deviceProc{ deviceProc }
	{
		__setupShaderCompiler();
		__createShaderModules();
	}

	PipelineFactory::~PipelineFactory() noexcept
	{
		if (__pipeline)
			__destroyPipeline();

		__destroyShaderModules();
	}

	void PipelineFactory::build(const BuildParam &param)
	{
		if (__pipeline)
			__destroyPipeline();

		__createPipeline(param);
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
	}

	void PipelineFactory::__createPipeline(const BuildParam &buildParam)
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

		const VkPipelineTessellationStateCreateInfo tessellationInfo
		{

		};

		const VkPipelineViewportStateCreateInfo viewportInfo
		{

		};

		const VkPipelineRasterizationStateCreateInfo rasterizationInfo
		{

		};

		const VkPipelineMultisampleStateCreateInfo multisampleInfo
		{

		};

		const VkPipelineDepthStencilStateCreateInfo depthStencilInfo
		{

		};

		const VkPipelineColorBlendStateCreateInfo colorBlendInfo
		{

		};

		const VkPipelineDynamicStateCreateInfo dynamicState
		{

		};
	}

	void PipelineFactory::__destroyPipeline() noexcept
	{

	}
}