#pragma once

#include "NoCopyMove.h"
#include <espeak-ng/speak_lib.h>
#include <miniaudio.h>
#include <vector>
#include <queue>
#include <cstdint>
#include <filesystem>
#include <fstream>
#include <mutex>
#include <future>
#include <thread>

namespace eg::bc
{
	extern "C" int espeak_callback_(short* samples, int num_samples, espeak_EVENT*);

	const uint32_t k_ma_sample_rate = 16'000;
	const uint32_t k_ma_channels = 1;

	class EspeakNg :
		eg::sys::NoCopyMove
	{
	private:
		EspeakNg() = default;

	public:

		static EspeakNg& instance()
		{
			static EspeakNg ng;
			return ng;
		}

		[[nodiscard]] bool init()
		{
			static bool is_init = false;

			if (is_init)
			{
				return true;
			}

			sample_rate = espeak_Initialize(AUDIO_OUTPUT_RETRIEVAL, 0, "assets/espeak-ng-data", 0);
			if (sample_rate <= 0)
			{
				return false;
			}

			espeak_SetSynthCallback(espeak_callback_);

			if (espeak_SetVoiceByName("en") not_eq EE_OK)
			{
				return false;
			}

			is_init = true;

			return true;
		}

		void shutdown()
		{
		}

		static inline std::vector<int16_t> pcm_samples;
		static inline int sample_rate = 0;

		bool is_shutdown_{ false };

		std::vector<int16_t> generate_pcm_from_text(const char* text)
		{
			static size_t ctr = 1;

			pcm_samples.clear();

			if (espeak_Synth(
				text,
				std::strlen(text) + 1,
				0,
				POS_CHARACTER,
				0,
				espeakCHARS_AUTO,
				nullptr,
				nullptr
			) not_eq EE_OK)
			{
				return {};
			}

			espeak_Synchronize();

			return pcm_samples;
		}
	};
}