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
    mChip = new ymfm::ymf262(mInterface);//OPL3

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
void OPL3Controller::generate(int16_t* buffer, int frames) {
    for (int i = 0; i < frames; ++i) {
        // 1. Ask ymfm to compute one sample of data
        // This populates mOutput.data[] with the current chip state
        mChip->generate(&mOutput);

        // 2. Interleave the Left and Right channels into your int16_t buffer
        // OPL3 standard output: data[0] = Left, data[1] = Right
        for (int chan = 0; chan < 2; ++chan) {
            int32_t sample = mOutput.data[chan];
            //raise volume distortion sound bad :P
            // sample = mOutput.data[chan] * 256;

            // 3. Optional: Manual Clamping for 16-bit safety
            if (sample > 32767)  sample = 32767;
            if (sample < -32768) sample = -32768;

            *buffer++ = static_cast<int16_t>(sample);
        }
    }
}
//------------------------------------------------------------------------------
void OPL3Controller::fillBuffer(int16_t* buffer, int total_frames) {
    if (!mSeqState.playing) {
        this->generate(buffer, total_frames);
        return;
    }

    int frames_left = total_frames;
    int buffer_offset = 0;

    while (frames_left > 0) {
        // Calculate remaining samples for the current tick
        double samples_needed = mSeqState.samples_per_tick - mSeqState.sample_accumulator;

        if (samples_needed <= 0.0) {
            this->tickSequencer();
            // Subtract the length of one tick to keep precision
            mSeqState.sample_accumulator -= mSeqState.samples_per_tick;

            // Re-calculate samples_needed for the NEW tick
            samples_needed = mSeqState.samples_per_tick - mSeqState.sample_accumulator;
        }

        // Determine how much we can generate in this pass
        int chunk = std::min((int)frames_left, (int)std::max(1.0, samples_needed));

        this->generate(&buffer[buffer_offset * 2], chunk);

        mSeqState.sample_accumulator += chunk;
        buffer_offset += chunk;
        frames_left -= chunk;
    }
}

// void OPL3Controller::fillBuffer(int16_t* buffer, int total_frames) {
//
//     if (!mSeqState.playing) {
//         // Generate the raw ymfm output into the buffer
//         // This makes manual playNote() calls audible
//         this->generate(buffer, total_frames);
//         return;
//     }
//
//     int frames_left = total_frames;
//     int buffer_offset = 0;
//
//     while (frames_left > 0) {
//         // 1. Determine how many frames until the next tick
//         int frames_to_next_tick = (int)(mSeqState.samples_per_tick - mSeqState.sample_accumulator);
//
//         // If it's time for a tick (or we missed one)
//         if (frames_to_next_tick <= 0) {
//             this->tickSequencer(); // This triggers playNote or processStepEffects
//             mSeqState.sample_accumulator = 0;
//             frames_to_next_tick = (int)mSeqState.samples_per_tick;
//         }
//
//         // 2. Determine how many frames we can safely render now
//         int chunk = std::min(frames_left, frames_to_next_tick);
//
//         // 3. Render OPL3 audio into the buffer for this chunk
//         this->generate(&buffer[buffer_offset * 2], chunk);
//
//         // 4. Update counters
//         mSeqState.sample_accumulator += chunk;
//         buffer_offset += chunk;
//         frames_left -= chunk;
//     }
// }
//------------------------------------------------------------------------------
void OPL3Controller::tickSequencer() {
    if (!mSeqState.playing || !mSeqState.current_song) return;

    const SongData& song = *mSeqState.current_song;
    const Pattern& pat = song.patterns[song.orderList[mSeqState.orderIdx]];

    if (mSeqState.rowIdx >= pat.rowCount) {
        mSeqState.rowIdx = 0;
        return;
    }

    for (int ch = 0; ch < MAX_CHANNELS; ++ch) {
        const SongStep& step = pat.steps[mSeqState.rowIdx * MAX_CHANNELS + ch];

        if (mSeqState.current_tick == 0) {
            // --- Tick 0: New Row Trigger ---

            // Logic: Trigger if there is a note, OR if the step contains
            // data (volume/pan) that differs from the current channel state.
            bool noteTrigger = (step.note > 0);
            bool volumeChange = (step.volume != mSeqState.last_steps[ch].volume);
            bool panningChange = (step.panning != mSeqState.last_steps[ch].panning);

            if (noteTrigger || volumeChange || panningChange) {
                // playNote handles Note 0 (modulation only) vs Note 1-255 (trigger/stop)
                this->playNote(ch, step);
            }

            // Process non-continuous effects that start on Tick 0 (like Position Jump)
            if (step.effectType != 0) {
                this->processStepEffects(ch, step);
            }
        } else {
            // --- Ticks 1+: Continuous Effects ---
            if (step.effectType != 0) {
                this->processStepEffects(ch, step);
            }
        }
    }

    // --- Timing Advancement ---
    mSeqState.current_tick++;

    if (mSeqState.current_tick >= mSeqState.ticks_per_row) {
        mSeqState.current_tick = 0;
        mSeqState.rowIdx++;

        if (mSeqState.rowIdx >= pat.rowCount) {
            mSeqState.rowIdx = 0;
            mSeqState.orderIdx++;

            if (mSeqState.orderIdx >= song.orderList.size() ||
                (mSeqState.orderStopAt > 0 && mSeqState.orderIdx > mSeqState.orderStopAt)) {
                if (mSeqState.loop) {
                    mSeqState.orderIdx = mSeqState.orderStartAt;
                } else {
                    mSeqState.playing = false;
                    mSeqState.orderIdx = 0;
                    this->silenceAll(true);
                    for (int i = 0; i < 18; ++i) {
                        mSeqState.last_steps[i] = {};
                    }
                    mSeqState.ui_dirty = true;
                }
            }
        }
    }
}

