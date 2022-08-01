#pragma once

#include "ShaderCompiler.h"
#include "../Infrastructure/Environment.h"
#include "../Infrastructure/TemporalDeleter.h"
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

		PipelineFactory(Vulkan::Device &device, Infra::TemporalDeleter &resourceDeleter) noexcept;
		~PipelineFactory() noexcept;
		
		void reset() noexcept;
		void build(const BuildParam &buildParam, tf::Subflow &subflow);

		[[nodiscard]]
		VkPipeline get() noexcept;

	private:
		class PipelineResource final
		{
		public:
			PipelineResource(
				Vulkan::Device &device, Infra::TemporalDeleter &resourceDeleter,
				const VkPipelineLayout pipelineLayout) noexcept;
			
			~PipelineResource() noexcept;

			void reset() noexcept;
			void build(const BuildParam &buildParam);

			[[nodiscard]]
			VkPipeline getPipeline() noexcept;

		private:
			Vulkan::Device &__device;
			Infra::TemporalDeleter &__resourceDeleter;
			const VkPipelineLayout __pipelineLayout;

			Vulkan::ShaderModule *__pVertexShader{};
			Vulkan::ShaderModule *__pFragShader{};
			Vulkan::PipelineCache *__pPipelineCache{};
			Vulkan::Pipeline *__pPipeline{};

			std::vector<VkPipelineShaderStageCreateInfo> __shaderStageInfos;
			std::vector<VkVertexInputBindingDescription> __vertexBindingDescs;
			std::vector<VkVertexInputAttributeDescription> __vertexAttribDescs;
			VkPipelineVertexInputStateCreateInfo __vertexInputInfo;

			void __createShaderModules();
			void __createPipelineCache();
			void __populatePipelineCreateInfos() noexcept;
			void __buildPipelines(const BuildParam &buildParam);
		};

		Vulkan::Device &__device;
		Infra::TemporalDeleter &__resourceDeleter;

		Vulkan::PipelineLayout *__pPipelineLayout{};
		std::unique_ptr<PipelineResource> __pResource;

		void __createPipelineLayouts();
		void __createResource() noexcept;
	};
}
