#include "ESpeakHelper.h"
#include <espeak-ng/speak_lib.h>

namespace eg::bc
{
	int espeak_callback_(short* samples, int num_samples, espeak_EVENT*)
	{
		if (samples not_eq nullptr and num_samples > 0)
		{
			EspeakNg::pcm_samples.insert
			(
				EspeakNg::pcm_samples.end(),
				samples,
				samples + num_samples
			);
		}

		return 0;
	}
}