//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------

#include "OPL3Controller.h"
#include "ymfmGlue.h"
#include "opl3.h"
#include "OPL3Instruments.h"
#include <mutex>

#ifdef FLUX_ENGINE
#include <audio/fluxAudio.h>
#endif

//------------------------------------------------------------------------------
OPL3Controller::OPL3Controller(){
    // mChip = new ymfm::ym3812(mInterface); //OPL2
    mChip = new ymfm::ymf262(mInterface);//OPL3
    // mChip = new ymfm::ymf289b(mInterface);//OPL3L

    reset();
}
//------------------------------------------------------------------------------
OPL3Controller::~OPL3Controller(){
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
void OPL3Controller::audio_callback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount) {
    auto* controller = static_cast<OPL3Controller*>(userdata);
    if (!controller) return;

    // S16 stereo = 4 bytes per frame
    int framesNeeded = additional_amount / 4;

    // Use a stack buffer for performance (typical 512-1024 frames)
    // 2048 samples * 2 bytes = 4096 bytes on stack (safe)
    int16_t buffer[2048];
    if (framesNeeded > 1024) framesNeeded = 1024;

    {
        std::lock_guard<std::recursive_mutex> lock(controller->mDataMutex);
        controller->fillBuffer(buffer, framesNeeded);
    }

    SDL_PutAudioStreamData(stream, buffer, framesNeeded * 4);
}

// void OPL3Controller::audio_callback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount)
// {
//
//     if (!userdata)
//         return;
//
//     auto* controller = static_cast<OPL3Controller*>(userdata);
//     if (controller && additional_amount > 0) {
//
//         std::lock_guard<std::recursive_mutex> lock(controller->mDataMutex);
//
//         int frames = additional_amount / 4; // S16 * 2 channels = 4 bytes
//
//         // Generate data directly into a temporary vector or stack buffer
//         std::vector<int16_t> temp(additional_amount / sizeof(int16_t));
//         controller->fillBuffer(temp.data(), frames);
//
//         SDL_PutAudioStreamData(stream, temp.data(), additional_amount);
//     }
// }

//------------------------------------------------------------------------------
bool OPL3Controller::initController()
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
    SDL_SetAudioStreamGetCallback(mStream, OPL3Controller::audio_callback, this);


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

    Log("OPL3 Controller initialized..");

    return true;
}

