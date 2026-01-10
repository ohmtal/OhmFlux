//-----------------------------------------------------------------------------
// Copyright (c) 1993 T.Huehn (XXTH)
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// 2026-10-09
// * actual_ins => getInstrumentName(int lChannel) / setInstrumentName
// * SongData also got a init function and getter / setter for the name
//   Pascal string is not 0 terminated! first byte is the length
//   after so many years i forgot this ;)
//-----------------------------------------------------------------------------
#include "OplController.h"
#include <mutex>

#ifdef FLUX_ENGINE
#include <audio/fluxAudio.h>
#endif


//------------------------------------------------------------------------------
OplController::OplController(){
    // mChip = new ymfm::ym3812(mInterface); //OPL2
    mChip = new ymfm::ymf262(mInterface);//OPL3
    reset();
}

OplController::~OplController() {

    if (mStream) {
        SDL_FlushAudioStream(mStream);
        SDL_SetAudioStreamGetCallback(mStream, NULL, NULL);
        SDL_DestroyAudioStream(mStream);
        mStream = nullptr;
    }

    if (mChip) {
        delete mChip;
        mChip = nullptr;
    }

}
//------------------------------------------------------------------------------
void OplController::audio_callback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount)
{

    if (!userdata)
        return;

    auto* controller = static_cast<OplController*>(userdata);
    if (controller && additional_amount > 0) {

        std::lock_guard<std::recursive_mutex> lock(controller->mDataMutex);

        int frames = additional_amount / 4; // S16 * 2 channels = 4 bytes

        // Generate data directly into a temporary vector or stack buffer
        std::vector<int16_t> temp(additional_amount / sizeof(int16_t));
        controller->fillBuffer(temp.data(), frames);

        SDL_PutAudioStreamData(stream, temp.data(), additional_amount);
    }
}
//------------------------------------------------------------------------------
bool OplController::initController()
{
    SDL_AudioSpec spec;
    spec.format = SDL_AUDIO_S16;
    spec.channels = 2;
    spec.freq = 44100;

    mStream = SDL_CreateAudioStream(&spec, &spec);
    if (!mStream) {
        Log("SDL_OpenAudioDeviceStream failed: %s", SDL_GetError());
        return false;
    }
    SDL_SetAudioStreamGetCallback(mStream, OplController::audio_callback, this);


    #ifdef FLUX_ENGINE
        AudioManager.bindStream(mStream);
    #else
        SDL_AudioDeviceID dev = SDL_OpenAudioDevice(SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK, NULL);

        if (dev == 0) {
            Log("Failed to open audio device: %s", SDL_GetError());
            return false;
        }

        if (!SDL_BindAudioStream(dev, mStream)) {
            Log("Failed to bind stream: %s", SDL_GetError());
            return false;
        }
    #endif


    SDL_ResumeAudioStreamDevice(mStream);

    // null the cache
    std::memset(m_instrument_cache, 0, sizeof(m_instrument_cache));
    std::memset(m_instrument_name_cache, 0, sizeof(m_instrument_name_cache));


    return true;
}

// bool OplController::initController() {
//     SDL_AudioSpec spec;
//     spec.format = SDL_AUDIO_S16;
//     spec.channels = 2;
//     spec.freq = 44100;
//
//     // Create the stream and a logical device connection in one go
//     mStream = SDL_OpenAudioDeviceStream(
//         SDL_AUDIO_DEVICE_DEFAULT_PLAYBACK,
//         &spec,
//         OplController::audio_callback, // The static bridge
//         this                           // Pass 'this' as userdata
//     );
//
//     if (!mStream) {
//         Log("SDL_OpenAudioDeviceStream failed: %s", SDL_GetError());
//         return false;
//     }
//
//     // Mandatory: Start the device (it is created paused)
//     SDL_ResumeAudioStreamDevice(mStream);
//     return true;
// }
//------------------------------------------------------------------------------
bool OplController::shutDownController()
{
    if (mStream) {
        SDL_DestroyAudioStream(mStream);
        mStream = nullptr;
    }
    return true;
}

