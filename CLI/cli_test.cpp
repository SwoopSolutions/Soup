#include "cli.hpp"

#include <x64.hpp>

// crypto
#include <aes.hpp>
#include <SegWitAddress.hpp>
#include <Hotp.hpp>
#include <rsa.hpp>
#include <ecc.hpp>
#include <X509Certificate.hpp>

// data
#include <base32.hpp>
#include <base58.hpp>
#include <base64.hpp>
#include <cat.hpp>
#include <punycode.hpp>
#include <ripemd160.hpp>
#include <sha1.hpp>
#include <sha256.hpp>
#include <sha384.hpp>
#include <sha512.hpp>
#include <unicode.hpp>

#include <json.hpp>
#include <JsonArray.hpp>
#include <JsonBool.hpp>
#include <JsonInt.hpp>
#include <JsonObject.hpp>
#include <JsonString.hpp>
#include <MessageStream.hpp>
#include <Regex.hpp>
#include <xml.hpp>

// hardware
#include <HidReportDescriptor.hpp>
#include <HidScancode.hpp>

// io
#include <BitReader.hpp>
#include <BitWriter.hpp>
#include <StringReader.hpp>
#include <StringWriter.hpp>
#include <MemoryRefReader.hpp>

// lang
#include <MathExpr.hpp>
#include <PhpState.hpp>
#include <wasm.hpp>

// lang.reflection
#include <rflParser.hpp>
#include <rflStruct.hpp>

// ling.chatbot
#include <Chatbot.hpp>

// math
#include <Bigint.hpp>
#include <math.hpp>

// net.email
#include <EmailAddress.hpp>

// net.web
#include <Uri.hpp>

// net
#include <Socket.hpp>

#include <StringMatch.hpp>
#include <format.hpp>

#include <string.hpp>
#include <time.hpp>
#include <version_compare.hpp>

#include <Rgb.hpp>

#include <unit_testing.hpp> // We want to have the final say on what 'assert' means

using namespace soup;
using namespace soup::literals;

static void unit_cpu()
{
	test("x64", []
	{
#define ASSERT_X64_DISASM(asm, ...) { uint8_t code[] = { __VA_ARGS__ }; const uint8_t* cp = code; assert(x64Disasm(cp).toString() == asm); assert(cp == &code[sizeof(code)]); }

		ASSERT_X64_DISASM("mov al, al", 0x88, 0xC0);
		ASSERT_X64_DISASM("mov r8, r8", 0x4D, 0x89, 0xC0);
		ASSERT_X64_DISASM("mov ah, ah", 0x88, 0xE4);
		ASSERT_X64_DISASM("mov ax, ax", 0x66, 0x89, 0xC0);
		ASSERT_X64_DISASM("mov eax, eax", 0x89, 0xC0);
		ASSERT_X64_DISASM("mov rax, rax", 0x48, 0x89, 0xC0);

		ASSERT_X64_DISASM("mov [eax], eax", 0x67, 0x89, 0x00);
		ASSERT_X64_DISASM("mov [eax], rax", 0x67, 0x48, 0x89, 0x00);
		ASSERT_X64_DISASM("mov [rax], eax", 0x89, 0x00);
		ASSERT_X64_DISASM("mov [rax], rax", 0x48, 0x89, 0x00);

		ASSERT_X64_DISASM("mov al, [eax]", 0x67, 0x8A, 0x00);
		ASSERT_X64_DISASM("mov ah, [eax]", 0x67, 0x8A, 0x20);
		ASSERT_X64_DISASM("mov ax, [eax]", 0x67, 0x66, 0x8B, 0x00);
		ASSERT_X64_DISASM("mov eax, [eax]", 0x67, 0x8B, 0x00);
		ASSERT_X64_DISASM("mov eax, [rax]", 0x8B, 0x00);
		ASSERT_X64_DISASM("mov rax, [eax]", 0x67, 0x48, 0x8B, 0x00);
		ASSERT_X64_DISASM("mov rax, [rax]", 0x48, 0x8B, 0x00);

		ASSERT_X64_DISASM("mov [rsp], rax", 0x48, 0x89, 0x04, 0x24);
		ASSERT_X64_DISASM("mov [rsp+0x69], rbx", 0x48, 0x89, 0x5C, 0x24, 0x69);
		ASSERT_X64_DISASM("mov [rax+0x69], rbx", 0x48, 0x89, 0x58, 0x69);

		ASSERT_X64_DISASM("mov al, 69", 0xB0, 0x45);
		ASSERT_X64_DISASM("mov ah, 69", 0xB4, 0x45);
		ASSERT_X64_DISASM("mov ax, 69", 0x66, 0xB8, 0x45, 0x00);
		ASSERT_X64_DISASM("mov eax, 69", 0xB8, 0x45, 0x00, 0x00, 0x00);
		ASSERT_X64_DISASM("mov rax, 69", 0x48, 0xC7, 0xC0, 0x45, 0x00, 0x00, 0x00);
		ASSERT_X64_DISASM("mov rax, 1384798605843239735", 0x48, 0xB8, 0x37, 0x13, 0xBE, 0xBA, 0xFE, 0xCA, 0x37, 0x13);

		ASSERT_X64_DISASM("mov rbp, rbp", 0x48, 0x89, 0xED);
		ASSERT_X64_DISASM("mov byte ptr [rip+0x69], 0", 0xC6, 0x05, 0x69, 0x00, 0x00, 0x00, 0x00);
		ASSERT_X64_DISASM("mov byte ptr [rbp+0x69], 0", 0xC6, 0x45, 0x69, 0x00);

		ASSERT_X64_DISASM("mov 0, 69", 0xC7, 0x04, 0x25, 0x00, 0x00, 0x00, 0x00, 0x45, 0x00, 0x00, 0x00);

		ASSERT_X64_DISASM("ret", 0xC3);

		ASSERT_X64_DISASM("push ax", 0x66, 0x50);
		ASSERT_X64_DISASM("push rax", 0x50);
		ASSERT_X64_DISASM("push rdi", 0x57);
		ASSERT_X64_DISASM("push r15", 0x41, 0x57);

		ASSERT_X64_DISASM("push word ptr [rax]", 0x66, 0xFF, 0x30);
		ASSERT_X64_DISASM("push word ptr [r15]", 0x66, 0x41, 0xFF, 0x37);
		ASSERT_X64_DISASM("push qword ptr [rax]", 0xFF, 0x30);

		ASSERT_X64_DISASM("sub rsp, 32", 0x48, 0x83, 0xEC, 0x20);

		ASSERT_X64_DISASM("cmp al, al", 0x38, 0xc0);
		ASSERT_X64_DISASM("cmp rax, rax", 0x48, 0x39, 0xc0);
		ASSERT_X64_DISASM("cmp al, 0", 0x3c, 0x00);
		ASSERT_X64_DISASM("cmp rax, 0", 0x48, 0x83, 0xf8, 0x00);
		ASSERT_X64_DISASM("cmp rax, 1762867049", 0x48, 0x3d, 0x69, 0x37, 0x13, 0x69);
		ASSERT_X64_DISASM("cmp byte ptr [rax+0x69], 0", 0x80, 0x78, 0x69, 0x00);
		ASSERT_X64_DISASM("cmp byte ptr [rax+0x69133769], 0", 0x80, 0xb8, 0x69, 0x37, 0x13, 0x69, 0x00);
		ASSERT_X64_DISASM("cmp byte ptr [rip+0x69], 0", 0x80, 0x3d, 0x69, 0x00, 0x00, 0x00, 0x00);

		ASSERT_X64_DISASM("and qword ptr [rbx], 0", 0x48, 0x83, 0x23, 0x00);

		ASSERT_X64_DISASM("lea rax, [rcx+rdx]", 0x48, 0x8D, 0x04, 0x11);

#define ASSERT_X64_ASM(asm, ...) { uint8_t code[] = { __VA_ARGS__ }; x64Instruction ins; ins.fromString(asm); assert(memcmp(ins.toBytecode().c_str(), code, sizeof(code)) == 0); }

		ASSERT_X64_ASM("mov rax, rax", 0x48, 0x89, 0xC0); // The 0x8B opcode would also be valid for two direct-access registers.
	});
}

static void unit_crypto()
{
	unit("aes")
	{
		test("128-bit", []
		{
			std::string data = "The quick brown fox jumps over the lazy dog.";
			const char key[] = "Super Secret Key";
			aes::ecbEncrypt(reinterpret_cast<uint8_t*>(data.data()), data.size(), reinterpret_cast<const uint8_t*>(key), 16);
			assert(base64::encode(data) == "HodBOXWJGvXUR96GBIBiPX5VqQ8FMB+xfullVkHYvRloZSBsYXp5IGRvZy4=");
			aes::ecbDecrypt(reinterpret_cast<uint8_t*>(data.data()), data.size(), reinterpret_cast<const uint8_t*>(key), 16);
			assert(data == "The quick brown fox jumps over the lazy dog.");
		});
		test("192-bit", []
		{
			std::string data = "The quick brown fox jumps over the lazy dog.";
			const char key[] = "Super Secret 192-Bit Key";
			aes::ecbEncrypt(reinterpret_cast<uint8_t*>(data.data()), data.size(), reinterpret_cast<const uint8_t*>(key), 24);
			assert(base64::encode(data) == "4tNkWHmb6IL5+ipVNcsVPxWpI6cbOIwA8eqgM8NtsCFoZSBsYXp5IGRvZy4=");
			aes::ecbDecrypt(reinterpret_cast<uint8_t*>(data.data()), data.size(), reinterpret_cast<const uint8_t*>(key), 24);
			assert(data == "The quick brown fox jumps over the lazy dog.");
		});
		test("256-bit", []
		{
			std::string data = "The quick brown fox jumps over the lazy dog.";
			const char key[] = "My Super Secret Key For 256-Bit";
			aes::ecbEncrypt(reinterpret_cast<uint8_t*>(data.data()), data.size(), reinterpret_cast<const uint8_t*>(key), 32);
			assert(base64::encode(data) == "aVvJVxH5t2p5eofHD627Hj5si+ERM0TAH3pJKnAzj6hoZSBsYXp5IGRvZy4=");
			aes::ecbDecrypt(reinterpret_cast<uint8_t*>(data.data()), data.size(), reinterpret_cast<const uint8_t*>(key), 32);
			assert(data == "The quick brown fox jumps over the lazy dog.");
		});
	}

	test("SegWitAddress", []
	{
		SegWitAddress addr;
		addr.compressed_pub_key = Bigint::fromStringHex("0279be667ef9dcbbac55a06295ce870b07029bfcdb2dce28d959f2815b16f81798", 66).toBinary();
		assert(addr.encode() == "bc1qw508d6qejxtdg4y5r3zarvary0c5xw7kv8f3t4");
	});

	test("hotp", []
	{
		Hotp gen("12345678901234567890");
		assert(gen.getValue(0) == 755224);
		assert(gen.getValue(1) == 287082);
		assert(gen.getValue(2) == 359152);
		assert(gen.getValue(3) == 969429);
		assert(gen.getValue(4) == 338314);
		assert(gen.getValue(5) == 254676);
		assert(gen.getValue(6) == 287922);
		assert(gen.getValue(7) == 162583);
		assert(gen.getValue(8) == 399871);
		assert(gen.getValue(9) == 520489);
	});

	test("rsa", []
	{
		RsaKeypair kp(
			"96529209707922958264660626622151327182265565708623147261613126577409795199887"_b,
			"87505296413890087200392682183900465764322220376584167643884573751015402662091"_b
		);
		auto enc = "3939991117139809241563517827579718715756222298160587806559781632547966505691296013680068230342942841852094486596819343548681582442588753971618922157744527"_b;
		assert(kp.getPrivate().encryptUnpadded("Soup") == enc);
		assert(kp.getPublic().decryptUnpadded(enc) == "Soup");
	});

	unit("ecc")
	{
		test("secp256k1", []
		{
			auto curve = EccCurve::secp256k1();
#if false
			// https://crypto.stackexchange.com/a/74491
			auto p = curve.derivePublic("0xEBB2C082FD7727890A28AC82F6BDF97BAD8DE9F5D7C9028692DE1A255CAD3E0F"_b);
			assert(p.x == "0x779DD197A5DF977ED2CF6CB31D82D43328B790DC6B3B7D4437A427BD5847DFCD"_b);
			assert(p.y == "0xE94B724A555B6D017BB7607C3E3281DAF5B1699D6EF4124975C9237B917D426F"_b);
#else
			auto p = curve.add(curve.G, curve.G);
			assert(p.x == "0xC6047F9441ED7D6D3045406E95C07CD85C778E4B8CEF3CA7ABAC09B95C709EE5"_b);
			assert(p.y == "0x1AE168FEA63DC339A3C58419466CEAEEF7F632653266D0E1236431A950CFE52A"_b);
#endif
		});
		test("secp256r1", []
		{
			auto curve = EccCurve::secp256r1();
			// https://asecuritysite.com/ecc/p256p
			auto p = curve.add(curve.G, curve.G);
			assert(p.x == "0x7CF27B188D034F7E8A52380304B51AC3C08969E277F21B35A60B48FC47669978"_b);
			assert(p.y == "0x7775510DB8ED040293D9AC69F7430DBBA7DADE63CE982299E04B79D227873D1"_b);
		});
		test("secp384r1", []
		{
			auto curve = EccCurve::secp384r1();
#if false
			// http://cryptomanager.com/tv.html
			auto p = curve.derivePublic("0x911540762B807060EBB1071D8B76F9C6B0C8570B2D56204B7D62448443171798EDF712E7CF55895D675FFE7B5CF35750"_b);
			assert(p.x == "0xB7828FF3F814932B531D3CD58947A77655CA12EE533333EE12E921C39114B752BEFDB3E45C05D6C1F8222C5C6B234E8D"_b);
			assert(p.y == "0x1F4B1BBA3434C6BAA34250744B4E109E09A55D5F3075BEC33256C94A468792C2B5650D24F85482C988B7328E825F488D"_b);
#else
			auto p = curve.add(curve.G, curve.G);
			assert(p.x == "0x8D999057BA3D2D969260045C55B97F089025959A6F434D651D207D19FB96E9E4FE0E86EBE0E64F85B96A9C75295DF61"_b);
			assert(p.y == "0x8E80F1FA5B1B3CEDB7BFE8DFFD6DBA74B275D875BC6CC43E904E505F256AB4255FFD43E94D39E22D61501E700A940E80"_b);
#endif
		});

		test("point compression on secp256r1", []
		{
			auto curve = EccCurve::secp256r1();
			// Even Y
			auto p = curve.decodePoint("0x024335E9B1328C5B5F3C7086EF78DE08519563703A572C8EF44974D11D793542D3"_b.toBinary());
			assert(p.x == "0x4335E9B1328C5B5F3C7086EF78DE08519563703A572C8EF44974D11D793542D3"_b);
			assert(p.y == "0x31C89D7DE56498EA51609AB1ECB94FCCDF2DCF0ECA08E19DFDC3A2EF07A8D734"_b);
			// Odd Y, https://github.com/w3c-ccg/did-method-key/issues/32#issuecomment-829586575
			p = curve.decodePoint("0x03413029CB9A5A4A0B087A9B8A060116D0D32BB22D14AEBF7778215744811BB6CE"_b.toBinary());
			assert(p.x == "0x413029CB9A5A4A0B087A9B8A060116D0D32BB22D14AEBF7778215744811BB6CE"_b);
			assert(p.y == "0x40780D7BB9E2E068879F443E05B21B8FC0B62C9C811008064D988856077E35E7"_b);
		});
	}

	test("X509Certificate::matchDomain", []
	{
		assert(X509Certificate::matchDomain("deez.nuts", "deez.nuts") == true);
		assert(X509Certificate::matchDomain("www.deez.nuts", "*.deez.nuts") == true);
		assert(X509Certificate::matchDomain("what.are.deez.nuts", "*.deez.nuts") == false);
	});
}

