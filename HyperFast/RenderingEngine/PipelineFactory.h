#pragma once

#include "ShaderCompiler.h"
#include "../Infrastructure/Environment.h"
#include "VertexAttribute.h"
#include "../Vulkan/ShaderModule.h"

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
			constexpr VkPipeline getPipeline() noexcept;

		private:
			Vulkan::Device &__device;
			const VkPipelineLayout __pipelineLayout;
			const VertexAttributeFlag __attribFlag;

			std::unique_ptr<Vulkan::ShaderModule> __pVertexShader;
			std::unique_ptr<Vulkan::ShaderModule> __pFragShader;
			VkPipelineCache __pipelineCache{};
			VkPipeline __pipeline{};

			void __createShaderModules();
			void __createPipelineCache();
			void __destroyPipelineCache() noexcept;

			void __createPipeline(const BuildParam &buildParam);
			void __destroyPipeline() noexcept;
		};

		Vulkan::Device &__device;

		VkPipelineLayout __pipelineLayout{};
		std::unordered_map<VertexAttributeFlag, PipelineResource> __attribFlag2ResourceMap;

		void __createPipelineLayouts();
		void __destroyPipelineLayouts() noexcept;
	};

	constexpr VkPipeline PipelineFactory::PipelineResource::getPipeline() noexcept
	{
		return __pipeline;
	}
}