//------------------------------------------------------------------------------
void OplController::set_speed(uint8_t songspeed)
{
    if (songspeed == 0) songspeed = 1;

    // TRY 70.0f (VGA/AdLib standard) or 50.0f (Amiga/Tracker standard)
    float base_hz = PLAYBACK_FREQUENCY;
    float ticks_per_second = base_hz / (float)songspeed;

    mSeqState.samples_per_tick = (int)(44100.0f / ticks_per_second);
}
//------------------------------------------------------------------------------
void OplController::setPlaying(bool value, bool hardStop)
{
    mSeqState.playing = value;

    if (!value) // We are Pausing or Stopping
    {
        this->silenceAll(hardStop);
    }
}
//------------------------------------------------------------------------------
void OplController::togglePause()
{
    if (mSeqState.playing){
        setPlaying(false,false);
    } else {
        setPlaying(true, false);
    }
}
//------------------------------------------------------------------------------
uint8_t OplController::get_carrier_offset(uint8_t channel) {
    if (channel > FMS_MAX_CHANNEL) return 0;

    // The Carrier is always 3 steps away from the Modulator base
    return Adr_add[channel] + 3;
}
//------------------------------------------------------------------------------
uint8_t OplController::get_modulator_offset(uint8_t channel){
    if (channel > FMS_MAX_CHANNEL) return 0;
    return Adr_add[channel];
}
//------------------------------------------------------------------------------
void OplController::silenceAll(bool hardStop) {
    // Stop all notes physically via Key-Off

    std::lock_guard<std::recursive_mutex> lock(mDataMutex);

    for (int i = FMS_MIN_CHANNEL; i <= FMS_MAX_CHANNEL; ++i) {
        stopNote(i);

        if (hardStop )
        {
            // Write TL=63 to both Modulator and Carrier for every channel
            // Modulator TL addresses: 0x40 - 0x55
            // Carrier TL addresses:   0x40 - 0x55 (offset by +3)
            uint8_t mod_offset = get_modulator_offset(i);
            uint8_t car_offset = get_carrier_offset(i);

            write(0x40 + mod_offset, 63); // Silence Modulator
            write(0x40 + car_offset, 63); // Silence Carrier
        }
    }
}
//------------------------------------------------------------------------------
void OplController::reset() {
    mChip->reset();
    m_pos = 0.0;

    // A truly "silent" instrument has Total Level = 63
    uint8_t silent_ins[24];
    memset(silent_ins, 0, 24);
    silent_ins[2] = 63; // Modulator TL (Silent)
    silent_ins[3] = 63; // Carrier TL (Silent)

    for (int i = 0; i < 9; ++i) {
        setInstrument(i, silent_ins);
    }

    mSeqState.song_needle = 0;
    this->silenceAll(true);
}
//------------------------------------------------------------------------------
void OplController::write(uint16_t reg, uint8_t val)
{
    #ifdef FM_DEBUG
    if (reg >= 0xB0 && reg <= 0xB8) {
        printf("HARDWARE WRITE: Reg %02X = %02B (KeyOn: %s)\n",
                reg, val, (val & 0x20) ? "YES" : "NO");
    }
    #endif
    mChip->write_address(reg);
    mChip->write_data(val);
}
//------------------------------------------------------------------------------
void OplController::playNoteDOS(int channel, int noteIndex) {
    if (channel < 0 || channel > 8) return;
    std::lock_guard<std::recursive_mutex> lock(mDataMutex);

    if (noteIndex <= 0 || noteIndex >= 85) {
        stopNote(channel);
        return;
    }
    // --- RHYTHM MODE LOGIC ---
    if (!mMelodicMode && channel >= 6) {
        // 1. Set the Frequency/Pitch for the drum (Drums still need a pitch!)
        uint8_t a0_val = myDosScale[noteIndex][1];
        uint8_t b0_val = myDosScale[noteIndex][0] & ~0x20; // REMOVE the melodic Key-On bit
        write(0xA0 + channel, a0_val);
        write(0xB0 + channel, b0_val); // Write block/fnum WITHOUT turning the note "on"

        // 2. Trigger the specific hardware drum bit in 0xBD
        uint8_t drumMask = 0;
        if (channel == 6) drumMask = 0x10; // Bass Drum
        if (channel == 7) drumMask = 0x08; // Snare (Note: You'd need logic to pick Snare vs HH)
        if (channel == 8) drumMask = 0x04; // Tom (Note: You'd need logic to pick Tom vs Cym)

        // Trigger: Off then On (to ensure a fresh hit)
        write(0xBD, 0xE0 & ~drumMask);
        write(0xBD, 0xE0 | drumMask);

        return; // Exit here so we don't execute the melodic code below
    }

    // --- STANDARD MELODIC LOGIC (Channels 0-5, or all if MelodicMode is true) ---
    stopNote(channel);
    uint8_t b0_val = myDosScale[noteIndex][0];
    uint8_t a0_val = myDosScale[noteIndex][1];

    write(0xA0 + channel, a0_val);
    write(0xB0 + channel, b0_val);
    last_block_values[channel] = b0_val;
}

//   ---------- before Rhythm Mode:  ---------------
// void OplController::playNoteDOS(int channel, int noteIndex) {
//     if (channel < 0 || channel > 8) return;
//
//     std::lock_guard<std::recursive_mutex> lock(mDataMutex);
//
//     // 1. Handle "No Note" or "Rest"
//     // Since your table is 1-based [85], index 0 is safety/padding.
//     if (noteIndex <= 0 || noteIndex >= 85) {
//         stopNote(channel);
//         return;
//     }
//
//     // 2. Force Key-Off to ensure the OPL envelope re-triggers
//     stopNote(channel);
//
//
//     // 3. Translate the index using your myDosScale table
//     uint8_t b0_val = myDosScale[noteIndex][0]; // The Key-On/Block/F-High byte
//     uint8_t a0_val = myDosScale[noteIndex][1]; // The F-Low byte
//
//     // 4. Write to the OPL registers
//     // Register A0-A8: F-Number Low
//     write(0xA0 + channel, a0_val);
//
//     // Register B0-B8: Key-On, Block, and F-Number High
//     write(0xB0 + channel, b0_val);
//
//     // 5. Save the value so stopNote() knows which block/frequency to turn off
//     last_block_values[channel] = b0_val;
// }
//------------------------------------------------------------------------------
void OplController::playNote(int channel, int noteIndex) {
    if (channel < 0 || channel > 8)
        return;

    std::lock_guard<std::recursive_mutex> lock(mDataMutex);

    stopNote(channel);


    int octave = noteIndex / 12;
    int note = noteIndex % 12;
    uint16_t fnum = f_numbers[note];

    // Register 0xA0: Low 8 bits of F-Number
    write(0xA0 + channel, fnum & 0xFF);

    // Register 0xB0: Key-On (0x20) | Octave | F-Number High bits
    uint8_t b0_val = 0x20 | (octave << 2) | (fnum >> 8);

    // SAVE the value (so we know the octave/frequency for later)
    last_block_values[channel] = b0_val;

    write(0xB0 + channel, b0_val);
}
//------------------------------------------------------------------------------
void OplController::stopNote(int channel) {
    if (channel < 0 || channel > 8) return;

    std::lock_guard<std::recursive_mutex> lock(mDataMutex);

    // --- RHYTHM MODE STOP ---
    if (!mMelodicMode && channel >= 6) {
        // In Rhythm Mode, we clear the trigger bits in 0xBD
        uint8_t drumMask = 0;
        if (channel == 6) drumMask = 0x10; // Bass Drum
        if (channel == 7) drumMask = 0x08 | 0x01; // Snare & Hi-Hat
        if (channel == 8) drumMask = 0x04 | 0x02; // Tom & Cymbal

        // Clear only the bits for this channel, keep Global Rhythm (0x20) and Depths (0xC0)
        uint8_t currentBD = 0xE0;
        write(0xBD, currentBD & ~drumMask);
    }
    else {
        // --- MELODIC MODE STOP ---
        uint8_t b0_off = last_block_values[channel] & ~0x20;
        write(0xB0 + channel, b0_off);
        last_block_values[channel] = b0_off;
    }

    // Your critical fix remains at the bottom
    mChip->generate(&mOutput);
}