// void OPL3Controller::tickSequencer() {
//     if (!mSeqState.playing || !mSeqState.current_song) return;
//
//     const SongData& song = *mSeqState.current_song;
//     const Pattern& pat = song.patterns[song.orderList[mSeqState.orderIdx]];
//
//     if (mSeqState.rowIdx >= pat.rowCount) {
//         mSeqState.rowIdx = 0; // Emergency reset
//         return;
//     }
//
//     for (int ch = 0; ch < MAX_CHANNELS; ++ch) {
//         // Use mutable step if processStepEffects needs to save state (like volume slides)
//         // SongStep& step = const_cast<SongStep&>(pat.steps[mSeqState.rowIdx * 18 + ch]);
//         const SongStep& step = pat.steps[mSeqState.rowIdx * 18 + ch];
//
//         if (mSeqState.current_tick == 0) {
//             // Start of a new Row
//             if (step.note > 0) {
//                 this->playNote(ch, step);
//                 // dLog("Playnote: ch:%d note:%d vol:%d Instrument:%d Effect:%d Panning:%d", ch, step.note, step.volume, step.instrument, step.effectType, step.panning); //FIXME
//                 mSeqState.last_steps[ch] = step;
//                 mSeqState.ui_dirty = true;
//             } else if (step.effectType != 0) {
//                 this->processStepEffects(ch, step);
//             }
//         } else {
//             // Sub-tick processing for continuous effects
//             if (step.effectType != 0) {
//                 this->processStepEffects(ch, step);
//             }
//         }
//     }
//
//     // 1. Advance Tick
//     mSeqState.current_tick++;
//
//     if (mSeqState.current_tick >= mSeqState.ticks_per_row) {
//         mSeqState.current_tick = 0;
//         mSeqState.rowIdx++; // Move to next row
//
//         // 2. Check if Row is out of pattern bounds
//         if (mSeqState.rowIdx >= pat.rowCount) {
//             mSeqState.rowIdx = 0;
//             mSeqState.orderIdx++; // Move to next pattern in orderList
//
//             // 3. Check if Song is finished
//             if (mSeqState.orderIdx >= song.orderList.size()) {
//                 if (mSeqState.loop) {
//                     mSeqState.orderIdx = mSeqState.orderStartAt;
//                 } else {
//                     mSeqState.playing = false;
//                     mSeqState.orderIdx = 0; // Reset for next play
//                 }
//             }
//         }
//     }
// }
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

    memset(mSeqState.last_steps, 0, sizeof(mSeqState.last_steps));
    mSeqState.ui_dirty = false;

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


    Log("OPL3 Controller Reset: 18 Channels enabled, Shadows cleared, Sequencer at Start.");
}
//------------------------------------------------------------------------------
/**
 * @param oplVolume Attenuation level (0 = Max Volume, 63 = Muted)
 */
