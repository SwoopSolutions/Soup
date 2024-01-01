#include "dnsResolver.hpp"

#if !SOUP_WASM

#include "DetachedScheduler.hpp"
#include "log.hpp"
#include "WeakRef.hpp"

namespace soup
{
	struct dnsAsyncScheduler : public DetachedScheduler
	{
		dnsAsyncScheduler()
			: DetachedScheduler()
		{
#if SOUP_EXCEPTIONS
			on_exception = &onException;
#endif
		}

#if SOUP_EXCEPTIONS
		static void onException(Worker&, const std::exception& e, Scheduler&)
		{
			std::string msg = "Exception during DNS lookup task: ";
			msg.append(e.what());
			logWriteLine(std::move(msg));
		}
#endif
	};

	static dnsAsyncScheduler dns_async_sched;

	struct dnsAsyncExecTask : public Task
	{
		WeakRef<const dnsResolver> resolv;
		dnsType qtype;
		std::string name;

		std::vector<UniquePtr<dnsRecord>> result;

		dnsAsyncExecTask(const dnsResolver& resolv, dnsType qtype, const std::string& name)
			: resolv(&resolv), qtype(qtype), name(name)
		{
		}

		void onTick() final
		{
			auto pResolv = resolv.getPointer();
			SOUP_ASSERT(pResolv); // Resolver has been deleted in the time the task was started. Was it stack-allocated?
			result = pResolv->lookup(qtype, name);
			setWorkDone();
		}
	};

	struct dnsAsyncWatcherTask : public dnsLookupTask
	{
		SharedPtr<dnsAsyncExecTask> watched_task;

		dnsAsyncWatcherTask(SharedPtr<dnsAsyncExecTask>&& watched_task)
			: watched_task(std::move(watched_task))
		{
		}

		void onTick() final
		{
			if (watched_task->isWorkDone())
			{
				result = std::move(watched_task->result);
				watched_task.reset();
				setWorkDone();
			}
		}
	};

	std::vector<IpAddr> dnsResolver::lookupIPv4(const std::string& name) const
	{
		return simplifyIPv4LookupResults(lookup(DNS_A, name));
	}

	std::vector<IpAddr> dnsResolver::lookupIPv6(const std::string& name) const
	{
		return simplifyIPv6LookupResults(lookup(DNS_AAAA, name));
	}

	std::vector<UniquePtr<dnsRecord>> dnsResolver::lookup(dnsType qtype, const std::string& name) const
	{
		auto task = makeLookupTask(qtype, name);
		task->run();
		return std::move(task->result);
	}

	UniquePtr<dnsLookupTask> dnsResolver::makeLookupTask(dnsType qtype, const std::string& name) const
	{
		return soup::make_unique<dnsAsyncWatcherTask>(dns_async_sched.add<dnsAsyncExecTask>(*this, qtype, name));
	}

	std::vector<IpAddr> dnsResolver::simplifyIPv4LookupResults(const std::vector<UniquePtr<dnsRecord>>& vec)
	{
		std::vector<IpAddr> res{};
		for (const auto& r : vec)
		{
			if (r->type == DNS_A)
			{
				res.emplace_back(static_cast<dnsARecord*>(r.get())->data);
			}
		}
		return res;
	}

	std::vector<IpAddr> dnsResolver::simplifyIPv6LookupResults(const std::vector<UniquePtr<dnsRecord>>& vec)
	{
		std::vector<IpAddr> res{};
		for (const auto& r : vec)
		{
			if (r->type == DNS_AAAA)
			{
				res.emplace_back(static_cast<dnsAaaaRecord*>(r.get())->data);
			}
		}
		return res;
	}
}

#endif