//   ---------- before Rhythm Mode:  ---------------
// void OplController::stopNote(int channel) {
//     if (channel < 0 || channel > 8) return;
//
//     std::lock_guard<std::recursive_mutex> lock(mDataMutex);
//
//     // CRITICAL FIX: Use the last value but REMOVE the Key-On bit (0x20)
//     // This keeps the Octave and Frequency bits identical during the release phase.
//     uint8_t b0_off = last_block_values[channel] & ~0x20;
//
//     // write(0xB0 + channel, 0x00);
//     write(0xB0 + channel, b0_off);
//
//     mChip->generate(&mOutput); // THIS IS CRITCAL to get it work !!! take me 8 hours to find out!
//
//     // Update the saved value so we don't accidentally re-enable Key-On
//     last_block_values[channel] = b0_off;
// }
//------------------------------------------------------------------------------
void OplController::render(int16_t* buffer, int frames) {
    for (int i = 0; i < frames; i++) {
        // Only generate new OPL data when the fractional position advances
        while (m_pos <= i) {
            mChip->generate(&mOutput);
            m_pos += m_step;
        }

        // Use the most recently generated output
        int16_t sample = (int16_t)mOutput.data[0];
        buffer[i * 2 + 0] = sample; // Left
        buffer[i * 2 + 1] = sample; // Right
    }
    // Reset m_pos for the next callback buffer
    m_pos -= frames;
}
// blended version but sounds the same
// void OplController::render(int16_t* buffer, int frames) {
//     static int32_t prev_sample = 0; // Store the last sample for blending
//
//     for (int i = 0; i < frames; i++) {
//         while (m_pos <= i) {
//             prev_sample = m_output.data[0]; // Save old sample before generating new
//             m_chip->generate(&m_output);
//             m_pos += m_step;
//         }
//
//         // 1. Calculate the fractional distance between the two OPL samples
//         double fraction = m_pos - i;
//
//         // 2. Linear Interpolation: Blend prev_sample and current m_output.data[0]
//         // This removes the "thinness" and adds the "depth" you heard in DOSBox
//         int32_t blended = (int32_t)((double)prev_sample * fraction +
//         (double)m_output.data[0] * (1.0 - fraction));
//
//         // 3. Output to buffer
//         int16_t sample = (int16_t)blended;
//         buffer[i * 2 + 0] = sample;
//         buffer[i * 2 + 1] = sample;
//     }
//     m_pos -= frames;
// }
//------------------------------------------------------------------------------
void OplController::dumpInstrumentFromCache(uint8_t channel)
{
    if (channel > FMS_MAX_CHANNEL) return;

    // 1. Get the pointer from your cache
    const uint8_t* patch = getInstrument(channel);

    if (patch == nullptr) {
        printf("Error: Channel %d has no instrument in cache!\n", channel);
        return;
    }

    printf("\n--- CACHE DUMP: OPL Channel %d ---\n", channel);
    // There are 24 parameters in your metadata
    for (size_t i = 0; i < 24; ++i) {
        printf("[%2zu] %-20s : %d\n", i, OplController::INSTRUMENT_METADATA[i].name.c_str(), patch[i]);
    }
    printf("----------------------------------\n");
}
//------------------------------------------------------------------------------
void OplController::setInstrument(uint8_t channel, const uint8_t lIns[24]) {
    if (channel > FMS_MAX_CHANNEL) return;

    std::lock_guard<std::recursive_mutex> lock(mDataMutex);

    memcpy(m_instrument_cache[channel], lIns, 24);

    // Pointer for our data (allows us to use the hi-hat test override)
    const uint8_t* p_ins = lIns;

    // Get the hardware offsets based on your Adr_add logic
    uint8_t mod_off = get_modulator_offset(channel); // Adr_add[channel]
    uint8_t car_off = get_carrier_offset(channel);   // Adr_add[channel] + 3

    // 1. Multiplier / Sustain Mode / Vibrato ($20 range)
    write(0x20 + mod_off, p_ins[0] | (p_ins[14] << 5) | (p_ins[16] << 6) | (p_ins[18] << 7));
    write(0x20 + car_off, p_ins[1] | (p_ins[15] << 5) | (p_ins[17] << 6) | (p_ins[19] << 7));

    // 2. Total Level / Scaling ($40 range)
    write(0x40 + mod_off, p_ins[2] | (p_ins[22] << 6));
    write(0x40 + car_off, p_ins[3] | (p_ins[23] << 6));

    // 3. Attack / Decay ($60 range)
    write(0x60 + mod_off, p_ins[6] | (p_ins[4] << 4));
    write(0x60 + car_off, p_ins[7] | (p_ins[5] << 4));

    // 4. Sustain Level / Release Rate ($80 range)
    write(0x80 + mod_off, p_ins[10] | (p_ins[8] << 4));
    write(0x80 + car_off, p_ins[11] | (p_ins[9] << 4));

    // 5. Waveform Select ($E0 range)
    write(0xE0 + mod_off, p_ins[12]);
    write(0xE0 + car_off, p_ins[13]);

    // 6. Connection / Feedback ($C0 range) - CHANNEL BASED, not operator based
    write(0xC0 + channel, p_ins[21] | (p_ins[20] << 1));

    // Global Setup (Ensure waveforms are enabled)
    write(0x01, 0x20);

    // 0xBD is the Rhythm Control / Depth Register.
    // 0xC0 Enables Deep Effects and Locks Melodic Mode
    //      This is how i set it in my dos fm class
    // Register 0xBD Bit Breakdown
    // Each bit in this 8-bit register has a specific function:
    // Bit
    // Name	Description
    // 7	AM Depth	Sets the depth of the Amplitude Modulation (Tremolo). 0 = 1dB, 1 = 4.8dB.
    // 6	Vib Depth	Sets the depth of the Vibrato. 0 = 7 cents, 1 = 14 cents.
    // 5	Rhythm Mode	0 = Melodic (9 voices); 1 = Percussion (6 voices + 5 drums).
    // 4	BD (Bass Drum)	Triggers the Bass Drum (if Rhythm Mode is 1).
    // 3	SD (Snare Drum)	Triggers the Snare Drum (if Rhythm Mode is 1).
    // 2	TT (Tom-Tom)	Triggers the Tom-Tom (if Rhythm Mode is 1).
    // 1	CY (Cymbal)	Triggers the Top Cymbal (if Rhythm Mode is 1).
    // 0	HH (Hi-Hat)	Triggers the High-Hat (if Rhythm Mode is 1).
    // -----------------------------------------------------------------
    // 0xC0	0	Melodic	9 FM Channels. 100% control over every sound.
    // 0xE0	1	Rhythm	6 FM Channels + 5 fixed Drum sounds.
    // -----------------------------------------------------------------
     if (mMelodicMode)
        write(0xBD, 0xC0);
    else
        write(0xBD, 0xE0);



}
//------------------------------------------------------------------------------
const uint8_t* OplController::getInstrument(uint8_t channel) const{
    if (channel > FMS_MAX_CHANNEL) return nullptr;
    return m_instrument_cache[channel];
}
//------------------------------------------------------------------------------
bool OplController::loadSongFMS(const std::string& filename, SongData& sd) {
    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) {
        Log("ERROR: Could not open song file: %s", filename.c_str());
        return false;
    }

    reset(); //reset everything !!

    // 1. Interleaved Load: Name then Settings for each channel
    for (int ch = 1; ch <= 9; ++ch) {
        if (!file.read(reinterpret_cast<char*>(sd.actual_ins[ch]), 256)) {
            Log("ERROR: Failed reading Name for Channel %d at offset %ld", ch, (long)file.tellg());
            return false;
        }
        if (!file.read(reinterpret_cast<char*>(sd.ins_set[ch]), 24)) {
            Log("ERROR: Failed reading Settings for Channel %d at offset %ld", ch, (long)file.tellg());
            return false;
        }
    }

    // 2. Load Speed (1 byte) and Length (2 bytes)
    if (!file.read(reinterpret_cast<char*>(&sd.song_delay), 1)) {
        Log("ERROR: Failed reading song_speed");
        return false;
    }
    if (!file.read(reinterpret_cast<char*>(&sd.song_length), 2)) {
        Log("ERROR: Failed reading song_length");
        return false;
    }

    Log("INFO: Song Header Loaded. Speed: %u, Length: %u", sd.song_delay, sd.song_length);

    // Safety check for 2026 memory limits
    if (sd.song_length > FMS_MAX_SONG_LENGTH) {
        Log("ERROR: song_length (%u) exceeds maximum allowed (1000)", sd.song_length);
        return false;
    }

    // 3. Load Note Grid (Adjusted for 0-based C++ logic)
    for (int i = 0; i < sd.song_length; ++i) { // Start at 0
        for (int j = 0; j <= FMS_MAX_CHANNEL; ++j) {           // Start at 0
            int16_t temp_note;
            if (!file.read(reinterpret_cast<char*>(&temp_note), 2)) {
                Log("ERROR: Failed reading Note at Tick %d, Channel %d", i, j);
                return false;
            }
            // Store it 0-based so sd.song[0][0] is the first note
            sd.song[i][j] = temp_note;
        }
    }

    Log("SUCCESS: Song '%s' loaded completely.", filename.c_str());

    // 4. Update OPL with new instruments
    for (int ch = FMS_MIN_CHANNEL; ch <= FMS_MAX_CHANNEL; ++ch) {
        // Hardware Channel 'i' (0-8) gets Instrument Data 'i+1' (1-9)
        setInstrument(ch, sd.ins_set[ch + 1]);
        setInstrumentNameInCache(ch, GetInstrumentName(sd,ch).c_str());
    }

    return true;
}
//------------------------------------------------------------------------------
/**
 *  Returns a null-terminated C-string for display/UI
 */
