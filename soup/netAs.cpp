#include "netAs.hpp"

#include "string.hpp"

namespace soup
{
	bool netAs::isHosting() const noexcept
	{
		switch (number)
		{
		case 3214: // xTom GmbH
		case 4785: // xTom Limited
		case 6233: // xTom
		case 8100: // QuadraNet Enterprises LLC
		case 9009: // M247 Ltd
		case 9304: // HGC Global Communications Limited
		case 9312: // xTom Hong Kong Limited
		case 9335: // CAT Telecom Public Company Limited
		case 11831: // eSecureData
		case 11878: // tzulo, inc.
		case 12876: // ONLINE S.A.S.
		case 13046: // ISKON INTERNET d.d. za informatiku i telekomunikacije
		case 13213: // UK-2 Limited
		case 16247: // M247 Ltd
		case 20278: // Nexeon Technologies, Inc.
		case 20473: // The Constant Company, LLC
		case 25369: // Hydra Communications Ltd
		case 29485: // A1 Hrvatska d.o.o.
		case 30083: // GoDaddy.com, LLC
		case 26496: // GoDaddy.com, LLC
		case 30736: // ASERGO Scandinavia ApS
		case 34594: // OT - OPTIMA TELEKOM d.d.
		case 35758: // Rachamim Aviel Twito trading as A.B INTERNET SOLUTIONS
		case 37518: // Fiber Grid INC
		case 38731: // Vietel - CHT Compamy Ltd
		case 39351: // 31173 Services AB
		case 40676: // Psychz Networks
		case 41564: // Orion Network Limited
		case 43289: // Trabia SRL
		case 43513: // Sia Nano IT
		case 45102: // Alibaba US Technology Co. Ltd.
		case 45899: // VNPT Corp
		case 46562: // Performive LLC
		case 46805: // Inter Connects Inc
		case 46844: // Sharktech
		case 47810: // Proservice LLC
		case 49981: // WorldStream B.V.
		case 50304: // Blix Solutions AS
		case 51747: // Internet Vikings International AB
		case 52423: // Data Miners S.A. ( Racknation.cr )
		case 53013: // W I X NET DO BRASIL LTDA - ME
		case 58182: // Wix.com Ltd.
		case 53667: // FranTech Solutions
		case 55664: // PT Inovasi Global Mumpuni
		case 58065: // Packet Exchange Limited
		case 58073: // YISP B.V.
		case 60068: // Datacamp Limited
		case 62240: // Clouvider Limited
		case 131199: // Nexeon Technologies, Inc.
		case 132203: // Tencent Building Kejizhongyi Avenue
		case 133480: // Intergrid Group Pty Ltd
		case 133752: // Leaseweb Asia Pacific pte. ltd.
		case 136787: // TEFINCOM S.A.
		case 136897: // EnjoyVC Cloud Group Limited
		case 137263: // NETEASE (HONG KONG) LIMITED
		case 137409: // GSL Networks Pty LTD
		case 141039: // TEFINCOM S.A.
		case 141167: // AgotoZ HK Limited
		case 146834: // XUNYOU SiChuan XunYou Network Technologe Limit Co
		case 147049: // PacketHub S.A.
		case 197706: // Keminet SHPK
		case 198371: // NINET Company Nis d.o.o.
		case 198605: // AVAST Software s.r.o.
		case 198621: // AVAST Software s.r.o.
		case 199524: // G-Core Labs S.A.
		case 205053: // Asimia Damaskou
		case 205119: // TELEKS DOOEL Skopje
		case 206804: // EstNOC OY
		case 212238: // Datacamp Limited
		case 213277: // ALMOUROLTEC SERVICOS DE INFORMATICA E INTERNET LDA
		case 263702: // GRUPO ZGH SPA
		case 268581: // QNAX LTDA
		case 22612: // Namecheap, Inc.
		case 8560: // IONOS SE
		case 47846: // SEDO GmbH
		case 46606: // Unified Layer
		case 35916: // MULTACOM CORPORATION
		case 3900: // SWITCH, LTD
		case 53831: // Squarespace, Inc.
		case 6724: // Strato AG
		case 18779: // EGIHosting
		case 32751: // Nuclearfallout Enterprises, Inc (NFO)
		case 206092: // IPXO Limited (Bandito Networks Inc), related to Express VPN
		case 36352: // ColoCrossing
		case 60087: // Netsons s.r.l. (Uania Cloud Service)
		case 8075: // Microsoft / Azure
		case 47674: // related to BLAZINGFAST
		case 206444: // KUBBUR
		case 14586: // Nuclearfallout Enterprises, Inc. (NFO)
			// Leaseweb USA, Inc.
		case 7203:
		case 19148:
		case 27411:
		case 30633:
		case 393886:
		case 394380:
		case 395954:
		case 396190:
		case 396362:
			// Cogent Communications
		case 174:
		case 2149:
		case 4550:
		case 6259:
		case 6299:
		case 6494:
		case 6496:
		case 7061:
		case 7458:
		case 10768:
		case 11024:
		case 11220:
		case 11526:
		case 12207:
		case 16631:
		case 19164:
		case 22099:
			return true;
		}
		std::string slug = handle;
		slug.push_back(' ');
		slug.append(name);
		string::lower(slug);
		if (slug.find("cdn") != std::string::npos
			|| slug.find("colocation") != std::string::npos // AS48950 GLOBAL COLOCATION LIMITED
			// Note: Not "colo" because "Telmex Colombia S.A."
			|| slug.find("cloud") != std::string::npos // AS39845 2 Cloud Ltd.
			|| slug.find("datacenter") != std::string::npos
			|| slug.find("data center") != std::string::npos
			|| slug.find("data-center") != std::string::npos
			|| slug.find("ddos") != std::string::npos
			|| slug.find("dedi") != std::string::npos // AS35913 DediPath, AS42831 UK Dedicated Servers Limited
			|| (slug.find("host") != std::string::npos // AS45382 EHOSTICT, AS51430 AltusHost B.V., AS55720 Gigabit Hosting Sdn Bhd, AS61493 InterBS S.R.L. (BAEHOST), AS64200 VIVID-HOSTING LLC, AS136557 Host Universal Pty Ltd, AS200698 Globalhost d.o.o., AS203020 HostRoyale Technologies Pvt Ltd, AS262287 Maxihost LTDA, AS396356 Maxihost LLC
				&& slug.find("afrihost") == std::string::npos
				)
			|| slug.find("layer") != std::string::npos // AS49453, AS57172 Global Layer B.V.
			|| slug.find("scale") != std::string::npos
			|| slug.find("server") != std::string::npos
			|| slug.find("vps") != std::string::npos
			|| slug.find("hetzner") != std::string::npos
			|| slug.find("ovh") != std::string::npos
			|| slug.find("contabo") != std::string::npos
			|| slug.find("digitalocean") != std::string::npos
			|| slug.find("amazon") != std::string::npos
			|| slug.find("google llc") != std::string::npos // added LLC to prevent detection of "Google Fiber Inc"
			|| slug.find("akamai") != std::string::npos
			|| slug.find("microsoft") != std::string::npos
			|| slug.find("alibaba") != std::string::npos
			|| slug.find("fastly") != std::string::npos
			|| slug.find("linode") != std::string::npos
			|| slug.find("aruba") != std::string::npos
			|| slug.find("godaddy") != std::string::npos
			|| slug.find("oracle") != std::string::npos
			)
		{
			return true;
		}
		return false;
	}
}
