#include "cli.hpp"

#include <iostream>

#include <base64.hpp>
#include <netInfo.hpp>
#include <netMesh.hpp>
#include <netMeshService.hpp>
#include <rand.hpp>
#include <Server.hpp>
#include <string.hpp>
#include <StringWriter.hpp>

using namespace soup;

int cli_mesh(int argc, const char** argv)
{
	if (argc > 0)
	{
		std::string subcommand = argv[0];
		string::lower(subcommand);

		if (subcommand == "link")
		{
			auto& my_config = netMesh::getMyConfig();
			/*if (argc > 1)
			{
				LinkData data;
				data.fromBinary(base64::decode(argv[1]));

				IpAddr ip((native_u32_t)data.ip);

				std::cout << "Connecting to " << ip.toString4() << "..." << std::flush;
				Scheduler sched;
				auto sock = sched.addSocket();
				if (sock->connect(ip, 7106))
				{
					std::cout << " Connected." << std::endl;

					netMesh::enableCryptoClient(*sock, data.n);

					StringWriter sw;
					{ uint8_t msg_type = MESH_MSG_LINK; sw.u8(msg_type); }
					sw.u64(cap.get<LinkData>().passnum);
					netMesh::sendRaw(s, sw.data + netMesh::getMyConfig().kp.n.toBinary());

					std::cout << "Linking..." << std::flush;
					s.recv([](Socket&, std::string&& data, Capture&& cap) SOUP_EXCAL
					{
						if (data.at(0) == MESH_MSG_AFFIRMATIVE)
						{
							LinkData& data = cap.get<LinkData>();
							netMesh::addPeerLocally(data.n, data.ip);
							std::cout << " Successfully linked.\n";
							exit(0);
						}
					}, std::move(cap));

					sched.run();

					std::cout << " Failed.\n";
				}
				else
				{
					std::cout << " Failed to connect.\n";
				}
				return 1;
			}
			else*/
			{
				std::cout << "Discovering IP address..." << std::flush;
				auto ip = netInfo::getPublicAddressV4();
				std::cout << " Found " << ip.toString4() << "\n";

				Server serv;
				netMeshService mesh_service;
				if (!mesh_service.bind(serv))
				{
					std::cout << "Failed to bind to TCP/7106\n";
					return 1;
				}

				mesh_service.link_passnum = soup::rand(0, -1);

				StringWriter sw;
				sw.u32le(ip.getV4NativeEndian());
				sw.u64le(mesh_service.link_passnum);

				std::cout << "\nReady to link: " << base64::encode(sw.data + my_config.kp.n.toBinary()) << "\n";

				serv.run();
			}
			return 0;
		}

		if (subcommand == "list")
		{
			auto& config = netMesh::getMyConfig();
			if (config.peers.empty())
			{
				std::cout << "No peers configured with this machine.\n";
			}
			else
			{
				for (const auto& peer : config.peers)
				{
					if (peer.ip)
					{
						std::cout << "- " << IpAddr((native_u32_t)peer.ip).toString4() << "\n";
					}
					else
					{
						std::cout << "- Administrative device " << string::hex(peer.n_hash) << "\n";
					}
				}
			}
			return 0;
		}
	}

	std::cout << "Syntax: mesh [link|list]\n";
	return 0;
}
