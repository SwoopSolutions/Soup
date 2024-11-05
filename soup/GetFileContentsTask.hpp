#pragma once

#include "Task.hpp"

#include "HttpRequestTask.hpp"
#include "Optional.hpp"
#include "string.hpp"

NAMESPACE_SOUP
{
	struct GetFileContentsTask : public PromiseTask<std::string>
	{
		Optional<HttpRequestTask> http;

		GetFileContentsTask(const Uri& uri)
		{
			if (uri.isFile())
			{
				result = string::fromFile(uri.getFilePath());
				setWorkDone();
			}
			else if (uri.isHttp())
			{
				http.emplace(uri);
			}
			else
			{
				throw 0;
			}
		}

		void onTick() final
		{
			if (http->tickUntilDone())
			{
				if (http->result)
				{
					result = std::move(http->result->body);
				}
				setWorkDone();
			}
		}
	};
}
