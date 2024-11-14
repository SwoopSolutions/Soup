#include "audPlayback.hpp"
#if (SOUP_WINDOWS && !SOUP_CROSS_COMPILE) || SOUP_LINUX

#include <atomic>
#include <cstring> // memset

#include "alloc.hpp"
#include "os.hpp"

NAMESPACE_SOUP
{
#if SOUP_LINUX
	using snd_pcm_hw_params_t = void;
	using snd_async_handler_t = void;
	using snd_pcm_stream_t = int;
	using snd_pcm_access_t = int;
	using snd_pcm_format_t = int;
	using snd_async_callback_t = void(*)(snd_async_handler_t*);

	using snd_pcm_open_t = int(*)(snd_pcm_t**, const char*, snd_pcm_stream_t, int);
	using snd_pcm_hw_params_malloc_t = int(*)(snd_pcm_hw_params_t**);
	using snd_pcm_hw_params_any_t = int(*)(snd_pcm_t*, snd_pcm_hw_params_t*);
	using snd_pcm_hw_params_set_access_t = int(*)(snd_pcm_t*, snd_pcm_hw_params_t*, snd_pcm_access_t);
	using snd_async_add_pcm_handler_t = int(*)(snd_async_handler_t**, snd_pcm_t*, snd_async_callback_t, void*);
	using snd_pcm_hw_params_set_format_t = int(*)(snd_pcm_t*, snd_pcm_hw_params_t*, snd_pcm_format_t);
	using snd_pcm_hw_params_set_rate_near_t = int(*)(snd_pcm_t*, snd_pcm_hw_params_t*, unsigned int*, int*);
	using snd_pcm_hw_params_set_channels_t = int(*)(snd_pcm_t*, snd_pcm_hw_params_t*, unsigned int);
	using snd_pcm_hw_params_func_t = int(*)(snd_pcm_t*, snd_pcm_hw_params_t*);
	using snd_pcm_hw_params_free_t = void(*)(snd_pcm_hw_params_t*);

	using snd_pcm_drop_t = int(*)(snd_pcm_t*);
	using snd_pcm_close_t = int(*)(snd_pcm_t*);
#endif

	// Kudos to:
	// - https://github.com/OneLoneCoder/synth
	// - https://gist.github.com/seungin/4779216eada24a5077ca1c5e857239ce
	// - https://github.com/Barracuda72/synth/blob/master/LinuxSoundSynthesizer/NoiseMaker.h

	constexpr int NUM_BLOCKS = 8;

#if SOUP_WINDOWS
	constexpr int HEAP_SIZE = (NUM_BLOCKS * (sizeof(WAVEHDR) + AUD_BLOCK_BYTES));
#else
	constexpr int HEAP_SIZE = (NUM_BLOCKS * AUD_BLOCK_BYTES);
#endif

	audPlayback::audPlayback()
		: heap(soup::malloc(HEAP_SIZE)), thrd(&threadFuncStatic, this)
	{
		memset(heap, 0, HEAP_SIZE);

#if SOUP_WINDOWS
		for (auto i = 0; i != NUM_BLOCKS; ++i)
		{
			auto hdr = heapGetHeader(i);
			hdr->dwBufferLength = AUD_BLOCK_BYTES;
			hdr->lpData = reinterpret_cast<LPSTR>(heapGetBuffer(i));
		}
#endif
	}

	audPlayback::~audPlayback()
	{
		stop();
		thrd.awaitCompletion();
#if SOUP_WINDOWS
		if (hWaveOut != nullptr)
		{
			waveOutReset(hWaveOut);
			waveOutClose(hWaveOut);
		}
#else
		if (hwDevice != nullptr)
		{
			auto snd_pcm_drop = (snd_pcm_drop_t)alsa.getAddressMandatory("snd_pcm_drop");
			auto snd_pcm_close = (snd_pcm_close_t)alsa.getAddressMandatory("snd_pcm_close");

			snd_pcm_drop(hwDevice);
			snd_pcm_close(hwDevice);
		}
#endif
		soup::free(heap);
	}

#if SOUP_WINDOWS
	static void CALLBACK waveCallbackStatic(HWAVEOUT hwo, UINT uMsg, DWORD_PTR dwInstance, DWORD_PTR dwParam1, DWORD_PTR dwParam2)
	{
		reinterpret_cast<audPlayback*>(dwInstance)->handleMessage(uMsg);
	}
#else
	[[nodiscard]] static void* snd_async_handler_get_callback_private(snd_async_handler_t* ahandler)
	{
#if SOUP_BITS == 32
		return *reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(ahandler) + 0x10);
#else
		return *reinterpret_cast<void**>(reinterpret_cast<uintptr_t>(ahandler) + 0x18);
#endif
	}

	static void waveCallbackStatic(snd_async_handler_t* ahandler)
	{
		reinterpret_cast<audPlayback*>(snd_async_handler_get_callback_private(ahandler))->maintainPcm();
	}
