#pragma once

#include <Windows.h>
#include <string>
#include "../Infrastructure/Unique.h"

namespace Win
{
	class WindowClass : public Infra::Unique
	{
	public:
		WindowClass(const HINSTANCE hInstance, const std::string_view &name);
		virtual ~WindowClass() noexcept;

		[[nodiscard]]
		constexpr HINSTANCE getHInstance() const noexcept;

		[[nodiscard]]
		constexpr const std::string &getName() const noexcept;

	private:
		const HINSTANCE __hInstance;
		const std::string __name;
		const ATOM __atom;

		static ATOM __register(const HINSTANCE hInstance, const std::string_view &name);

		static LRESULT CALLBACK __winProc(
			const HWND hwnd, const UINT uMsg, const WPARAM wParam, const LPARAM lParam) noexcept;
	};

	constexpr HINSTANCE WindowClass::getHInstance() const noexcept
	{
		return __hInstance;
	}

	constexpr const std::string &WindowClass::getName() const noexcept
	{
		return __name;
	}
}
