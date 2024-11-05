#pragma once

#include "base.hpp"
#if !SOUP_WASM

#include "kbRgb.hpp"

#include "DetachedScheduler.hpp"
#include "HttpRequestTask.hpp"
#include "JsonObject.hpp"
#include "Optional.hpp"

NAMESPACE_SOUP
{
	class kbRgbRazerChroma : public kbRgb
	{
	protected:
		struct MaintainTask : public Task
		{
			const std::string base;
			Optional<HttpRequestTask> hrt;
			SharedPtr<JsonObject> payload;
			bool deinit_requested = false;

			MaintainTask(std::string&& base)
				: base(std::move(base))
			{
			}

			void onTick() final;
		};

		DetachedScheduler sched;
		SharedPtr<MaintainTask> task;
		uint32_t colours[22 * 6]{};

	public:
		kbRgbRazerChroma()
			: kbRgb("Razer Chroma")
		{
		}

		[[nodiscard]] static bool isAvailable();

	protected:
		void updateMaintainTask();
		void ensureMaintainTask();
		[[nodiscard]] MaintainTask& getMaintainTask();
		void commitColours();

	public:
		[[nodiscard]] bool controlsDevice(const hwHid& hid) const noexcept final;

		void deinit() final;

		void setKey(Key key, Rgb colour) final;
		void setKeys(const Rgb(&colours)[NUM_KEYS]) final;
		void setAllKeys(Rgb colour) final;

		[[nodiscard]] uint8_t getNumColumns() const noexcept final;
		[[nodiscard]] Key mapPosToKey(uint8_t row, uint8_t column) const noexcept final;
	};
}
#endif
