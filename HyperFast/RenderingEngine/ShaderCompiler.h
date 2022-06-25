#pragma once

#include <shaderc/shaderc.hpp>
#include "../libshaderc_util/file_finder.h"
#include "../Infrastructure/Unique.h"
#include "VertexAttribute.h"

namespace HyperFast
{
	class ShaderCompiler : public Infra::Unique
	{
	public:
		ShaderCompiler() noexcept;

		void setOptimizationLevel(const shaderc_optimization_level level) noexcept;
		void setVertexAttributeFlag(const VertexAttributeFlag flag) noexcept;

		[[nodiscard]]
		std::vector<uint32_t> compile(const std::string_view &shaderPath, const shaderc_shader_kind shaderKind);

	private:
		shaderc::CompileOptions __options;
		shaderc_util::FileFinder __fileFinder;
		shaderc::Compiler __compiler;
	};
}
