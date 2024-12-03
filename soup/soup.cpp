#include "base.hpp"

// Don't want to produce bindings when building Soup as a static lib.
#if defined(SOUP_STANDALONE) || defined(_WINDLL) || SOUP_WASM || SOUP_CODE_INSPECTOR

#include <stdexcept>
#include <string>

using stdexception = std::exception;
using stdstring = std::string;

#if SOUP_CODE_INSPECTOR
#include "soup.h" // Dummy include, prevents IntelliSense from adding this
#else
typedef void (*void_func_t)();
#endif

#include "RaiiEmulator.hpp"

#include "aes.hpp"
#include "base32.hpp"
#include "base40.hpp"
#include "base64.hpp"
#include "Canvas.hpp"
#include "cbResult.hpp"
#include "Chatbot.hpp"
#include "CidrSubnetInterface.hpp"
#include "country_names.hpp"
#include "DetachedScheduler.hpp"
#include "EstablishWebSocketConnectionTask.hpp"
#include "Hotp.hpp"
#include "HttpRequest.hpp"
#include "HttpRequestTask.hpp"
#include "InquiryLang.hpp"
#include "IpAddr.hpp"
#include "MimeMessage.hpp"
#include "Mixed.hpp"
#include "Notifyable.hpp"
#include "Promise.hpp"
#include "QrCode.hpp"
#include "rsa.hpp"
#include "Server.hpp"
#include "ServerWebService.hpp"
#include "Totp.hpp"
#include "Uri.hpp"
#include "WebSocketMessage.hpp"
#include "Worker.hpp"
#include "YubikeyValidator.hpp"

using namespace soup;

static RaiiEmulator heap{};
static std::string ret_str_buf{};

[[nodiscard]] static const char* ret_str(std::string&& str)
{
	ret_str_buf = std::move(str);
	return ret_str_buf.c_str();
}

#define returnString(x) return ret_str(x);

// [global namespace]

SOUP_CEXPORT void beginScope()
{
	heap.beginScope();
}

SOUP_CEXPORT void endScope()
{
	heap.endScope();
}

SOUP_CEXPORT void broadenScope(void* inst)
{
	heap.broadenScope(inst);
}

SOUP_CEXPORT void endLifetime(void* inst)
{
	heap.free(inst);
}

SOUP_CEXPORT const char* tryCatch(void_func_t f)
{
	try
	{
		f();
	}
	catch (const std::exception& e)
	{
		returnString(e.what());
	}
	catch (...)
	{
		returnString("...");
	}
	return nullptr;
}

SOUP_CEXPORT void throwException(const char* msg)
{
	throw Exception(msg);
}

SOUP_CEXPORT const char* getCountryName(const char* country_code, const char* language_code)
{
	return soup::getCountryName(country_code, language_code);
}

// aes

SOUP_CEXPORT void aes_ecbEncrypt(stdstring* data, const stdstring* key)
{
	heap.validate(data);
	heap.validate(key);
	aes::ecbEncrypt(
		reinterpret_cast<uint8_t*>(data->data()), data->size(),
		reinterpret_cast<const uint8_t*>(key->data()), key->size()
	);
}

SOUP_CEXPORT void aes_ecbDecrypt(stdstring* data, const stdstring* key)
{
	heap.validate(data);
	heap.validate(key);
	aes::ecbDecrypt(
		reinterpret_cast<uint8_t*>(data->data()), data->size(),
		reinterpret_cast<const uint8_t*>(key->data()), key->size()
	);
}

// base32

SOUP_CEXPORT const char* base32_encode(const stdstring* x, bool pad)
{
	returnString(base32::encode(heap.get(x), pad));
}

SOUP_CEXPORT stdstring* base32_decode(const char* x)
{
	return heap.add(new std::string(base32::decode(x)));
}

// base40

SOUP_CEXPORT const char* base40_encode(const stdstring* x)
{
	returnString(base40::encode(heap.get(x)));
}

SOUP_CEXPORT stdstring* base40_decode(const char* x)
{
	return heap.add(new std::string(base40::decode(x)));
}

// base64

SOUP_CEXPORT const char* base64_encode(const stdstring* x)
{
	returnString(base64::encode(heap.get(x)));
}

SOUP_CEXPORT stdstring* base64_decode(const char* x)
{
	return heap.add(new std::string(base64::decode(x)));
}

