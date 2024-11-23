#include "MidiComposition.hpp"

#include "Reader.hpp"
#include "StringReader.hpp"

NAMESPACE_SOUP
{
	struct MidiTrack
	{
		uint32_t ticks_to_wait = 0;
		StringReader sr;

		MidiTrack() = default;
	};

	// https://www.music.mcgill.ca/~ich/classes/mumt306/StandardMIDIfileformat.html
	MidiComposition::MidiComposition(Reader& r)
	{
		uint32_t chunk_name;
		r.u32be(chunk_name);
		SOUP_ASSERT(chunk_name == 'MThd', "Bad MIDI data");
		uint32_t length;
		r.u32be(length);
		SOUP_ASSERT(length >= 6, "Bad MIDI data");
		uint16_t format, ntrks, division;
		r.u16be(format);
		r.u16be(ntrks);
		r.u16be(division);
		r.skip(length - 6);
		const float ticks_per_quarter_note = static_cast<float>(division);

		std::vector<MidiTrack> tracks{};
		while (r.hasMore())
		{
			r.u32be(chunk_name);
			SOUP_ASSERT(chunk_name == 'MTrk', "Bad MIDI data");
			MidiTrack& track = tracks.emplace_back();
			r.str_lp<u32be_t>(track.sr.data);
		}
		SOUP_ASSERT(tracks.size() == ntrks, "Bad MIDI data");

		float bpm = 120.0f;
		double t = 0.0;
		while (true)
		{
			uint32_t ticks_to_wait = -1;
			for (auto& track : tracks)
			{
				if (track.ticks_to_wait != 0)
				{
					continue;
				}

				auto& r = track.sr;
				SOUP_ASSERT(r.hasMore());

				r.om<uint32_t>(track.ticks_to_wait);
				if (ticks_to_wait > track.ticks_to_wait)
				{
					ticks_to_wait = track.ticks_to_wait;
				}

				uint8_t event_type;
				r.u8(event_type);
				if (event_type == 0xFF)
				{
					r.u8(event_type);
					if (event_type == 0x51) // Set Tempo
					{
						r.skip(1);
						uint32_t microseconds_per_beat;
						r.u24be(microseconds_per_beat);
						float seconds_per_beat = static_cast<float>(microseconds_per_beat) / 1'000'000.0f;
						bpm = (60.0f / seconds_per_beat);
						//std::cout << "UPDATED BPM: " << bpm << " (" << microseconds_per_beat << " microseconds per beat)\n";
					}
					else if (event_type == 0x2f)
					{
						//std::cout << "END OF TRACK\n";
						r.skip(1);
						track.ticks_to_wait = -1;
					}
					else
					{
						uint32_t len;
						r.om<uint32_t>(len);
						r.skip(len);
						//std::cout << "Ignoring meta event: " << (int)event_type << "\n";
					}
				}
				else if ((event_type >> 4) == 0x8) // Note Off
				{
					uint8_t key, vel;
					r.u8(key);
					r.u8(vel);
					//std::cout << "Key up: " << (int)(event_type & 0xF) << ", " << (int)key << ", " << (int)vel << "\n";

					events.emplace_back(t, MIDI_NOTE_OFF, key);
				}
				else if ((event_type >> 4) == 0x9) // Note On
				{
					uint8_t key, vel;
					r.u8(key);
					r.u8(vel);
					//std::cout << "Key down: " << (int)(event_type & 0xF) << ", " << (int)key << ", " << (int)vel << "\n";

					events.emplace_back(t, MIDI_NOTE_ON, key);

				}
				else if ((event_type >> 4) == 0xB) // Control Change
				{
					r.skip(2);
				}
				else if ((event_type >> 4) == 0xC) // Program Change
				{
					r.skip(1);
				}
				else
				{
					//std::cout << "Unknown event type " << (int)event_type << " at offset " << (r.getPosition() - 1) << "\n";
					track.ticks_to_wait = -1;
				}
			}

			if (ticks_to_wait == -1)
			{
				//std::cout << "END OF COMPOSITION\n";
				break;
			}

			float quarter_notes_to_skip = (ticks_to_wait / ticks_per_quarter_note);
			float seconds_to_skip = quarter_notes_to_skip * (60.0f / bpm);
			//std::cout << "ticks_to_wait = " << ticks_to_wait << " (" << quarter_notes_to_skip << " QNs/" << seconds_to_skip << " seconds)\n";
			t += seconds_to_skip;

			for (auto& track : tracks)
			{
				track.ticks_to_wait -= ticks_to_wait;
			}
		}
	}
}
