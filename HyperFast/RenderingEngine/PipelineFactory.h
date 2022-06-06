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
			VkRenderPass renderPass{};
			VkViewport viewport{};
			VkRect2D scissor{};
		};

		PipelineFactory(const VkDevice device, const VKL::DeviceProcedure &deviceProc) noexcept;
		~PipelineFactory() noexcept;

		void build(const BuildParam &param);
		void reset() noexcept;

		[[nodiscard]]
		VkPipeline get() const noexcept;

	private:
		const VkDevice __device;
		const VKL::DeviceProcedure &__deviceProc;

		ShaderCompiler __shaderCompiler;
		VkShaderModule __vertexShader{};
		VkShaderModule __fragShader{};
		VkPipelineLayout __pipelineLayout{};
		VkPipelineCache __pipelineCache{};
		VkPipeline __pipeline{};

		void __setupShaderCompiler() noexcept;
		void __createShaderModules();
		void __destroyShaderModules() noexcept;
		void __createPipelineLayouts();
		void __destroyPipelineLayouts() noexcept;
		void __createPipelineCache();
		void __destroyPipelineCache() noexcept;

		void __createPipelines(const BuildParam &buildParam);
		void __destroyPipelines() noexcept;
	};
}
