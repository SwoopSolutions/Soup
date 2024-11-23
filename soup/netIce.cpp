#include "netIce.hpp"
#if !SOUP_WASM

#include "netStun.hpp"
#include "Scheduler.hpp"
#include "Socket.hpp"
#include "StringWriter.hpp"

NAMESPACE_SOUP
{
	bool netIce::validateCandidateAsClient(const IpAddr& addr, uint16_t port, const std::string& offer_username, const std::string& answer_username, const std::string& offer_password, unsigned int timeout_ms)
	{
		std::string username = offer_username + ":" + answer_username;
		size_t username_pad = (4 - (username.size() % 4));

		StringWriter sw;
		uint32_t i = static_cast<uint32_t>((0x0001 << 16) | (4 + username.size() + username_pad + 8 + 12 + 24 + 8)); // Binding Request, length: USERNAME + PRIORITY + ICE-CONTROLLED + MESSAGE-INTEGRITY + FINGERPRINT
		sw.u32be(i);
		i = 0x2112A442;
		sw.u32be(i);
		i = 0;
		sw.u32be(i);
		sw.u32be(i);
		sw.u32be(i);

		i = (0x0006 << 16) | static_cast<uint32_t>(static_cast<uint16_t>(username.size())); // USERNAME
		sw.u32be(i);
		sw.str(username.size(), username.data());
		sw.skip(username_pad);

		i = (0x0024 << 16) | 4; // PRIORITY
		sw.u32be(i);
		i = 1;
		sw.u32be(i);

		i = (0x8029 << 16) | 8; // ICE-CONTROLLED
		sw.u32be(i);
		i = 0;
		sw.u32be(i); // Tiebreaker
		sw.u32be(i); // Tiebreaker

		netStun::addMessageIntegrity(sw.data, offer_password);
		netStun::addFingerprint(sw.data);

		bool ret = false;
		Scheduler sched;
		Socket& sock = *sched.addSocket();
		if (sock.udpClientSend(addr, port, sw.data))
		{
			sock.udpRecv([](Socket&, SocketAddr&&, std::string&& data, Capture&& cap)
			{
				*cap.get<bool*>() = true;
			}, &ret);
			sched.runFor(timeout_ms);
		}
		return ret;
	}
}

#endif
