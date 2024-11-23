#include "netMeshService.hpp"
#if !SOUP_WASM

#include <iostream>

#include "aes.hpp"
#include "MemoryRefReader.hpp"
#include "netMesh.hpp"
#include "netMeshMsgType.hpp"
#include "rand.hpp"
#include "Server.hpp"
#include "sha256.hpp"
#include "Socket.hpp"
#include "StringWriter.hpp"
#include "WebSocketMessage.hpp"

NAMESPACE_SOUP
{
	struct netMeshConnectionInfo
	{
		alignas(16) uint8_t recv_key[16];
		alignas(16) uint8_t send_key[16];

		std::string challenge;
		uint32_t authenticated_as = 0;

		uint32_t recv_seq = 0;
		uint32_t send_seq = 0;

		void send(Socket& s, std::string data)
		{
			aes::pkcs7Pad(data);
			aes::ecbEncrypt(
				(uint8_t*)data.data(), data.size(),
				send_key, sizeof(send_key)
			);
			ServerWebService::wsSend(s, data, false);
		}
	};

	netMeshService::netMeshService()
	{
		should_accept_websocket_connection = [](Socket&, const HttpRequest&, ServerWebService&) SOUP_EXCAL -> bool
		{
			return true;
		};

		on_websocket_message = [](WebSocketMessage& msg, Socket& s, ServerWebService& srv) -> void
		{
			if (s.custom_data.isStructInMap(netMeshConnectionInfo))
			{
				auto& con_info = s.custom_data.getStructFromMapConst(netMeshConnectionInfo);
				aes::ecbDecrypt(
					(uint8_t*)msg.data.data(), msg.data.size(),
					con_info.recv_key, sizeof(netMeshConnectionInfo::recv_key)
				);
				if (aes::pkcs7Unpad(msg.data))
				{
					MemoryRefReader sr(msg.data);
					if (uint8_t msg_type; sr.u8(msg_type))
					{
						if (msg_type == MESH_MSG_LINK)
						{
							if (uint64_t passnum; sr.u64le(passnum))
							{
								if (passnum == static_cast<netMeshService&>(srv).link_passnum)
								{
									con_info.send(s, std::string(1, MESH_MSG_AFFIRMATIVE));
									auto remote_pub_n = Bigint::fromBinary(msg.data.data() + 9, msg.data.size() - 9);
									netMesh::addPeerLocally(std::move(remote_pub_n));
									std::cout << "\nSuccessfully linked.\n";
									exit(0);
								}
								else
								{
									con_info.send(s, std::string(1, MESH_MSG_NEGATIVE));
								}
							}
						}
						else if (msg_type == MESH_MSG_AUTH_REQUEST)
						{
							if (con_info.authenticated_as == 0)
							{
								con_info.challenge = soup::rand.binstr(14);

								std::string challenge_msg(1, MESH_MSG_AUTH_CHALLENGE);
								challenge_msg.append(con_info.challenge);
								con_info.send(s, std::move(challenge_msg));
								return;
							}
						}
						else if (msg_type == MESH_MSG_AUTH_FINISH)
						{
							if (!con_info.challenge.empty())
							{
								uint32_t n_hash;
								if (sr.u32le(n_hash))
								{
									auto sig = Bigint::fromBinary(msg.data.data() + 5, msg.data.size() - 5);
									if (auto peer = netMesh::getMyConfig().findPeer(n_hash, s.peer.ip.getV4NativeEndian()))
									{
										if (peer->getPublicKey().verify<sha256>(con_info.challenge, sig))
										{
											con_info.authenticated_as = n_hash;
											con_info.send(s, std::string(1, MESH_MSG_AFFIRMATIVE));
											return;
										}
									}
								}
							}
						}
						else if (msg_type == MESH_MSG_CAPABILITIES)
						{
							if (con_info.authenticated_as != 0)
							{
								std::string resp(1, MESH_MSG_CAPABILITIES);
								for (const auto& e : static_cast<netMeshService&>(srv).app_msg_handlers)
								{
									resp.append(e.first);
									resp.push_back(',');
								}
								if (resp.back() == ',')
								{
									resp.pop_back();
								}
								con_info.send(s, std::move(resp));
								return;
							}
						}
						else if (msg_type == MESH_MSG_APPLICATION)
						{
							if (con_info.authenticated_as != 0)
							{
								uint32_t seq;
								if (sr.u32le(seq)
									&& con_info.recv_seq++ == seq
									)
								{
									auto sep = msg.data.find(',', 5);
									if (sep != std::string::npos)
									{
										if (auto e = static_cast<netMeshService&>(srv).app_msg_handlers.find(msg.data.substr(5, sep - 5));
											e != static_cast<netMeshService&>(srv).app_msg_handlers.end()
											)
										{
											try
											{
												e->second(s, msg.data.substr(sep + 1));
											}
											catch (const std::exception& e)
											{
												replyNegative(s, e.what());
											}
											return;
										}
									}
								}
							}
						}
					}
				}
			}
			else
			{
				auto data = netMesh::getMyConfig().kp.getPrivate().decryptUnpadded(Bigint::fromBinary(msg.data));
				if (RsaMod::unpad(data)
					&& data.size() == 32
					)
				{
					memcpy(s.custom_data.getStructFromMap(netMeshConnectionInfo).recv_key, data.data(), 16);
					memcpy(s.custom_data.getStructFromMap(netMeshConnectionInfo).send_key, data.data() + 16, 16);
					return;
				}
			}
			s.close();
		};
	}

	bool netMeshService::bind(Server& serv)
	{
		return serv.bind(7106, this);
	}

	void netMeshService::reply(Socket& s, const std::string& data)
	{
		return reply(s, MESH_MSG_APPLICATION, data);
	}

	void netMeshService::replyAffirmative(Socket& s, const std::string& data)
	{
		return reply(s, MESH_MSG_AFFIRMATIVE, data);
	}

	void netMeshService::replyNegative(Socket& s, const std::string& data)
	{
		return reply(s, MESH_MSG_NEGATIVE, data);
	}

	void netMeshService::reply(Socket& s, uint8_t msg_type, const std::string& data)
	{
		auto& con_info = s.custom_data.getStructFromMapConst(netMeshConnectionInfo);
		StringWriter sw;
		sw.u8(msg_type);
		sw.u32le(con_info.send_seq);
		con_info.send(s, sw.data + data);
		++con_info.send_seq;
	}
}

#endif