void OPL3Controller::setChannelVolume(uint8_t channel, uint8_t oplVolume) {
    // 1. Hardware Boundary Checks
    if (channel >= MAX_CHANNELS) return;


    if (oplVolume > 63) oplVolume = 63;
    uint8_t vol = oplVolume & 0x3F;

    // if (mSeqState.rowIdx == 33){
    //     assert(false);
    // }

    // dLog("setChannelVolume: chan:%d row:%d oplVolume: %d ", channel, mSeqState.rowIdx, oplVolume); //FIXME remove this line


    uint16_t mod_off = get_modulator_offset(channel);
    uint16_t car_off = get_carrier_offset(channel);

    // 2. Update Carrier (Operator 1)
    // Preservation of KSL (bits 6-7) is critical to maintain instrument scaling
    uint8_t carReg = readShadow(0x40 + car_off);
    write(0x40 + car_off, (carReg & 0xC0) | vol);

    // 3. Update Modulator (Operator 0)
    // Only applied if in Additive mode (Connection Bit 0 of $C0 is set)
    uint16_t bank = (channel < 9) ? 0x000 : 0x100;
    uint16_t c0Addr = bank + 0xC0 + (channel % 9);
    bool isAdditive = (readShadow(c0Addr) & 0x01);

    if (isAdditive) {
        uint8_t modReg = readShadow(0x40 + mod_off);
        write(0x40 + mod_off, (modReg & 0xC0) | vol);
    }
}
//------------------------------------------------------------------------------
bool OPL3Controller::applyInstrument(uint8_t channel, uint8_t instrumentIndex) {
    if (channel >= MAX_CHANNELS) return false;
    if (instrumentIndex >= mSoundBank.size()) return false;

    const auto& ins = mSoundBank[instrumentIndex];

    // --- Configure 4-operator mode for this channel ---
    uint8_t fourOpReg = readShadow(0x104);
    uint8_t fourOpBit = 0;
    uint8_t masterChannelRelativeIndex = channel % 9; // 0-8 for both banks

    // Determine the bit to set in 0x104 for this channel group
    // 0x104 bits: 0-2 for channels 0-2, 3-5 for channels 9-11
    if (channel < 3) { // Channels 0, 1, 2 are masters in bank 0
        fourOpBit = (1 << masterChannelRelativeIndex);
    } else if (channel >= 9 && channel < 12) { // Channels 9, 10, 11 are masters in bank 1
        fourOpBit = (1 << (masterChannelRelativeIndex + 3)); // Map to bits 3, 4, 5 of 0x104
    }

    if (ins.isFourOp) {
        write(0x104, fourOpReg | fourOpBit); // Enable 4-op mode for this channel pair
    } else {
        write(0x104, fourOpReg & ~fourOpBit); // Disable 4-op mode
    }


    uint16_t m_off = get_modulator_offset(channel);
    uint16_t c_off = get_carrier_offset(channel);

    // Operator 0 (Modulator) of Pair 0
    const auto& mod0 = ins.pairs[0].ops[0];
    write(0x20 + m_off, mod0.multi | (mod0.ksr << 4) | (mod0.egTyp << 5) | (mod0.vib << 6) | (mod0.am << 7));
    write(0x40 + m_off, mod0.tl | (mod0.ksl << 6));
    write(0x60 + m_off, mod0.decay | (mod0.attack << 4));
    write(0x80 + m_off, mod0.release | (mod0.sustain << 4));
    write(0xE0 + m_off, mod0.wave & 0x07);

    // Operator 1 (Carrier) of Pair 0
    const auto& car0 = ins.pairs[0].ops[1];
    write(0x20 + c_off, car0.multi | (car0.ksr << 4) | (car0.egTyp << 5) | (car0.vib << 6) | (car0.am << 7));
    write(0x40 + c_off, car0.tl | (car0.ksl << 6));
    write(0x60 + c_off, car0.decay | (car0.attack << 4));
    write(0x80 + c_off, car0.release | (car0.sustain << 4));
    write(0xE0 + c_off, car0.wave & 0x07);

    // Connection / Feedback / Panning ($C0 Register) for the main channel
    uint16_t c0Addr = ((channel <= 8) ? 0x000 : 0x100) + 0xC0 + (channel % 9);
    uint8_t c0Val = (ins.pairs[0].panning << 4) | (ins.pairs[0].feedback << 1) | (ins.isFourOp ? 0x01 : ins.pairs[0].connection);
    write(c0Addr, c0Val);

    if (ins.isFourOp) {
        // Configure operators for the linked channel (channel + 3)
        uint8_t linkedChannel = channel + 3;
        uint16_t m1_off = get_modulator_offset(linkedChannel);
        uint16_t c1_off = get_carrier_offset(linkedChannel);

        // Operator 0 (Modulator) of Pair 1 (which maps to Modulator 2 for 4-op)
        const auto& mod1 = ins.pairs[1].ops[0];
        write(0x20 + m1_off, mod1.multi | (mod1.ksr << 4) | (mod1.egTyp << 5) | (mod1.vib << 6) | (mod1.am << 7));
        write(0x40 + m1_off, mod1.tl | (mod1.ksl << 6));
        write(0x60 + m1_off, mod1.decay | (mod1.attack << 4));
        write(0x80 + m1_off, mod1.release | (mod1.sustain << 4));
        write(0xE0 + m1_off, mod1.wave & 0x07);

        // Operator 1 (Carrier) of Pair 1 (which maps to Carrier 2 for 4-op)
        const auto& car1 = ins.pairs[1].ops[1];
        write(0x20 + c1_off, car1.multi | (car1.ksr << 4) | (car1.egTyp << 5) | (car1.vib << 6) | (car1.am << 7));
        write(0x40 + c1_off, car1.tl | (car1.ksl << 6));
        write(0x60 + c1_off, car1.decay | (car1.attack << 4));
        write(0x80 + c1_off, car1.release | (car1.sustain << 4));
        write(0xE0 + c1_off, car1.wave & 0x07);

        // For 4-op mode, the linked channel's connection is always FM (0)
        uint16_t c0LinkedAddr = ((linkedChannel <= 8) ? 0x000 : 0x100) + 0xC0 + (linkedChannel % 9);
        uint8_t c0LinkedVal = (ins.pairs[1].panning << 4) | (ins.pairs[1].feedback << 1) | 0x00; // Force FM connection
        write(c0LinkedAddr, c0LinkedVal);
    } else {
        // If not 4-op, and this channel *could* be a 4-op master, ensure its linked channel is reset to 2-op FM mode.
        // This handles cases where a 4-op instrument was previously on this channel.
        if ((channel >=0 && channel < 3) || (channel >= 9 && channel < 12)) {
            uint8_t linkedChannel = channel + 3;
            uint16_t c0LinkedAddr = ((linkedChannel <= 8) ? 0x000 : 0x100) + 0xC0 + (linkedChannel % 9);
            uint8_t c0LinkedVal = readShadow(c0LinkedAddr);
            write(c0LinkedAddr, c0LinkedVal & ~0x01); // Ensure FM connection
        }
    }

    return true;
}

