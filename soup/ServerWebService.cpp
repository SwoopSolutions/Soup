#include "ServerWebService.hpp"

#if !SOUP_WASM

#include "HttpRequest.hpp"
#include "MimeType.hpp"
#include "Socket.hpp"
#include "StringWriter.hpp"
#include "WebSocket.hpp"
#include "WebSocketFrameType.hpp"
#include "WebSocketMessage.hpp"

NAMESPACE_SOUP
{
	struct WebServerClientData
	{
		bool keep_alive = false;
	};

	ServerWebService::ServerWebService(handle_request_t handle_request)
		: ServerService([](Socket& s, ServerService& srv, Server&)
		{
			static_cast<ServerWebService&>(srv).httpRecv(s);
		}), handle_request(handle_request)
	{
	}

	void ServerWebService::disableKeepAlive(Socket& s)
	{
		s.custom_data.getStructFromMap(WebServerClientData).keep_alive = false;
	}

	void ServerWebService::sendContent(Socket& s, std::string body)
	{
		sendContent(s, "200", std::move(body));
	}

	void ServerWebService::sendContent(Socket& s, const char* status, std::string body)
	{
		HttpResponse resp;
		resp.body = std::move(body);
		sendContent(s, status, std::move(resp));
	}

	void ServerWebService::sendContent(Socket& s, HttpResponse&& resp)
	{
		sendContent(s, "200", std::move(resp));
	}

	void ServerWebService::sendContent(Socket& s, const char* status, HttpResponse&& resp)
	{
		resp.setContentLength();
		resp.setContentType();
		sendResponse(s, status, resp.toString());
	}

	void ServerWebService::sendHtml(Socket& s, const std::string& body)
	{
		sendData(s, MimeType::TEXT_HTML, body, true);
	}

	void ServerWebService::sendText(Socket& s, const std::string& body)
	{
		sendData(s, MimeType::TEXT_PLAIN, body, false);
	}

	void ServerWebService::sendData(Socket& s, const char* mime_type, const std::string& body, bool is_private)
	{
		std::string data;
		data.reserve(body.size() + 120);
		if (is_private)
		{
			data.append("Cache-Control: private");
		}
		else
		{
			data.append("Access-Control-Allow-Origin: *");
		}
		data.append("\r\nContent-Type: ").append(mime_type);
		data.append("\r\nContent-Length: ").append(std::to_string(body.size()));
		data.append("\r\n\r\n");
		data.append(body);
		sendResponse(s, "200", data);
	}

	void ServerWebService::sendRedirect(Socket& s, const std::string& location)
	{
		std::string cont = "Location: ";
		cont.append(location);
		cont.append("\r\nContent-Length: 0\r\n\r\n");
		sendResponse(s, "302", cont);
	}

	void ServerWebService::send204(Socket& s)
	{
		sendResponse(s, "204", "Content-Length: 0\r\n\r\n");
	}

	void ServerWebService::send400(Socket& s)
	{
		sendResponse(s, "400", "Content-Length: 0\r\n\r\n");
	}

	void ServerWebService::send404(Socket& s)
	{
		sendResponse(s, "404", "Content-Length: 0\r\n\r\n");
	}

	void ServerWebService::sendResponse(Socket& s, const char* status, const std::string& headers_and_body)
	{
		std::string cont = "HTTP/1.0 ";
		cont.append(status);
		cont.append("\r\nServer: Soup\r\nConnection: ");
		cont.append(s.custom_data.getStructFromMap(WebServerClientData).keep_alive ? "keep-alive" : "close");
		cont.append("\r\n");
		cont.append(headers_and_body);
		s.send(std::move(cont));
	}
	
	void ServerWebService::wsSendText(Socket& s, const std::string& data)
	{
		wsSend(s, WebSocketFrameType::TEXT, data);
	}

	void ServerWebService::wsSendBin(Socket& s, const std::string& data)
	{
		wsSend(s, WebSocketFrameType::BINARY, data);
	}

	void ServerWebService::wsSend(Socket& s, const std::string& data, bool is_text)
	{
		wsSend(s, (is_text ? WebSocketFrameType::TEXT : WebSocketFrameType::BINARY), data);
	}

	void ServerWebService::wsSend(Socket& s, uint8_t opcode, const std::string& payload)
	{
		StringWriter w;
		opcode |= 0x80; // fin
		if (w.u8(opcode))
		{
			if (payload.size() <= 125)
			{
				uint8_t buf = static_cast<uint8_t>(payload.size());
				if (!w.u8(buf))
				{
					return;
				}
			}
			else if (payload.size() <= 0xFFFF)
			{
				if (uint8_t buf = 126; !w.u8(buf))
				{
					return;
				}
				if (uint16_t buf = static_cast<uint16_t>(payload.size()); !w.u16_be(buf))
				{
					return;
				}
			}
			else
			{
				if (uint8_t buf = 127; !w.u8(buf))
				{
					return;
				}
				if (uint64_t buf = payload.size(); !w.u64_be(buf))
				{
					return;
				}
			}
		}
		w.data.append(payload);
		s.send(w.data);
	}

	void ServerWebService::httpRecv(Socket& s)
	{
		s.recv([](Socket& s, std::string&& data, Capture&& cap)
		{
			HttpRequest req{};
			auto method_end = data.find(' ');
			if (method_end == std::string::npos)
			{
			_bad_request:
				s.send("HTTP/1.0 400\r\n\r\n");
				s.close();
				return;
			}
			req.method = data.substr(0, method_end);
			method_end += 1;
			auto path_end = data.find(' ', method_end);
			if (path_end == std::string::npos)
			{
				goto _bad_request;
			}
			req.path = data.substr(method_end, path_end - method_end);
			path_end += 1;
			auto message_start = data.find("\r\n", path_end);
			if (message_start == std::string::npos)
			{
				goto _bad_request;
			}
			message_start += 2;
			req.loadMessage(data.substr(message_start));

			ServerWebService& srv = *cap.get<ServerWebService*>();

			if (auto upgrade_value = req.findHeader("Upgrade"))
			{
				if (*upgrade_value == "websocket")
				{
					if (auto key_value = req.findHeader("Sec-WebSocket-Key"))
					{
						if (srv.should_accept_websocket_connection
							&& srv.should_accept_websocket_connection(s, req, srv)
							)
						{
							// Firefox throws a SkillIssueException if we say HTTP/1.0
							std::string cont = "HTTP/1.1 101\r\nConnection: Upgrade\r\nUpgrade: websocket\r\nServer: Soup\r\nSec-WebSocket-Accept: ";
							cont.append(WebSocket::hashKey(*key_value));
							cont.append("\r\n\r\n");
							s.send(cont);

							s.custom_data.removeStructFromMap(WebServerClientData);

							if (srv.on_websocket_connection_established)
							{
								srv.on_websocket_connection_established(s, req, srv);
							}

							srv.wsRecv(s);
						}
					}
				}
				return;
			}

			if (srv.handle_request)
			{
				if (auto connection_entry = req.header_fields.find("Connection"); connection_entry != req.header_fields.end())
				{
					if (connection_entry->second == "keep-alive")
					{
						s.custom_data.getStructFromMap(WebServerClientData).keep_alive = true;
					}
				}

				srv.handle_request(s, std::move(req), srv);

				if (s.custom_data.getStructFromMap(WebServerClientData).keep_alive)
				{
					srv.httpRecv(s);
				}
			}
		}, this);
	}

	void ServerWebService::wsRecv(Socket& s)
	{
		s.recv([](Socket& s, std::string&& data, Capture&& cap) // on_websocket_message may throw
		{
			ServerWebService& srv = *cap.get<ServerWebService*>();

			bool fin;
			uint8_t opcode;
			std::string payload;
			while (WebSocket::readFrame(data, fin, opcode, payload) == WebSocket::OK)
			{
				if (opcode <= WebSocketFrameType::_NON_CONTROL_MAX) // non-control frame
				{
					WebSocketMessage& msg_buf = s.custom_data.getStructFromMap(WebSocketMessage);

					if (opcode != 0)
					{
						msg_buf.data = std::move(payload);
						msg_buf.is_text = (opcode == WebSocketFrameType::TEXT);
					}
					else
					{
						msg_buf.data.append(payload);
					}

					if (fin)
					{
						if (srv.on_websocket_message)
						{
							srv.on_websocket_message(msg_buf, s, srv);
						}
						msg_buf.data.clear();
					}
				}
				else // control frame
				{
					if (opcode == WebSocketFrameType::PING)
					{
						wsSend(s, WebSocketFrameType::PONG, payload);
					}
					else if (opcode != WebSocketFrameType::PONG)
					{
						s.close();
						return;
					}
				}

				srv.wsRecv(s);
			}
		}, this);
	}
}

#endif