#endif

	void audPlayback::open(const audDevice& dev, int channels)
	{
		return open(dev, channels, &fillBlockSilenceSrc);
	}
	
	void audPlayback::open(const audDevice& dev, audFillBlock src, void* user_data)
	{
		return open(dev, 1, src, user_data);
	}

	void audPlayback::open(const audDevice& dev, int channels, audFillBlock src, void* user_data)
	{
#if SOUP_WINDOWS
		SOUP_ASSERT(hWaveOut == nullptr);
#else
		SOUP_ASSERT(hwDevice == nullptr);
#endif

		this->dev = dev;
		this->channels = channels;
		this->src = src;
		this->user_data = user_data;

#if SOUP_WINDOWS
		WAVEFORMATEX wfx;
		wfx.wFormatTag = WAVE_FORMAT_PCM;
		wfx.nSamplesPerSec = AUD_SAMPLE_RATE;
		wfx.wBitsPerSample = sizeof(audSample) * 8;
		wfx.nChannels = channels;
		wfx.nBlockAlign = static_cast<WORD>(sizeof(audSample) * channels);
		wfx.nAvgBytesPerSec = wfx.nSamplesPerSec * wfx.nBlockAlign;
		wfx.cbSize = 0;
		waveOutOpen(&hWaveOut, dev.i, &wfx, reinterpret_cast<DWORD_PTR>(&waveCallbackStatic), reinterpret_cast<DWORD_PTR>(this), CALLBACK_FUNCTION);
#else
		SOUP_ASSERT(alsa.load("libasound.so.2"));

		snd_pcm_state = (snd_pcm_state_func_t)alsa.getAddressMandatory("snd_pcm_state");
		snd_pcm_prepare = (snd_pcm_prepare_t)alsa.getAddressMandatory("snd_pcm_prepare");
		snd_pcm_writei = (snd_pcm_writei_t)alsa.getAddressMandatory("snd_pcm_writei");

		auto snd_pcm_open = (snd_pcm_open_t)alsa.getAddressMandatory("snd_pcm_open");
		auto snd_pcm_hw_params_malloc = (snd_pcm_hw_params_malloc_t)alsa.getAddressMandatory("snd_pcm_hw_params_malloc");
		auto snd_pcm_hw_params_any = (snd_pcm_hw_params_any_t)alsa.getAddressMandatory("snd_pcm_hw_params_any");
		auto snd_pcm_hw_params_set_access = (snd_pcm_hw_params_set_access_t)alsa.getAddressMandatory("snd_pcm_hw_params_set_access");
		auto snd_async_add_pcm_handler = (snd_async_add_pcm_handler_t)alsa.getAddressMandatory("snd_async_add_pcm_handler");
		auto snd_pcm_hw_params_set_format = (snd_pcm_hw_params_set_format_t)alsa.getAddressMandatory("snd_pcm_hw_params_set_format");
		auto snd_pcm_hw_params_set_rate_near = (snd_pcm_hw_params_set_rate_near_t)alsa.getAddressMandatory("snd_pcm_hw_params_set_rate_near");
		auto snd_pcm_hw_params_set_channels = (snd_pcm_hw_params_set_channels_t)alsa.getAddressMandatory("snd_pcm_hw_params_set_channels");
		auto snd_pcm_hw_params = (snd_pcm_hw_params_func_t)alsa.getAddressMandatory("snd_pcm_hw_params");
		auto snd_pcm_hw_params_free = (snd_pcm_hw_params_free_t)alsa.getAddressMandatory("snd_pcm_hw_params_free");

		// BUG: snd_pcm_hw_params_free is not called in exception case
		snd_pcm_hw_params_t* hw_params;
		// Using "sysdefault" instead of "default" because it may select SND_PCM_TYPE_IOPLUG instead of SND_PCM_TYPE_PLUG for non-root users, but that does not support async.
		snd_pcm_open(&hwDevice, "sysdefault", /*SND_PCM_STREAM_PLAYBACK*/ 0, 0);
		//std::cout << "pcm type: " << *reinterpret_cast<int*>(reinterpret_cast<uintptr_t>(hwDevice) + 0x10) << "\n";
		snd_pcm_hw_params_malloc(&hw_params);
		snd_pcm_hw_params_any(hwDevice, hw_params);
		SOUP_ASSERT(snd_pcm_hw_params_set_access(hwDevice, hw_params, /*SND_PCM_ACCESS_RW_INTERLEAVED*/ 3) >= 0);
		snd_async_handler_t* handler;
		SOUP_ASSERT(snd_async_add_pcm_handler(&handler, hwDevice, waveCallbackStatic, this) >= 0);
		snd_pcm_hw_params_set_format(hwDevice, hw_params, /*SND_PCM_FORMAT_S16_LE*/ 2); static_assert(std::is_same_v<int16_t, audSample>);
		unsigned int sample_rate = AUD_SAMPLE_RATE;
		snd_pcm_hw_params_set_rate_near(hwDevice, hw_params, &sample_rate, 0);
		SOUP_ASSERT(sample_rate == AUD_SAMPLE_RATE);
		snd_pcm_hw_params_set_channels(hwDevice, hw_params, channels);
		snd_pcm_hw_params(hwDevice, hw_params);
		snd_pcm_hw_params_free(hw_params);
		SOUP_ASSERT(snd_pcm_prepare(hwDevice) >= 0);
		free_blocks = NUM_BLOCKS;
#endif
	}

	bool audPlayback::isPlaying() const noexcept
	{
		return thrd.isRunning()
			|| free_blocks != NUM_BLOCKS
			;
	}

	void audPlayback::awaitCompletion() noexcept
	{
		thrd.awaitCompletion();
		while (free_blocks != NUM_BLOCKS)
		{
			os::sleep(10);
		}
	}

	void audPlayback::stop() noexcept
	{
		stopping = true;
	}

	void audPlayback::fillBlockSilence(audSample block[AUD_BLOCK_SAMPLES])
	{
		memset(block, 0, AUD_BLOCK_BYTES);
	}

	void audPlayback::fillBlockSilenceSrc(audPlayback&, audSample block[AUD_BLOCK_SAMPLES])
	{
		fillBlockSilence(block);
	}

	void audPlayback::fillBlockImpl(audSample block[AUD_BLOCK_SAMPLES], audGetAmplitude src)
	{
		for (int i = 0; i != AUD_BLOCK_SAMPLES; i += channels)
		{
			for (int c = 0; c != channels; ++c)
			{
				block[i + c] = static_cast<audSample>(src(*this) * AUD_MAX_SAMPLE);
			}
			time += AUD_TIME_STEP;
		}
	}