//------------------------------------------------------------------------------
bool OPL3Controller::shutDownController()
{
    if (mStream) {
        SDL_DestroyAudioStream(mStream);
        mStream = nullptr;
    }
    return true;
}
//------------------------------------------------------------------------------
void OPL3Controller::fillBuffer(int16_t* buffer, int total_frames){
    double step = m_step;
    double current_pos = m_pos;

    for (int i = 0; i < total_frames; i++) {
        // --- SEQUENCER ---
        if (mSeqState.playing && mSeqState.current_song) {
            mSeqState.sample_accumulator += 1.0;
            while (mSeqState.sample_accumulator >= mSeqState.samples_per_tick) {
                mSeqState.sample_accumulator -= mSeqState.samples_per_tick;
                this->tickSequencer();
            }
        }

        // --- RENDER ---
        while (current_pos <= i) {
            // 1. OPL3 Summing: Previous sample
            // We sum (A+C) for Left and (B+D) for Right
            mRender_prev_l = mOutput.data[0] + mOutput.data[2];
            mRender_prev_r = mOutput.data[1] + mOutput.data[3];

            // 2. Generate new chip state
            mChip->generate(&mOutput);

            // 3. OPL3 Summing: Current raw sample
            int32_t raw_l = mOutput.data[0] + mOutput.data[2];
            int32_t raw_r = mOutput.data[1] + mOutput.data[3];

            // 4. Apply Low Pass Filter
            if (mRenderAlpha < 1.0f) {
                mRender_lpf_l += mRenderAlpha * (static_cast<float>(raw_l) - mRender_lpf_l);
                mRender_lpf_r += mRenderAlpha * (static_cast<float>(raw_r) - mRender_lpf_r);
                mCurrentFiltered_l = mRender_lpf_l;
                mCurrentFiltered_r = mRender_lpf_r;
            } else {
                mCurrentFiltered_l = static_cast<float>(raw_l);
                mCurrentFiltered_r = static_cast<float>(raw_r);
            }

            current_pos += step;
        }

        // --- OUTPUT ---
        if (mRenderUseBlending) {
            double fraction = current_pos - i;

            // Linear interpolation between the previous summed sample and current filtered sample
            double blended_l = (static_cast<double>(mRender_prev_l) * fraction +
            static_cast<double>(mCurrentFiltered_l) * (1.0 - fraction));
            double blended_r = (static_cast<double>(mRender_prev_r) * fraction +
            static_cast<double>(mCurrentFiltered_r) * (1.0 - fraction));

            buffer[i * 2 + 0] = static_cast<int16_t>(std::clamp(blended_l * mRenderGain, -32768.0, 32767.0));
            buffer[i * 2 + 1] = static_cast<int16_t>(std::clamp(blended_r * mRenderGain, -32768.0, 32767.0));
        } else {
            buffer[i * 2 + 0] = static_cast<int16_t>(std::clamp(mCurrentFiltered_l * mRenderGain, -32768.0f, 32767.0f));
            buffer[i * 2 + 1] = static_cast<int16_t>(std::clamp(mCurrentFiltered_r * mRenderGain, -32768.0f, 32767.0f));
        }
    }
    m_pos = current_pos - total_frames;
}
//------------------------------------------------------------------------------
void OPL3Controller::tickSequencer() {
    if (!mSeqState.current_song || !mSeqState.playing) return;

    const SongData& song = *mSeqState.current_song;

    // 1. Safety Check for Order List
    if (mSeqState.orderIdx >= song.orderList.size()) {
        if (mSeqState.loop) mSeqState.orderIdx = mSeqState.orderStartAt;
        else { setPlaying(false); return; }
    }

    // 2. Resolve Pattern
    uint8_t patternIdx = song.orderList[mSeqState.orderIdx];
    const Pattern& pat = song.patterns[patternIdx];

    // 3. Process the Row for all 18 OPL3 channels
    for (int ch = 0; ch < 18; ++ch) {
        // Steps are stored: [row * CHANNELS + channel]
        const SongStep& step = pat.steps[mSeqState.rowIdx * 18 + ch];

        // 0 = No action, 1-96 = Note, 255 = Note Off
        if (step.note > 0) {
            this->playNote(ch, step);
            mSeqState.last_notes[ch] = step.note;
            mSeqState.note_updated = true;
        }
    }

    // 4. Advance Row
    mSeqState.rowIdx++;

    // 5. Check for Pattern End or Order Stop Limit
    if (mSeqState.rowIdx >= pat.rowCount) {
        mSeqState.rowIdx = 0;
        mSeqState.orderIdx++;

        // Handle Song End or Selection Loop
        if (mSeqState.orderIdx >= song.orderList.size() ||
            (mSeqState.orderStopAt > 0 && mSeqState.orderIdx > mSeqState.orderStopAt)) {
                if (mSeqState.loop) {
                    mSeqState.orderIdx = mSeqState.orderStartAt;
                } else {
                    setPlaying(false);
                }
            }
    }
}
//------------------------------------------------------------------------------
uint16_t OPL3Controller::get_modulator_offset(uint8_t channel) {
    static const uint8_t op_offsets[] = {
        0x00, 0x01, 0x02, 0x08, 0x09, 0x0A, 0x10, 0x11, 0x12
    };

    if (channel >= MAX_CHANNELS) return 0;

    // Use the BANK_LIMIT to decide between $000 and $100
    uint16_t base = (channel <= opl3::BANK_LIMIT) ? 0x000 : 0x100;
    uint8_t relative_chan = channel % 9;

    return base + op_offsets[relative_chan];
}

uint16_t OPL3Controller::get_carrier_offset(uint8_t channel) {
    if (channel >= MAX_CHANNELS) return 0; // Out of range
    // The carrier offset is always the modulator offset + 3
    return get_modulator_offset(channel) + 0x03;
}

