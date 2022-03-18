#pragma once

#include "WindowClass.h"
#include <string_view>

namespace Win
{
	class Window : public Infra::Unique
	{
	public:
		Window(
			const WindowClass &windowClass, const std::string_view &title);
		
		Window(
			const WindowClass &windowClass, const std::string_view &title,
			const int x, const int y);

		Window(
			const WindowClass &windowClass, const std::string_view &title,
			const int x, const int y, const int width, const int height);

		virtual ~Window() noexcept;

		void setShow(const bool show) noexcept;

	private:
		const HWND __handle;

		static HWND __create(
			const WindowClass &windowClass, const std::string_view &title,
			const int x, const int y, const int width, const int height, Window *const pThis);
	};
}