//------------------------------------------------------------------------------
bool OPL3Controller::playNote(uint8_t channel, SongStep step) {
    if (channel >= MAX_CHANNELS) return false;

    std::lock_guard<std::recursive_mutex> lock(mDataMutex);

    SongStep prevStep = mSeqState.last_steps[channel];
    mSeqState.last_steps[channel] = step;
    mSeqState.ui_dirty = true;

    // Helper for hardware volume conversion (0-63 -> 63-0)
    auto getOplVol = [](uint8_t v) {
        uint8_t clamped = (v > 63) ? 63 : v;
        return (uint8_t)(63 - clamped);
    };

    // --- CASE 1: MODULATION ONLY ---
    if (step.note == 0) {
        // THIS SUCKS !!!
        // if (step.volume != prevStep.volume) {
        //     setChannelVolume(channel, getOplVol(step.volume));
        // }
        // if (step.panning != prevStep.panning) {
        //     setChannelPanning(channel, step.panning);
        // }
        return true;
    }

    // --- CASE 2: NOTE OFF ---
    if (step.note == 255) {
        stopNote(channel);
        return true;
    }

    // --- CASE 3: TRIGGER NEW NOTE ---
    stopNote(channel);
    applyInstrument(channel, step.instrument);

    // Get the instrument for fixedNote and fineTune
    const auto& currentIns = mSoundBank[step.instrument];

    // Apply converted volume and panning
    setChannelVolume(channel, getOplVol(step.volume));
    setChannelPanning(channel, step.panning);

    // Frequency Setup
    int midiNote = step.note;

    // Apply fixedNote if valid (1-96 tracker range)
    if (currentIns.fixedNote > 0 && currentIns.fixedNote <= 96 && currentIns.fixedNote != 255) {
        // Convert tracker note (1-96) to MIDI note number
        // Tracker note 1 (C-0) often corresponds to MIDI note 12 (C-0).
        midiNote = currentIns.fixedNote + 11;
    }

    // Clamp midiNote to a reasonable range (e.g., 0-127)
    if (midiNote < 0) midiNote = 0;
    if (midiNote > 127) midiNote = 127;

    // Calculate OPL Octave (Block) and F-Number Index from MIDI note
    // Assuming OPL Block 0 corresponds to MIDI C-0 (MIDI note 12)
    const int MIDI_C0_FOR_OPL_BLOCK_0 = 12;

    int octave = (midiNote - MIDI_C0_FOR_OPL_BLOCK_0) / 12; // OPL Block 0-7
    int noteIndex = (midiNote - MIDI_C0_FOR_OPL_BLOCK_0) % 12; // Index into opl3::f_numbers (0-11)

    // Clamp octave to valid OPL range (0-7)
    if (octave < 0) octave = 0;
    if (octave > 7) octave = 7;

    // Clamp noteIndex to valid range (0-11)
    if (noteIndex < 0) noteIndex = 0;
    if (noteIndex > 11) noteIndex = 11;

    uint16_t fnum = opl3::f_numbers[noteIndex];

    // Apply fineTune
    fnum += currentIns.fineTune;

    dLog("new fnum:%d, fineTune:%d",fnum, currentIns.fineTune); //FIXME remove this
    // Clamp fnum to valid range (0-1023)
    if (fnum > 1023) fnum = 1023;
    if (fnum < 0) fnum = 0;

    uint16_t bankOffset = (channel <= 8) ? 0x000 : 0x100;
    uint8_t relChan = channel % 9;

    write(bankOffset + 0xA0 + relChan, fnum & 0xFF);

    // Key-On Bit (0x20)
    uint8_t b0_val = 0x20 | (octave << 2) | ((fnum >> 8) & 0x03);
    write(bankOffset + 0xB0 + relChan, b0_val);

    return true;
}