std::string OplController::GetInstrumentName(SongData& sd, int channel) {
    if (channel < FMS_MIN_CHANNEL || channel > FMS_MAX_CHANNEL)
    {
        Log("Error: GetInstrumentName with invalid channel ! %d", channel);
        return "";
    }
    return sd.getInstrumentName(channel);
}

/**
 *  Converts a C-string back into the Pascal [Length][Data...] format
 */
bool OplController::SetInstrumentName(SongData& sd, int channel, const char* name) {
    if (channel < FMS_MIN_CHANNEL || channel > FMS_MAX_CHANNEL)
    {
        Log("Error: setInstrumentName with invalid channel ! %d", channel);
        return false;
    }
    setInstrumentNameInCache(channel,name);
    return sd.setInstrumentName(channel, name);
}

//------------------------------------------------------------------------------
bool OplController::saveSongFMS(const std::string& filename, SongData& sd) {
    std::ofstream file(filename, std::ios::binary);
    if (!file.is_open()) return false;

    // Sync Live Cache to SongData (Slots 1-9)
    // Clear Slot 0 so it remains "empty"
    std::memset(sd.ins_set[0], 0, 24);
    // Copy 9 live instruments from cache into sd.ins_set[1...9]
    std::memcpy(&sd.ins_set[1][0], m_instrument_cache, sizeof(m_instrument_cache));

    // Write Instruments (Indices 1-9)
    for (int ch = 1; ch <= 9; ++ch) {
        // FIX: Must write 256 bytes for name to match your loader's file.read(..., 256)
        file.write(reinterpret_cast<const char*>(sd.actual_ins[ch]), 256);

        // Write 24 bytes for instrument settings
        file.write(reinterpret_cast<const char*>(sd.ins_set[ch]), 24);
    }

    // Write Metadata
    // Loader expects 1 byte for delay
    uint8_t delay8 = static_cast<uint8_t>(sd.song_delay);
    file.write(reinterpret_cast<const char*>(&delay8), 1);

    // Loader expects 2 bytes for length
    file.write(reinterpret_cast<const char*>(&sd.song_length), 2);

    // Write Notes
    for (int i = 0; i < sd.song_length; ++i) {
        for (int j = 0; j <= FMS_MAX_CHANNEL; ++j) {
            file.write(reinterpret_cast<const char*>(&sd.song[i][j]), 2);
        }
    }

    file.close();
    return file.good();
}
//------------------------------------------------------------------------------
void OplController::start_song(SongData& sd, bool loopit, int startAt, int stopAt)
{
    // SDL2 SDL_LockAudio(); // Stop the callback thread for a microsecond

    mSeqState.current_song = &sd;

    // when playing a part of the song
    mSeqState.song_startAt  = startAt;

    if ( stopAt > startAt )
        mSeqState.song_stopAt  = stopAt;
    else
        mSeqState.song_stopAt  = sd.song_length;

    // the needle at which position the song is:
    mSeqState.song_needle = mSeqState.song_startAt;


    mSeqState.sample_accumulator = 0;
    mSeqState.loop = loopit;
    set_speed(sd.song_delay);
    mSeqState.playing = true;
    // SDL2 SDL_UnlockAudio(); // Let the callback thread resume
}
//------------------------------------------------------------------------------