//------------------------------------------------------------------------------
void OPL3Controller::setPlaying(bool value, bool hardStop) {
    mSeqState.playing = value;
    if (!value) // We are Pausing or Stopping
    {
        this->silenceAll(hardStop);
    }
}
//------------------------------------------------------------------------------
void OPL3Controller::silenceAll(bool hardStop) {
    std::lock_guard<std::recursive_mutex> lock(mDataMutex);

    // Loop through all 18 channels (0-17)
    // Note: If you defined MAX_CHANNEL as 16, you are missing the last channel.
    // Standard OPL3 uses 0 to 17.
    for (int i = 0; i < MAX_CHANNELS; ++i) {
        stopNote(i); // Sends Key-Off to 0xB0 + channel_offset

        if (hardStop) {
            // Get 16-bit offsets to support the 0x100 bank switch
            uint16_t mod_offset = get_modulator_offset(i);
            uint16_t car_offset = get_carrier_offset(i);

            // Set Total Level to maximum attenuation (63 = silent)
            // Bit 6-7 are KSL, we set them to 0 here for a clean silence.
            write(0x40 + mod_offset, 63);
            write(0x40 + car_offset, 63);

            // Optional: Force Release Rate to fastest to kill any long release phases
            // This writes to the $80-$95 range
            write(0x80 + mod_offset, 0x0F); // Max Release
            write(0x80 + car_offset, 0x0F); // Max Release
        }
    }
}
//------------------------------------------------------------------------------
void OPL3Controller::togglePause() {
    if (mSeqState.playing){
        setPlaying(false,false);
    } else {
        setPlaying(true, false);
    }
}
//------------------------------------------------------------------------------
void OPL3Controller::reset() {
    std::lock_guard<std::recursive_mutex> lock(mDataMutex);

    // 1. Hardware & Shadow Reset
    mChip->reset();
    m_pos = 0.0;

    // Crucial: Reset your shadow registers to 0 to match ymfm's fresh state
    memset(mShadowRegs, 0, sizeof(mShadowRegs));

    // 2. Reset Sequencer Position
    // We replace 'song_needle' with the new hierarchical indices
    mSeqState.orderIdx = 0;
    mSeqState.rowIdx = 0;
    mSeqState.sample_accumulator = 0.0;
    memset(mSeqState.last_notes, 0, sizeof(mSeqState.last_notes));

    // 3. Enable OPL3 extensions (Bank 2 access)
    // Writing 0x01 to register 0x105 is mandatory for 18-channel support
    write(0x105, 0x01);

    // 4. Disable 4-Operator modes (Register 0x104)
    // Ensures all channels 0-17 start as 2-operator channels
    write(0x104, 0x00);

    // 5. Global Depth & Mode (Register 0xBD)
    // 0xC0: AM/Vibrato depth high, Melodic mode (all channels independent)
    write(0xBD, 0xC0);


    // 6. Silence Hardware
    // This sets TL=63 for all 18 channels
    this->silenceAll(true);

    this->initDefaultBank();


    dLog("OPL3 Controller Reset: 18 Channels enabled, Shadows cleared, Sequencer at Start.");
}
//------------------------------------------------------------------------------
void OPL3Controller::setChannelVolume(uint8_t channel, uint8_t oplVolume) {
    uint16_t mod_off = get_modulator_offset(channel);
    uint16_t car_off = get_carrier_offset(channel);
    uint8_t vol = oplVolume & 0x3F;

    // 1. Always update the Carrier Volume
    uint8_t carKsl = readShadow(0x40 + car_off) & 0xC0;
    write(0x40 + car_off, carKsl | vol);

    // 2. If Additive mode, update Modulator Volume too
    // Otherwise, Modulator volume stays at its instrument-defined 'Total Level'
    if (isChannelAdditive(channel)) {
        uint8_t modKsl = readShadow(0x40 + mod_off) & 0xC0;
        write(0x40 + mod_off, modKsl | vol);
    }
}
//------------------------------------------------------------------------------
bool OPL3Controller::applyInstrument(uint8_t channel, uint8_t instrumentIndex) {
    if (channel >= MAX_CHANNELS) return false;
    if (instrumentIndex >= mSoundBank.size()) return false;

    const auto& ins = mSoundBank[instrumentIndex];
    uint16_t m_off = get_modulator_offset(channel);
    uint16_t c_off = get_carrier_offset(channel);

    // Operator 0 (Modulator)
    const auto& mod = ins.pairs[0].ops[0];
    write(0x20 + m_off, mod.multi | (mod.ksr << 4) | (mod.egTyp << 5) | (mod.vib << 6) | (mod.am << 7));
    write(0x40 + m_off, mod.tl | (mod.ksl << 6));
    write(0x60 + m_off, mod.decay | (mod.attack << 4));
    write(0x80 + m_off, mod.release | (mod.sustain << 4));
    write(0xE0 + m_off, mod.wave & 0x07);

    // Operator 1 (Carrier)
    const auto& car = ins.pairs[0].ops[1];
    write(0x20 + c_off, car.multi | (car.ksr << 4) | (car.egTyp << 5) | (car.vib << 6) | (car.am << 7));
    write(0x40 + c_off, car.tl | (car.ksl << 6));
    write(0x60 + c_off, car.decay | (car.attack << 4));
    write(0x80 + c_off, car.release | (car.sustain << 4));
    write(0xE0 + c_off, car.wave & 0x07);

    // Connection / Feedback / Panning ($C0 Register)
    uint16_t c0Addr = ((channel <= 8) ? 0x000 : 0x100) + 0xC0 + (channel % 9);
    // Bits 4-5: Panning, Bits 1-3: Feedback, Bit 0: Connection
    uint8_t c0Val = (ins.pairs[0].panning << 4) | (ins.pairs[0].feedback << 1) | ins.pairs[0].connection;
    write(c0Addr, c0Val);



    return true;
}