// bool OPL3Controller::playNote(uint8_t channel, SongStep step) {
//     // 1. Hardware Limit Validation
//     if (channel >= MAX_CHANNELS) return false;
//
//     // Recursive mutex for thread safety (Audio Thread vs. UI Thread)
//     std::lock_guard<std::recursive_mutex> lock(mDataMutex);
//
//     // Capture the 'old' state before we overwrite it
//     SongStep prevStep = mSeqState.last_steps[channel];
//
//     // Update the state and UI flag
//     mSeqState.last_steps[channel] = step;
//     mSeqState.ui_dirty = true;
//
//     // --- CASE 1: VOLUME / PANNING ONLY UPDATE (Note == 0) ---
//     // If no new note is provided, update modulation without re-triggering the sound.
//     if (step.note == 0) {
//         if (step.volume != prevStep.volume) {
//             // Convert tracker volume (0-64) to OPL attenuation (63-0)
//             uint8_t trackerVol = (step.volume > 63) ? 63 : step.volume;
//             uint8_t oplVolume = 63 - trackerVol;
//             setChannelVolume(channel, oplVolume);
//         }
//         if (step.panning != prevStep.panning) {
//             setChannelPanning(channel, step.panning);
//         }
//         // if (stateChanged) mSeqState.note_updated = true;
//         return true;
//     }
//     // --- CASE 2: NOTE OFF (Note == 255) ---
//     if (step.note == 255) {
//         stopNote(channel);
//         return true;
//     }
//     // --- CASE 3: TRIGGER NEW NOTE (Note 1-127) ---
//     // 1. Key-Off: Stop previous note so the Envelope Generator resets to the Attack phase
//     stopNote(channel);
//     // 2. Load Instrument: Must be set before Key-On to configure operators and connections
//     applyInstrument(channel, step.instrument);
//     // 3. Apply Volume & Panning
//     uint8_t oplVolume = 63 - (step.volume & 0x3F);
//     setChannelVolume(channel, oplVolume);
//     setChannelPanning(channel, step.panning);
//     // 4. Frequency Calculation (C-0 to B-7)
//     int internalNote = step.note - 1; // 0-based index
//     int octave = internalNote / 12;
//     int noteIndex = internalNote % 12;
//     uint16_t fnum = opl3::f_numbers[noteIndex];
//     // Determine hardware bank (Bank 0: channels 0-8, Bank 1: channels 9-17)
//     uint16_t bankOffset = (channel <= 8) ? 0x000 : 0x100;
//     uint8_t relChan = channel % 9;
//     // 5. Write Frequency Low-Byte ($A0-$A8)
//     write(bankOffset + 0xA0 + relChan, fnum & 0xFF);
//     // 6. Write Octave + F-Num High + Key-On ($B0-$B8)
//     // Bit 5 (0x20) is the Key-On bit
//     uint8_t b0_val = 0x20 | ((octave & 0x07) << 2) | ((fnum >> 8) & 0x03);
//     write(bankOffset + 0xB0 + relChan, b0_val);
//     return true;
// }
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
    uint16_t c0Addr = ((channel < 9) ? 0x000 : 0x100) + 0xC0 + (channel % 9);
    uint8_t c0Val = readShadow(c0Addr) & 0xCF; // Keep feedback/connection bits

    dLog("setChannelPanning: chan:%d row:%d pan:%d ", channel, mSeqState.rowIdx, pan); //FIXME remove this line

    if (pan < 21)      c0Val |= 0x10; // Left
    else if (pan > 43) c0Val |= 0x20; // Right
    else               c0Val |= 0x30; // Center

    write(c0Addr, c0Val);
}