// Bigint

SOUP_CEXPORT const char* Bigint_toString(const Bigint* x)
{
	returnString(x->toString());
}

// Canvas

SOUP_CEXPORT unsigned int Canvas_getWidth(const Canvas* x)
{
	return x->width;
}

SOUP_CEXPORT unsigned int Canvas_getHeight(const Canvas* x)
{
	return x->height;
}

SOUP_CEXPORT void Canvas_resizeNearestNeighbour(Canvas* x, unsigned int desired_width, unsigned int desired_height)
{
	x->resizeNearestNeighbour(desired_width, desired_height);
}

SOUP_CEXPORT const char* Canvas_toSvg(const Canvas* x, unsigned int scale)
{
	returnString(x->toSvg(scale));
}

SOUP_CEXPORT stdstring* Canvas_toNewPngString(const Canvas* x)
{
	return heap.add(x->toPng());
}

// cbResult

SOUP_CEXPORT const char* cbResult_getResponse(const cbResult* x)
{
	return heap.get(x).response.c_str();
}

SOUP_CEXPORT bool cbResult_isDelete(const cbResult* x)
{
	return heap.get(x).isDelete();
}

SOUP_CEXPORT int cbResult_getDeleteNum(const cbResult* x)
{
	return heap.get(x).getDeleteNum();
}

// Chatbot

SOUP_CEXPORT cbResult* Chatbot_process(const char* text)
{
	return heap.add(Chatbot::process(text));
}

// CidrSubnetInterface

SOUP_CEXPORT CidrSubnetInterface* CidrSubnetInterface_new(const char* range)
{
	return heap.add(CidrSubnetInterface::construct(range).release());
}

SOUP_CEXPORT bool CidrSubnetInterface_contains(CidrSubnetInterface* x, const char* ip_addr)
{
	IpAddr addr;
	SOUP_ASSERT(addr.fromString(ip_addr));
	return heap.get(x).contains(addr);
}

// DetachedScheduler

SOUP_CEXPORT DetachedScheduler* DetachedScheduler_new()
{
#if SOUP_WASM
	return nullptr;
#else
	return heap.add(new DetachedScheduler());
#endif
}

SOUP_CEXPORT bool DetachedScheduler_isActive(DetachedScheduler* sched)
{
#if SOUP_WASM
	return false;
#else
	return heap.get(sched).isActive();
#endif
}

// EstablishWebSocketConnectionTask

SOUP_CEXPORT void* EstablishWebSocketConnectionTask_new(const char* uri)
{
#if SOUP_WASM
	return nullptr;
#else
	return heap.add(new SharedPtr<EstablishWebSocketConnectionTask>(new EstablishWebSocketConnectionTask(Uri(uri))));
#endif
}

SOUP_CEXPORT void* EstablishWebSocketConnectionTask_getSocket(void* x)
{
#if SOUP_WASM
	return nullptr;
#else
	return heap.add(new SharedPtr<WebSocketConnection>(heap.get<SharedPtr<EstablishWebSocketConnectionTask>>(x)->sock));
#endif
}

// Hotp

SOUP_CEXPORT stdstring* Hotp_generateSecret(size_t bytes)
{
	return heap.add(Hotp::generateSecret(bytes));
}

// HttpRequest

SOUP_CEXPORT HttpRequest* HttpRequest_new(const char* uri)
{
#if SOUP_WASM
	return nullptr;
#else
	return heap.add(new HttpRequest(Uri(uri)));
#endif
}

SOUP_CEXPORT void HttpRequest_setPayload(HttpRequest* x, const char* data)
{
#if !SOUP_WASM
	heap.get(x).setPayload(data);
#endif
}

SOUP_CEXPORT const char* HttpRequest_getPath(HttpRequest* x)
{
#if !SOUP_WASM
	return heap.get(x).path.c_str();
#else
	return nullptr;
#endif
}

// HttpRequestTask

SOUP_CEXPORT void* HttpRequestTask_newFromRequest(const HttpRequest* hr)
{
#if SOUP_WASM
	return nullptr;
#else
	return heap.add(new SharedPtr<HttpRequestTask>(new HttpRequestTask(HttpRequest(*hr))));
#endif
}

