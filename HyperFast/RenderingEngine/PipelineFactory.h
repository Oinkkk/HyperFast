#pragma once

#include "ShaderCompiler.h"
#include "../Infrastructure/Environment.h"
#include "VertexAttribute.h"
#include "../Vulkan/ShaderModule.h"
#include "../Vulkan/PipelineLayout.h"
#include "../Vulkan/PipelineCache.h"
#include "../Vulkan/Pipeline.h"

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

		PipelineFactory(Vulkan::Device &device) noexcept;
		~PipelineFactory() noexcept;

		void build(
			const std::vector<VertexAttributeFlag> &usedAttribFlags,
			const BuildParam &buildParam, tf::Subflow &subflow);
		
		void reset() noexcept;

		[[nodiscard]]
		VkPipeline get(const VertexAttributeFlag attribFlag) noexcept;

	private:
		class PipelineResource final
		{
		public:
			PipelineResource(
				Vulkan::Device &device, const VkPipelineLayout pipelineLayout,
				const VertexAttributeFlag attribFlag) noexcept;
			
			~PipelineResource() noexcept;

			void build(const BuildParam &buildParam);
			void reset() noexcept;

			[[nodiscard]]
			VkPipeline getPipeline() noexcept;

		private:
			Vulkan::Device &__device;
			const VkPipelineLayout __pipelineLayout;
			const VertexAttributeFlag __attribFlag;

			std::unique_ptr<Vulkan::ShaderModule> __pVertexShader;
			std::unique_ptr<Vulkan::ShaderModule> __pFragShader;
			std::unique_ptr<Vulkan::PipelineCache> __pPipelineCache;
			std::unique_ptr<Vulkan::Pipeline> __pPipeline;

			void __createShaderModules();
			void __createPipelineCache();
			void __createPipeline(const BuildParam &buildParam);
		};

		Vulkan::Device &__device;

		std::unique_ptr<Vulkan::PipelineLayout> __pPipelineLayout;
		std::unordered_map<VertexAttributeFlag, PipelineResource> __attribFlag2ResourceMap;

		void __createPipelineLayouts();
	};
}