//------------------------------------------------------------------------------
void OPL3Controller::initDefaultBank(){
    mSoundBank.clear();
    //FIXME need a better default bank !!
    mSoundBank.push_back(GetDefaultInstrument());
    mSoundBank.push_back(GetDefaultInstrument());
    mSoundBank.push_back(GetDefaultInstrument());
    mSoundBank.push_back(GetDefaultInstrument());
    mSoundBank.push_back(GetDefaultInstrument());
    mSoundBank.push_back(GetDefaultInstrument());
    mSoundBank.push_back(GetDefaultInstrument());
    mSoundBank.push_back(GetDefaultInstrument());


}
//------------------------------------------------------------------------------
void OPL3Controller::dumpInstrument(uint8_t instrumentIndex) {
    if (instrumentIndex >= mSoundBank.size()) {
        LogFMT("Error: Instrument index {} out of bounds (max {}).", instrumentIndex, mSoundBank.size() - 1);
        return;
    }

    const auto& ins = mSoundBank[instrumentIndex];

    LogFMT("--- Instrument Dump: {} (Index {}) ---", ins.name, instrumentIndex);
    LogFMT("  Is Four-Op: {}", ins.isFourOp ? "Yes" : "No");
    LogFMT("  Fine Tune: {}", (int)ins.fineTune);
    LogFMT("  Fixed Note: {}", (ins.fixedNote == 255) ? "None" : std::to_string(ins.fixedNote));

    for (int pIdx = 0; pIdx < (ins.isFourOp ? 2 : 1); ++pIdx) {
        const auto& pair = ins.pairs[pIdx];
        LogFMT("  --- Operator Pair {} ---", pIdx);
        LogFMT("    Feedback: {}", pair.feedback);
        LogFMT("    Connection: {}", pair.connection == 0 ? "FM" : "Additive");
        LogFMT("    Panning: {}", pair.panning);

        for (int opIdx = 0; opIdx < 2; ++opIdx) {
            const auto& op = pair.ops[opIdx];
            LogFMT("    {} Operator:", opIdx == 0 ? "Modulator" : "Carrier");

            // Manual mapping to OplInstrument::OpParams members based on OPL_OP_METADATA order
            LogFMT("      Multi:   0x{:02X}", op.multi);
            LogFMT("      TL:      0x{:02X}", op.tl);
            LogFMT("      Attack:  0x{:02X}", op.attack);
            LogFMT("      Decay:   0x{:02X}", op.decay);
            LogFMT("      Sustain: 0x{:02X}", op.sustain);
            LogFMT("      Release: 0x{:02X}", op.release);
            LogFMT("      Wave:    0x{:02X}", op.wave);
            LogFMT("      KSR:     0x{:02X}", op.ksr);
            LogFMT("      EGType:  0x{:02X}", op.egTyp);
            LogFMT("      Vib:     0x{:02X}", op.vib);
            LogFMT("      AM:      0x{:02X}", op.am);
            LogFMT("      KSL:     0x{:02X}", op.ksl);
        }
    }
    LogFMT("-----------------------------------");
}

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
void OPL3Controller::processStepEffects(uint8_t channel, const SongStep& step) {
    uint8_t type = step.effectType;
    uint8_t val  = step.effectVal;

    // Helper for hardware volume conversion (0-63 -> 63-0)
    auto getOplVol = [](uint8_t v) {
        uint8_t clamped = (v > 63) ? 63 : v;
        return (uint8_t)(63 - clamped);
    };

    switch (type) {
        case EFF_VOL_SLIDE: {
            uint8_t slideUp = (val >> 4);
            uint8_t slideDown = (val & 0x0F);

            // Get current volume from the step-mirror
            uint8_t currentVol = mSeqState.last_steps[channel].volume;

            if (slideUp > 0) {
                currentVol = (uint8_t)std::min(63, (int)currentVol + slideUp);
            } else if (slideDown > 0) {
                currentVol = (currentVol > slideDown) ? currentVol - slideDown : 0;
            }


            dLog("Slide...channel:%d row:%d step.volume:%d( oplVolume: %d)", channel , mSeqState.rowIdx, currentVol, getOplVol(currentVol)); //FIXME remove this line

            // Update the state mirror
            mSeqState.last_steps[channel].volume = currentVol;
            mSeqState.ui_dirty = true;

            // Apply to hardware
            setChannelVolume(channel, getOplVol(currentVol));
            break;
        }

        case EFF_PORTA_UP: {
            modifyChannelPitch(channel, (int8_t)val);
            break;
        }

        case EFF_SET_VOLUME: {
            uint8_t newVol = std::min((uint8_t)63, val);
            mSeqState.last_steps[channel].volume = newVol;
            mSeqState.ui_dirty = true;

            setChannelVolume(channel, getOplVol(newVol));
            break;
        }

        case EFF_SET_PANNING: {

            setChannelPanning(channel, val);
            break;
        }
    }
}
//------------------------------------------------------------------------------
void OPL3Controller::modifyChannelPitch(uint8_t channel, int8_t amount) {
    uint16_t bank = (channel < 9) ? 0x000 : 0x100;
    uint8_t relChan = channel % 9;

    // 1. Read current F-Number/Octave from your Shadow Registers
    uint8_t low = readShadow(bank + 0xA0 + relChan);
    uint8_t high = readShadow(bank + 0xB0 + relChan);

    uint16_t fnum = ((high & 0x03) << 8) | low;
    uint8_t octave = (high >> 2) & 0x07;

    // 2. Adjust F-Number
    int newFnum = (int)fnum + amount;

    // 3. Handle F-Number overflow into the next octave
    if (newFnum > 1023) {
        if (octave < 7) { octave++; newFnum = 512; } // Crude octave jump
        else newFnum = 1023;
    } else if (newFnum < 0) {
        if (octave > 0) { octave--; newFnum = 512; }
        else newFnum = 0;
    }

    // 4. Write back to hardware (A0 then B0)
    write(bank + 0xA0 + relChan, newFnum & 0xFF);
    write(bank + 0xB0 + relChan, (high & 0xE0) | ((octave & 0x07) << 2) | ((newFnum >> 8) & 0x03));
}
//------------------------------------------------------------------------------