SOUP_CEXPORT void* HttpRequestTask_newFromUrl(const char* url)
{
	return heap.add(new SharedPtr<HttpRequestTask>(new HttpRequestTask(Uri(url))));
}

SOUP_CEXPORT stdstring* HttpRequestTask_getResponseBody(void* hrt)
{
	return heap.add(new std::string(heap.get<SharedPtr<HttpRequestTask>>(hrt)->result->body));
}

SOUP_CEXPORT const char* HttpRequestTask_getResponseBodyCStr(void* hrt)
{
	return heap.get<SharedPtr<HttpRequestTask>>(hrt)->result->body.c_str();
}

// InquiryLang

SOUP_CEXPORT Mixed* InquiryLang_execute(const char* x)
{
	if (auto res = InquiryLang::execute(x))
	{
		return heap.add(res.release());
	}
	return nullptr;
}

SOUP_CEXPORT const char* InquiryLang_formatResultLine(const Mixed* x)
{
	returnString(InquiryLang::formatResultLine(heap.get(x)));
}

// MimeMessage

SOUP_CEXPORT void MimeMessage_addHeader(MimeMessage* x, const char* key, const char* value)
{
	MimeMessage& mm = heap.get(x);
	if (strcmp(key, "User-Agent") == 0)
	{
		if (auto e = mm.header_fields.find(key); e != mm.header_fields.end())
		{
			e->second = value;
			return;
		}
	}
	mm.header_fields.emplace(key, value);
}

// Mixed

SOUP_CEXPORT bool Mixed_isCanvas(const Mixed* x)
{
	return heap.get(x).isCanvas();
}

SOUP_CEXPORT Canvas* Mixed_getCanvas(const Mixed* x)
{
	return &heap.get(x).getCanvas();
}

// PromiseBase

SOUP_CEXPORT bool PromiseBase_isPending(void* x)
{
	return heap.get<SharedPtr<PromiseBase>>(x)->isPending();
}

// Promise_WebSocketMessage

SOUP_CEXPORT const char* Promise_WebSocketMessage_getData(void* x)
{
	return heap.get<SharedPtr<Promise<WebSocketMessage>>>(x)->getResult().data.c_str();
}

// QrCode

SOUP_CEXPORT QrCode* QrCode_newFromText(const char* x)
{
	return heap.add(QrCode::encodeText(x));
}

SOUP_CEXPORT Canvas* QrCode_toNewCanvas(const QrCode* x, unsigned int border, bool black_bg)
{
	return heap.add(heap.get(x).toCanvas(border, black_bg));
}

// RsaKeypair

SOUP_CEXPORT const Bigint* RsaKeypair_getN(const RsaKeypair* x)
{
	return &heap.get(x).n;
}

SOUP_CEXPORT const Bigint* RsaKeypair_getP(const RsaKeypair* x)
{
	return &heap.get(x).p;
}

SOUP_CEXPORT const Bigint* RsaKeypair_getQ(const RsaKeypair* x)
{
	return &heap.get(x).q;
}

// Scheduler

SOUP_CEXPORT Scheduler* Scheduler_new()
{
	return heap.add(new Scheduler());
}

SOUP_CEXPORT void Scheduler_setDontMakeReusableSockets(Scheduler* sched)
{
#if !SOUP_WASM
	heap.get(sched).dont_make_reusable_sockets = true;
#endif
}

SOUP_CEXPORT void Scheduler_add(Scheduler* sched, void* spWorker)
{
	heap.get(sched).addWorker(SharedPtr<Worker>(*reinterpret_cast<SharedPtr<Worker>*>(spWorker)));
}

SOUP_CEXPORT bool Scheduler_shouldKeepRunning(Scheduler* sched)
{
	return heap.get(sched).shouldKeepRunning();
}

SOUP_CEXPORT void Scheduler_tick(Scheduler* sched)
{
	heap.get(sched).tick();
}

// Server

SOUP_CEXPORT bool Server_bind(Scheduler* serv, int port, ServerService* srv)
{
#if !SOUP_WASM
	static_assert(sizeof(Scheduler) == sizeof(Server));
	return heap.get<Server>(serv).bind(static_cast<uint16_t>(port), srv);
#else
	return false;
#endif
}

// ServerWebService

