#pragma once

#include "ShaderCompiler.h"
#include "../VulkanLoader/Procedure.h"

namespace HyperFast
{
	class PipelineFactory : public Infra::Unique
	{
	public:
		class BuildParam
		{
		public:
			VkFormat swapchainFormat{};
			VkViewport viewport{};
			VkRect2D scissor{};
		};

		PipelineFactory(const VkDevice device, const VKL::DeviceProcedure &deviceProc) noexcept;
		~PipelineFactory() noexcept;

		void build(const BuildParam &param);

		[[nodiscard]]
		VkPipeline get() const noexcept;

	private:
		const VkDevice __device;
		const VKL::DeviceProcedure &__deviceProc;

		ShaderCompiler __shaderCompiler;
		VkShaderModule __vertexShader{};
		VkShaderModule __fragShader{};
		VkRenderPass __renderPass{};
		VkPipelineLayout __pipelineLayout{};
		VkPipelineCache __pipelineCache{};
		VkPipeline __pipeline{};

		void __setupShaderCompiler() noexcept;
		void __createShaderModules();
		void __destroyShaderModules() noexcept;
		void __createPipelineLayouts();
		void __destroyPipelineLayouts() noexcept;
		void __createPipelineCaches();
		void __destroyPipelineCaches() noexcept;

		void __createRenderPasses(const BuildParam &buildParam);
		void __destroyRenderPasses() noexcept;
		void __createPipelines(const BuildParam &buildParam);
		void __destroyPipelines() noexcept;
	};
}