#if SOUP_WINDOWS
	void audPlayback::handleMessage(UINT msg)
	{
		if (msg == WOM_OPEN)
		{
			free_blocks = NUM_BLOCKS;
		}
		else if (msg == WOM_DONE)
		{
			++free_blocks;
		}
	}
#else
	void audPlayback::maintainPcm()
	{
		if (snd_pcm_state(hwDevice) == /*SND_PCM_STATE_XRUN*/ 4)
        {
        	SOUP_ASSERT(snd_pcm_prepare(hwDevice) >= 0);
        }

		++free_blocks;
	}
#endif

#if SOUP_WINDOWS
	WAVEHDR* audPlayback::heapGetHeader(int i) const noexcept
	{
		return reinterpret_cast<WAVEHDR*>(reinterpret_cast<uintptr_t>(heap) + (i * sizeof(WAVEHDR)));
	}
#endif

	void* audPlayback::heapGetBuffer(int i) const noexcept
	{
#if SOUP_WINDOWS
		return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(heap) + (NUM_BLOCKS * sizeof(WAVEHDR)) + (i * AUD_BLOCK_BYTES));
#else
		return reinterpret_cast<void*>(reinterpret_cast<uintptr_t>(heap) + (i * AUD_BLOCK_BYTES));
#endif
	}

	void audPlayback::threadFuncStatic(Capture&& cap)
	{
		cap.get<audPlayback*>()->threadFunc();
	}

	void audPlayback::threadFunc()
	{
		do
		{
			while (free_blocks == 0)
			{
				os::sleep(10);
			}

#if SOUP_WINDOWS
			auto hdr = heapGetHeader(current_block);
			if (hdr->dwFlags & WHDR_PREPARED)
			{
				waveOutUnprepareHeader(hWaveOut, hdr, sizeof(WAVEHDR));
			}
#endif

			auto buf = heapGetBuffer(current_block);
			src(*this, reinterpret_cast<audSample*>(buf));

#if SOUP_WINDOWS
			waveOutPrepareHeader(hWaveOut, hdr, sizeof(WAVEHDR));
			waveOutWrite(hWaveOut, hdr, sizeof(WAVEHDR));
#else
			snd_pcm_writei(hwDevice, buf, AUD_BLOCK_SAMPLES);
#endif

			--free_blocks;
			current_block += 1;
			current_block %= NUM_BLOCKS;
		} while (!stopping);
	}
}

#endif