#if !SOUP_WASM
class ServerWebServiceFfi : public ServerWebService
{
public:
	HttpRequest* pending_request = nullptr;
	Socket* pending_request_socket;
private:
	Notifyable notifyable;

public:
	ServerWebServiceFfi()
		: ServerWebService(&onRequest)
	{
	}

private:
	static void onRequest(Socket& s, HttpRequest&& hr, ServerWebService& _self)
	{
		ServerWebServiceFfi& self = static_cast<ServerWebServiceFfi&>(_self);
		self.pending_request = &hr;
		self.pending_request_socket = &s;
		self.notifyable.wait();
	}

public:
	void setRequestHandled()
	{
		pending_request = nullptr;
		notifyable.wakeAll();
	}
};
#endif

SOUP_CEXPORT ServerWebService* ServerWebService_new()
{
#if !SOUP_WASM
	return heap.add(new ServerWebServiceFfi());
#else
	return nullptr;
#endif
}

SOUP_CEXPORT bool ServerWebService_hasPendingRequest(ServerWebService* x)
{
#if !SOUP_WASM
	return heap.get<ServerWebServiceFfi>(x).pending_request != nullptr;
#else
	return false;
#endif
}

SOUP_CEXPORT const char* ServerWebService_getPendingRequestPath(ServerWebService* x)
{
#if !SOUP_WASM
	return heap.get<ServerWebServiceFfi>(x).pending_request->path.c_str();
#else
	return nullptr;
#endif
}

SOUP_CEXPORT void ServerWebService_ignoreRequest(ServerWebService* x)
{
#if !SOUP_WASM
	heap.get<ServerWebServiceFfi>(x).setRequestHandled();
#endif
}

SOUP_CEXPORT void ServerWebService_replyWithHtml(ServerWebService* x, const char* html)
{
#if !SOUP_WASM
	auto& srv = heap.get<ServerWebServiceFfi>(x);
	ServerWebService::sendHtml(*srv.pending_request_socket, html);
	srv.setRequestHandled();
#endif
}

SOUP_CEXPORT void ServerWebService_replyWith404(ServerWebService* x)
{
#if !SOUP_WASM
	auto& srv = heap.get<ServerWebServiceFfi>(x);
	ServerWebService::send404(*srv.pending_request_socket);
	srv.setRequestHandled();
#endif
}

// Totp

SOUP_CEXPORT Totp* Totp_new(const stdstring* secret)
{
	return heap.add(new Totp(heap.get(secret)));
}

SOUP_CEXPORT const char* Totp_getQrCodeUri(const Totp* x, const char* label, const char* issuer)
{
	returnString(heap.get(x).getQrCodeUri(label, issuer));
}

SOUP_CEXPORT int Totp_getValue(const Totp* x)
{
	return heap.get(x).getValue();
}

// WebSocketConnection

SOUP_CEXPORT void WebSocketConnection_wsSend(void* con, const char* text)
{
#if !SOUP_WASM
	return heap.get<SharedPtr<WebSocketConnection>>(con)->wsSend(text);
#endif
}

SOUP_CEXPORT void* WebSocketConnection_wsRecv(void* con)
{
#if SOUP_WASM
	return nullptr;
#else
	return heap.add(new SharedPtr(heap.get<SharedPtr<WebSocketConnection>>(con)->wsRecv()));
#endif
}

// Worker

SOUP_CEXPORT bool Worker_isWorkDone(void* x)
{
	return heap.get<SharedPtr<Worker>>(x)->isWorkDone();
}

// YubikeyValidator

SOUP_CEXPORT YubikeyValidator* YubikeyValidator_new(const char* id, const char* secret)
{
#if SOUP_WASM
	return nullptr;
#else
	return heap.add(new YubikeyValidator(id, secret));
#endif
}

SOUP_CEXPORT const char* YubikeyValidator_validate(const YubikeyValidator* x, const char* otp)
{
#if SOUP_WASM
	return "";
#else
	returnString(heap.get(x).validate(otp).device_id);
#endif
}

// std::string

SOUP_CEXPORT stdstring* stdstring_new(const char* data, size_t len)
{
	return heap.add(new std::string(data, len));
}

SOUP_CEXPORT const char* stdstring_c_str(const stdstring* x)
{
	return heap.get(x).c_str();
}

// std::exception

SOUP_CEXPORT const char* exception_what(const stdexception* x)
{
	returnString(x->what());
}

#endif
