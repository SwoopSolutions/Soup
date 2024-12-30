#include "Window.hpp"

#if SOUP_WINDOWS
#include <windowsx.h> // GET_X_LPARAM, GET_Y_LPARAM
#endif

#include <unordered_map>

#include "RenderTargetWindow.hpp"

#if SOUP_WINDOWS
#include "rand.hpp"
#include "unicode.hpp"
#endif

NAMESPACE_SOUP
{
	static std::unordered_map<Window::handle_t, Window::Config> window_configs{};

#if SOUP_WINDOWS
	LRESULT CALLBACK WndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
	{
		if (auto window_config_entry = window_configs.find(hWnd); window_config_entry != window_configs.end())
		{
			Window::Config& wc = window_config_entry->second;
			switch (message)
			{
			case WM_PAINT:
				if (wc.draw_func)
				{
					RECT rect;
					//GetUpdateRect(hWnd, &rect, FALSE);
					GetWindowRect(hWnd, &rect);

					PAINTSTRUCT ps;
					HDC hdc = BeginPaint(hWnd, &ps);
					auto buf_hdc = CreateCompatibleDC(hdc);
					RenderTargetWindow rt{ rect.right - rect.left, rect.bottom - rect.top, buf_hdc };

					auto buf_bitmap = CreateCompatibleBitmap(hdc, rt.width, rt.height);
					SelectObject(buf_hdc, buf_bitmap);

					wc.draw_func(Window{ hWnd }, rt);

					BitBlt(hdc, 0, 0, rt.width, rt.height, buf_hdc, 0, 0, SRCCOPY);

					DeleteObject(buf_bitmap);
					DeleteDC(buf_hdc);

					EndPaint(hWnd, &ps);
				}
				return 0;

			case WM_NCHITTEST:
				{
					// Make window movable from any position
					LRESULT hit = DefWindowProcW(hWnd, message, wParam, lParam);
					if (hit == HTCLIENT)
					{
						POINT p;
						p.x = GET_X_LPARAM(lParam);
						p.y = GET_Y_LPARAM(lParam);
						MapWindowPoints(HWND_DESKTOP, hWnd, &p, 1);

						if (wc.resizable)
						{
							RECT r;
							GetWindowRect(hWnd, &r);
							const auto dx = ((r.right - r.left) - 5);
							const auto dy = ((r.bottom - r.top) - 5);

							if (p.x <= 5 && p.y <= 5)
							{
								hit = HTTOPLEFT;
							}
							else if (p.x >= dx && p.y <= 5)
							{
								hit = HTTOPRIGHT;
							}
							else if (p.x <= 5 && p.y >= dy)
							{
								hit = HTTOPRIGHT;
							}
							else if (p.x >= dx && p.y >= dy)
							{
								hit = HTBOTTOMRIGHT;
							}
							else if (p.x <= 5)
							{
								hit = HTLEFT;
							}
							else if (p.y <= 5)
							{
								hit = HTTOP;
							}
							else if (p.x >= dx)
							{
								hit = HTRIGHT;
							}
							else if (p.y >= dy)
							{
								hit = HTBOTTOM;
							}
						}
						if (hit == HTCLIENT
							&& (wc.mouse_informer == nullptr
								|| wc.mouse_informer(Window{ hWnd }, p.x, p.y) == nullptr
								)
							)
						{
							hit = HTCAPTION;
						}
					}
					return hit;
				}
				break;

			case WM_LBUTTONDOWN:
				if (wc.mouse_informer != nullptr)
				{
					POINT p;
					p.x = GET_X_LPARAM(lParam);
					p.y = GET_Y_LPARAM(lParam);
					if (auto on_click = wc.mouse_informer(Window{ hWnd }, p.x, p.y))
					{
						on_click(Window{ hWnd }, p.x, p.y);
					}
				}
				return 0;

			case WM_CHAR:
				if (wc.char_callback != nullptr)
				{
					wc.char_callback(Window{ hWnd }, static_cast<char32_t>(wParam));
				}
				return 0;

			case WM_KEYUP:
			case WM_SYSKEYUP:
			case WM_KEYDOWN:
			case WM_SYSKEYDOWN:
				if (wc.key_callback != nullptr)
				{
					const UINT scancode = ((lParam >> 16) & 0b11111111);
					unsigned int vk = MapVirtualKeyExA(scancode, MAPVK_VSC_TO_VK, GetKeyboardLayout(0));
					if (vk < 0x80)
					{
						const bool prev_down = ((lParam >> 30) & 0b1);
						const bool down = (message == WM_KEYDOWN || message == WM_SYSKEYDOWN);
						wc.key_callback(Window{ hWnd }, vk, down, (down == prev_down));
					}
				}
				return 0;

			case WM_GETMINMAXINFO:
				{
					LPMINMAXINFO lpMMI = (LPMINMAXINFO)lParam;
					lpMMI->ptMinTrackSize.x = 15;
					lpMMI->ptMinTrackSize.y = 15;
				}
				return 0;

			case WM_HOTKEY:
				if (wParam >= 0)
				{
					wc.hotkey_callbacks.at(wParam)(Window{ hWnd });
				}
				return 0;

			case WM_SETFOCUS:
				if (wc.on_focus)
				{
					wc.on_focus(Window{ hWnd });
				}
				break;

			case WM_KILLFOCUS:
				if (wc.on_blur)
				{
					wc.on_blur(Window{ hWnd });
				}
				break;

			case WM_CLOSE:
				if (wc.on_close)
				{
					wc.on_close(Window{ hWnd });
				}
				return 0;

			case WM_DESTROY:
				window_configs.erase(window_config_entry);
				DestroyWindow(hWnd); // need to do this as otherwise window would stay around if this thread is not exiting now
				PostQuitMessage(0);
				return 0;
			}
		}
		return DefWindowProcW(hWnd, message, wParam, lParam);
	}

	Window Window::create(const std::string& title, unsigned int width, unsigned int height, const std::string& icon_ico) noexcept
	{
		std::wstring icon_ico_w = unicode::utf8_to_utf16(icon_ico);
		HICON icon = (HICON)LoadImageW(NULL, icon_ico_w.c_str(), IMAGE_ICON, 0, 0, LR_LOADFROMFILE | LR_DEFAULTSIZE | LR_SHARED);
		return create(title, width, height, icon);
	}

	Window Window::create(const std::string& title, unsigned int width, unsigned int height, HICON icon) noexcept
	{
		HINSTANCE hInstance = GetModuleHandle(NULL);

		std::wstring menu_name = unicode::utf8_to_utf16(title);

		std::wstring class_name = L"SOUP_";
		class_name.append(rand.str<std::wstring>(5));

		WNDCLASSEXW wcex{};
		wcex.cbSize = sizeof(WNDCLASSEX);
		wcex.style = CS_HREDRAW | CS_VREDRAW;
		wcex.lpfnWndProc = &WndProc;
		wcex.cbClsExtra = 0;
		wcex.cbWndExtra = 0;
		wcex.hInstance = hInstance;
		wcex.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wcex.lpszMenuName = menu_name.c_str();
		wcex.lpszClassName = class_name.c_str();
		wcex.hIconSm = wcex.hIcon = icon;
		RegisterClassExW(&wcex);

		HWND hWnd = CreateWindowW(wcex.lpszClassName, wcex.lpszMenuName, WS_POPUP, CW_USEDEFAULT, CW_USEDEFAULT, width, height, nullptr, nullptr, hInstance, nullptr);
		SetWindowLong(hWnd, GWL_STYLE, 0);
		window_configs.emplace(hWnd, Window::Config{});
		Window w{ hWnd };
		w.show();
		w.onClose([](Window w)
		{
			w.close();
		});
		return w;
	}
#else
	Window Window::create(const std::string& title, unsigned int width, unsigned int height) noexcept
	{
		const X11Api& x = X11Api::get();
		int screen = x.defaultScreen(x.display);
		auto h = x.createSimpleWindow(x.display, x.rootWindow(x.display, screen), 100, 100, width, height, 0, 0, 0);
		SOUP_IF_LIKELY (!title.empty())
		{
			x.storeName(x.display, h, title.c_str());
		}
		auto& wc = window_configs.emplace(h, Window::Config{}).first->second;
		wc.width = width;
		wc.height = height;
		x.selectInput(x.display, h, X11Api::KeyPressMask | X11Api::KeyReleaseMask | X11Api::ExposureMask);
		x.mapWindow(x.display, h);
		x.flush(x.display); // if user doesn't call runMessageLoop, this allows the window to still appear.
		return Window{ h };
	}
#endif

	Window::Config& Window::getConfig()
	{
		return window_configs.at(h);
	}

	Capture& Window::customData()
	{
		return getConfig().custom_data;
	}

	Window& Window::setDrawFunc(draw_func_t draw_func)
	{
		getConfig().draw_func = draw_func;
		return *this;
	}

	Window& Window::setKeyCallback(key_callback_t key_callback)
	{
		getConfig().key_callback = key_callback;
		return *this;
	}

#if SOUP_WINDOWS
	Window& Window::setMouseInformer(mouse_informer_t mouse_informer)
	{
		getConfig().mouse_informer = mouse_informer;
		return *this;
	}

	Window& Window::setCharCallback(char_callback_t char_callback)
	{
		getConfig().char_callback = char_callback;
		return *this;
	}

	Window& Window::registerHotkey(bool meta, bool ctrl, bool shift, bool alt, unsigned int key, callback_t callback)
	{
		unsigned int mod = alt;
		mod |= (ctrl << 1);
		mod |= (shift << 2);
		mod |= (meta << 3);

		Window::Config& wc = getConfig();
		RegisterHotKey(h, (int)wc.hotkey_callbacks.size(), mod, key);
		wc.hotkey_callbacks.emplace_back(callback);

		return *this;
	}

	Window& Window::onFocus(callback_t on_focus)
	{
		getConfig().on_focus = on_focus;
		return *this;
	}

	Window& Window::onBlur(callback_t on_blur)
	{
		getConfig().on_blur = on_blur;
		return *this;
	}

	Window& Window::onClose(callback_t on_close)
	{
		getConfig().on_close = on_close;
		return *this;
	}

	bool Window::getIsVisible() noexcept
	{
		return IsWindowVisible(h);
	}

	Window& Window::setIsVisible(bool show) noexcept
	{
		ShowWindow(h, show * SW_SHOW);
		return *this;
	}

	Window& Window::show() noexcept
	{
		ShowWindow(h, SW_SHOW);
		return *this;
	}

	Window& Window::hide() noexcept
	{
		ShowWindow(h, SW_HIDE);
		return *this;
	}

	Window& Window::hideFromTaskbar() noexcept
	{
		auto style = GetWindowLong(h, GWL_EXSTYLE);
		style |= WS_EX_TOOLWINDOW;
		style &= ~(WS_EX_APPWINDOW);
		SetWindowLong(h, GWL_EXSTYLE, style);
		return *this;
	}

	std::pair<unsigned int, unsigned int> Window::getPos() noexcept
	{
		RECT r;
		GetWindowRect(h, &r);
		return { r.left, r.top };
	}

	std::pair<unsigned int, unsigned int> Window::getSize() noexcept
	{
		RECT r;
		GetWindowRect(h, &r);
		return { r.right - r.left, r.bottom - r.top };
	}

	Window& Window::setPos(int x, int y) noexcept
	{
		SetWindowPos(h, 0, x, y, 0, 0, SWP_NOSIZE | SWP_NOZORDER);
		return *this;
	}

	Window& Window::setSize(int width, int height) noexcept
	{
		SetWindowPos(h, 0, 0, 0, width, height, SWP_NOMOVE | SWP_NOZORDER);
		return *this;
	}

	Window& Window::setResizable(bool b) noexcept
	{
		getConfig().resizable = b;
		return *this;
	}

	Window& Window::bringToFront() noexcept
	{
		SetForegroundWindow(h);
		return *this;
	}
#endif

	Window& Window::redraw() noexcept
	{
#if SOUP_WINDOWS
		InvalidateRect(h, NULL, FALSE);
#else
		Window::Config& wc = getConfig();
		if (wc.draw_func)
		{
			const X11Api& x = X11Api::get();
			if (x.running_message_loop) // This function call would never finish otherwise
			{
				const auto gc = x.createGc(x.display, h, 0, nullptr);
				RenderTargetWindow rt(wc.width, wc.height, gc, x.createPixmap(x.display, h, wc.width, wc.height, 24));

				wc.draw_func(*this, rt);

				x.copyArea(x.display, rt.d, h, gc, 0, 0, wc.width, wc.height, 0, 0);

				x.freePixmap(x.display, rt.d);
				x.freeGc(x.display, gc);
				x.flush(x.display);
			}
		}
#endif
		return *this;
	}

#if SOUP_WINDOWS
	Window& Window::setInvisibleColour(Rgb rgb) noexcept
	{
		// Also setting WS_EX_TRANSPARENT to imply setClickThrough(true), may want to change this behaviour in future.
		SetWindowLong(h, GWL_EXSTYLE, GetWindowLong(h, GWL_EXSTYLE) | WS_EX_LAYERED | WS_EX_TRANSPARENT);
		SetLayeredWindowAttributes(h, RGB(rgb.r, rgb.g, rgb.b), 0, LWA_COLORKEY);
		return *this;
	}

	Window& Window::setTransparency(int a) noexcept
	{
		// Also setting WS_EX_TRANSPARENT to imply setClickThrough(true), may want to change this behaviour in future.
		SetWindowLong(h, GWL_EXSTYLE, GetWindowLong(h, GWL_EXSTYLE) | WS_EX_LAYERED | WS_EX_TRANSPARENT);
		SetLayeredWindowAttributes(h, 0, a, LWA_ALPHA);
		return *this;
	}

	Window& Window::setTopmost(bool on) noexcept
	{
		if (on)
		{
			SetWindowPos(h, HWND_TOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}
		else
		{
			SetWindowPos(h, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE);
		}
		return *this;
	}

	Window& Window::setClickThrough(bool on) noexcept
	{
		if (on)
		{
			SetWindowLong(h, GWL_EXSTYLE, GetWindowLong(h, GWL_EXSTYLE) | WS_EX_LAYERED | WS_EX_TRANSPARENT);
		}
		else
		{
			SetWindowLong(h, GWL_EXSTYLE, GetWindowLong(h, GWL_EXSTYLE) & ~(WS_EX_LAYERED | WS_EX_TRANSPARENT));
		}
		return *this;
	}

	void Window::runMessageLoop() noexcept
	{
		MSG msg;
		while (GetMessage(&msg, nullptr, 0, 0))
		{
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		//return (int)msg.wParam;
	}

	void Window::close() noexcept
	{
		PostMessage(h, WM_DESTROY, 0, 0);
	}
#else
	void Window::runMessageLoop() noexcept
	{
		const X11Api& x = X11Api::get();
		x.running_message_loop = true;
		X11Api::XEvent event, prev_event;
		while (true)
		{
			try
			{
				x.nextEvent(x.display, &event);
			}
			catch (const X11Api::IoError&)
			{
				break;
			}
			if (event.type == X11Api::KeyPress || event.type == X11Api::KeyRelease)
			{
				// https://gist.github.com/baines/5a49f1334281b2685af5dcae81a6fa8a
				const bool down = (event.type == X11Api::KeyPress);
				const bool repeat = (down && prev_event.type == X11Api::KeyRelease && event.xkey.time == prev_event.xkey.time && event.xkey.keycode == prev_event.xkey.keycode);
				Window w{ event.xkey.window };
				auto& wc = w.getConfig();
				if (wc.key_callback)
				{
					wc.key_callback(w, x.lookupKeysym(&event.xkey, 0), down, repeat);
				}
			}
			else if (event.type == X11Api::Expose)
			{
				//std::cout << "Got expose event for " << event.xexpose.window << "\n";
				Window{ event.xexpose.window }.redraw();
			}
			prev_event = std::move(event);
		}
		x.running_message_loop = false;
	}
#endif
}
