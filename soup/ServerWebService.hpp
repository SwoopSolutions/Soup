#pragma once

#include "base.hpp"
#if !SOUP_WASM
#include "fwd.hpp"
#include "type.hpp"

#include "ServerService.hpp"

NAMESPACE_SOUP
{
	class ServerWebService : public ServerService
	{
	public:
		using handle_request_t = void(*)(Socket&, HttpRequest&&, ServerWebService&);
		using should_accept_websocket_connection_t = bool(*)(Socket&, const HttpRequest&, ServerWebService&);
		using on_websocket_connection_established_t = void(*)(Socket&, const HttpRequest&, ServerWebService&);
		using on_websocket_message_t = void(*)(WebSocketMessage&, Socket&, ServerWebService&);

		handle_request_t handle_request = nullptr;
		should_accept_websocket_connection_t should_accept_websocket_connection = nullptr;
		on_websocket_connection_established_t on_websocket_connection_established = nullptr;
		on_websocket_message_t on_websocket_message = nullptr;

		ServerWebService(handle_request_t handle_request = nullptr);

		// HTTP
		static void disableKeepAlive(Socket& s);
		static void sendContent(Socket& s, std::string body);
		static void sendContent(Socket& s, const char* status, std::string body);
		static void sendContent(Socket& s, HttpResponse&& resp);
		static void sendContent(Socket& s, const char* status, HttpResponse&& resp);
		static void sendHtml(Socket& s, const std::string& body);
		static void sendHtml(Socket& s, const char* data, size_t size);
		static void sendText(Socket& s, const std::string& body);
		static void sendText(Socket& s, const char* data, size_t size);
		static void sendData(Socket& s, const char* mime_type, const std::string& body, bool is_private = true) { return sendData(s, mime_type, body.data(), body.size(), is_private); }
		static void sendData(Socket& s, const char* mime_type, const char* data, size_t size, bool is_private = true);
		static void sendRedirect(Socket& s, const std::string& location);
		static void send204(Socket& s);
		static void send400(Socket& s);
		static void send404(Socket& s);
		static void send500(Socket& s);
		static void sendResponse(Socket& s, const char* status, const std::string& headers_and_body);

		// WebSocket
		static void wsSendText(Socket& s, const std::string& data);
		static void wsSendBin(Socket& s, const std::string& data);
		static void wsSend(Socket& s, const std::string& data, bool is_text);
		static void wsSend(Socket& s, uint8_t opcode, const std::string& payload);

	protected:
		void httpRecv(Socket& s);
		void wsRecv(Socket& s);
	};
}
#endif
