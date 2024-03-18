#pragma once

#include "base.hpp"

#include <cstdint>
#include <cmath>

namespace soup
{
	// Soup represents notes just like MIDI does.
	// See Appendix 1.3 of https://www.music.mcgill.ca/~ich/classes/mumt306/StandardMIDIfileformat.html.

	using audNote_t = uint8_t;

	enum audNote : audNote_t
	{
		AUDNOTE_C1 = 24,
		AUDNOTE_CSHARP1,
		AUDNOTE_D1,
		AUDNOTE_DSHARP1,
		AUDNOTE_E1,
		AUDNOTE_F1,
		AUDNOTE_FSHARP1,
		AUDNOTE_G1,
		AUDNOTE_GSHARP1,
		AUDNOTE_A1,
		AUDNOTE_ASHARP1,
		AUDNOTE_B1,

		AUDNOTE_C2,
		AUDNOTE_CSHARP2,
		AUDNOTE_D2,
		AUDNOTE_DSHARP2,
		AUDNOTE_E2,
		AUDNOTE_F2,
		AUDNOTE_FSHARP2,
		AUDNOTE_G2,
		AUDNOTE_GSHARP2,
		AUDNOTE_A2,
		AUDNOTE_ASHARP2,
		AUDNOTE_B2,

		AUDNOTE_C3,
		AUDNOTE_CSHARP3,
		AUDNOTE_D3,
		AUDNOTE_DSHARP3,
		AUDNOTE_E3,
		AUDNOTE_F3,
		AUDNOTE_FSHARP3,
		AUDNOTE_G3,
		AUDNOTE_GSHARP3,
		AUDNOTE_A3,
		AUDNOTE_ASHARP3,
		AUDNOTE_B3,

		AUDNOTE_C4,
		AUDNOTE_CSHARP4,
		AUDNOTE_D4,
		AUDNOTE_DSHARP4,
		AUDNOTE_E4,
		AUDNOTE_F4,
		AUDNOTE_FSHARP4,
		AUDNOTE_G4,
		AUDNOTE_GSHARP4,
		AUDNOTE_A4,
		AUDNOTE_ASHARP4,
		AUDNOTE_B4,

		AUDNOTE_C5,
		AUDNOTE_CSHARP5,
		AUDNOTE_D5,
		AUDNOTE_DSHARP5,
		AUDNOTE_E5,
		AUDNOTE_F5,
		AUDNOTE_FSHARP5,
		AUDNOTE_G5,
		AUDNOTE_GSHARP5,
		AUDNOTE_A5,
		AUDNOTE_ASHARP5,
		AUDNOTE_B5,

		AUDNOTE_C6,
		AUDNOTE_CSHARP6,
		AUDNOTE_D6,
		AUDNOTE_DSHARP6,
		AUDNOTE_E6,
		AUDNOTE_F6,
		AUDNOTE_FSHARP6,
		AUDNOTE_G6,
		AUDNOTE_GSHARP6,
		AUDNOTE_A6,
		AUDNOTE_ASHARP6,
		AUDNOTE_B6,

		AUDNOTE_C7,
		AUDNOTE_CSHARP7,
		AUDNOTE_D7,
		AUDNOTE_DSHARP7,
		AUDNOTE_E7,
		AUDNOTE_F7,
		AUDNOTE_FSHARP7,
		AUDNOTE_G7,
		AUDNOTE_GSHARP7,
		AUDNOTE_A7,
		AUDNOTE_ASHARP7,
		AUDNOTE_B7,

		AUDNOTE_C8,
		AUDNOTE_CSHARP8,
		AUDNOTE_D8,
		AUDNOTE_DSHARP8,
		AUDNOTE_E8,
		AUDNOTE_F8,
		AUDNOTE_FSHARP8,
		AUDNOTE_G8,
		AUDNOTE_GSHARP8,
		AUDNOTE_A8,
		AUDNOTE_ASHARP8,
		AUDNOTE_B8,

		AUDNOTE_SIZE = 128
	};
	static_assert(AUDNOTE_A4 == 69);

	[[nodiscard]] inline float audNoteToHz(audNote_t note)
	{
		return 440.0f * powf(2.0f, (static_cast<float>(note) - static_cast<float>(AUDNOTE_A4)) / 12.0f);
	}
}
