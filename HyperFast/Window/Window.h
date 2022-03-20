#pragma once

#include "WindowClass.h"
#include <string_view>
#include "../Infrastructure/Event.h"

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
		void destroy();

		LRESULT sendRawMessage(const UINT uMsg, const WPARAM wParam, const LPARAM lParam);

		[[nodiscard]]
		constexpr Infra::EventView<Window *> &getCloseEvent() noexcept;

	private:
		const HWND __handle;
		bool __destroyed{};

		Infra::Event<Window *> __closeEvent;

		static HWND __create(
			const WindowClass &windowClass, const std::string_view &title,
			const int x, const int y, const int width, const int height, Window *const pThis);
	};

	constexpr Infra::EventView<Window *> &Window::getCloseEvent() noexcept
	{
		return __closeEvent;
	}
}