static void unit_data()
{
	unit("base32")
	{
		test("encode", []
		{
			assert(base32::encode("a") == "ME======");
			assert(base32::encode("aa") == "MFQQ====");
			assert(base32::encode("aaa") == "MFQWC===");
			assert(base32::encode("aaaa") == "MFQWCYI=");
			assert(base32::encode("aaaaa") == "MFQWCYLB");
			assert(base32::encode("aaaaaa") == "MFQWCYLBME======");
			assert(base32::encode("0.1.2.3.4.5.6.7.8.9.10.11.12.13.14.15.16.17.18.19.20.21.22.23.24.25.26.27.28.29.30.31.32.33.34.35.36.37.38.39.40.41.42.43.44.45.46.47.48.49.50.51.52.53.54.55.56.57.58.59.60.61.62.63.64.65.66.67.68.69.70.71.72.73.74.75.76.77.78.79.80.81.82.83.84.85.86.87.88.89.90.91.92.93.94.95.96.97.98.99.100.101.102.103.104.105.106.107.108.109.110.111.112.113.114.115.116.117.118.119.120.121.122.123.124.125.126.127.128.129.130.131.132.133.134.135.136.137.138.139.140.141.142.143.144.145.146.147.148.149.150.151.152.153.154.155.156.157.158.159.160.161.162.163.164.165.166.167.168.169.170.171.172.173.174.175.176.177.178.179.180.181.182.183.184.185.186.187.188.189.190.191.192.193.194.195.196.197.198.199.200.201.202.203.204.205.206.207.208.209.210.211.212.213.214.215.216.217.218.219.220.221.222.223.224.225.226.227.228.229.230.231.232.233.234.235.236.237.238.239.240.241.242.243.244.245.246.247.248.249.250.251.252.253.254.255") == "GAXDCLRSFYZS4NBOGUXDMLRXFY4C4OJOGEYC4MJRFYYTELRRGMXDCNBOGE2S4MJWFYYTOLRRHAXDCOJOGIYC4MRRFYZDELRSGMXDENBOGI2S4MRWFYZDOLRSHAXDEOJOGMYC4MZRFYZTELRTGMXDGNBOGM2S4MZWFYZTOLRTHAXDGOJOGQYC4NBRFY2DELRUGMXDINBOGQ2S4NBWFY2DOLRUHAXDIOJOGUYC4NJRFY2TELRVGMXDKNBOGU2S4NJWFY2TOLRVHAXDKOJOGYYC4NRRFY3DELRWGMXDMNBOGY2S4NRWFY3DOLRWHAXDMOJOG4YC4NZRFY3TELRXGMXDONBOG42S4NZWFY3TOLRXHAXDOOJOHAYC4OBRFY4DELRYGMXDQNBOHA2S4OBWFY4DOLRYHAXDQOJOHEYC4OJRFY4TELRZGMXDSNBOHE2S4OJWFY4TOLRZHAXDSOJOGEYDALRRGAYS4MJQGIXDCMBTFYYTANBOGEYDKLRRGA3C4MJQG4XDCMBYFYYTAOJOGEYTALRRGEYS4MJRGIXDCMJTFYYTCNBOGEYTKLRRGE3C4MJRG4XDCMJYFYYTCOJOGEZDALRRGIYS4MJSGIXDCMRTFYYTENBOGEZDKLRRGI3C4MJSG4XDCMRYFYYTEOJOGEZTALRRGMYS4MJTGIXDCMZTFYYTGNBOGEZTKLRRGM3C4MJTG4XDCMZYFYYTGOJOGE2DALRRGQYS4MJUGIXDCNBTFYYTINBOGE2DKLRRGQ3C4MJUG4XDCNBYFYYTIOJOGE2TALRRGUYS4MJVGIXDCNJTFYYTKNBOGE2TKLRRGU3C4MJVG4XDCNJYFYYTKOJOGE3DALRRGYYS4MJWGIXDCNRTFYYTMNBOGE3DKLRRGY3C4MJWG4XDCNRYFYYTMOJOGE3TALRRG4YS4MJXGIXDCNZTFYYTONBOGE3TKLRRG43C4MJXG4XDCNZYFYYTOOJOGE4DALRRHAYS4MJYGIXDCOBTFYYTQNBOGE4DKLRRHA3C4MJYG4XDCOBYFYYTQOJOGE4TALRRHEYS4MJZGIXDCOJTFYYTSNBOGE4TKLRRHE3C4MJZG4XDCOJYFYYTSOJOGIYDALRSGAYS4MRQGIXDEMBTFYZDANBOGIYDKLRSGA3C4MRQG4XDEMBYFYZDAOJOGIYTALRSGEYS4MRRGIXDEMJTFYZDCNBOGIYTKLRSGE3C4MRRG4XDEMJYFYZDCOJOGIZDALRSGIYS4MRSGIXDEMRTFYZDENBOGIZDKLRSGI3C4MRSG4XDEMRYFYZDEOJOGIZTALRSGMYS4MRTGIXDEMZTFYZDGNBOGIZTKLRSGM3C4MRTG4XDEMZYFYZDGOJOGI2DALRSGQYS4MRUGIXDENBTFYZDINBOGI2DKLRSGQ3C4MRUG4XDENBYFYZDIOJOGI2TALRSGUYS4MRVGIXDENJTFYZDKNBOGI2TK===");
		});
		test("decode", []
		{
			assert(base32::decode("ME======") == "a");
			assert(base32::decode("MFQQ====") == "aa");
			assert(base32::decode("MFQWC===") == "aaa");
			assert(base32::decode("MFQWCYI=") == "aaaa");
			assert(base32::decode("MFQWCYLB") == "aaaaa");
			assert(base32::decode("MFQWCYLBME======") == "aaaaaa");
			assert(base32::decode("GAXDCLRSFYZS4NBOGUXDMLRXFY4C4OJOGEYC4MJRFYYTELRRGMXDCNBOGE2S4MJWFYYTOLRRHAXDCOJOGIYC4MRRFYZDELRSGMXDENBOGI2S4MRWFYZDOLRSHAXDEOJOGMYC4MZRFYZTELRTGMXDGNBOGM2S4MZWFYZTOLRTHAXDGOJOGQYC4NBRFY2DELRUGMXDINBOGQ2S4NBWFY2DOLRUHAXDIOJOGUYC4NJRFY2TELRVGMXDKNBOGU2S4NJWFY2TOLRVHAXDKOJOGYYC4NRRFY3DELRWGMXDMNBOGY2S4NRWFY3DOLRWHAXDMOJOG4YC4NZRFY3TELRXGMXDONBOG42S4NZWFY3TOLRXHAXDOOJOHAYC4OBRFY4DELRYGMXDQNBOHA2S4OBWFY4DOLRYHAXDQOJOHEYC4OJRFY4TELRZGMXDSNBOHE2S4OJWFY4TOLRZHAXDSOJOGEYDALRRGAYS4MJQGIXDCMBTFYYTANBOGEYDKLRRGA3C4MJQG4XDCMBYFYYTAOJOGEYTALRRGEYS4MJRGIXDCMJTFYYTCNBOGEYTKLRRGE3C4MJRG4XDCMJYFYYTCOJOGEZDALRRGIYS4MJSGIXDCMRTFYYTENBOGEZDKLRRGI3C4MJSG4XDCMRYFYYTEOJOGEZTALRRGMYS4MJTGIXDCMZTFYYTGNBOGEZTKLRRGM3C4MJTG4XDCMZYFYYTGOJOGE2DALRRGQYS4MJUGIXDCNBTFYYTINBOGE2DKLRRGQ3C4MJUG4XDCNBYFYYTIOJOGE2TALRRGUYS4MJVGIXDCNJTFYYTKNBOGE2TKLRRGU3C4MJVG4XDCNJYFYYTKOJOGE3DALRRGYYS4MJWGIXDCNRTFYYTMNBOGE3DKLRRGY3C4MJWG4XDCNRYFYYTMOJOGE3TALRRG4YS4MJXGIXDCNZTFYYTONBOGE3TKLRRG43C4MJXG4XDCNZYFYYTOOJOGE4DALRRHAYS4MJYGIXDCOBTFYYTQNBOGE4DKLRRHA3C4MJYG4XDCOBYFYYTQOJOGE4TALRRHEYS4MJZGIXDCOJTFYYTSNBOGE4TKLRRHE3C4MJZG4XDCOJYFYYTSOJOGIYDALRSGAYS4MRQGIXDEMBTFYZDANBOGIYDKLRSGA3C4MRQG4XDEMBYFYZDAOJOGIYTALRSGEYS4MRRGIXDEMJTFYZDCNBOGIYTKLRSGE3C4MRRG4XDEMJYFYZDCOJOGIZDALRSGIYS4MRSGIXDEMRTFYZDENBOGIZDKLRSGI3C4MRSG4XDEMRYFYZDEOJOGIZTALRSGMYS4MRTGIXDEMZTFYZDGNBOGIZTKLRSGM3C4MRTG4XDEMZYFYZDGOJOGI2DALRSGQYS4MRUGIXDENBTFYZDINBOGI2DKLRSGQ3C4MRUG4XDENBYFYZDIOJOGI2TALRSGUYS4MRVGIXDENJTFYZDKNBOGI2TK===") == "0.1.2.3.4.5.6.7.8.9.10.11.12.13.14.15.16.17.18.19.20.21.22.23.24.25.26.27.28.29.30.31.32.33.34.35.36.37.38.39.40.41.42.43.44.45.46.47.48.49.50.51.52.53.54.55.56.57.58.59.60.61.62.63.64.65.66.67.68.69.70.71.72.73.74.75.76.77.78.79.80.81.82.83.84.85.86.87.88.89.90.91.92.93.94.95.96.97.98.99.100.101.102.103.104.105.106.107.108.109.110.111.112.113.114.115.116.117.118.119.120.121.122.123.124.125.126.127.128.129.130.131.132.133.134.135.136.137.138.139.140.141.142.143.144.145.146.147.148.149.150.151.152.153.154.155.156.157.158.159.160.161.162.163.164.165.166.167.168.169.170.171.172.173.174.175.176.177.178.179.180.181.182.183.184.185.186.187.188.189.190.191.192.193.194.195.196.197.198.199.200.201.202.203.204.205.206.207.208.209.210.211.212.213.214.215.216.217.218.219.220.221.222.223.224.225.226.227.228.229.230.231.232.233.234.235.236.237.238.239.240.241.242.243.244.245.246.247.248.249.250.251.252.253.254.255");
		});
	}

	unit("base58")
	{
		test("decode", []
		{
			assert(string::bin2hex(base58::decode("5HueCGU8rMjxEXxiPuD5BDku4MkFqeZyd4dZ1jvhTVqvbTLvyTJ")) == "800C28FCA386C7A227600B2FE50B7CAE11EC86D3BF1FBE471BE89827E19D72AA1D507A5B8D");
		});
	}
	
	unit("base64")
	{
		test("encode", []
		{
			assert(base64::encode("a") == "YQ==");
			assert(base64::encode("Hello") == "SGVsbG8=");
			assert(base64::encode("😀") == "8J+YgA==");
		});
		test("urlEncode", []
		{
			assert(base64::urlEncode("Hello", true) == "SGVsbG8=");
			assert(base64::urlEncode("😀", true) == "8J-YgA==");
		});
		test("decode", []
		{
			assert(base64::decode("YQ==") == "a");
			assert(base64::decode("SGVsbG8=") == "Hello");
			assert(base64::decode("8J+YgA==") == "😀");
		});
		test("urlDecode", []
		{
			assert(base64::urlDecode("SGVsbG8=") == "Hello");
			assert(base64::urlDecode("8J-YgA==") == "😀");
		});
	}

	test("unicode", []
	{
		auto utf32 = unicode::utf8_to_utf32("\xF0\x41");
		assert(utf32.size() == 2);
		assert(utf32.at(0) == unicode::REPLACEMENT_CHAR);
		assert(utf32.at(1) == 0x41);
	});

	test("punycode", []
	{
		auto test_punycode_pair = [](const std::string& encoded, const std::u32string& decoded)
		{
			assert(punycode::encode(decoded) == encoded);
			assert(punycode::decode(encoded) == decoded);
		};
		test_punycode_pair("4ca", U"ä");
		test_punycode_pair("4ca0b", U"äö");
		test_punycode_pair("4ca0bs", U"äöü");
		test_punycode_pair("bcher-kva", U"bücher");
		test_punycode_pair("bcher-ova", U"bcherü");
		test_punycode_pair("bcher-kvaf", U"ýbücher");
		test_punycode_pair("ihqwcrb4cv8a8dqg056pqjye", U"他们为什么不说中文");
	});

	test("cat", []
	{
		StringReader sr("Hello: World");
		auto tree = cat::parse(sr);
		assert(tree);
		assert(tree->children.size() == 1);
		assert(tree->children.at(0)->name == "Hello");
		assert(tree->children.at(0)->value == "World");

		sr = (
			"Hello: World\n"
			"Name-only List\n"
			"\tName-only value\n"
			"List: With Value\n"
			"\tChild: With Value\n"
			"\tNesting\n"
			"\t\tNesting\n"
			"Final\n"
			"\n"
		);
		tree = cat::parse(sr);
		assert(tree);
		assert(tree->children.size() == 4);
		assert(tree->children.at(0)->name == "Hello");
		assert(tree->children.at(0)->value == "World");
		assert(tree->children.at(1)->name == "Name-only List");
		assert(tree->children.at(1)->value.empty());
		assert(tree->children.at(2)->name == "List");
		assert(tree->children.at(2)->value == "With Value");
		assert(tree->children.at(3)->name == "Final");
		assert(tree->children.at(3)->value.empty());

		sr = ("Colon (\\:): Valid");
		tree = cat::parse(sr);
		assert(tree);
		assert(tree->children.size() == 1);
		assert(tree->children.at(0)->name == "Colon (:)");
		assert(tree->children.at(0)->value == "Valid");

		sr = (
			"List: With Value\n"
			"  Child: With Value\n"
			"  Nesting\n"
			"    Nesting\n"
		);
		tree = cat::parse(sr);
		assert(tree);
		assert(tree->children.size() == 1);
		assert(tree->children.at(0)->name == "List");
		assert(tree->children.at(0)->value == "With Value");
		assert(tree->children.at(0)->children.size() == 2);
		assert(tree->children.at(0)->children.at(0)->name == "Child");
		assert(tree->children.at(0)->children.at(0)->value == "With Value");
		assert(tree->children.at(0)->children.at(1)->name == "Nesting");
		assert(tree->children.at(0)->children.at(1)->value.empty());

		sr = " ";
		tree = cat::parse(sr);
		assert(tree);
		assert(tree->children.size() == 0);

		sr = (
			"Some: Value\n"
			" "
		);
		tree = cat::parse(sr);
		assert(tree);
		assert(tree->children.size() == 1);
		assert(tree->children.at(0)->name == "Some");
		assert(tree->children.at(0)->value == "Value");
		assert(tree->children.at(0)->children.empty());

		sr = (
			"Some: Value\n"
			"Empty:\n" // Notice the lack of space after colon. Normally, ": " is the key-value separator, but in the "empty value" case, the space is optional.
		);
		tree = cat::parse(sr);
		assert(tree);
		assert(tree->children.size() == 2);
		assert(tree->children.at(0)->name == "Some");
		assert(tree->children.at(0)->value == "Value");
		assert(tree->children.at(1)->name == "Empty");
		assert(tree->children.at(1)->value == "");

		sr = (
			":\n"
			"\tNested"
		);
		tree = cat::parse(sr);
		assert(tree);
		assert(tree->children.size() == 1);
		assert(tree->children.at(0)->name == "");
		assert(tree->children.at(0)->children.size() == 1);
		assert(tree->children.at(0)->children.at(0)->name == "Nested");

		// An issue that might occur is that an editor automatically indents a line that is otherwise empty. CaT should process this just like an empty line: ignore it.
		sr = (
			"List:\n"
			"\tItem 1\n"
			"\t\n"
			"\tItem 2\n"
		);
		tree = cat::parse(sr);
		assert(tree);
		assert(tree->children.size() == 1);
		assert(tree->children.at(0)->name == "List");
		assert(tree->children.at(0)->children.size() == 2);

		// Optional feature: Value escaping.
		sr = R"(Text: "Look at this backslash: \\\r\nLook at this quote: \"")";
		tree = cat::parse(sr);
		assert(tree);
		assert(tree->children.size() == 1);
		assert(tree->children.at(0)->name == "Text");
		assert(tree->children.at(0)->value == "Look at this backslash: \\\r\nLook at this quote: \"");
	});

	test("ripemd160", []
	{
		assert(string::bin2hexLower(soup::ripemd160("The quick brown fox jumps over the lazy dog")) == "37f332f68db77bd9d7edd4969571ad671cf9dd3b");
		assert(string::bin2hexLower(soup::ripemd160("The quick brown fox jumps over the lazy cog")) == "132072df690933835eb8b6ad0b77e7b6f14acad7");
		assert(string::bin2hexLower(soup::ripemd160("")) == "9c1185a5c5e9fc54612808977ee8f548b2258d31");
	});

	test("sha1", []
	{
		assert(sha1::hash("") == std::string("\xda\x39\xa3\xee\x5e\x6b\x4b\x0d\x32\x55\xbf\xef\x95\x60\x18\x90\xaf\xd8\x07\x09", 20));
		assert(sha1::hash("abc") == std::string("\xa9\x99\x3e\x36\x47\x06\x81\x6a\xba\x3e\x25\x71\x78\x50\xc2\x6c\x9c\xd0\xd8\x9d", 20));
		assert(sha1::hash("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa") == std::string("\x8c\xd9\x6a\xf2\x17\xb5\x19\x86\x55\xe7\x37\x80\xf3\x5d\x52\x2e\xba\x76\x22\x44", 20));
	});

	test("sha256", []
	{
		assert(sha256::hash("") == std::string("\xe3\xb0\xc4\x42\x98\xfc\x1c\x14\x9a\xfb\xf4\xc8\x99\x6f\xb9\x24\x27\xae\x41\xe4\x64\x9b\x93\x4c\xa4\x95\x99\x1b\x78\x52\xb8\x55", 32));
		assert(sha256::hash("abc") == std::string("\xba\x78\x16\xbf\x8f\x01\xcf\xea\x41\x41\x40\xde\x5d\xae\x22\x23\xb0\x03\x61\xa3\x96\x17\x7a\x9c\xb4\x10\xff\x61\xf2\x00\x15\xad", 32));
		assert(sha256::hash("aaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaaa") == std::string("\x13\x28\x44\x68\x1d\x57\xd5\xae\x76\xdf\xd3\x5a\x07\xe4\xfc\x10\xfd\xc0\xce\x9f\x08\x82\xa2\xb0\xc9\xdb\x0e\x84\x11\x6d\x5b\xad", 32));
	});

	test("sha256 hmac", []
	{
		assert(sha256::hmac("Hi There", "\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b") == std::string("\xb0\x34\x4c\x61\xd8\xdb\x38\x53\x5c\xa8\xaf\xce\xaf\x0b\xf1\x2b\x88\x1d\xc2\x00\xc9\x83\x3d\xa7\x26\xe9\x37\x6c\x2e\x32\xcf\xf7", 32));

		sha256::HmacState st("\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b\x0b");
		st.append("Hi There", 8);
		st.finalise();
		assert(st.getDigest() == std::string("\xb0\x34\x4c\x61\xd8\xdb\x38\x53\x5c\xa8\xaf\xce\xaf\x0b\xf1\x2b\x88\x1d\xc2\x00\xc9\x83\x3d\xa7\x26\xe9\x37\x6c\x2e\x32\xcf\xf7", 32));
	});

	test("sha256 tls prf", []
	{
		assert(sha256::tls_prf("test label", 100,
			"\x9b\xbe\x43\x6b\xa9\x40\xf0\x17\xb1\x76\x52\x84\x9a\x71\xdb\x35",
			"\xa0\xba\x9f\x93\x6c\xda\x31\x18\x27\xa6\xf7\x96\xff\xd5\x19\x8c")
			== std::string("\xe3\xf2\x29\xba\x72\x7b\xe1\x7b\x8d\x12\x26\x20\x55\x7c\xd4\x53\xc2\xaa\xb2\x1d\x07\xc3\xd4\x95\x32\x9b\x52\xd4\xe6\x1e\xdb\x5a\x6b\x30\x17\x91\xe9\x0d\x35\xc9\xc9\xa4\x6b\x4e\x14\xba\xf9\xaf\x0f\xa0\x22\xf7\x07\x7d\xef\x17\xab\xfd\x37\x97\xc0\x56\x4b\xab\x4f\xbc\x91\x66\x6e\x9d\xef\x9b\x97\xfc\xe3\x4f\x79\x67\x89\xba\xa4\x80\x82\xd1\x22\xee\x42\xc5\xa7\x2e\x5a\x51\x10\xff\xf7\x01\x87\x34\x7b\x66", 100)
		);
	});

	test("sha384 tls prf", []
	{
		assert(sha384::tls_prf("test label", 148,
			"\xb8\x0b\x73\x3d\x6c\xee\xfc\xdc\x71\x56\x6e\xa4\x8e\x55\x67\xdf",
			"\xcd\x66\x5c\xf6\xa8\x44\x7d\xd6\xff\x8b\x27\x55\x5e\xdb\x74\x65")
			== std::string("\x7b\x0c\x18\xe9\xce\xd4\x10\xed\x18\x04\xf2\xcf\xa3\x4a\x33\x6a\x1c\x14\xdf\xfb\x49\x00\xbb\x5f\xd7\x94\x21\x07\xe8\x1c\x83\xcd\xe9\xca\x0f\xaa\x60\xbe\x9f\xe3\x4f\x82\xb1\x23\x3c\x91\x46\xa0\xe5\x34\xcb\x40\x0f\xed\x27\x00\x88\x4f\x9d\xc2\x36\xf8\x0e\xdd\x8b\xfa\x96\x11\x44\xc9\xe8\xd7\x92\xec\xa7\x22\xa7\xb3\x2f\xc3\xd4\x16\xd4\x73\xeb\xc2\xc5\xfd\x4a\xbf\xda\xd0\x5d\x91\x84\x25\x9b\x5b\xf8\xcd\x4d\x90\xfa\x0d\x31\xe2\xde\xc4\x79\xe4\xf1\xa2\x60\x66\xf2\xee\xa9\xa6\x92\x36\xa3\xe5\x26\x55\xc9\xe9\xae\xe6\x91\xc8\xf3\xa2\x68\x54\x30\x8d\x5e\xaa\x3b\xe8\x5e\x09\x90\x70\x3d\x73\xe5\x6f", 148));
	});

	test("sha512 & sha384", []
	{
		assert(string::bin2hex(sha512::hash("Deez")) == "9CACE758CF2AF8556F3DCFC60B1D54017D3FE9A095F78060A1DA6011662C3C652BD9914E5AB1C3A17A8619D549A99F56A251FDCC24DCC3EDB1E9CE99D3E0E623");
		assert(string::bin2hex(sha512::hash(std::string(1000, 'a'))) == "67BA5535A46E3F86DBFBED8CBBAF0125C76ED549FF8B0B9E03E0C88CF90FA634FA7B12B47D77B694DE488ACE8D9A65967DC96DF599727D3292A8D9D447709C97");

		assert(string::bin2hex(sha384::hash("Deez")) == "7844EE01401FC28076539A250ED344F945AE1E4253F12DA35D22C2068C06891DEA9C95E56099ED075EF8F2F56816B52B");
		assert(string::bin2hex(sha384::hash(std::string(1000, 'a'))) == "F54480689C6B0B11D0303285D9A81B21A93BCA6BA5A1B4472765DCA4DA45EE328082D469C650CD3B61B16D3266AB8CED");
	});

	test("json", []
	{
		// Basic test
		{
			auto tree = json::decode(R"({
"firstName": "John",
"lastName": "Smith",
"isAlive": true,
"age": 27,
"address": {
"streetAddress": "21 2nd Street",
"city": "New York",
"state": "NY",
"postalCode": "10021-3100"
},
"phoneNumbers": [
{
	"type": "home",
	"number": "212 555-1234",
	"emptyobject": {}
},
{
	"type": "office",
	"number": "646 555-4567"
}
],
"children": [],
"spouse": null
}
)");
			assert(tree);
			JsonObject& obj = tree->asObj();
			assert(obj.at("firstName").asStr() == "John");
			assert(obj.at("lastName").asStr() == "Smith");
			assert(obj.at("isAlive").asBool() == true);
			assert(obj.at("age").asInt() == 27);
			assert(obj.at("phoneNumbers").asArr().at(0).asObj().at("type").asStr() == "home");
			assert(obj.at("spouse").isNull());
		}

		// Comments
		{
			auto tree = json::decode(R"(// This is a comment

/* This is a comment
spanning over multiple lines */

{
"body":"Hello, world!"
}
			)");
			assert(tree);
			JsonObject& obj = tree->asObj();
			assert(obj.at("body").asStr() == "Hello, world!");
		}
		{
			auto tree = json::decode(R"(
{
// This is the body
"body":"Hello, world!"
}
			)");
			assert(tree);
			JsonObject& obj = tree->asObj();
			assert(obj.at("body").asStr() == "Hello, world!");
		}

		// Scientific notation
		{
			auto tree = json::decode(R"({"perc":1.23e-1,"score":1e+1})");
			assert(tree);
			assert(tree->asObj().contains("perc"));
			assert(tree->asObj().at("perc").isFloat());
			assert(tree->asObj().at("perc").asFloat().value == 0.123);
			assert(tree->asObj().at("score").asFloat().value == 10.0);
		}

		// Lefthand space on key-value separator (this is valid JSON as per the specs)
		{
			auto tree = json::decode(R"({"hello" : "world"})");
			assert(tree);
			assert(tree->asObj().contains("hello"));
			assert(tree->asObj().at("hello").asStr() == "world");
		}
	});

	test("xml", []
	{
		UniquePtr<XmlTag> tag;

		tag = xml::parseAndDiscardMetadata("<html></html>"); assert(tag->encode() == "<html></html>");
		tag = xml::parseAndDiscardMetadata("<html>Hello</html>"); assert(tag->encode() == "<html>Hello</html>");
		tag = xml::parseAndDiscardMetadata("<html>Hello"); assert(tag->encode() == "<html>Hello</html>");
		tag = xml::parseAndDiscardMetadata("<html>"); assert(tag->encode() == "<html></html>");
		tag = xml::parseAndDiscardMetadata("<html><body>Hello</body></html>"); assert(tag->encode() == "<html><body>Hello</body></html>");
		tag = xml::parseAndDiscardMetadata(R"(<html lang="en">Hello</html>)"); assert(tag->encode() == R"(<html lang="en">Hello</html>)");
		tag = xml::parseAndDiscardMetadata(R"(<html><body><h1></body>test)"); assert(tag->encode() == R"(<html><body><h1></h1></body>test</html>)");
		tag = xml::parseAndDiscardMetadata(R"(<img src="soup"/>)"); assert(tag->encode() == R"(<img src="soup"></img>)");
		tag = xml::parseAndDiscardMetadata(""); assert(tag->encode() == "<body></body>");
		tag = xml::parseAndDiscardMetadata(" \t\r\n<html></html>"); assert(tag->encode() == "<html></html>");
		tag = xml::parseAndDiscardMetadata(R"(<elm attr="value"></elm>)"); assert(tag->encode() == R"(<elm attr="value"></elm>)");
		tag = xml::parseAndDiscardMetadata(R"(<elm attr='value'></elm>)"); assert(tag->encode() == R"(<elm attr="value"></elm>)");

		// parseAndDiscardMetadata should imply <body> when multiple top-level nodes were found
		tag = xml::parseAndDiscardMetadata("<p>foo</p><p>bar</p>"); assert(tag->encode() == "<body><p>foo</p><p>bar</p></body>");
		tag = xml::parseAndDiscardMetadata("No root element? <emoji>sad</emoji>"); assert(tag->encode() == "<body>No root element? <emoji>sad</emoji></body>");

		// Test encoding & decoding of entities
		tag = xml::parseAndDiscardMetadata("<p>&amp;&lt;&gt;</p>"); assert(tag->encode() == "<p>&amp;&lt;&gt;</p>");

		// parseAndDiscardMetadata should discard metadata and return only the real top-level tag
		tag = xml::parseAndDiscardMetadata(R"(<?xml version="1.0" encoding="UTF-8"?><!DOCTYPE html><html></html>)"); assert(tag->encode() == "<html></html>");

		// Test CDATA section parsing
		tag = xml::parseAndDiscardMetadata("<pre><![CDATA[<sender>John Smith</sender>]]></pre>"); assert(tag->encode() == "<pre>&lt;sender&gt;John Smith&lt;/sender&gt;</pre>");
		tag = xml::parseAndDiscardMetadata("<pre>before<![CDATA[during]]>after</pre>"); assert(tag->encode() == "<pre>beforeduringafter</pre>");

		// Cosmetic whitespace should be ignored
		tag = xml::parseAndDiscardMetadata(R"EOC(
<entries>
	<entry primary="">
		<name>primary</name>
	</entry>
</entries>
)EOC");
		assert(tag->encode() == R"(<entries><entry primary=""><name>primary</name></entry></entries>)");

		// Self-closing tags & different modes
		tag = xml::parseAndDiscardMetadata("<root><div/>Hello</root>"); assert(tag->encode() == "<root><div></div>Hello</root>");
		tag = xml::parseAndDiscardMetadata("<root><div/>Hello</root>", xml::MODE_HTML); assert(tag->encode(xml::MODE_HTML) == "<root><div>Hello</div></root>");
		tag = xml::parseAndDiscardMetadata("<root><br>Hello</root>"); assert(tag->encode() == "<root><br>Hello</br></root>");
		tag = xml::parseAndDiscardMetadata("<root><br>Hello</root>", xml::MODE_HTML); assert(tag->encode(xml::MODE_HTML) == "<root><br />Hello</root>");
		tag = xml::parseAndDiscardMetadata(R"(<html><body/>test)"); assert(tag->encode() == R"(<html><body></body>test</html>)");
		tag = xml::parseAndDiscardMetadata(R"(<html><body/>test)", xml::MODE_HTML); assert(tag->encode(xml::MODE_HTML) == R"(<html><body>test</body></html>)");

		// Unquoted attributes
		tag = xml::parseAndDiscardMetadata(R"(<input value=yes disabled>)", xml::MODE_HTML); assert(tag->encode(xml::MODE_HTML) == R"(<input value="yes" disabled />)");
	});

	test("Endianness", []
	{
		assert(Endianness::invert((uint16_t)0x1234u) == 0x3412u);
		assert(Endianness::invert((uint16_t)0x3412u) == 0x1234u);
		assert(Endianness::invert((uint32_t)0x12345678u) == 0x78563412u);
		assert(Endianness::invert((uint32_t)0x78563412u) == 0x12345678u);
		assert(Endianness::invert((uint64_t)0x1234567890ABCDEFull) == 0xEFCDAB9078563412ull);
		assert(Endianness::invert((uint64_t)0xEFCDAB9078563412ull) == 0x1234567890ABCDEFull);
	});

	test("Regex", []
	{
		assert(Regex("ABC|XYZ").matches("ABC") == true);
		assert(Regex("ABC|XYZ").matches("AB") == false);
		assert(Regex("ABC|XYZ").matches("XYZ") == true);
		assert(Regex("ABC|XYZ").matches("XY") == false);

		assert(Regex("(A|B)C").matches("AC") == true);
		assert(Regex("(A|B)C").matches("AD") == false);
		assert(Regex("(A|B)C").matches("BC") == true);
		assert(Regex("(A|B)C").matches("BD") == false);

		assert(Regex("A(.B|.C)D").match("ABCD").toString() == R"(0="ABCD", 1="BC")");
		assert(Regex("A(?:.B|.C)D").match("ABCD").toString() == R"(0="ABCD")");
		assert(Regex("(?'deez'abc)").match("abc").toString() == R"(0="abc", 1{deez}="abc")");
		assert(Regex("(?<deez>abc)").match("abc").toString() == R"(0="abc", 1{deez}="abc")");

		assert(Regex(".").matches("\n") == false);
		assert(Regex(".", "s").matches("\n") == true);

		assert(Regex("^ABC").matches("ABC") == true);
		assert(Regex(".^ABC", "sm").matches("\nABC") == true);
		assert(Regex(".^ABC", "s").matches("\nABC") == false);

		assert(Regex("ABC$").matches("ABC") == true);
		assert(Regex("ABC$").matches("ABC\n") == true);
		assert(Regex("ABC$", "D").matches("ABC\n") == false);

		assert(Regex("[abc]+").matchesFully("abc") == true);
		assert(Regex("[abc]+").matchesFully("abcdef") == false);
		assert(Regex("[a-z]+").matchesFully("abc") == true);
		assert(Regex("[a-z]+").matchesFully("abcdef") == true);
		assert(Regex("[^def]+").match("abcdef").toString() == R"(0="abc")");

		assert(Regex("A?BC").matches("BC") == true);
		assert(Regex("A?BC").matches("ABC") == true);
		assert(Regex("A?BC").matches("DBC") == false);

		assert(Regex("az?b").toString() == "az?b");
		assert(Regex("az?b").matchesFully("ab") == true);
		assert(Regex("az?b").matchesFully("azb") == true);
		assert(Regex("a(bc)?b").matchesFully("ab") == true);
		assert(Regex("a(bc)?b").matchesFully("abcb") == true);

		assert(Regex("").match("ABC").toString() == R"(0="")");

		assert(Regex("AB*C").matchesFully("AC") == true);
		assert(Regex("AB*C").matchesFully("ABC") == true);
		assert(Regex("AB*C").matchesFully("ABBC") == true);

		assert(Regex("A(?=BC)B").match("AB").toString() == "");
		assert(Regex("A(?=BC)B").match("ABC").toString() == R"(0="AB")");
		assert(Regex("A(?=B+C)B").match("ABC").toString() == R"(0="AB")");
		assert(Regex("A(?=)B").matchesFully("AB"));

		assert(Regex("A(?!BC)B").match("AB").toString() == R"(0="AB")");
		assert(Regex("A(?!BC)B").match("ABC").toString() == "");
		assert(Regex("A(?!)B").match("AB").toString() == "");

		assert(Regex("A(?!B)").matches("A") == true);
		assert(Regex("A(?!B)").matches("AB") == false);

		assert(Regex("A{0}B").matchesFully("B") == true);
		assert(Regex("A{0}B").matches("AB") == false);
		assert(Regex("A{3}B").matches("AAB") == false);
		assert(Regex("A{3}B").matchesFully("AAAB") == true);
		assert(Regex("[A]{3}").matchesFully("AAA") == true);

		assert(Regex("(?<=A)BC").search("BC").toString() == "");
		assert(Regex("(?<=A)BC").search("ABC").toString() == R"(0="BC")");
		assert(Regex("(?<=A)BC").search("DBC").toString() == "");

		assert(Regex("(?<!A)BC").search("BC").toString() == R"(0="BC")");
		assert(Regex("(?<!A)BC").search("ABC").toString() == "");
		assert(Regex("(?<!A)BC").search("DBC").toString() == R"(0="BC")");

		assert(Regex(R"(\babc\b)").search("abc").toString() == R"(0="abc")");
		assert(Regex(R"(\babc\b)").search(" abc ").toString() == R"(0="abc")");
		assert(Regex(R"(\babc\b)").search("_abc_").toString() == "");

		assert(Regex(R"(\Bdef)").search("abcdef").toString() == R"(0="def")");
		assert(Regex(R"(\Bdef)").search("abc def").toString() == "");
		assert(Regex(R"(\Bdef)").search("def").toString() == "");

		assert(Regex("(abc|)").matchesFully("abc") == true);
		assert(Regex("(abc|)").matchesFully("") == true);
		assert(Regex("(abc|)").matchesFully("abcdef") == false);
		assert(Regex("a(z|)b").matchesFully("ab") == true);

		assert(Regex(".*B").match("ABAB").toString() == R"(0="ABAB")");
		assert(Regex(".*?B").match("ABAB").toString() == R"(0="AB")");
		assert(Regex(".+B").match("ABAB").toString() == R"(0="ABAB")");
		assert(Regex(".+?B").match("ABAB").toString() == R"(0="AB")");

		assert(Regex(".*B", RE_UNGREEDY).match("ABAB").toString() == R"(0="AB")");
		assert(Regex(".*?B", RE_UNGREEDY).match("ABAB").toString() == R"(0="ABAB")");

		assert(Regex(".").matchesFully("💯") == false);
		assert(Regex(".", RE_UNICODE).matchesFully("💯") == true);

		assert(Regex("(?<=^.)A").search("💯A").isSuccess() == false);
		assert(Regex("(?<=^.)A", RE_UNICODE).search("💯A").toString() == R"(0="A")");
		
		assert(Regex("\\w\\W\\w").matchesFully("A B") == true);

		assert(Regex("\\AA\\Z").matches("A") == true);
		assert(Regex("\\AA\\Z").matches("A\n") == true);
		assert(Regex("\\AA\\z").matches("A") == true);
		assert(Regex("\\AA\\z").matches("A\n") == false);

		SOUP_UNUSED(Regex("A{3,0}")); // this shouldn't infinitely loop at parse-time
		SOUP_UNUSED(Regex("A{3,1}")); // this shouldn't infinitely loop at parse-time
		SOUP_UNUSED(Regex("A{3,2}")); // this shouldn't infinitely loop at parse-time
		SOUP_UNUSED(Regex("A{3,3}")); // this shouldn't infinitely loop at parse-time

		assert(Regex("A{3,6}").matchesFully("A") == false);
		assert(Regex("A{3,6}").matchesFully("AA") == false);
		assert(Regex("A{3,6}").matchesFully("AAA") == true);
		assert(Regex("A{3,6}").matchesFully("AAAA") == true);
		assert(Regex("A{3,6}").matchesFully("AAAAA") == true);
		assert(Regex("A{3,6}").matchesFully("AAAAAA") == true);
		assert(Regex("A{3,6}").matchesFully("AAAAAAA") == false);

		assert(Regex("A{3,}B").matchesFully("B") == false);
		assert(Regex("A{3,}B").matchesFully("AB") == false);
		assert(Regex("A{3,}B").matchesFully("AAB") == false);
		assert(Regex("A{3,}B").matchesFully("AAAB") == true);
		assert(Regex("A{3,}B").matchesFully("AAAAB") == true);

		assert(Regex(R"(\d+)").matchesFully("123"));

		assert(Regex("Abc", "i").matchesFully("abC"));
		assert(Regex("[Abc]{3}", "i").matchesFully("abC"));

		assert(Regex(R"((a)(?:b)(?'deez'c)").match("abc").toString() == R"(0="abc", 1="a", 2{deez}="c")");
		assert(Regex(R"((a)(?:b)(?'deez'c)", "n").match("abc").toString() == R"(0="abc", 1{deez}="c")");

		assert(Regex(R"(\d\s[\d][\s])").matchesFully("1 2 "));

		assert(Regex("a(?i)b(?-i)c").matchesFully("aBc") == true);
		assert(Regex("a(?i)b(?-i)c").matchesFully("aBC") == false);
		assert(Regex("a(?i)b(?-i)c").match("aBc").toString() == R"(0="aBc")");
		assert(Regex("a(?i)b(?-i)(c)").match("aBc").toString() == R"(0="aBc", 1="c")");
		assert(Regex("a(?i:b)c").matchesFully("aBc") == true);
		assert(Regex("a(?i:b)c").matchesFully("aBC") == false);
		assert(Regex("a(?i:b)c").match("aBc").toString() == R"(0="aBc")");

		assert(Regex("a(.+)").match("abc").toString() == R"(0="abc", 1="bc")");
		assert(Regex("(..)+").match("abc").toString() == R"(0="ab", 1="ab")");
		assert(Regex("(..)+").match("abcd").toString() == R"(0="abcd", 1="cd")");
		assert(Regex("(?:.)+").match("abc").toString() == R"(0="abc")");

		assert(Regex("(..){2}").match("abcdef").toString() == R"(0="abcd", 1="cd")");
		assert(Regex("(..){3}").match("abcdef").toString() == R"(0="abcdef", 1="ef")");
		assert(Regex("(..){2,}").match("abcdef").toString() == R"(0="abcdef", 1="ef")");
		assert(Regex("(..){2,3}").match("abcdef").toString() == R"(0="abcdef", 1="ef")");
		assert(Regex("(..){2,}").match("abcde").toString() == R"(0="abcd", 1="cd")");
		assert(Regex("(..){2,3}").match("abcde").toString() == R"(0="abcd", 1="cd")");

		assert(Regex("[a-z]{3,}?").match("abcdef").toString() == R"(0="abc")");
		assert(Regex("[a-z]{3,}?$").match("abcdef").toString() == R"(0="abcdef")");

		assert(Regex("[a-z]{3,6}?").match("abcdef").toString() == R"(0="abc")");
		assert(Regex("[a-z]{3,6}?$").match("abcdef").toString() == R"(0="abcdef")");

		assert(Regex("(?=.)a").matchesFully("a") == true);

		assert(Regex(R"((\w)\1)").matchesFully("aa") == true);
		assert(Regex(R"((\w)\1)").matchesFully("ab") == false);
		assert(Regex(R"(()\1)").matchesFully("") == true);
		assert(Regex(R"(\1())").matchesFully("") == false);

		assert(Regex(R"((?'group'\w)\k'group')").matchesFully("aa") == true);
		assert(Regex(R"((?<group>\w)\k<group>)").matchesFully("aa") == true);

		assert(Regex(R"EOR((?x)(
			[a-z]  # Any letter of the alphabet
			+      # 1 or more times
		))EOR").matchesFully("abc") == true);

		assert(Regex(R"((?<!(?<!\.)\.)\w)").search("a").isSuccess() == true);
		assert(Regex(R"((?<!(?<!\.)\.)\w)").search(".a").isSuccess() == false);
		assert(Regex(R"((?<!(?<!\.)\.)\w)").search("..a").isSuccess() == true);

		assert(Regex(R"EOR((\w+,){3})EOR").match("abc,def,ghi,").toString() == R"(0="abc,def,ghi,", 1="ghi,")");
		assert(Regex(R"EOR((\w+,)+)EOR").match("abc,def,ghi,").toString() == R"(0="abc,def,ghi,", 1="ghi,")");

		assert(Regex("a(.*)z").match("az").toString() == R"(0="az", 1="")");
		assert(Regex("a(.*)z").match("abz").toString() == R"(0="abz", 1="b")");

		assert(Regex("(A)(B)?").match("A").toString() == R"(0="A", 1="A")");
		assert(Regex("A(B)?C").match("AC").toString() == R"(0="AC")");
		assert(Regex("A(B?)C").match("AC").toString() == R"(0="AC", 1="")");

		assert(Regex("(.+a|b.+)").match("bca").toString() == R"(0="bca", 1="bca")");
	});

	test("MessageStream", []
	{
		MessageStream<std::string, int> ms{};
		constexpr int RECIPIENT_ALL = 0; // This recipient will consume the MessageStream via getAllUnread.
		constexpr int RECIPIENT_UNREAD = 1; // This recipient will consume the MessageStream via getOldestUnread.
		assert(ms.getAllUnread(RECIPIENT_ALL).empty());
		assert(ms.getOldestUnread(RECIPIENT_UNREAD) == nullptr);
		ms.add("Foo");
		ms.add("Bar");
		assert(ms.getAllUnread(RECIPIENT_ALL).size() == 2);
		ms.add("Baz");
		assert(ms.getAllUnread(RECIPIENT_ALL).size() == 1);
		const std::string* str;
		str = ms.getOldestUnread(RECIPIENT_UNREAD); assert(str && *str == "Foo");
		str = ms.getOldestUnread(RECIPIENT_UNREAD); assert(str && *str == "Bar");
		str = ms.getOldestUnread(RECIPIENT_UNREAD); assert(str && *str == "Baz");
	});
}

