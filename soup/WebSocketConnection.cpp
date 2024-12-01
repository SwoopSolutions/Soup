#include "WebSocketConnection.hpp"
#if !SOUP_WASM

#include <queue>

#include "HttpRequest.hpp"
#include "rand.hpp"
#include "StringWriter.hpp"
#include "WebSocket.hpp"
#include "WebSocketFrameType.hpp"
#include "WebSocketMessage.hpp"

NAMESPACE_SOUP
{
	struct CaptureWsUpgrade
	{
		WebSocketConnection::callback_t cb;
		Capture cap;
	};

	static void upgradeRecvLoop(Socket& s, Capture&& cap) SOUP_EXCAL
	{
		s.recv([](Socket& s, std::string&& data, Capture&& cap) SOUP_EXCAL
		{
			if (data.find("\r\n\r\n") != std::string::npos)
			{
				CaptureWsUpgrade& c = cap.get<CaptureWsUpgrade>();
				c.cb(static_cast<WebSocketConnection&>(s), std::move(c.cap));
			}
			else
			{
				upgradeRecvLoop(s, std::move(cap));
			}
		}, std::move(cap));
	}

	void WebSocketConnection::upgrade(std::string host, std::string path, callback_t cb, Capture&& cap) SOUP_EXCAL
	{
		sendUpgradeRequest(std::move(host), std::move(path));
		upgradeRecvLoop(*this, CaptureWsUpgrade{ cb, std::move(cap) });
	}

	void WebSocketConnection::sendUpgradeRequest(std::string host, std::string path) SOUP_EXCAL
	{
		HttpRequest req(std::move(host), std::move(path));
		req.header_fields.at("Connection") = "Upgrade";
		req.header_fields.emplace("Upgrade", "websocket");
		req.header_fields.emplace("Sec-WebSocket-Key", WebSocket::generateKey());
		req.header_fields.emplace("Sec-WebSocket-Version", "13");
		req.send(*this);
	}

	void WebSocketConnection::wsSend(std::string data, bool is_text) SOUP_EXCAL
	{
		wsSend((is_text ? WebSocketFrameType::TEXT : WebSocketFrameType::BINARY), std::move(data));
	}

	void WebSocketConnection::wsSend(uint8_t opcode, std::string payload) SOUP_EXCAL
	{
		StringWriter w;
		opcode |= 0x80; // fin
		if (w.u8(opcode))
		{
			if (payload.size() <= 125)
			{
				uint8_t buf = static_cast<uint8_t>(payload.size());
				buf |= 0x80; // has mask
				if (!w.u8(buf))
				{
					return;
				}
			}
			else if (payload.size() <= 0xFFFF)
			{
				if (uint8_t buf = (126 | 0x80); !w.u8(buf))
				{
					return;
				}
				if (uint16_t buf = static_cast<uint16_t>(payload.size()); !w.u16be(buf))
				{
					return;
				}
			}
			else
			{
				if (uint8_t buf = (127 | 0x80); !w.u8(buf))
				{
					return;
				}
				if (uint64_t buf = payload.size(); !w.u64be(buf))
				{
					return;
				}
			}
		}

		uint8_t mask[4];
		rand.fill(mask);

		for (size_t i = 0; i != payload.size(); ++i)
		{
			payload[i] ^= mask[i % 4];
		}

		if (!w.str(4, (const char*)mask))
		{
			return;
		}

		w.data.append(payload);
		this->send(w.data);
	}

	struct WsRecvBuffer
	{
		std::string buf;
	};

	struct CaptureWsRecv
	{
		WebSocketConnection::recv_callback_t cb;
		Capture cap;
	};

	void WebSocketConnection::wsRecv(recv_callback_t cb, Capture&& cap) SOUP_EXCAL
	{
		recv([](Socket& s, std::string&& app, Capture&& _cap) SOUP_EXCAL
		{
			auto& buf = s.custom_data.getStructFromMap(WsRecvBuffer).buf;
			auto& cap = _cap.get<CaptureWsRecv>();

			buf.append(app);

			bool fin;
			uint8_t opcode;
			std::string payload;
			while (true)
			{
				auto res = WebSocket::readFrame(buf, fin, opcode, payload);
				if (res != WebSocket::OK)
				{
					if (res == WebSocket::PAYLOAD_INCOMPLETE)
					{
						static_cast<WebSocketConnection&>(s).wsRecv(cap.cb, std::move(cap.cap));
					}
					break;
				}

				SOUP_IF_UNLIKELY (!fin || (opcode != WebSocketFrameType::TEXT && opcode != WebSocketFrameType::BINARY))
				{
					SOUP_IF_LIKELY (opcode == WebSocketFrameType::CLOSE)
					{
						payload.erase(0, 1);
						s.remote_closed = true;
						s.custom_data.getStructFromMap(SocketCloseReason) = std::move(payload);
					}
					cap.cb(static_cast<WebSocketConnection&>(s), {}, std::move(cap.cap));
					return;
				}

				WebSocketMessage msg;
				msg.is_text = (opcode == WebSocketFrameType::TEXT);
				msg.data = std::move(payload);

				cap.cb(static_cast<WebSocketConnection&>(s), std::move(msg), std::move(cap.cap));
			}
		}, CaptureWsRecv{ cb, std::move(cap) });
	}

	struct WebSocketPromiseOverflowData
	{
		std::queue<WebSocketMessage> data;
	};

	SharedPtr<Promise<WebSocketMessage>> WebSocketConnection::wsRecv() SOUP_EXCAL
	{
		if (custom_data.isStructInMap(WebSocketPromiseOverflowData))
		{
			std::queue<WebSocketMessage>& data = custom_data.getStructFromMapConst(WebSocketPromiseOverflowData).data;
			if (!data.empty())
			{
				auto p = soup::make_shared<Promise<WebSocketMessage>>(std::move(data.front()));
				data.pop();
				return p;
			}
		}
		auto p = soup::make_shared<Promise<WebSocketMessage>>();
		wsRecv([](WebSocketConnection& s, WebSocketMessage&& msg, Capture&& cap) SOUP_EXCAL
		{
			if (!cap.get<SharedPtr<Promise<WebSocketMessage>>>()->isFulfilled())
			{
				cap.get<SharedPtr<Promise<WebSocketMessage>>>()->fulfil(std::move(msg));
			}
			else
			{
				s.custom_data.getStructFromMap(WebSocketPromiseOverflowData).data.push(std::move(msg));
			}
		}, p);
		return p;
	}
}

#endif
