#pragma once

#include "base.hpp"
#include "fwd.hpp"

#include <string>
#include <vector>

#if SOUP_WINDOWS
#include <windows.h>
#else
#include "X11Api.hpp"
#endif

#include "Capture.hpp"

NAMESPACE_SOUP
{
	struct Window
	{
#if SOUP_WINDOWS
		using handle_t = HWND;
#else
		using handle_t = X11Api::Window;
#endif

		using callback_t = void(*)(Window);
		using draw_func_t = void(*)(Window, RenderTarget&);
		using key_callback_t = void(*)(Window, char32_t, bool down, bool repeat);
#if SOUP_WINDOWS
		using on_click_t = void(*)(Window, unsigned int, unsigned int);
		using mouse_informer_t = on_click_t(*)(Window, unsigned int, unsigned int);
		using char_callback_t = void(*)(Window, char32_t);
#endif

		struct Config
		{
			Capture custom_data;
			draw_func_t draw_func = nullptr;
			key_callback_t key_callback = nullptr;
#if SOUP_WINDOWS
			bool resizable = false;
			mouse_informer_t mouse_informer = nullptr;
			char_callback_t char_callback = nullptr;
			std::vector<callback_t> hotkey_callbacks{};
			callback_t on_focus = nullptr;
			callback_t on_blur = nullptr;
			callback_t on_close = nullptr;
#else
			unsigned int width, height;
#endif
		};

		handle_t h = 0;

		operator bool() const noexcept
		{
			return h != 0;
		}

		void reset() noexcept
		{
			h = 0;
		}

#if SOUP_WINDOWS
		[[nodiscard]] static Window create(const std::string& title, unsigned int width, unsigned int height, const std::string& icon_ico) noexcept;
		[[nodiscard]] static Window create(const std::string& title, unsigned int width, unsigned int height, HICON icon = NULL) noexcept;
#else
		[[nodiscard]] static Window create(const std::string& title, unsigned int width, unsigned int height) noexcept;
#endif
#if SOUP_WINDOWS
		[[nodiscard]] static Window getFocused() noexcept;

		[[nodiscard]] DWORD getOwnerPid() const noexcept;
		[[nodiscard]] DWORD getOwnerTid() const noexcept;
#endif

		[[nodiscard]] Window::Config& getConfig();
		Capture& customData();
		Window& setDrawFunc(draw_func_t draw_func);
		Window& setKeyCallback(key_callback_t key_callback);
#if SOUP_WINDOWS
		Window& setMouseInformer(mouse_informer_t mouse_informer);
		Window& setCharCallback(char_callback_t char_callback);
		Window& registerHotkey(bool meta, bool ctrl, bool shift, bool alt, unsigned int key, callback_t callback);
		Window& onFocus(callback_t on_focus);
		Window& onBlur(callback_t on_blur);
		Window& onClose(callback_t on_close);

		[[nodiscard]] bool getIsVisible() noexcept;
		Window& setIsVisible(bool visible) noexcept;
		Window& show() noexcept;
		Window& hide() noexcept;

		Window& hideFromTaskbar() noexcept;

		[[nodiscard]] std::pair<unsigned int, unsigned int> getPos() noexcept;
		[[nodiscard]] std::pair<unsigned int, unsigned int> getSize() noexcept;
		Window& setPos(int x, int y) noexcept;
		Window& setSize(int width, int height) noexcept;
		Window& setResizable(bool b) noexcept;

		Window& bringToFront() noexcept;
#endif
		Window& redraw() noexcept;

#if SOUP_WINDOWS
		Window& setInvisibleColour(Rgb rgb) noexcept; // Windows is weird. When the invisible colour is black or { 1, 0, 0 }, you can only drag visible pixels, but if it's red or { 1, 1, 1 }, you can drag everywhere.
		Window& setTransparency(int a) noexcept; // 0-100. 100 is fully invisible.
		Window& setTopmost(bool on) noexcept;
		Window& setClickThrough(bool on) noexcept; // If true, window cannot be clicked or dragged.
#endif

		static void runMessageLoop() noexcept;
#if SOUP_WINDOWS
		void close() noexcept;
#endif
	};

#if SOUP_WINDOWS
	inline Window Window::getFocused() noexcept
	{
		return Window{ GetForegroundWindow() };
	}

	inline DWORD Window::getOwnerPid() const noexcept
	{
		DWORD pid;
		GetWindowThreadProcessId(h, &pid);
		return pid;
	}

	inline DWORD Window::getOwnerTid() const noexcept
	{
		return GetWindowThreadProcessId(h, nullptr);
	}
#endif
}