//------------------------------------------------------------------------------
bool OPL3Controller::playNote(uint8_t channel, SongStep step) {
    if (channel >= 18 || step.note == 0) return false;

    std::lock_guard<std::recursive_mutex> lock(mDataMutex);

    if (step.note == 255) {
        stopNote(channel);
        return false;
    }

    // Prepare Hardware
    applyInstrument(channel, step.instrument);

    // Invert volume: 63 (max) -> 0 (no attenuation)
    uint8_t oplVolume = 63 - (step.volume & 0x3F);
    setChannelVolume(channel, oplVolume);

    // Frequency Setup
    int internalNote = step.note - 1;
    int octave = internalNote / 12;
    int note = internalNote % 12;
    uint16_t fnum = opl3::f_numbers[note];

    uint16_t bankOffset = (channel <= 8) ? 0x000 : 0x100;
    uint8_t relChan = channel % 9;
    uint16_t b0Addr = bankOffset + 0xB0 + relChan;

    // --- Optimization: Clean Re-trigger ---
    uint8_t currentB0 = readShadow(b0Addr);
    if (currentB0 & 0x20) {
        write(b0Addr, currentB0 & ~0x20); // Release if currently playing
    }

    // Write Frequency Low
    write(bankOffset + 0xA0 + relChan, fnum & 0xFF);

    // Trigger Key-On
    uint8_t b0_val = 0x20 | ((octave & 0x07) << 2) | ((fnum >> 8) & 0x03);
    write(b0Addr, b0_val);

    return true;
}

//------------------------------------------------------------------------------
void OPL3Controller::stopNote(uint8_t channel) {
    // Range check for OPL3 (0-17)
    if (channel < 0 || channel >= 18) {
        return;
    }

    std::lock_guard<std::recursive_mutex> lock(mDataMutex);

    // Determine the register bank offset
    // Channels 0-8  -> Bank 1 (0x000)
    // Channels 9-17 -> Bank 2 (0x100)
    uint16_t bankOffset = (channel <= 8) ? 0x000 : 0x100;
    uint8_t relativeChan = channel % 9;
    uint16_t regAddr = bankOffset + 0xB0 + relativeChan;

    // Read current value from shadow to preserve Block (Octave) and F-Number bits
    uint8_t currentB0 = readShadow(regAddr);

    // Clear bit 5 (Key-On) to trigger the release phase
    // 0x20 = 0010 0000 in binary
    write(regAddr, currentB0 & ~0x20);

    // Your critical fix remains at the bottom
    mChip->generate(&mOutput);
}
//------------------------------------------------------------------------------
void OPL3Controller::write(uint16_t reg, uint8_t val){
    mShadowRegs[reg] = val;
    mChip->write_address(reg);
    mChip->write_data(val);
}
//------------------------------------------------------------------------------
uint8_t OPL3Controller::readShadow(uint16_t reg) {
    // Standard OPL3 register range check (Bank 1: 0-0xFF, Bank 2: 0x100-0x1FF)
    if (reg >= 512) {
        return 0;
    }
    return mShadowRegs[reg];

}
//------------------------------------------------------------------------------
bool OPL3Controller::isChannelAdditive(uint8_t channel) {
    if (channel >= MAX_CHANNELS) return false;

    // Determine the base address for the C0 register
    // Bank 1: 0xC0-0xC8 | Bank 2: 0x1C0-0x1C8
    uint16_t bankOffset = (channel < 9) ? 0x000 : 0x100;
    uint8_t relChan = channel % 9;
    uint16_t regAddr = bankOffset + 0xC0 + relChan;

    // Bit 0 is the Connection bit (0 = FM, 1 = Additive/AM)
    return (readShadow(regAddr) & 0x01);
}
//------------------------------------------------------------------------------
void OPL3Controller::setChannelPanning(uint8_t channel, uint8_t pan) {
    // pan: 0 = Mute, 1 = Left, 2 = Right, 3 = Center
    if (channel >= MAX_CHANNELS) return;

    uint16_t bankOffset = (channel <= 8) ? 0x000 : 0x100;
    uint8_t relChan = channel % 9;
    uint16_t regAddr = bankOffset + 0xC0 + relChan;

    // Preserve Feedback (bits 1-3) and Connection/Algorithm (bit 0)
    uint8_t currentC0 = readShadow(regAddr) & 0x0F;

    uint8_t panBits = 0;
    if (pan == 1) panBits = 0x20;      // Left only (bit 5)
    else if (pan == 2) panBits = 0x10; // Right only (bit 4)
    else if (pan == 3) panBits = 0x30; // Center (both bits)
    // pan == 0 stays 0x00 (Mute)

    write(regAddr, currentC0 | panBits);
}

