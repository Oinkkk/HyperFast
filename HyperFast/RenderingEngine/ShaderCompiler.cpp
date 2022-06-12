#include "ShaderCompiler.h"
#include "../glslc/file_includer.h"
#include <fstream>
#include <sstream>

namespace HyperFast
{
	ShaderCompiler::ShaderCompiler() noexcept
	{
		__options.SetIncluder(std::make_unique<glslc::FileIncluder>(&__fileFinder));
	}

	void ShaderCompiler::setOptimizationLevel(const shaderc_optimization_level level) noexcept
	{
		__options.SetOptimizationLevel(level);
	}

	void ShaderCompiler::setVertexAttributeFlag(const VertexAttributeFlag flag) noexcept
	{
		__setVertexAttributeFlagMacro(
			"VERTEX_ATTRIB_POS", flag & VertexAttributeFlagBit::POS);

		__setVertexAttributeFlagMacro(
			"VERTEX_ATTRIB_COLOR", flag & VertexAttributeFlagBit::COLOR);
	}

	std::vector<uint32_t> ShaderCompiler::compile(const std::string_view &shaderPath, const shaderc_shader_kind shaderKind)
	{
		std::ifstream fin{ shaderPath.data() };
		if (!fin)
			throw std::exception{ "Cannot open a given shaderPath." };

		std::ostringstream oss;
		oss << fin.rdbuf();
		fin.close();

		const std::string &source{ oss.str() };

		const shaderc::SpvCompilationResult &result
		{
			__compiler.CompileGlslToSpv(source, shaderKind, shaderPath.data(), __options)
		};

		if (result.GetCompilationStatus() != shaderc_compilation_status::shaderc_compilation_status_success)
		{
			const std::string &errMsg{ result.GetErrorMessage() };
			throw std::exception{ errMsg.c_str() };
		}

		return { result.begin(), result.end() };
	}

	void ShaderCompiler::__setVertexAttributeFlagMacro(const std::string_view &macroName, const bool value) noexcept
	{
		__options.AddMacroDefinition("VERTEX_ATTRIB_POS", value ? "true" : "false");
	}
}