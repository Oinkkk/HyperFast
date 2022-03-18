#pragma once

#include "../Infrastructure/Unique.h"
#include <string_view>

namespace Win
{
	class Window : public Infra::Unique
	{
	public:
		Window(const std::string_view &) noexcept;
		virtual ~Window() noexcept;
	};
}