static void unit_hardware()
{
	test("HidUsage", []
	{
		HidUsage usage(1, 0);
		assert(usage.value == (1 << 16));
	});
	test("HidReportDescriptor", []
	{
		// VMware Virtual USB Mouse
		{
			const uint8_t rawdesc[] = { 0x05, 0x01, 0x09, 0x02, 0xA1, 0x01, 0x09, 0x01, 0xA1, 0x00, 0x05, 0x09, 0x19, 0x01, 0x29, 0x08, 0x15, 0x00, 0x25, 0x01, 0x75, 0x01, 0x95, 0x08, 0x81, 0x02, 0x05, 0x01, 0x09, 0x30, 0x09, 0x31, 0x09, 0x38, 0x15, 0x81, 0x25, 0x7F, 0x75, 0x08, 0x95, 0x03, 0x81, 0x06, 0xC0, 0xC0 };
			/* `hid-decode` Output:
			# 0x05, 0x01,                    // Usage Page (Generic Desktop)        0
			# 0x09, 0x02,                    // Usage (Mouse)                       2
			# 0xa1, 0x01,                    // Collection (Application)            4
			# 0x09, 0x01,                    //  Usage (Pointer)                    6
			# 0xa1, 0x00,                    //  Collection (Physical)              8
			# 0x05, 0x09,                    //   Usage Page (Button)               10
			# 0x19, 0x01,                    //   Usage Minimum (1)                 12
			# 0x29, 0x08,                    //   Usage Maximum (8)                 14
			# 0x15, 0x00,                    //   Logical Minimum (0)               16
			# 0x25, 0x01,                    //   Logical Maximum (1)               18
			# 0x75, 0x01,                    //   Report Size (1)                   20
			# 0x95, 0x08,                    //   Report Count (8)                  22
			# 0x81, 0x02,                    //   Input (Data,Var,Abs)              24
			# 0x05, 0x01,                    //   Usage Page (Generic Desktop)      26
			# 0x09, 0x30,                    //   Usage (X)                         28
			# 0x09, 0x31,                    //   Usage (Y)                         30
			# 0x09, 0x38,                    //   Usage (Wheel)                     32
			# 0x15, 0x81,                    //   Logical Minimum (-127)            34
			# 0x25, 0x7f,                    //   Logical Maximum (127)             36
			# 0x75, 0x08,                    //   Report Size (8)                   38
			# 0x95, 0x03,                    //   Report Count (3)                  40
			# 0x81, 0x06,                    //   Input (Data,Var,Rel)              42
			# 0xc0,                          //  End Collection                     44
			# 0xc0,                          // End Collection                      45
			*/
			const auto desc = HidReportDescriptor::parse(rawdesc, sizeof(rawdesc));
			assert(desc.usage_page == 0x01); // Generic Desktop
			assert(desc.usage == 0x02); // Mouse
		}

		// Wooting Two HE, Usage Page 0x1337
		{
			const uint8_t rawdesc[] = { 0x06, 0x37, 0x13, 0x09, 0x01, 0xA1, 0x01, 0x09, 0x02, 0x15, 0x00, 0x26, 0xFF, 0x00, 0x75, 0x08, 0x96, 0x00, 0x01, 0x81, 0x02, 0x09, 0x04, 0x96, 0x00, 0x01, 0x91, 0x02, 0x09, 0x06, 0x95, 0x07, 0xB1, 0x02, 0xC0 };
			/* `hid-decode` Output:
			# 0x06, 0x37, 0x13,              // Usage Page (Vendor Usage Page 0x1337) 0
			# 0x09, 0x01,                    // Usage (Vendor Usage 0x01)           3
			# 0xa1, 0x01,                    // Collection (Application)            5
			# 0x09, 0x02,                    //  Usage (Vendor Usage 0x02)          7
			# 0x15, 0x00,                    //  Logical Minimum (0)                9
			# 0x26, 0xff, 0x00,              //  Logical Maximum (255)              11
			# 0x75, 0x08,                    //  Report Size (8)                    14
			# 0x96, 0x00, 0x01,              //  Report Count (256)                 16
			# 0x81, 0x02,                    //  Input (Data,Var,Abs)               19
			# 0x09, 0x04,                    //  Usage (Vendor Usage 0x04)          21
			# 0x96, 0x00, 0x01,              //  Report Count (256)                 23
			# 0x91, 0x02,                    //  Output (Data,Var,Abs)              26
			# 0x09, 0x06,                    //  Usage (Vendor Usage 0x06)          28
			# 0x95, 0x07,                    //  Report Count (7)                   30
			# 0xb1, 0x02,                    //  Feature (Data,Var,Abs)             32
			# 0xc0,                          // End Collection                      34
			*/
			auto desc = HidReportDescriptor::parse(rawdesc, sizeof(rawdesc));
			assert(desc.usage_page == 0x1337);
			assert(desc.input_report_byte_length == 257);
			assert(desc.output_report_byte_length == 257);
			assert(desc.feature_report_byte_length == 8);
		}

		// Wooting Two HE, Keyboard Usage
		{
			const uint8_t rawdesc[] = { 0x05, 0x01, 0x09, 0x06, 0xA1, 0x01, 0x05, 0x08, 0x15, 0x00, 0x25, 0x01, 0x75, 0x01, 0x95, 0x05, 0x19, 0x01, 0x29, 0x05, 0x91, 0x02, 0x75, 0x03, 0x95, 0x01, 0x91, 0x01, 0x05, 0x07, 0x15, 0x00, 0x25, 0x01, 0x75, 0x01, 0x95, 0x08, 0x19, 0xE0, 0x29, 0xE7, 0x81, 0x02, 0x75, 0x01, 0x95, 0x2E, 0x19, 0x04, 0x29, 0x31, 0x81, 0x02, 0x75, 0x02, 0x95, 0x01, 0x81, 0x01, 0x75, 0x01, 0x95, 0x69, 0x19, 0x33, 0x29, 0x9B, 0x81, 0x02, 0x75, 0x07, 0x95, 0x01, 0x81, 0x01, 0x75, 0x01, 0x95, 0x08, 0x19, 0x9D, 0x29, 0xA4, 0x81, 0x02, 0x75, 0x01, 0x95, 0x2E, 0x19, 0xB0, 0x29, 0xDD, 0x81, 0x02, 0x75, 0x02, 0x95, 0x01, 0x81, 0x01, 0xC0 };
			auto desc = HidReportDescriptor::parse(rawdesc, sizeof(rawdesc));
			assert(desc.usage_page == 0x01); // Generic Desktop
			assert(desc.usage == 0x06); // Keyboard
			{
				const uint8_t report_w[] = { 0x00, 0x00, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
				assert(desc.parseInputReport(report_w, sizeof(report_w)).active_selectors.count(HidUsage(0x07, HID_W)));
			}
			{
				const uint8_t report_ctrl[] = { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
				assert(desc.parseInputReport(report_ctrl, sizeof(report_ctrl)).active_selectors.count(HidUsage(0x07, HID_CONTROL_LEFT)));
			}
			{
				const uint8_t report_ctrl_c[] = { 0x01, 0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
				const auto parsed_report = desc.parseInputReport(report_ctrl_c, sizeof(report_ctrl_c));
				assert(parsed_report.active_selectors.count(HidUsage(0x07, HID_CONTROL_LEFT)));
				assert(parsed_report.active_selectors.count(HidUsage(0x07, HID_C)));
			}
		}

		// DrunkDeer A75 US, Keyboard Usage
		{
			const uint8_t rawdesc[] = { 0x05, 0x01, 0x09, 0x06, 0xA1, 0x01, 0x05, 0x07, 0x19, 0xE0, 0x29, 0xE7, 0x15, 0x00, 0x25, 0x01, 0x75, 0x01, 0x95, 0x08, 0x81, 0x02, 0x95, 0x01, 0x75, 0x08, 0x81, 0x01, 0x95, 0x05, 0x75, 0x01, 0x05, 0x08, 0x19, 0x01, 0x29, 0x05, 0x91, 0x02, 0x95, 0x01, 0x75, 0x03, 0x91, 0x01, 0x95, 0x05, 0x75, 0x08, 0x15, 0x00, 0x26, 0x94, 0x00, 0x05, 0x07, 0x19, 0x00, 0x2A, 0x94, 0x00, 0x81, 0x00, 0x05, 0xFF, 0x09, 0x03, 0x75, 0x08, 0x95, 0x01, 0x81, 0x02, 0xC0 };
			const auto desc = HidReportDescriptor::parse(rawdesc, sizeof(rawdesc));
			assert(desc.usage_page == 0x01); // Generic Desktop
			assert(desc.usage == 0x06); // Keyboard
			{
				const uint8_t report_w[] = { 0x00, 0x00, 0x1A, 0x00, 0x00, 0x00, 0x00, 0x00 };
				assert(desc.parseInputReport(report_w, sizeof(report_w)).active_selectors.count(HidUsage(0x07, HID_W)));
			}
			{
				const uint8_t report_ctrl[] = { 0x01, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
				assert(desc.parseInputReport(report_ctrl, sizeof(report_ctrl)).active_selectors.count(HidUsage(0x07, HID_CONTROL_LEFT)));
			}
			{
				const uint8_t report_ctrl_c[] = { 0x01, 0x00, 0x06, 0x00, 0x00, 0x00, 0x00, 0x00 };
				const auto parsed_report = desc.parseInputReport(report_ctrl_c, sizeof(report_ctrl_c));
				assert(parsed_report.active_selectors.count(HidUsage(0x07, HID_CONTROL_LEFT)));
				assert(parsed_report.active_selectors.count(HidUsage(0x07, HID_C)));
			}
		}
	});
}

static void unit_io()
{
	test("BitReader", []
	{
		StringReader r("\xF0\x0F");
		BitReader br(&r);
		uint8_t b;

		assert(br.u8(4, b) && b == 0x0);
		assert(br.u8(4, b) && b == 0xF);
		assert(br.u8(4, b) && b == 0xF);
		assert(br.u8(4, b) && b == 0x0);

		r = "\xE0\x03";
		assert(br.u8(5, b) && b == 0);
		assert(br.u8(5, b) && b == 0b11111);
		assert(br.u8(6, b) && b == 0);
	});
	test("BitWriter", []
	{
		StringWriter w;
		BitWriter bw(&w);

		w.data.clear();
		bw.u8(4, 0x0);
		bw.u8(4, 0xF);
		bw.u8(4, 0xF);
		bw.u8(4, 0x0);
		bw.finishByte();
		assert(w.data == "\xF0\x0F");

		w.data.clear();
		bw.u8(5, 0b11111);
		bw.u8(5, 0);
		bw.u8(6, 0b111111);
		bw.finishByte();
		assert(w.data == "\x1F\xFC");

		w.data.clear();
		bw.u8(5, 0b11111);
		bw.u8(4, 0b101);
		bw.finishByte();
		assert(w.data == "\x5F\x01");
	});
	test("StringRefReader", []
	{
		std::string s = "Hello\nWorld";
		MemoryRefReader sr(s);
		std::string line;
		assert(sr.getLine(line));
		assert(line == "Hello");
		assert(sr.getLine(line));
		assert(line == "World");
		assert(!sr.getLine(line));
	});
}

static void unit_lang()
{
	unit("php")
	{
		test("echo", []
		{
			PhpState php;
			assert(php.evaluate(R"(<?php echo "Hello";)") == "Hello");
			assert(php.evaluate(R"(<?php echo 123;)") == "123");
		});
		test("variables", []
		{
			PhpState php;
			assert(php.evaluate(R"(<?php $a = "Hello"; echo $a;)") == "Hello");
			assert(php.evaluate(R"(<?php $a = 1; echo $a;)") == "1");
			assert(php.evaluate(R"(<?php
$a = "Hi";
function f()
{
	$a = "Hello";
}
f();
echo $a;
)") == "Hi");
		});
		test("concat", []
		{
			PhpState php;
			assert(php.evaluate(R"(<?php $a = "Hello"; echo $a;)") == "Hello");
			assert(php.evaluate(R"(<?php $a = "Hello, "."world!"; echo $a;)") == "Hello, world!");
		});
		test("functions", []
		{
			PhpState php;
			assert(php.evaluate(R"(<?php $a = function(){ echo "Hello"; }; $a();)") == "Hello");
			assert(php.evaluate(R"(<?php $a = function(){ ?>Hello<?php }; $a();)") == "Hello");
			assert(php.evaluate(R"(<?php $a = function(){ ?>Hello<?php };)") == "");
			assert(php.evaluate(R"(<?php $a = function(){ $a = "Hello"; echo $a; }; $a();)") == "Hello");
			assert(php.evaluate(R"(<?php function a() { echo "Hello"; } a();)") == "Hello");
			assert(php.evaluate(R"(<?php function a($a) { echo $a; } a("Hi");)") == "Hi");
			assert(php.evaluate(R"(<?php
function greet($greeting, $subject)
{
	echo $greeting.", ".$subject."!";
}
greet("Hello", "world");)") == "Hello, world!");
		});
		test("comments", []
		{
			PhpState php;
			assert(php.evaluate(R"(<?php
//echo "Hello";
echo "Hello, world!";)") == "Hello, world!");
		});
		test("comparisons", []
		{
			PhpState php;
			assert(php.evaluate(R"(<?php echo 1 == 1;)") == "1");
		});
		test("conditionals", []
		{
			PhpState php;
			assert(php.evaluate(R"(<?php
$a = 1;
if($a == 1)
{
	echo "true";
}
else
{
	echo "false";
})") == "true");
			assert(php.evaluate(R"(<?php
$a = 2;
if($a == 1)
{
	echo "true";
}
else
{
	echo "false";
})") == "false");
			assert(php.evaluate(R"(<?php
$a = 1;
if($a == 1):
echo "Hello";
endif;)") == "Hello");
			assert(php.evaluate(R"(<?php
$a = 1;
if($a == 2):
echo "Hello";
endif;)") == "");
		});
		test("comparisons", []
		{
			PhpState php;
			php.request_uri = "/1337";
			assert(php.evaluate(R"(<?php echo "Request URI: ".$_SERVER["REQUEST_URI"];)") == "Request URI: /1337");
		});
	}

	unit("MathExpr")
	{
		test("order of operations", []
		{
			assert(MathExpr::evaluate("3 - 2 + 1") == 2);
			assert(MathExpr::evaluate("1 * 2 + 3 * 4") == 14);
		});
		test("unspaced equations", []
		{
			assert(MathExpr::evaluate("1+2") == 3);
		});
	}

	unit("WASM")
	{
		test("addTwo", []
		{
			WasmScript ws;
			assert(ws.load(base64::decode("AGFzbQEAAAABBwFgAn9/AX8DAgEABwoBBmFkZFR3bwAACgkBBwAgACABagsACgRuYW1lAgMBAAA=")));
			auto code = ws.getExportedFuntion("addTwo");
			assert(code);
			WasmVm vm(ws);
			vm.locals.emplace_back(1);
			vm.locals.emplace_back(2);
			assert(vm.run(*code));
			assert(!vm.stack.empty());
			assert(vm.stack.top().i32 == 3);
			assert(vm.stack.pop(), vm.stack.empty());
		});
		test("Memory", []
		{
			WasmScript scr;
			assert(scr.load(base64::decode("AGFzbQEAAAABCgJgAAF/YAF/AX8DAwIAAQUDAQABByIDCmdldF9zdHJpbmcAAAhnZXRfYnl0ZQABBm1lbW9yeQIACg8CBQBBoAgLBwAgAC0AAAsLGwIAQYwICwEcAEGYCAsNAgAAAAYAAABsAG8AbABOBG5hbWUBIwIAEGluZGV4L2dldF9zdHJpbmcBDmluZGV4L2dldF9ieXRlAggCAAABAQABMAQHAgABMAEBMQYEAQABMAkJAgABMAEDMC4x")));
			{
				auto code = scr.getExportedFuntion("get_string");
				assert(code);
				WasmVm vm(scr);
				assert(vm.run(*code));
				assert(!vm.stack.empty());
				assert(unicode::utf16_to_utf8<UTF16_STRING_TYPE>(scr.getMemory<const UTF16_CHAR_TYPE>(vm.stack.top().i32)) == "lol");
				assert(vm.stack.pop(), vm.stack.empty());
			}
			{
				auto code = scr.getExportedFuntion("get_byte");
				assert(code);
				WasmVm vm(scr);
				vm.locals.emplace_back(1036);
				assert(vm.run(*code));
				assert(!vm.stack.empty());
				assert(vm.stack.top().i32 == 0x1c);
				assert(vm.stack.pop(), vm.stack.empty());
			}
		});
		test("Loop", []
		{
			WasmScript scr;
			assert(scr.load(base64::decode("AGFzbQEAAAABBwFgA39/fwADAgEABQMBAAAHEwIGbWVtc2V0AAAGbWVtb3J5AgAKKQEnAQF/A0AgAiIDQQFrIQIgAwRAIAAiA0EBaiEAIAMgAToAAAwBCwsLADMEbmFtZQEPAQAMaW5kZXgvbWVtc2V0Ag8BAAQAATABATECATIDATMEBAEAATAGBAEAATA=")));
			auto code = scr.getExportedFuntion("memset");
			assert(code);
			WasmVm vm(scr);
			vm.locals.emplace_back(0x10);
			vm.locals.emplace_back(69);
			vm.locals.emplace_back(0x10);
			assert(vm.run(*code));
			assert(string::bin2hex(std::string(scr.getMemory<const char>(0x00), 0x10)) == "00000000000000000000000000000000");
			assert(string::bin2hex(std::string(scr.getMemory<const char>(0x10), 0x10)) == "45454545454545454545454545454545");
			assert(string::bin2hex(std::string(scr.getMemory<const char>(0x20), 0x10)) == "00000000000000000000000000000000");
		});
		test("Imports", []
		{
			WasmScript scr;
			assert(scr.load(base64::decode("AGFzbQEAAAABDAJgAn9/AX9gAX8BfwINAQVpbmRleANhZGQAAAMCAQEFAwEAAAcTAgZhZGRUd28AAQZtZW1vcnkCAAoKAQgAIABBAhAACwA6BG5hbWUBGgIACWluZGV4L2FkZAEMaW5kZXgvYWRkVHdvAggCAAABAQABMAQHAgABMAEBMQYEAQABMA==")));
			auto fi = scr.getImportedFunction("index", "add");
			assert(fi);
			fi->ptr = [](WasmVm& vm)
			{
				auto b = vm.stack.top(); vm.stack.pop();
				auto a = vm.stack.top(); vm.stack.pop();
				vm.stack.push(a.i32 + b.i32);
			};
			auto code = scr.getExportedFuntion("addTwo");
			assert(code);
			WasmVm vm(scr);
			vm.locals.emplace_back(40);
			assert(vm.run(*code));
			assert(!vm.stack.empty());
			assert(vm.stack.top().i32 == 42);
			assert(vm.stack.pop(), vm.stack.empty());			
		});
		test("Call Non-Imported", []
		{
			WasmScript scr;
			assert(scr.load(base64::decode("AGFzbQEAAAABDAJgAX8Bf2ACf38BfwMDAgEABQMBAAEHDAEIaXNfbWFnaWMAAQo9AjIBAn8DQCAAIgNBAWohACABIgJBAWohASADLQAAIgMgAi0AAEcEQEEADwsgAw0AC0EBCwgAIABBARAACwsLAQBBAQsFZGVlegAANARuYW1lARoCAAZzdHJjbXABD2lzX2hvc3Rpbmdfc2x1ZwIRAgAEAAEwAQExAgEyAwEzAQA=")));
			auto scrap = scr.allocateMemory(sizeof("deez"));
			assert(scr.setMemory(scrap, "deez", sizeof("deez")));
			auto code = scr.getExportedFuntion("is_magic");
			assert(code);
			WasmVm vm(scr);
			vm.locals.emplace_back(scrap);
			assert(vm.run(*code));
			assert(!vm.stack.empty());
			assert(vm.stack.top().i32 == 1);
			assert(vm.stack.pop(), vm.stack.empty());
		});
		test("Call Indirect", []
		{
			// (module
			//   (type $FUNCSIG$i (func (param i32) (result i32)))
			//   (table funcref
			//     (elem $test)
			//   )
			//   (memory $0 1)
			//   (func $test (type $FUNCSIG$i) (param $0 i32) (result i32)
			//     (i32.add (local.get $0) (i32.const 40))
			//   )
			//   (func $main (result i32)
			//     (call_indirect (type $FUNCSIG$i)
			//       (i32.const 2) ;; argument
			//       (i32.const 0) ;; table index
			//     )
			//   )
			//   (export "main" (func $main))
			// )
			WasmScript scr;
			assert(scr.load(base64::decode("AGFzbQEAAAABCgJgAX8Bf2AAAX8DAwIAAQQFAXABAQEFAwEAAQcIAQRtYWluAAEJBwEAQQALAQAKEwIHACAAQShqCwkAQQJBABEAAAsAMgRuYW1lAQ0CAAR0ZXN0AQRtYWluAggCAAEAATABAAQMAQAJRlVOQ1NJRyRpBgQBAAEw")));
			auto code = scr.getExportedFuntion("main");
			assert(code);
			WasmVm vm(scr);
			assert(vm.run(*code));
			assert(!vm.stack.empty());
			assert(vm.stack.top().i32 == 42);
			assert(vm.stack.pop(), vm.stack.empty());
		});
		test("Forward Branching", []
		{
			// (module  
			//   (func $f64 (result i32)
			//     (block $B0
			//       (loop $L1
			//         (block $B2
			//           (br $B0)
			//          )
			//         (br $L1)))
			//   	i32.const 42
			//   )
			//   (export "main" (func $f64))
			// )
			WasmScript scr;
			assert(scr.load(base64::decode("AGFzbQEAAAABBQFgAAF/AwIBAAcIAQRtYWluAAAKEwERAAJAA0ACQAwCCwwACwtBKgsAEgRuYW1lAQYBAANmNjQCAwEAAA==")));
			auto code = scr.getExportedFuntion("main");
			assert(code);
			WasmVm vm(scr);
			assert(vm.run(*code));
			assert(!vm.stack.empty());
			assert(vm.stack.top().i32 == 42);
			assert(vm.stack.pop(), vm.stack.empty());
		});
		test("Floats", []
		{
			WasmScript scr;
			assert(scr.load(base64::decode("AGFzbQEAAAABBgFgAXwBfAMCAQAHBwEDZmFjAAAKLgEsACAARAAAAAAAAPA/YwR8RAAAAAAAAPA/BSAAIABEAAAAAAAA8D+hEACiCwsAEgRuYW1lAQYBAANmYWMCAwEAAA==")));
			auto code = scr.getExportedFuntion("fac");
			assert(code);
			WasmVm vm(scr);
			vm.locals.emplace_back(5.0);
			assert(vm.run(*code));
			assert(!vm.stack.empty());
			assert(vm.stack.top().f64 == 120.0);
			assert(vm.stack.pop(), vm.stack.empty());
		});
	}

	test("reflection", []
	{
		{
			struct Person { const char* name; int age; };
			rflParser par(R"EOC(struct Person { const char* name; int age; };)EOC");
			auto t = par.readStruct();
			assert(t.name == "Person");
			assert(t.members.size() == 2);
			assert(t.members.at(0).type.name == "const char");
			assert(t.members.at(0).type.toString() == "const char*");
			assert(t.members.at(0).name == "name");
			assert(t.members.at(1).type.name == "int");
			assert(t.members.at(1).type.toString() == "int");
			assert(t.members.at(1).name == "age");
			assert(t.getOffsetOf("name") == offsetof(Person, name));
			assert(t.getOffsetOf("age") == offsetof(Person, age));
			assert(t.getSize() == sizeof(Person));
		}

		{
			rflParser par(R"EOC(struct Person { const char *name; };)EOC");
			auto t = par.readStruct();
			assert(t.name == "Person");
			assert(t.members.size() == 1);
			assert(t.members.at(0).type.name == "const char");
			assert(t.members.at(0).type.toString() == "const char*");
			assert(t.members.at(0).name == "name");
		}

		{
			struct AlignmentTest { bool a; bool b; bool c; int i; };
			rflParser par(R"EOC(struct AlignmentTest { bool a; bool b; bool c; int i; };)EOC");
			auto t = par.readStruct();
			assert(t.name == "AlignmentTest");
			assert(t.getOffsetOf("a") == offsetof(AlignmentTest, a));
			assert(t.getOffsetOf("b") == offsetof(AlignmentTest, b));
			assert(t.getOffsetOf("c") == offsetof(AlignmentTest, c));
			assert(t.getOffsetOf("i") == offsetof(AlignmentTest, i));
			assert(t.getSize() == sizeof(AlignmentTest));
		}

		{
			struct PackedTest { bool a, b, *c; };
			rflParser par(R"EOC(struct PackedTest { bool a, b, *c; };)EOC");
			auto t = par.readStruct();
			assert(t.name == "PackedTest");
			assert(t.getOffsetOf("a") == offsetof(PackedTest, a));
			assert(t.getOffsetOf("b") == offsetof(PackedTest, b));
			assert(t.getOffsetOf("c") == offsetof(PackedTest, c));
			assert(t.getSize() == sizeof(PackedTest));
		}
	});
}

static void test_chatbot_implementables()
{
	{
		auto res = Chatbot::process("delete 10");
		assert(res.type == CB_RES_DELETE);
		assert(res.getDeleteNum() == 10);
	}
}

static void test_chatbot_results()
{
	// cbCmdArithmetic
	assert(Chatbot::process("1000 + 234").response.find("1234") != std::string::npos);
	assert(Chatbot::process("1000+234").response.find("1234") != std::string::npos);
	assert(Chatbot::process("What's 1 + 2 + 3?").response.find("6") != std::string::npos);

	// cbCmdConvert
	assert(Chatbot::process("6900 kg in tonnes").response.find("6.9") != std::string::npos);
	assert(Chatbot::process("6900 KG in tonnes").response.find("6.9") != std::string::npos);
}

static void unit_math_bigint()
{
	test("getTrailingZeroesBinary", []
	{
		assert("1"_b.getTrailingZeroesBinary() == 0);
		assert("2"_b.getTrailingZeroesBinary() == 1);
		assert(Bigint::_2pow(100).getTrailingZeroesBinary() == 100);
	});
}

static void unit_math()
{
	test("pow", []
	{
		assert(soup::pow(10, 6) == 1000000);
	});
}

static void unit_net_email()
{
	test("EmailAddress", []
	{
		{
			EmailAddress addr("Deez Nuts <deez@nuts.co>");
			assert(addr.address == "deez@nuts.co");
			assert(addr.name == "Deez Nuts");
			assert(addr.toString() == "Deez Nuts <deez@nuts.co>");
			assert(addr.getDomain() == "nuts.co");
		}
		{
			EmailAddress addr("<deez@nuts.co>");
			assert(addr.address == "deez@nuts.co");
			assert(addr.name == "");
			assert(addr.toString() == "<deez@nuts.co>");
			assert(addr.getDomain() == "nuts.co");
		}
	});
}

static void test_uri()
{
	std::string str;
	Uri uri;

	str = "google.com";
	uri = Uri(str);
	assert(uri.scheme == "");
	assert(uri.host == "google.com");
	assert(uri.port == 0);
	assert(uri.user == "");
	assert(uri.pass == "");
	assert(uri.path == "");
	assert(uri.query == "");
	assert(uri.fragment == "");
	// Not asserting `uri.toString() == str` because Soup encodes it as "//google.com", which is perfectly fine as well.

	str = "//google.com";
	uri = Uri(str);
	assert(uri.scheme == "");
	assert(uri.host == "google.com");
	assert(uri.port == 0);
	assert(uri.user == "");
	assert(uri.pass == "");
	assert(uri.path == "");
	assert(uri.query == "");
	assert(uri.fragment == "");
	assert(uri.toString() == str);

	str = "https://google.com";
	uri = Uri(str);
	assert(uri.scheme == "https");
	assert(uri.host == "google.com");
	assert(uri.port == 0);
	assert(uri.user == "");
	assert(uri.pass == "");
	assert(uri.path == "");
	assert(uri.query == "");
	assert(uri.fragment == "");
	assert(uri.toString() == str);

	str = "//google.com/";
	uri = Uri(str);
	assert(uri.scheme == "");
	assert(uri.host == "google.com");
	assert(uri.port == 0);
	assert(uri.user == "");
	assert(uri.pass == "");
	assert(uri.path == "/");
	assert(uri.query == "");
	assert(uri.fragment == "");
	assert(uri.toString() == str);

	str = "//shady.nz/path#anchor";
	uri = Uri(str);
	assert(uri.scheme == "");
	assert(uri.host == "shady.nz");
	assert(uri.port == 0);
	assert(uri.user == "");
	assert(uri.pass == "");
	assert(uri.path == "/path");
	assert(uri.query == "");
	assert(uri.fragment == "anchor");
	assert(uri.toString() == str);

	str = "http://username:password@hostname:9090/path?arg=value#anchor";
	uri = Uri(str);
	assert(uri.scheme == "http");
	assert(uri.host == "hostname");
	assert(uri.port == 9090);
	assert(uri.user == "username");
	assert(uri.pass == "password");
	assert(uri.path == "/path");
	assert(uri.query == "arg=value");
	assert(uri.fragment == "anchor");
	assert(uri.toString() == str);

	str = "https://web-safety.net/..;@www.google.com:%3443/";
	uri = Uri(str);
	assert(uri.scheme == "https");
	assert(uri.host == "web-safety.net");
	assert(uri.port == 0);
	assert(uri.user == "");
	assert(uri.pass == "");
	assert(uri.path == "/..;@www.google.com:%3443/");
	assert(uri.query == "");
	assert(uri.fragment == "");
	// Not testing uri.toString because this one is very malformatted so it's expected that Soup re-encodes it more sanely.

	str = "https://google.com?q=Soup";
	uri = Uri(str);
	assert(uri.scheme == "https");
	assert(uri.host == "google.com");
	assert(uri.port == 0);
	assert(uri.user == "");
	assert(uri.pass == "");
	assert(uri.path == "");
	assert(uri.query == "q=Soup");
	assert(uri.fragment == "");
	assert(uri.toString() == str);

	str = "https://google.com#q=Soup";
	uri = Uri(str);
	assert(uri.scheme == "https");
	assert(uri.host == "google.com");
	assert(uri.port == 0);
	assert(uri.user == "");
	assert(uri.pass == "");
	assert(uri.path == "");
	assert(uri.query == "");
	assert(uri.fragment == "q=Soup");
	assert(uri.toString() == str);

	str = "//example.com:443";
	uri = Uri(str);
	assert(uri.scheme == "");
	assert(uri.host == "example.com");
	assert(uri.port == 443);
	assert(uri.user == "");
	assert(uri.pass == "");
	assert(uri.path == "");
	assert(uri.query == "");
	assert(uri.fragment == "");
	assert(uri.toString() == str);

	str = "https://clients5.google.com/translate_a/t?client=dict-chrome-ex&sl=auto&tl=zh-CN&q=How+are+you%3F";
	uri = Uri(str);
	assert(uri.getRequestPath() == "/translate_a/t?client=dict-chrome-ex&sl=auto&tl=zh-CN&q=How+are+you%3F");
}

static void test_socket_raii_semantics()
{
	Socket s;
	assert(!s.hasConnection());
	s.fd = 1337;
	assert(s.hasConnection());
	Socket s2 = std::move(s);
	assert(!s.hasConnection());
	assert(s2.hasConnection());
	Socket s3;
	s3 = std::move(s2);
	assert(!s2.hasConnection());
	assert(s3.hasConnection());
	s3.fd.setMovedAway(); // don't try to actually close() fd 1337 now lol
}

static void unit_util_string()
{
	test("equalsIgnoreCase", []
	{
		assert(string::equalsIgnoreCase<std::string>("java", "java") == true);
		assert(string::equalsIgnoreCase<std::string>("Java", "java") == true);
		assert(string::equalsIgnoreCase<std::string>("Java", "Lava") == false);
		assert(string::equalsIgnoreCase<std::string>("Java", "Javaa") == false);
	});
	test("levenshtein", []
	{
		assert(string::levenshtein<std::string>("successfully", "success") == 5);
		assert(string::levenshtein<std::string>("alpha", "scope") == 5);
		assert(string::levenshtein<std::string>("equal", "equal") == 0);
		assert(string::levenshtein<std::string>("Hello, world!", "The sun is shining and it lets me know the day has just begun.") == 57);
	});
	test("StringMatch::search", []
	{
		assert(StringMatch::search("run program", "Run Script/Program") == true);
		assert(StringMatch::search("apple", "orange") == false);
		assert(StringMatch::search("bad", "The quick brown fox jumps over the lazy dog.") == false);
		assert(StringMatch::search("the bow", "The quick brown fox jumps over the lazy dog.") == false);
		assert(StringMatch::search("the same", "the same") == true);
		assert(StringMatch::search("play level", "Play next level") == true);
		assert(StringMatch::search("play next", "Play next level") == true);
		assert(StringMatch::search("find", "Find Objective") == true);
		assert(StringMatch::search("find", "Finish Dagger") == false);
	});
	test("StringMatch::wildcard", []
	{
		assert(StringMatch::wildcard("*.cpp", "foo.cpp") == true);
		assert(StringMatch::wildcard("*.cpp", "foo.bar.cpp") == true);

		assert(StringMatch::wildcard("*.*.cpp", "f.cpp") == false);
		assert(StringMatch::wildcard("*.*.cpp", "fo.cpp") == false);
		assert(StringMatch::wildcard("*.*.cpp", "foo.cpp") == false);
		assert(StringMatch::wildcard("*.*.cpp", "fooo.cpp") == false);
		assert(StringMatch::wildcard("*.*.cpp", "foooo.cpp") == false);
		assert(StringMatch::wildcard("*.*.cpp", "foo.bar.cpp") == true);

		assert(StringMatch::wildcard("****.cpp", "foo.cpp", 0) == true);
		assert(StringMatch::wildcard("***.cpp", "foo.cpp", 1) == true);
		assert(StringMatch::wildcard("****.cpp", "foo.cpp", 1) == false);

		assert(StringMatch::wildcard("lib*.so", "libsoup.so") == true);

		assert(StringMatch::wildcard("lib*_ext*.cpp", "libsoup_extspoon.cpp") == true);
	});
	test("format", []
	{
		assert(format("{} has {}") == " has ");
		assert(format("{} has {}", "John") == "John has ");
		assert(format("{} has {}", "John", 10) == "John has 10");
		assert(format("{} has {}", "John", 10, 20) == "John has 10");
	});
	test("toInt", []
	{
		assert(string::toInt<int>("1337", 0) == 1337);
		assert(string::toInt<int>("1337.", 0) == 1337);
		assert(string::toInt<int, string::TI_FULL>("1337", 0) == 1337);
		assert(string::toInt<int, string::TI_FULL>("1337.", 0) == 0);
		assert(string::hexToInt<int>("1337", 0) == 0x1337);
		assert(string::hexToInt<int>("1337.", 0) == 0x1337);
		assert(string::hexToInt<int, string::TI_FULL>("1337", 0) == 0x1337);
		assert(string::hexToInt<int, string::TI_FULL>("1337.", 0) == 0);
		assert(string::toInt<int>(L"1337", 0) == 1337);
		assert(string::toInt<int>(L"1337.", 0) == 1337);
		assert(string::toInt<int, string::TI_FULL>(L"1337", 0) == 1337);
		assert(string::toInt<int, string::TI_FULL>(L"1337.", 0) == 0);
		assert(string::hexToInt<int>(L"1337", 0) == 0x1337);
		assert(string::hexToInt<int>(L"1337.", 0) == 0x1337);
		assert(string::hexToInt<int, string::TI_FULL>(L"1337", 0) == 0x1337);
		assert(string::hexToInt<int, string::TI_FULL>(L"1337.", 0) == 0);
	});
	test("truncateWithEllipsis", []
	{
		assert(string::truncateWithEllipsis<std::string>("Hello, world!", 12) == "Hello, wo...");
		assert(string::truncateWithEllipsis<std::string>("Hello, world!", 13) == "Hello, world!");
	});
}

static void unit_util_time()
{
	test("ISO 8601", []
	{
		assert(Datetime::fromIso8601("2024-04-25T00:00:00Z").value().toTimestamp() == 1714003200);
		assert(Datetime::fromIso8601("2023-08-10T03:00:00.000Z").value().toTimestamp() == 1691636400);
	});
}

static void unit_util()
{
	test("bin2hex", []
	{
		assert(string::bin2hex("\x1\x2\x3") == "010203");
	});
	test("version_compare", []
	{
		assert(version_compare("1.0", "0.1") > 0);
		assert(version_compare("0.1", "0") > 0);
		assert(version_compare("1.0", "1.0-dev") > 0);
	});
}

static void unit_vis()
{
	test("Rgb", []
	{
		assert(Rgb(0x00, 0x00, 0x00).toHex() == "000");
		assert(Rgb(0xFF, 0x00, 0xFF).toHex() == "F0F");
		assert(Rgb(0xFF, 0xFF, 0xFF).toHex() == "FFF");
		assert(Rgb(0x12, 0x34, 0x56).toHex() == "123456");
	});
}

void cli_test()
{
	unit("soup")
	{
		unit("cpu")
		{
			unit_cpu();
		}
		unit("crypto")
		{
			unit_crypto();
		}
		unit("data")
		{
			unit_data();
		}
		unit("hardware")
		{
			unit_hardware();
		}
		unit("io")
		{
			unit_io();
		}
		unit("lang")
		{
			unit_lang();
		}
		unit("ling")
		{
			unit("chatbot")
			{
				test("implementables", &test_chatbot_implementables);
				test("results", &test_chatbot_results);
			}
		}
		unit("math")
		{
			unit("bigint")
			{
				unit_math_bigint();
			}
			unit_math();
		}
		unit("net")
		{
			unit("email")
			{
				unit_net_email();
			}
			unit("web")
			{
				test("uri", &test_uri);
			}
			test("socket raii semantics", &test_socket_raii_semantics);
		}
		unit("util")
		{
			unit("string")
			{
				unit_util_string();
			}
			unit("time")
			{
				unit_util_time();
			}
			unit_util();
		}
		unit("vis")
		{
			unit_vis();
		}
	}
}