//------------------------------------------------------------------------------
void OPL3Controller::initDefaultBank(){
    mSoundBank.clear();
    // Index 0 is often used as the "Silent" or "Default" slot
    mSoundBank.push_back(GetDefaultInstrument());

    //FIXME need a better default bank !!
    for (uint8_t ch = 0; ch < MAX_CHANNELS; ch++)
    {
        applyInstrument(ch, 0);
    }
    mChip->generate(&mOutput); //force write
}
//------------------------------------------------------------------------------
void OPL3Controller::setFrequencyLinear(uint8_t channel, float linearFreq) {
    if (channel >= MAX_CHANNELS) return;

    int octave = static_cast<int>(linearFreq / 12.0f);
    int noteInOctave = static_cast<int>(linearFreq) % 12;
    float fineTune = linearFreq - static_cast<int>(linearFreq);

    // Get base F-Number from your table
    uint16_t fnumStart = opl3::f_numbers[noteInOctave];
    uint16_t fnumNext = opl3::f_numbers[(noteInOctave + 1) % 12];

    // Linear interpolation between semitones for smooth sweep
    uint16_t fnum = fnumStart + static_cast<uint16_t>((fnumNext - fnumStart) * fineTune);

    // Constrain to OPL3 hardware limits
    if (octave > 7) octave = 7;
    if (octave < 0) octave = 0;

    this->setFrequency(channel, fnum, octave);
}
//------------------------------------------------------------------------------
void OPL3Controller::setFrequency(uint8_t channel, uint16_t fnum, uint8_t octave) {
    if (channel >= MAX_CHANNELS) return;

    std::lock_guard<std::recursive_mutex> lock(mDataMutex);

    // Determine dual-bank addressing
    uint16_t bankOffset = (channel <= 8) ? 0x000 : 0x100;
    uint8_t relChan = channel % 9;

    // 1. Write Frequency Low (Register 0xA0)
    write(bankOffset + 0xA0 + relChan, fnum & 0xFF);

    // 2. Write Key-On + Octave + Frequency High (Register 0xB0)
    // We read the shadow register to see if the note is currently ON or OFF
    uint8_t currentB0 = readShadow(bankOffset + 0xB0 + relChan);
    uint8_t keyOnBit = currentB0 & 0x20; // Preserve existing Key-On state

    // Construct the new B0 value
    // Octave (Block) is bits 2-4, F-Num High is bits 0-1
    uint8_t b0_val = keyOnBit | ((octave & 0x07) << 2) | ((fnum >> 8) & 0x03);

    write(bankOffset + 0xB0 + relChan, b0_val);
}
//------------------------------------------------------------------------------
void OPL3Controller::setChannelOn(uint8_t channel) {
    if (channel >= MAX_CHANNELS) return;

    std::lock_guard<std::recursive_mutex> lock(mDataMutex);

    uint16_t bankOffset = (channel <= 8) ? 0x000 : 0x100;
    uint8_t relChan = channel % 9;
    uint16_t reg = bankOffset + 0xB0 + relChan;

    // 1. Read current state from your shadow register
    uint8_t currentB0 = mShadowRegs[reg];

    // 2. Force the Key-On bit (bit 5) to 1
    uint8_t newB0 = currentB0 | 0x20;

    // 3. Write back to hardware and update shadow
    write(reg, newB0);
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
