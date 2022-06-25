#pragma once

#include "ShaderCompiler.h"
#include "../VulkanLoader/Procedure.h"
#include "../Infrastructure/Environment.h"
#include "VertexAttribute.h"

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

		void build(
			const std::vector<VertexAttributeFlag> &usedAttribFlags,
			const BuildParam &param, tf::Subflow &subflow);
		
		void reset() noexcept;

		[[nodiscard]]
		VkPipeline get(const VertexAttributeFlag attribFlag) noexcept;

	private:
		class PipelineResource final
		{
		public:
			PipelineResource(const VkDevice device, const VKL::DeviceProcedure &deviceProc) noexcept;
			~PipelineResource() noexcept;

			void build(const VertexAttributeFlag attribFlag, const BuildParam &param);
			void reset() noexcept;

			[[nodiscard]]
			constexpr VkPipeline getPipeline() noexcept;

		private:
			const VkDevice __device;
			const VKL::DeviceProcedure &__deviceProc;

			VkShaderModule __vertexShader{};
			VkShaderModule __fragShader{};
			VkPipelineCache __pipelineCache{};
			VkPipeline __pipeline{};
		};

		const VkDevice __device;
		const VKL::DeviceProcedure &__deviceProc;

		VkPipelineLayout __pipelineLayout{};
		std::unordered_map<VertexAttributeFlag, PipelineResource> __attribFlag2ResourceMap;

		void __createPipelineLayouts();
		void __destroyPipelineLayouts() noexcept;

		void __setupShaderCompiler() noexcept;
		void __createShaderModules();
		void __destroyShaderModules() noexcept;
		void __createPipelineCache();
		void __destroyPipelineCache() noexcept;

		void __createPipelines(const BuildParam &buildParam, tf::Subflow &subflow);
		void __destroyPipelines() noexcept;
	};

	constexpr VkPipeline PipelineFactory::PipelineResource::getPipeline() noexcept
	{
		return __pipeline;
	}
}
