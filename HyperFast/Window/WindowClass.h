#pragma once

#include <Windows.h>
#include "../Infrastructure/Unique.h"
#include <string>

namespace Win
{
	class WindowClass : public Infra::Unique
	{
	public:
		WindowClass(const std::string_view &name);
		virtual ~WindowClass() noexcept;

		[[nodiscard]]
		constexpr const std::string &getName() const noexcept;

	private:
		const std::string __name;
		const ATOM __atom;

		static ATOM __register(const std::string_view &name);
		static LRESULT CALLBACK __winProc(
			const HWND hwnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam) noexcept;
	};

	constexpr const std::string &WindowClass::getName() const noexcept
	{
		return __name;
	}
}
