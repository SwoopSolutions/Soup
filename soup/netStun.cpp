#include "netStun.hpp"
#if !SOUP_WASM

#include "crc32.hpp"
#include "Scheduler.hpp"
#include "sha1.hpp"
#include "Socket.hpp"
#include "StringReader.hpp"
#include "StringWriter.hpp"

NAMESPACE_SOUP
{
	IpAddr netStun::queryBinding(const IpAddr& server_addr, uint16_t server_port, unsigned int timeout_ms)
	{
		IpAddr ret;

		StringWriter sw;
		uint32_t i = 0x0001 << 16; // Binding Request, Length 0
		sw.u32be(i);
		i = 0x2112A442;
		sw.u32be(i);
		i = 0;
		sw.u32be(i);
		sw.u32be(i);
		sw.u32be(i);

		Scheduler sched;
		Socket& sock = *sched.addSocket();
		if (sock.udpClientSend(server_addr, server_port, sw.data))
		{
			sock.udpRecv([](Socket&, SocketAddr&&, std::string&& data, Capture&& cap)
			{
				StringReader sr(std::move(data));
				sr.skip(20);
				uint32_t i;
				while (sr.u32be(i))
				{
					if ((i >> 16) == 0x20) // XOR-MAPPED-ADDRESS
					{
						sr.u32be(i);
						if ((i >> 16) == 1) // IPv4
						{
							sr.u32be(i);
							i ^= 0x2112A442;
							*cap.get<IpAddr*>() = IpAddr((native_u32_t)i);
						}
						else
						{
							uint32_t a, b, c, d;
							sr.u32be(a);
							sr.u32be(b);
							sr.u32be(c);
							sr.u32be(d);
							a ^= 0x2112A442;
							// remaining ints are XORed with transaction id, but we use all zeroes, so we're good.
							*cap.get<IpAddr*>() = IpAddr(a, b, c, d);
						}
					}
					else
					{
						sr.skip(i & 0xffff);
					}
					sr.skip(4 - (i % 4));
				}
			}, &ret);
			sched.runFor(timeout_ms);
		}

		return ret;
	}

	void netStun::addMessageIntegrity(std::string& data, const std::string& key) SOUP_EXCAL
	{
		// Compute data to HMAC
		StringWriter sw;
		sw.data = data.substr(0, 2); // keep type
		uint16_t s = static_cast<uint16_t>((data.size() - 20) + 24); sw.u16be(s); // message length excludes header but includes MESSAGE-INTEGRITY
		sw.data.append(data.substr(4));

		// Compute checksum
		auto chksum = sha1::hmac(sw.data, key);

		// Add MESSAGE-INTEGRITY to data
		sw.data = std::move(data);
		uint32_t i = (0x0008 << 16) | 20; // MESSAGE-INTEGRITY, length 20
		sw.u32be(i);
		sw.str(20, chksum);
		data = std::move(sw.data);
	}

	void netStun::addFingerprint(std::string& data) noexcept
	{
		StringWriter sw;
		sw.data = std::move(data);

		uint32_t chksum = crc32::hash(sw.data) ^ 0x5354554e;
		uint32_t i = (0x8028 << 16) | 4; // FINGERPRINT, length 4
		sw.u32be(i);
		sw.u32be(chksum);

		data = std::move(sw.data);
	}
}

#endif
