#pragma once

#include "Capture.hpp"
#include "SelfDeletingThread.hpp"

namespace soup
{
	class PromiseBase
	{
	protected:
		Capture res{};

	public:
		PromiseBase() = default;

	protected:
		PromiseBase(Capture&& res)
			: res(std::move(res))
		{
		}

	public:
		[[nodiscard]] bool isPending() const noexcept
		{
			return res.empty();
		}

		[[nodiscard]] bool isFulfilled() const noexcept
		{
			return !isPending();
		}

		void awaitCompletion();

	protected:
		void fulfil(Capture&& res) noexcept
		{
			this->res = std::move(res);
		}
	};

	template <typename T>
	class Promise : public PromiseBase
	{
	public:
		Promise() = default;

		// Creates a fulfilled promise
		Promise(T&& res)
			: PromiseBase(std::move(res))
		{
		}

		// If this is used, the Promise MUST NOT be stack-allocated.
		Promise(T(*f)(Capture&&), Capture&& cap = {})
			: Promise()
		{
			fulfilOffThread(f, std::move(cap));
		}

		void fulfil(T&& res)
		{
			PromiseBase::fulfil(std::move(res));
		}

		[[nodiscard]] T& getResult() const noexcept
		{
			return res.get<T>();
		}

		// If this is used, the Promise MUST NOT be stack-allocated.
		void fulfilOffThread(T(*f)(Capture&&), Capture&& cap = {})
		{
			new SelfDeletingThread([](Capture&& _cap)
			{
				auto& cap = _cap.get<CaptureFulfillOffThread>();
				cap.promise.fulfil(cap.f(std::move(cap.cap)));
			}, CaptureFulfillOffThread{ *this, f, std::move(cap) });
		}

	protected:
		struct CaptureFulfillOffThread
		{
			Promise& promise;
			T(*f)(Capture&&);
			Capture cap;
		};
	};
}
