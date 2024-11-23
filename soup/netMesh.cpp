#include "netMesh.hpp"

#include "base.hpp"
#if !SOUP_WASM

#include <iostream>

#include "FileReader.hpp"
#include "FileWriter.hpp"
#include "filesystem.hpp"
#include "fnv.hpp"

NAMESPACE_SOUP
{
	using Peer = netMesh::Peer;
	using MyConfig = netMesh::MyConfig;

	[[nodiscard]] static std::filesystem::path getDataPath() noexcept
	{
		auto path = filesystem::getProgramData();
		path /= "Calamity, Inc";
		path /= "Soup";
		path /= "Mesh Network";
		return path;
	}

	bool netMesh::isEnabled() noexcept
	{
		std::error_code ec;
		return std::filesystem::is_regular_file(getDataPath() / "keypair.bin", ec)
			&& !getMyConfig().peers.empty()
			;
	}

	static MyConfig s_my_config;

	MyConfig& netMesh::getMyConfig() SOUP_EXCAL
	{
		std::error_code ec;
		if (s_my_config.kp.n.isZero())
		{
			const auto path = getDataPath();
			if (!std::filesystem::is_directory(path, ec))
			{
				std::filesystem::create_directories(path, ec);
			}

			const auto kp_path = path / "keypair.bin";
			if (std::filesystem::is_regular_file(kp_path, ec))
			{
				FileReader fr(kp_path);
				Bigint p, q;
				if (p.io(fr) && q.io(fr))
				{
					s_my_config.kp = RsaKeypair(std::move(p), std::move(q));
				}
			}
			if (s_my_config.kp.n.isZero())
			{
				std::cout << "One-time setup: Generating keypair for this machine...\n";
				s_my_config.kp = RsaKeypair::generate(1536, true);
				FileWriter fw(kp_path);
				fw.throwIfFailed();
				s_my_config.kp.p.io(fw);
				s_my_config.kp.q.io(fw);
			}

			const auto peers_path = path / "peers.bin";
			if (std::filesystem::is_regular_file(peers_path, ec))
			{
				FileReader fr(peers_path);
				fr.skip(1); // version
				if (uint64_t num_peers; fr.u64_dyn(num_peers))
				{
					while (num_peers--)
					{
						Peer& peer = s_my_config.peers.emplace_back();
						peer.n.io(fr);
						fr.u32le(peer.ip);
						peer.n_hash = hashN(peer.n);
					}
				}
			}
		}
		return s_my_config;
	}

	void netMesh::addPeerLocally(Bigint n, uint32_t ip) SOUP_EXCAL
	{
		const auto n_hash = hashN(n);

		if (ip == 0) // Administrative device?
		{
			if (s_my_config.findPeerByPublicKey(n_hash))
			{
				return; // Already known, no need to add it again.
			}
		}
		else
		{
			if (auto peer = s_my_config.findPeer(ip))
			{
				// Already know this peer; update key but don't create a duplicate entry.
				peer->n_hash = n_hash;
				peer->n = std::move(n);
				goto _save_peers;
			}
		}

		s_my_config.peers.emplace_back(Peer{ n_hash, ip, std::move(n) });

	_save_peers:
		FileWriter fw(getDataPath() / "peers.bin");
		fw.throwIfFailed();
		{ uint8_t version = 0; fw.u8(version); }
		fw.u64_dyn(s_my_config.peers.size());
		for (auto& peer : s_my_config.peers)
		{
			peer.n.io(fw);
			fw.u32le(peer.ip);
		}
	}

	uint32_t netMesh::hashN(const Bigint& n)
	{
		return fnv1a_32(n.toBinary());
	}

#if false
	struct netMeshClientInfo
	{
		alignas(16) uint8_t send_key[16];
		alignas(16) uint8_t recv_key[16];
		uint32_t send_seq = 0;
		uint32_t recv_seq = 0;
	};

	void netMesh::enableCryptoClient(Socket& s, const Bigint& remote_pub_n) SOUP_EXCAL
	{
		auto& info = s.custom_data.getStructFromMap(netMeshClientInfo);

		auto bytes = soup::rand.binstr(32);
		memcpy(info.send_key, bytes.data(), 16);
		memcpy(info.recv_key, bytes.data() + 16, 16);

		static_cast<WebSocketConnection&>(s).wsSend(RsaPublicKey(remote_pub_n).encryptPkcs1(bytes).toBinary());
	}
#endif

	Peer* netMesh::MyConfig::findPeer(uint32_t ip) noexcept
	{
		for (auto& peer : peers)
		{
			if (peer.ip == ip)
			{
				return &peer;
			}
		}
		return nullptr;
	}

	const Peer* netMesh::MyConfig::findPeerByPublicKey(uint32_t n_hash) const noexcept
	{
		for (const auto& peer : peers)
		{
			if (peer.n_hash == n_hash)
			{
				return &peer;
			}
		}
		return nullptr;
	}

	const Peer* netMesh::MyConfig::findPeer(uint32_t n_hash, uint32_t ip) const noexcept
	{
		for (const auto& peer : peers)
		{
			if (peer.n_hash == n_hash
				&& peer.isRepresentedByIp(ip)
				)
			{
				return &peer;
			}
		}
		return nullptr;
	}
}

#endif
