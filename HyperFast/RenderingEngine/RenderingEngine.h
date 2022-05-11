#pragma once

#include "../Infrastructure/Unique.h"
#include <string_view>
#include <vulkan/vulkan.h>

namespace HyperFast
{
	class RenderingEngine : public Infra::Unique
	{
	public:
		RenderingEngine(const std::string_view &appName, const std::string_view &engineName);

	private:
		uint32_t __instanceVersion{};

		void __getInstanceVersion() noexcept;
		void __checkInstanceVersionSupport() const;
	};
}