bool OplController::loadInstrument(const std::string& filename, uint8_t channel)
{
    if (channel > FMS_MAX_CHANNEL)
        return false;

    std::ifstream file(filename, std::ios::binary);
    if (!file.is_open()) return false;

    uint8_t instrumentData[24];
    file.read(reinterpret_cast<char*>(instrumentData), 24);

    if (file.gcount() == 24) {
        setInstrument(channel, instrumentData);
        setInstrumentNameInCache(channel, filename.c_str());
        return true;
    }

    return false;
}
//------------------------------------------------------------------------------
bool OplController::saveInstrument(const std::string& filename, uint8_t channel)
{
    if (channel > FMS_MAX_CHANNEL)
        return false;
    // std::ios::binary is essential to prevent CRLF translation on Windows
    std::ofstream file(filename, std::ios::binary);

    if (!file.is_open()) {
        return false;
    }

    // Write the 24-byte block exactly as it exists in memory
    file.write(reinterpret_cast<const char*>(getInstrument(channel)), 24);

    if  (file.good() ) {
        setInstrumentNameInCache(channel, filename.c_str());
        return true;
    }
    return false;
}
//------------------------------------------------------------------------------
void OplController::TestInstrumentDOS(uint8_t channel, const uint8_t ins[24], int noteIndex) {
    if (!mChip || channel > FMS_MAX_CHANNEL) return;

    std::lock_guard<std::recursive_mutex> lock(mDataMutex);

    printf("--- Testing Channel %d with Note Index %d ---\n", channel, noteIndex);

    // 1. Apply the instrument (Ensure this uses your p_ins logic from before)
    setInstrument(channel, ins);

    // 2. Force Volume to Maximum (OPL: 0 is loudest)
    // We override the Carrier Total Level for this specific channel to ensure we hear it.
    int car_off = get_carrier_offset(channel);
    write(0x40 + car_off, 0x00);

    // 3. Clear the Key-On bit first (Force 0 -> 1 transition)
    write(0xB0 + channel, 0x00);

    // 4. Wait a few samples for the chip to register the Key-Off
    OplChip::output_data output;
    for (int i = 0; i < 100; i++) mChip->generate(&output);

    // 5. Trigger the Note using your myDosScale table
    if (noteIndex < 1 || noteIndex > 84) noteIndex = 48; // Default to C-4
    uint8_t b0_val = myDosScale[noteIndex][0];
    uint8_t a0_val = myDosScale[noteIndex][1];

    write(0xA0 + channel, a0_val);
    write(0xB0 + channel, b0_val);

    // 6. Monitor for sound over the next 4410 samples (0.1 seconds)
    bool heard_sound = false;
    for (int i = 0; i < 4410; i++) {
        mChip->generate(&output);
        if (output.data[0] != 0) {
            heard_sound = true;
            if (i % 1000 == 0) {
                printf("Sample %d: Output Level %d\n", i, output.data[0]);
            }
        }
    }

    if (!heard_sound) {
        printf("RESULT: Channel %d is SILENT. Check instrument registers ($20-$80 range).\n", channel);
    } else {
        printf("RESULT: Channel %d is PRODUCING SOUND.\n", channel);
    }
}
//------------------------------------------------------------------------------
void OplController::replaceSongNotes(SongData& sd, uint8_t targetChannel, int16_t oldNote, int16_t newNote) {
    if (targetChannel > FMS_MAX_CHANNEL)
        return;

    int count = 0;

    // Start at 0, end before song_length
    for (int i = 0; i < sd.song_length; ++i) {

        int16_t currentNote = sd.song[i][targetChannel];

        // DEBUG: This will now show the actual notes in memory
        if (i < 10) { // Limit debug output to first 10 ticks to avoid lag
            printf("remap: tick:%d, chan:%d, note:%d\n", i, targetChannel, currentNote);
        }

        if (currentNote == oldNote) {
            sd.song[i][targetChannel] = newNote;
            count++;
        }
    }
    printf("REMAPPER: Replaced %d instances of note %d with %d.\n", count, oldNote, newNote);
}
//------------------------------------------------------------------------------
void OplController::fillBuffer(int16_t* buffer, int total_frames) {
    // 1. Local cache of volatile values for speed
    double step = m_step;
    double current_pos = m_pos;

    for (int i = 0; i < total_frames; i++) {

        // --- SEQUENCER LOGIC ---
        if (mSeqState.playing && mSeqState.current_song) {
            mSeqState.sample_accumulator += 1.0;

            while (mSeqState.sample_accumulator >= mSeqState.samples_per_tick) {
                mSeqState.sample_accumulator -= mSeqState.samples_per_tick;
                this->tickSequencer(); // Encapsulate the "next step" logic
            }
        }

        // --- RENDER LOGIC ---
        while (current_pos <= i) {
            mChip->generate(&mOutput);
            current_pos += step;
        }

        int16_t sample = (int16_t)mOutput.data[0];
        buffer[i * 2 + 0] = sample; // Left
        buffer[i * 2 + 1] = sample; // Right
    }

    // 2. Save back once
    m_pos = current_pos - total_frames;
}
//------------------------------------------------------------------------------
void OplController::tickSequencer() {
    const SongData& s = *mSeqState.current_song;

    if ( mSeqState.song_stopAt > s.song_length )
    {
        Log("ERROR: song stop is greater than song_length. thats bad!!!");
        mSeqState.song_stopAt = s.song_length;
    }
    // if (mSeqState.song_counter < s.song_length)
    if (mSeqState.song_needle < mSeqState.song_stopAt)
    {
        for (int ch = FMS_MIN_CHANNEL; ch <= FMS_MAX_CHANNEL; ch++) {
            int16_t raw_note = s.song[mSeqState.song_needle][ch];

            // Update UI/Debug state
            mSeqState.last_notes[ch + 1] = raw_note;
            mSeqState.note_updated = true;

            if (raw_note == -1) {
                this->stopNote(ch);
            } else if (raw_note > 0) {
                this->playNoteDOS(ch, (uint8_t)raw_note);
            }
        }
        mSeqState.song_needle++;
    } else {
        if (mSeqState.loop)
        {
            if (mSeqState.song_startAt > mSeqState.song_stopAt)
            {
                Log("ERROR: song start is greater than song stop. thats bad!!!");
                mSeqState.song_startAt = 0;

            }
            mSeqState.song_needle = mSeqState.song_startAt;
        }
        else setPlaying(false);
    }
}
//------------------------------------------------------------------------------
void OplController::consoleSongOutput(bool useNumbers)
{
    // DEBUG / UI VIEW
    if (mSeqState.note_updated) {
        // We print "song_needle - 1" because the counter was already
        // incremented in the callback after the note was played.
        printf("Step %3d: ", mSeqState.song_needle - 1);

        // Loop through Tracker Columns 1 to 9
        for (int ch = 1; ch <= 9; ch++) {
            int16_t note = mSeqState.last_notes[ch];

            if (note == -1) {
                printf(" === "); // Visual "Note Off"
            } else if (note == 0) {
                printf(" ... "); // Visual "Empty/Rest"
            } else {
                if ( useNumbers )
                    printf("%4d ", note); // The actual note index
                else
                    printf("%4s ", getNoteNameFromId(note).c_str());
            }
        }
        printf("\n");
        mSeqState.note_updated = false;
    }
}
//------------------------------------------------------------------------------
std::string OplController::getNoteNameFromId(int noteID) {
    // 0 is the padding/empty entry in your array
    if (noteID == 0 || noteID > 84) {
        return "...";
    } else if (noteID < 0) {
        return "===";
    }



    // The note names in the order of your chromatic table (C, C#, D, D#...)
    static const char* noteNames[] = {
        "C-", "C#", "D-", "D#", "E-", "F-",
        "F#", "G-", "G#", "A-", "A#", "B-"
    };

    // 1. Calculate the Octave (I-VIII)
    // IDs 1-12 = Octave 1, 13-24 = Octave 2, etc.
    int octave = ((noteID ) / 12 )  + 1;

    int positionInOctave = (noteID - 1) % 12;

    // Note: If your myDosScale matches the DOCUMENT numbers exactly:
    // 1=D, 2=E, 3=F, 4=G, 5=A, 6=H, 7=C#, 8=D#, 9=F#, 10=G#, 11=A#, 12=C
    static const int documentToMusicalMap[] = {
        2,  // 1 -> D
        4,  // 2 -> E
        5,  // 3 -> F
        7,  // 4 -> G
        9,  // 5 -> A
        11, // 6 -> H
        1,  // 7 -> C#
        3,  // 8 -> D#
        6,  // 9 -> F#
        8,  // 10 -> G#
        10, // 11 -> A#
        0   // 12 -> C
    };

    int noteIndex = documentToMusicalMap[positionInOctave];

    return std::string(noteNames[noteIndex]) + std::to_string(octave);
}
//------------------------------------------------------------------------------
int OplController::getIdFromNoteName(std::string name)
{

    if (name.length() < 3 || name == "...")
        return 0;
    else if (name == "===")
        return -1;

    static const std::vector<std::string> noteNames = {
        "C-", "C#", "D-", "D#", "E-", "F-",
        "F#", "G-", "G#", "A-", "A#", "B-"
    };

    // The reverse map of your documentToMusicalMap
    // Maps musical index (0=C, 1=C#, 2=D...) back to array position (0-11)
    static const int musicalToPositionMap[] = {
        11, // C- (0) -> index 11
        6,  // C# (1) -> index 6
        0,  // D- (2) -> index 0
        7,  // D# (3) -> index 7
        1,  // E- (4) -> index 1
        2,  // F- (5) -> index 2
        8,  // F# (6) -> index 8
        3,  // G- (7) -> index 3
        9,  // G# (8) -> index 9
        4,  // A- (9) -> index 4
        10, // A# (10)-> index 10
        5   // B- (11)-> index 5
    };

    std::string notePart = name.substr(0, 2);
    int octave;
    try {
        octave = std::stoi(name.substr(2));
    } catch (...) { return 0; }

    // Find the musical index (0-11)
    int musicalIndex = -1;
    for (int i = 0; i < 12; ++i) {
        if (noteNames[i] == notePart) {
            musicalIndex = i;
            break;
        }
    }
    if (musicalIndex == -1) return 0;

    int positionInOctave = musicalToPositionMap[musicalIndex];

    // THE REVERSE MATH:
    // In your working function: octave = ((noteID - 1) / 12) + 1 + (noteIndex == 0 ? 1 : 0)
    // If the note is C (index 0), the octave was incremented, so we subtract it back.
    int adjustedOctave = (musicalIndex == 0) ? (octave - 1) : octave;

    int noteID = ((adjustedOctave - 1) * 12) + (positionInOctave + 1);

    return (noteID >= 1 && noteID <= 84) ? noteID : 0;
}
//------------------------------------------------------------------------------
std::array< uint8_t, 24 > OplController::GetDefaultInstrument()
{
    return {
        0x01, 0x01, // 0-1: Modulator/Carrier Frequency
        0x10, 0x00, // 2-3: Modulator/Carrier Output
        0x0F, 0x0F, // 4-5: Modulator/Carrier Attack
        0x00, 0x00, // 6-7: Modulator/Carrier Decay
        0x07, 0x07, // 8-9: Modulator/Carrier Sustain
        0x07, 0x07, // 10-11: Modulator/Carrier Release
        0x00, 0x00, // 12-13: Modulator/Carrier Waveform
        0x00, 0x00, // 14-15: Modulator/Carrier EG Typ
        0x00, 0x00, // 16-17: Modulator/Carrier Vibrato
        0x00, 0x00, // 18-19: Modulator/Carrier Amp Mod
        0x00, 0x00, // 20-21: Feedback / Modulation Mode
        0x00, 0x00  // 22-23: Modulator/Carrier Scaling
    };
}
//------------------------------------------------------------------------------


std::array< uint8_t, 24 > OplController::GetDefaultBassDrum(){
    return {
        0x01, 0x02, // Freq: Mod=1, Car=2 (gives a thicker sound)
        0x10, 0x00, // Output: Both loud
        0x0F, 0x0F, // Attack: Instant
        0x08, 0x06, // Decay: Rapid for punch
        0x00, 0x00, // Sustain: 0 (Drums shouldn't sustain)
        0x0A, 0x0A, // Release: Quick fade
        0x00, 0x00, // Waveform: Sine
        0x00, 0x00, // EG Typ: 0 (Drums always decay)
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
}

std::array< uint8_t, 24 > OplController::GetDefaultSnareHiHat(){
    return {
        0x01, 0x01,
        0x12, 0x00, // Mod(HH) slightly quieter than Car(SD)
        0x0F, 0x0F, // Attack: Instant
        0x0D, 0x07, // Decay: HH is very short (D), SD is longer (7)
        0x00, 0x00, // Sustain: 0
        0x0D, 0x07, // Release: Match decay
        0x00, 0x00, // Waveform: Sine (OPL noise gen overrides this)
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
    };
}

std::array< uint8_t, 24 > OplController::GetDefaultTomCymbal(){
    return {
        0x02, 0x01, // 0-1: Multiplier (Mod/Tom: 2 for a hollower tone, Car/Cym: 1)
        0x12, 0x02, // 2-3: Output (Tom: 18, Cym: 2 [Loud])
        0x0F, 0x0F, // 4-5: Attack (Instant for both)
        0x07, 0x09, // 6-7: Decay (Tom: 7 [Snappy], Cym: 9 [Linger])
        0x00, 0x00, // 8-9: Sustain (Must be 0 for drums)
        0x07, 0x09, // 10-11: Release (Match Decay for clean trigger-off)
        0x00, 0x00, // 12-13: Waveform (Sine is standard)
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, // 14-21 (Flags/Feedback)
        0x00, 0x00  // 22-23: Scaling
    };
}

std::array< uint8_t, 24 > OplController::GetDefaultLeadSynth()
{
    return {
        0x01, 0x01, // 0-1: Multiplier (1:1 Verhältnis für klaren Ton)
        0x12, 0x00, // 2-3: Output (Modulator auf 18 für FM-Grit, Carrier auf 0 [Laut])
        0x0F, 0x0F, // 4-5: Attack (Maximal schnell)
        0x05, 0x02, // 6-7: Decay (Modulator fällt schnell ab, Carrier hält länger)
        0x0F, 0x0F, // 8-9: Sustain (Maximal - Lead Sounds stehen solange Taste gedrückt)
        0x05, 0x05, // 10-11: Release (Kurzes Ausklingen)
        0x02, 0x00, // 12-13: Waveform (Modulator: Absolute Sine [0x02] für mehr Obertöne)
        0x01, 0x01, // 14-15: EG Typ (Sustain-Modus AN)
        0x00, 0x00, // 16-17: Vibrato (Kann im Editor manuell erhöht werden)
        0x00, 0x00, // 18-19: Amp Mod (Aus)
        0x05, 0x00, // 20-21: Feedback auf 5 (WICHTIG für den "Sägezahn"-Charakter), Mode: FM
        0x00, 0x00  // 22-23: Scaling (Aus)
    };
}

std::array< uint8_t, 24 > OplController::GetDefaultOrgan()
{
    return {
        0x01, 0x02, // 0-1: Multiplier (Mod=1 [Base], Car=2 [Octave/Harmonic])
        0x08, 0x00, // 2-3: Output (Both loud; Adjust Mod output to change 'drawbar' mix)
        0x0F, 0x0F, // 4-5: Attack (Instant for both)
        0x00, 0x00, // 6-7: Decay (None)
        0x0F, 0x0F, // 8-9: Sustain (Full - Organs don't fade)
        0x02, 0x02, // 10-11: Release (Very short - stops immediately)
        0x00, 0x00, // 12-13: Waveform (Pure Sines)
        0x01, 0x01, // 14-15: EG Typ (Sustain ON)
        0x01, 0x01, // 16-17: Vibrato (ON - Adds the 'Leslie Speaker' feel)
        0x00, 0x00, // 18-19: Amp Mod (OFF)
        0x00, 0x01, // 20-21: Feedback 0, Connection 1 (Additive Mode - CRITICAL)
        0x00, 0x00  // 22-23: Scaling (OFF)
    };
}

std::array< uint8_t, 24 > OplController::GetDefaultCowbell()
{
    return {
        0x01, 0x04, // 0-1: Multiplier (Mod=1, Car=4: Creates a high, resonant 'clink')
        0x15, 0x00, // 2-3: Output (Mod=21 [Moderate FM bite], Car=0 [Loud])
        0x0F, 0x0F, // 4-5: Attack (Instant hit)
        0x06, 0x08, // 6-7: Decay (Modulator drops fast to clean the tone, Carrier lingers)
        0x00, 0x00, // 8-9: Sustain (Must be 0 for percussion)
        0x08, 0x08, // 10-11: Release (Fast fade-out)
        0x00, 0x00, // 12-13: Waveform (Pure Sine)
        0x00, 0x00, // 14-15: EG Typ (Decay mode)
        0x00, 0x00, // 16-17: Vibrato (Off)
        0x00, 0x00, // 18-19: Amp Mod (Off)
        0x07, 0x00, // 20-21: Feedback 7 (Max grit for 'metal' feel), Mode FM
        0x00, 0x00  // 22-23: Scaling
    };
}

void OplController::resetInstrument(uint8_t channel)
{
    std::array<uint8_t, 24> defaultData;

    if (!mMelodicMode) {
        if (channel == 6) defaultData = GetDefaultBassDrum();
        else if (channel == 7) defaultData = GetDefaultSnareHiHat();
        else if (channel == 8) defaultData = GetDefaultTomCymbal(); // similar logic
        else defaultData = GetDefaultInstrument();
    } else {
        defaultData = GetDefaultInstrument();
    }
    setInstrument(channel, defaultData.data());
}

std::array< uint8_t, 24 > OplController::GetMelodicDefault(uint8_t index)
{
    auto data = GetDefaultInstrument(); // Start with your basic Sine template

    switch (index % 6) {
        case 0: // Basic Piano
            data[4] = 0xF; data[5] = 0xF; // Fast Attack
            data[6] = 0x4; data[7] = 0x4; // Moderate Decay
            data[8] = 0x2; data[9] = 0x2; // Low Sustain
            break;
        case 1: // FM Bass
            data[20] = 0x5;               // High Feedback (Index 20)
            data[2] = 0x15;               // Higher Modulator Output for grit
            break;
        case 2: // Strings
            data[4] = 0x3; data[5] = 0x2; // Slow Attack
            data[10] = 0x5; data[11] = 0x5; // Slower Release
            break;
        case 3: return GetDefaultLeadSynth();
        case 4: return GetDefaultOrgan();
        case 5: return GetDefaultCowbell();
    }
    return data;
}

void OplController::loadInstrumentPreset()
{
    std::string lDefaultName;
    for (uint8_t ch = FMS_MIN_CHANNEL; ch <= FMS_MAX_CHANNEL; ch++) {
        std::array<uint8_t, 24> defaultData;

        if (!mMelodicMode) {
            // Rhythm Mode Logic
            if (ch == 6)      defaultData = GetDefaultBassDrum();
            else if (ch == 7) defaultData = GetDefaultSnareHiHat();
            else if (ch == 8) defaultData = GetDefaultTomCymbal();
            else             defaultData = GetMelodicDefault(ch);

            lDefaultName = std::format("RythmDefaultChannel {}",ch+1);
        } else {
            // Full Melodic Mode: 9 Melodic Voices
            defaultData = GetMelodicDefault(ch);
            lDefaultName = std::format("MelodicDefault {}",ch+1);
        }

        setInstrumentNameInCache(ch, lDefaultName.c_str());
        setInstrument(ch, defaultData.data());
    }
}


