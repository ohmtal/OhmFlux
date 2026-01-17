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

    uint32_t master_clock = 14318180;
    mOutputSampleRate = mChip->sample_rate(master_clock);

    Log("OPL SampleRate is: %d" , mOutputSampleRate );

    m_opl3_accumulator = 0.0;
    m_lastSampleL = 0;
    m_lastSampleR = 0;

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

    m_step = mOutputSampleRate / spec.freq;


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
void OPL3Controller::fillBuffer(int16_t* buffer, int total_frames)
{
    //XXTH TEST: added again
        if (!mSeqState.playing) {
            this->generate(buffer, total_frames);
            return;
        }

    int frames_left = total_frames;
    int buffer_offset = 0;

    // The ratio of OPL3 native rate to your output rate (49715 / 44100)
    double step = this->getStep();

    while (frames_left > 0) {
        if (mSeqState.playing) {
            // Check if we need to tick the sequencer
            double samples_needed = mSeqState.samples_per_tick - mSeqState.sample_accumulator;

            if (samples_needed <= 0.0) {
                this->tickSequencer(); // Effects/Notes processed here
                mSeqState.sample_accumulator -= mSeqState.samples_per_tick;
                samples_needed = mSeqState.samples_per_tick;
            }

            // Chunk calculation for your effects (e.g., 6 ticks per row)
            int chunk = std::min((int)frames_left, (int)std::max(1.0, samples_needed));

            // Generate OPL3 samples with resampling for this chunk
            for (int i = 0; i < chunk; i++) {
                m_opl3_accumulator += step;

                // If we've accumulated enough "time" for an OPL3 sample, generate it
                while (m_opl3_accumulator >= 1.0) {
                    mChip->generate(&mOutput);

                    // Mix 4-channel OPL3 to Stereo
                    m_lastSampleL = mOutput.data[0] + mOutput.data[2];
                    m_lastSampleR = mOutput.data[1] + mOutput.data[3];

                    m_opl3_accumulator -= 1.0;
                }

                // Write the resampled/last-known sample to the 44.1k buffer
                buffer[(buffer_offset + i) * 2]     = m_lastSampleL;
                buffer[(buffer_offset + i) * 2 + 1] = m_lastSampleR;
            }

            mSeqState.sample_accumulator += chunk;
            buffer_offset += chunk;
            frames_left -= chunk;
        } else {
            // Not playing: Just generate silent/idle OPL3 samples
            this->generate(&buffer[buffer_offset * 2], frames_left);
            break;
        }
    }
}

// void OPL3Controller::fillBuffer(int16_t* buffer, int total_frames) {
//     if (!mSeqState.playing) {
//         this->generate(buffer, total_frames);
//         return;
//     }
//
//     int frames_left = total_frames;
//     int buffer_offset = 0;
//
//     while (frames_left > 0) {
//         // Calculate remaining samples for the current tick
//         double samples_needed = mSeqState.samples_per_tick - mSeqState.sample_accumulator;
//
//         if (samples_needed <= 0.0) {
//             this->tickSequencer();
//             // Subtract the length of one tick to keep precision
//             mSeqState.sample_accumulator -= mSeqState.samples_per_tick;
//
//             // Re-calculate samples_needed for the NEW tick
//             samples_needed = mSeqState.samples_per_tick - mSeqState.sample_accumulator;
//         }
//
//         // Determine how much we can generate in this pass
//         int chunk = std::min((int)frames_left, (int)std::max(1.0, samples_needed));
//
//         this->generate(&buffer[buffer_offset * 2], chunk);
//
//         mSeqState.sample_accumulator += chunk;
//         buffer_offset += chunk;
//         frames_left -= chunk;
//     }
// }
//
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
            bool noteTrigger = (step.note <= LAST_NOTE || step.note == STOP_NOTE);
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
                    for (int i = 0; i < MAX_CHANNELS; ++i) {
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

    // 2. Enable OPL3 extensions (Bank 1 access)
    //XXTH TEST write(0x105, 0x01);

    // 3. Enable Waveform Select (Global for all 18 channels)
    write(0x01, 0x20);

    // 4. Disable 4-Operator modes initially
    write(0x104, 0x00);

    // 5. Set Global Depth and Mode
    write(0xBD, 0xC0);

    // 6. Clear Waveforms (0xE0 range) for BOTH banks
    for (int i = 0x00; i < 22; i++) {
        write(0xE0 + i, 0x00);  // Bank 0
        write(0x1E0 + i, 0x00); // Bank 1
    }

    // 7. Silence all channels
    for(int i = 0; i < 22; i++) {
        // Total Level to 0x3F (silent)
        write(0x40 + i, 0x3F);  // Bank 0
        write(0x140 + i, 0x3F); // Bank 1

        if (i < 9) {
            write(0xB0 + i, 0x00);  // Key-Off Bank 0
            write(0x1B0 + i, 0x00); // Key-Off Bank 1
        }
    }

    mChip->generate(&mOutput); //flush

    // 6. Silence Hardware
    // This sets TL=63 for all 18 channels
    this->silenceAll(true);

    this->initDefaultBank();


    Log("OPL3 Controller Reset: 18 Channels enabled, Shadows cleared, Sequencer at Start.");
}
//------------------------------------------------------------------------------
void OPL3Controller::setChannelVolume(uint8_t channel, uint8_t oplVolume) {
    if (channel >= MAX_CHANNELS) return;
    uint8_t vol = oplVolume & 0x3F;

    // If this is Channel 3, 4, 5, 12, 13, or 14, check if its master is in 4-OP mode
    if ((channel >= 3 && channel <= 5) || (channel >= 12 && channel <= 14)) {
        uint8_t master = channel - 3;
        if (readShadow(0x104) & (1 << (master % 9))) {
            return; // Skip volume update for slave channel!
        }
    }

    // Determine if this channel is part of a 4-OP pair
    // 4-OP pairs are enabled via register 0x104 (bits 0-5 for channels 0,1,2,9,10,11)
    uint8_t fourOpEnabled = readShadow(0x104);
    bool isFourOpMaster = (channel < 3 || (channel >= 9 && channel < 12)) && (fourOpEnabled & (1 << (channel % 9)));
    bool isFourOpSlave = (channel >= 3 && channel < 6) || (channel >= 12 && channel < 15);

    // If this is a slave channel (3,4,5,12,13,14) in 4-OP mode, do nothing.
    // The master channel volume call will handle all 4 operators.
    if (isFourOpSlave) {
        uint8_t masterChannel = channel - 3;
        if (fourOpEnabled & (1 << (masterChannel % 9))) return;
    }

    if (isFourOpMaster) {
        // 4-OP Volume Logic
        // You must scale the output operators based on the 4-OP connection bits in C0 and C3
        uint16_t c0Addr = (channel < 9 ? 0x000 : 0x100) + 0xC0 + (channel % 9);
        uint16_t c3Addr = c0Addr + 3; // The paired channel is always +3

        uint8_t conn0 = readShadow(c0Addr) & 0x01;
        uint8_t conn1 = readShadow(c3Addr) & 0x01;

        // There are 4 possible 4-OP configurations (algorithms)
        // Rule: Only operators that directly output to the mixer should be scaled.

        // Always scale Carrier 2 (The final operator in the chain)
        updateOpVolume(channel + 3, true, vol); // Operator Pair 1, Carrier

        if (conn0 == 1 && conn1 == 1) { // 4-OP Additive: Scale all 4
            updateOpVolume(channel, false, vol); // Mod 1
            updateOpVolume(channel, true, vol);  // Car 1
            updateOpVolume(channel + 3, false, vol); // Mod 2
        } else if (conn0 == 1 && conn1 == 0) { // 2-OP FM + 2-OP FM
            updateOpVolume(channel, true, vol);  // Car 1
        }
        // ... add other algorithm cases as needed
    } else {

        // 2 OP
        uint16_t mod_off = get_modulator_offset(channel);
        uint16_t car_off = get_carrier_offset(channel);

        // 2. Update Carrier (Operator 1)
        // Preservation of KSL (bits 6-7) is critical to maintain instrument scaling
        uint8_t carReg = readShadow(0x40 + car_off);
        write(0x40 + car_off, (carReg & 0xC0) | vol, true); //FIXME

        // 3. Update Modulator (Operator 0)
        // Only applied if in Additive mode (Connection Bit 0 of $C0 is set)
        uint16_t bank = (channel < 9) ? 0x000 : 0x100;
        uint16_t c0Addr = bank + 0xC0 + (channel % 9);
        bool isAdditive = (readShadow(c0Addr) & 0x01);

        if (isAdditive) {
            uint8_t modReg = readShadow(0x40 + mod_off);
            write(0x40 + mod_off, (modReg & 0xC0) | vol, true);//FIXME
        }

    }
}



/**
 * @param oplVolume Attenuation level (0 = Max Volume, 63 = Muted)
 */
// void OPL3Controller::setChannelVolume(uint8_t channel, uint8_t oplVolume) {
//     // 1. Hardware Boundary Checks
//     if (channel >= MAX_CHANNELS) return;
//
//
//     if (oplVolume > 63) oplVolume = 63;
//     uint8_t vol = oplVolume & 0x3F;
//
//     // if (mSeqState.rowIdx == 33){
//     //     assert(false);
//     // }
//
//     // dLog("setChannelVolume: chan:%d row:%d oplVolume: %d ", channel, mSeqState.rowIdx, oplVolume); //FIXME remove this line
//
//
//     uint16_t mod_off = get_modulator_offset(channel);
//     uint16_t car_off = get_carrier_offset(channel);
//
//     // 2. Update Carrier (Operator 1)
//     // Preservation of KSL (bits 6-7) is critical to maintain instrument scaling
//     uint8_t carReg = readShadow(0x40 + car_off);
//     write(0x40 + car_off, (carReg & 0xC0) | vol);
//
//     // 3. Update Modulator (Operator 0)
//     // Only applied if in Additive mode (Connection Bit 0 of $C0 is set)
//     uint16_t bank = (channel < 9) ? 0x000 : 0x100;
//     uint16_t c0Addr = bank + 0xC0 + (channel % 9);
//     bool isAdditive = (readShadow(c0Addr) & 0x01);
//
//     if (isAdditive) {
//         uint8_t modReg = readShadow(0x40 + mod_off);
//         write(0x40 + mod_off, (modReg & 0xC0) | vol);
//     }
// }
//------------------------------------------------------------------------------
bool OPL3Controller::applyInstrument(uint8_t channel, uint8_t instrumentIndex) {
    if (channel >= 18 || instrumentIndex >= mSoundBank.size()) return false;
    const auto& ins = mSoundBank[instrumentIndex];


    bool isFourOp = ins.isFourOp;

    // 1. Handle 4-Operator Enable Bit (Register 0x104)
    // Bits 0,1,2 = Bank 0 (Ch 0,1,2); Bits 3,4,5 = Bank 1 (Ch 9,10,11)
    uint8_t fourOpReg = readShadow(0x104);
    uint8_t fourOpBit = 0;
    bool isMasterChannel = false;

    if (channel < 3) {
        fourOpBit = (1 << channel);
        isMasterChannel = true;
    } else if (channel >= 9 && channel < 12) {
        fourOpBit = (1 << (channel - 9 + 3));
        isMasterChannel = true;
    }

    if (isMasterChannel) {
        if (isFourOp) write(0x104, fourOpReg | fourOpBit);
        else          write(0x104, fourOpReg & ~fourOpBit);
    }

    // 2. Set Pair 0 (Modulator and Carrier)
    setOperatorRegisters(get_modulator_offset(channel), ins.pairs[0].ops[0]);
    setOperatorRegisters(get_carrier_offset(channel),   ins.pairs[0].ops[1]);

    // $C0 Register for Pair 0
    uint8_t c0Val = (ins.pairs[0].panning << 4) | (ins.pairs[0].feedback << 1) | ins.pairs[0].connection;
    writeChannelReg(0xC0, channel, c0Val);

    // 3. Set Pair 1 (If 4-Op instrument and valid 4-Op slot)
    if (isFourOp && isMasterChannel) {
        uint8_t linkedChannel = channel + 3;
        setOperatorRegisters(get_modulator_offset(linkedChannel), ins.pairs[1].ops[0]);
        setOperatorRegisters(get_carrier_offset(linkedChannel),   ins.pairs[1].ops[1]);

        // $C0 Register for Pair 1
        uint8_t c0LinkedVal = (ins.pairs[1].panning << 4) | (ins.pairs[1].feedback << 1) | ins.pairs[1].connection;
        writeChannelReg(0xC0, linkedChannel, c0LinkedVal);
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
    if (step.note == NONE_NOTE) {
        // TODO: keep it and revisit it for later optimation
        if (step.volume <= MAX_VOLUME &&  step.volume != prevStep.volume) {
            // dLog("Setting volume on NONE_NOTE step: %d ", step.volume);
            setChannelVolume(channel, getOplVol(step.volume));
        }
        // if (step.panning != prevStep.panning) {
        //     setChannelPanning(channel, step.panning);
        // }
        return true;
    }

    // --- CASE 2: NOTE OFF ---
    if (step.note == STOP_NOTE) {
        stopNote(channel);
        return true;
    }

    // --- CASE 3: TRIGGER NEW NOTE ---
    stopNote(channel);

    if (step.instrument >= mSoundBank.size()) return false;
    applyInstrument(channel, step.instrument);
    const auto& currentIns = mSoundBank[step.instrument];

    setChannelVolume(channel, getOplVol(step.volume));
    setChannelPanning(channel, step.panning);

    // 1. Calculate Internal Note (MIDI 0 = C-X)
    // Since mGlobalNoteOffSet is 0, internalNote = step.note + instrument offset
    int internalNote = (int)step.note + currentIns.noteOffset;

    // 2. Derive Initial Block and Note Index
    // Note: OPL3 Block 0 starts roughly at MIDI 12.
    // If internalNote is < 12, it will result in Block 0 or negative.
    int block = internalNote / 12;
    int noteIndex = internalNote % 12;

    // Handle negative notes (C-X range) to keep index 0-11
    if (internalNote < 0) {
        noteIndex = (12 + (internalNote % 12)) % 12;
        block = (internalNote - 11) / 12;
    }

    // Initial fnum is high (e.g., 517 for C)
    uint32_t fnum = f_numbers[noteIndex];
    fnum = static_cast<uint32_t>(fnum * fineTuneTable[currentIns.fineTune + 128]);

    // Normalize: If the instrument offset or high MIDI note makes fnum > 1023,
    // we shift down and increase the OPL block to keep the pitch correct.
    while (fnum >= 0x400 && block < 7) {
        fnum >>= 1;
        block++;
    }

    // Normalize: If it's too low, shift up and decrease block.
    // This is where Note 0 (C-X) will live if block > 0.
    while (fnum < 0x200 && block > 0) {
        fnum <<= 1;
        block--;
    }


    // 5. Final Hardware Clamps
    // OPL3 cannot go below Block 0. For C-X (Note 0), fnum will naturally be very low.
    if (block < 0) block = 0;
    if (block > 7) block = 7;
    if (fnum > 1023) fnum = 1023;

    playNote(channel, (uint16_t)fnum, (uint8_t)block);

    // <<<<<<<<<<<<<<<<<<<<<<<<<<<<

    return true;
}

// bool OPL3Controller::playNote(uint8_t channel, SongStep step) {
//     if (channel >= MAX_CHANNELS) return false;
//
//     std::lock_guard<std::recursive_mutex> lock(mDataMutex);
//
//     SongStep prevStep = mSeqState.last_steps[channel];
//     mSeqState.last_steps[channel] = step;
//     mSeqState.ui_dirty = true;
//
//     // Helper for hardware volume conversion (0-63 -> 63-0)
//     auto getOplVol = [](uint8_t v) {
//         uint8_t clamped = (v > 63) ? 63 : v;
//         return (uint8_t)(63 - clamped);
//     };
//
//     // --- CASE 1: MODULATION ONLY ---
//     if (step.note == NONE_NOTE) {
//         // TODO: keep it and revisit it for later optimation
//         // if (step.volume != prevStep.volume) {
//         //     setChannelVolume(channel, getOplVol(step.volume));
//         // }
//         // if (step.panning != prevStep.panning) {
//         //     setChannelPanning(channel, step.panning);
//         // }
//         return true;
//     }
//
//     // --- CASE 2: NOTE OFF ---
//     if (step.note == STOP_NOTE) {
//         stopNote(channel);
//         return true;
//     }
//
//     // --- CASE 3: TRIGGER NEW NOTE ---
//     stopNote(channel);
//
//     if (step.instrument >= mSoundBank.size())
//     {
//         Log("[error] Instrument out of bounds!");
//         return false;
//     }
//
//     applyInstrument(channel, step.instrument);
//
//     // Get the instrument for fixedNote and fineTune
//     const auto& currentIns = mSoundBank[step.instrument];
//
//     // Apply converted volume and panning
//     setChannelVolume(channel, getOplVol(step.volume));
//     setChannelPanning(channel, step.panning);
//
//
//     //>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>
//
//     // 1. Base calculation for op2 it must be much lower ! => - 24
//     int internalNote = (step.note - 1) + currentIns.noteOffset + mGlobalNoteOffSet;
//     int block = internalNote / 12;
//     int noteIndex = internalNote % 12;
//
//     // 2. Fetch base F-Number (Octave 0 table: 0x0B5, etc.)
//     uint32_t fnum = opl3::f_numbers[noteIndex];
//
//     // 3. Apply Fine-Tune Table
//     fnum = (uint32_t)(fnum * fineTuneTable[currentIns.fineTune + 128]);
//
//     // 4. THE FIX: Normalize F-Number to the 10-bit range (0x200 - 0x3FF)
//     // If fnum is too high (causing tinnitus), shift it down and increase block
//     while (fnum >= 0x400 && block < 7) {
//         fnum >>= 1;
//         block++;
//     }
//     // If fnum is too low (losing resolution), shift it up and decrease block
//     while (fnum < 0x200 && block > 0) {
//         fnum <<= 1;
//         block--;
//     }
//
//     // 5. Final safety clamps
//     if (block > 7) block = 7;
//     if (fnum > 1023) fnum = 1023;
//
//     // Now call playNote with these values
//
//     playNote(channel, (uint16_t)fnum, (uint8_t)block);
//
//     // <<<<<<<<<<<<<<<<<<<<<<<<<<<<
//
//     return true;
// }

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
    if (channel >= 18) return;
    std::lock_guard<std::recursive_mutex> lock(mDataMutex);

    auto keyOff = [&](uint8_t chan) {
        uint16_t bankOffset = (chan <= 8) ? 0x000 : 0x100;
        uint8_t relativeChan = chan % 9;
        uint16_t regAddr = bankOffset + 0xB0 + relativeChan;
        uint8_t currentB0 = readShadow(regAddr);

        // Clear bit 5 (Key-On) to trigger the release phase
        write(regAddr, currentB0 & ~0x20);
    };

    // Apply Key-Off to the primary channel
    keyOff(channel);

    // Check if this channel is the master of an active 4-Op group

    // TEST not ?!
    // uint8_t fourOpReg = readShadow(0x104);
    // bool isFourOpMaster = false;
    //
    // if (channel < 3 && (fourOpReg & (1 << channel))) {
    //     isFourOpMaster = true;
    // } else if (channel >= 9 && channel < 12 && (fourOpReg & (1 << (channel - 9 + 3)))) {
    //     isFourOpMaster = true;
    // }
    //
    // if (isFourOpMaster) {
    //     // Apply Key-Off to the linked channel (+3) as well
    //     keyOff(channel + 3);
    // }

    // Your critical fix remains at the bottom
    mChip->generate(&mOutput);
}

// void OPL3Controller::stopNote(uint8_t channel) {
//     // Range check for OPL3 (0-17)
//     if (channel < 0 || channel >= 18) {
//         return;
//     }
//
//     std::lock_guard<std::recursive_mutex> lock(mDataMutex);
//
//     // Determine the register bank offset
//     // Channels 0-8  -> Bank 1 (0x000)
//     // Channels 9-17 -> Bank 2 (0x100)
//     uint16_t bankOffset = (channel <= 8) ? 0x000 : 0x100;
//     uint8_t relativeChan = channel % 9;
//     uint16_t regAddr = bankOffset + 0xB0 + relativeChan;
//
//     // Read current value from shadow to preserve Block (Octave) and F-Number bits
//     uint8_t currentB0 = readShadow(regAddr);
//
//     // Clear bit 5 (Key-On) to trigger the release phase
//     // 0x20 = 0010 0000 in binary
//     write(regAddr, currentB0 & ~0x20);
//
//
//
//     // Your critical fix remains at the bottom
//     mChip->generate(&mOutput);
// }
//------------------------------------------------------------------------------
void OPL3Controller::write(uint16_t reg, uint8_t val, bool doLog){
    mShadowRegs[reg] = val;
    mChip->write_address(reg);
    mChip->write_data(val);
    if (doLog) {
        Log("OPL_WRITE %02X=>%02X",reg,val);
    }

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

    // dLog("setChannelPanning: chan:%d row:%d pan:%d ", channel, mSeqState.rowIdx, pan); //FIXME remove this line

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
    LogFMT("  Fixed Note: {}", (ins.fixedNote == NONE_NOTE) ? "None" : std::to_string(ins.fixedNote));
    LogFMT("  Note offset: {}", (int)ins.noteOffset);


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


            // dLog("Slide...channel:%d row:%d step.volume:%d( oplVolume: %d)", channel , mSeqState.rowIdx, currentVol, getOplVol(currentVol)); //FIXME remove this line

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

void OPL3Controller::consoleSongOutput(bool useNumbers, uint8_t upToChannel)
    {
        if (!mSeqState.playing)
            return;

        char buffer[1024]; // Increased size: 18 channels * ~15 chars each + padding
        char *ptr = buffer;

        if (upToChannel > MAX_CHANNELS)
            upToChannel = MAX_CHANNELS;

        if (mSeqState.ui_dirty) {
            // sprintf returns the number of characters written.
            // Use that to advance the pointer.
            ptr += sprintf(ptr, "[%02d:%03d] ", mSeqState.orderIdx, mSeqState.rowIdx);

            for (int ch = 0; ch < upToChannel; ch++) {
                const SongStep& step = mSeqState.last_steps[ch];

                // 1. Note Column
                if (step.note == STOP_NOTE) {
                    ptr += sprintf(ptr, "===");
                } else if (step.note == NONE_NOTE) {
                    ptr += sprintf(ptr, "...");
                } else {
                    if (useNumbers) {
                        ptr += sprintf(ptr, "%02X ", step.note); // Added space for alignment
                    } else {
                        ptr += sprintf(ptr, "%3s", opl3::ValueToNote(step.note).c_str());
                    }
                }

                // 2. Instrument & Volume
                ptr += sprintf(ptr, " %02X %02X", step.instrument, step.volume);

                // 3. Effect Column
                if (step.effectType > 0) {
                    ptr += sprintf(ptr, "|%1X%02X ", step.effectType, step.effectVal);
                } else {
                    ptr += sprintf(ptr, "|... ");
                }

                // Separator between channels
                if (ch == 8) ptr += sprintf(ptr, " | ");
            }

            Log("%s", buffer);
            mSeqState.ui_dirty = false;
        }
    }

void OPL3Controller::TESTChip() {
        SDL_PauseAudioStreamDevice(mStream);
        SDL_SetAudioStreamGetCallback(mStream, NULL, NULL);

        // 1. Hard Reset
        // mChip->reset();


        // 2. Enable OPL3 (Bank 2, Reg 0x05, Bit 0)
        write(0x105, 0x01);

        // 3. Setup a Channel (Channel 0)
        // $20: Multiplier/Vibrato ($20=Mod, $23=Car)
        write(0x20, 0x01); write(0x23, 0x01);
        // $40: Total Level ($40=Mod, $43=Car). 0 is LOUDEST.
        write(0x40, 0x10); write(0x43, 0x00);
        // $60: Attack/Decay. 0xF is INSTANT ATTACK.
        write(0x60, 0xF0); write(0x63, 0xF0);
        // $80: Sustain/Release.
        write(0x80, 0xFF); write(0x83, 0xFF);
        // $C0: Panning (Bits 4-5). 0x30 is BOTH (Center).
        write(0xC0, 0x31); // 0x30 (Pan) | 0x01 (Connection)

        // 4. Trigger Note ($A0/$B0)
        write(0xA0, 0x6B); // F-Number Low
        write(0xB0, 0x31); // 0x20 (Key-On) | 0x10 (Octave 4) | 0x01 (F-Num High)

        // 5. Generate
        mChip->generate(&mOutput);
        // mOutput.data[0...3] should now contain non-zero values.
        LogFMT("TEST OPL3 Chip data: {} {} {} {}",
               mOutput.data[0],
               mOutput.data[1],
               mOutput.data[2],
               mOutput.data[3]);

        // rebind the audio stream!
        SDL_SetAudioStreamGetCallback(mStream, OPL3Controller::audio_callback, this);
        SDL_ResumeAudioStreamDevice(mStream);


    }

opl3::SongData OPL3Controller::createScaleSong(uint8_t instrumentIndex) {
        SongData song;
        song.title = "OPL3 Scale Test";
        song.bpm = 125.0f;
        song.speed = 6;

        // 1. Create a pattern with 32 rows
        Pattern scalePat(32, 18);

        for (int i = 0; i < 8; ++i) {
            // Place a note every 4th row on channel 0
            int row = i * 4;
            SongStep& step = scalePat.steps[row * 18 + 0];

            step.note = 60 + i;
            step.instrument = instrumentIndex; // Ensure your soundbank has at least one instrument
            step.volume = MAX_VOLUME;
        }

        // 3. Add pattern and set order
        song.patterns.push_back(scalePat);
        song.orderList.push_back(0); // Play pattern 0

        return song;
    }

opl3::SongData OPL3Controller::createEffectTestSong(uint8_t ins) {
        SongData song;
        song.title = "OPL3 Effects Stress Test";
        song.bpm = 90.f; //125.0f;
        song.speed = 6;


        // Create pattern
        Pattern testPat(64, 18);

        // --- Channel 0: Volume Slide Test ---
        // Trigger a long note on Row 0
        SongStep& startNote = testPat.steps[0 * 18 + 0];
        startNote.note = 48; // C-3
        startNote.instrument = ins;
        startNote.volume = 63;

        // Starting at Row 1, slide volume DOWN
        for (int r = 1; r < 16; ++r) {
            SongStep& s = testPat.steps[r * 18 + 0];
            s.effectType = EFF_VOL_SLIDE;
            s.effectVal  = 0x04; // Slide down by 4 per tick (0x0y)
        }

        // --- Channel 1: Panning Test ---
        // Trigger a note that jumps across speakers
        for (int i = 0; i < 4; ++i) {
            int row = i * 8;
            SongStep& s = testPat.steps[row * 18 + 1];
            s.note = 60; // C-5
            s.instrument = ins;
            s.volume = 50;

            // Alternate Panning: 0 (Hard Left), 64 (Hard Right)
            s.effectType = EFF_SET_PANNING;
            s.effectVal  = (i % 2 == 0) ? 0 : 64;
        }

        // --- Channel 2: Note-Off Test ---
        // Test if STOP_NOTE correctly stops the OPL3 operators
        for (int i = 0; i < 4; ++i) {
            testPat.steps[(i * 8) * 18 + 2].note = 55;       // G-4 Key-On
            testPat.steps[(i * 8) * 18 + 2].instrument = ins;
            testPat.steps[(i * 8 + 4) * 18 + 2].note = STOP_NOTE;  // Key-Off 4 rows later
        }

        // --- Channel 3: Set Volume Test ---
        // Directly setting volume via effect Cxx
        for (int i = 0; i < 8; ++i) {
            SongStep& s = testPat.steps[(i * 4) * 18 + 3];
            s.note = 52; // E-4
            s.instrument = ins;
            s.effectType = EFF_SET_VOLUME;
            s.effectVal  = (i % 2 == 0) ? 20 : 60; // Alternate quiet/loud
        }


        for (int i = 0; i < MAX_CHANNELS; i++)
            testPat.steps[31 * 18 + i].note = STOP_NOTE;

        // indexing for Row 32, Channel 0
        SongStep& s = testPat.steps[32 * 18 + 0];
        s.note = 52;
        s.instrument = ins;
        s.volume = 63;           // Start at max volume
        s.effectType = EFF_VOL_SLIDE;
        s.effectVal  = 0x08;

        SongStep& s2 = testPat.steps[36 * 18 + 0];
        s2.note = 52;
        s2.instrument = 0;
        s2.volume = 0;
        s2.effectType = EFF_VOL_SLIDE;
        s2.effectVal  = 0x40; // Slides volume UP by 4 units per tick

        testPat.steps[40 * 18 + 0].volume=0;



        song.patterns.push_back(testPat);
        song.orderList.push_back(0);

        return song;
    }

void OPL3Controller::playSong(opl3::SongData& songData) {
        std::lock_guard<std::recursive_mutex> lock(mDataMutex);

        // 1. Assign Song Data
        mSeqState.current_song = &songData;

        // 2. Reset Sequencer Positions
        mSeqState.orderIdx = 0;
        mSeqState.rowIdx = 0;
        mSeqState.current_tick = 0;
        mSeqState.sample_accumulator = 0.f;

        // 3. Reset Channel States using the new struct array
        // This clears all redundancy checks so the first row of the song
        // is guaranteed to write to the hardware registers.
        for (int i = 0; i < 18; ++i) {
            mSeqState.last_steps[i] = {}; // Resets note, vol, pan, effect
            mSeqState.last_steps[i].volume = 255; // Force update on first row
        }

        // 4. Calculate timing
        // In 2026, ensure hostRate is queried from your actual SDL audio device
        double hostRate = 44100.0;
        double ticks_per_sec = songData.bpm * 0.4;

        // 2. Calculate samples per tick
        if (ticks_per_sec > 0) {
            // At 125 BPM: 44100 / 50 = 882 samples per tick
            mSeqState.samples_per_tick = hostRate / ticks_per_sec;
        } else {
            mSeqState.samples_per_tick = hostRate;
        }


        // 5. Hard Reset Chip
        mChip->reset();
        //XXTH TEST: replace write(0x105, 0x01); // RE-ENABLE OPL3 MODE (Mandatory after reset)
        write(0x05, 0x01);

        // 6. Start Playback
        mSeqState.playing = true;
        mSeqState.ui_dirty = true;
    }

void OPL3Controller::playNote(uint8_t channel, uint16_t fnum, uint8_t octave) {
        // Write to the primary channel
        uint16_t bankOffset = (channel <= 8) ? 0x000 : 0x100;
        uint8_t relChan = channel % 9;

        // //XXTH TEST >>>>
        // write(bankOffset + 0xC0 + relChan, 0x03, true);
        // write(0x104, relChan, true);
        // //<<<<


        write(bankOffset + 0xA0 + relChan, fnum & 0xFF);
        write(bankOffset + 0xB0 + relChan, 0x20 | ((octave & 0x07) << 2) | ((fnum >> 8) & 0x03));

        // Check if 4-OP is enabled for this channel via shadow register 0x104
        uint8_t fourOpReg = readShadow(0x104);
        bool isFourOp = false;
        if (channel < 3 && (fourOpReg & (1 << channel))) isFourOp = true;
        else if (channel >= 9 && channel < 12 && (fourOpReg & (1 << (channel - 9 + 3)))) isFourOp = true;

        if (isFourOp) {

            // Sync the linked channel (Channel + 3)
            uint8_t linkedChan = channel + 3;
            uint16_t linkedBankOffset = (linkedChan <= 8) ? 0x000 : 0x100;
            uint8_t linkedRelChan = linkedChan % 9;

            // //XXTH TEST >>>>
            // write(linkedBankOffset + 0xC0 + linkedRelChan, 0x03, true);
            // write(0x104, linkedRelChan, true);
            //
            // //<<<<



            //XXTH TEST write(linkedBankOffset + 0xA0 + linkedRelChan, fnum & 0xFF);
            // We write the same block and fnum, but often 4-op voices
            // keep the linked channel's Key-On bit (0x20) active as well.
            //XXTH TEST write(linkedBankOffset + 0xB0 + linkedRelChan, 0x20 | ((octave & 0x07) << 2) | ((fnum >> 8) & 0x03));

        }



    }

void OPL3Controller::setOperatorRegisters(uint16_t opOffset, const opl3::OplInstrument::OpPair::OpParams& op) {
        // Standard OPL Register layout: 0x20, 0x40, 0x60, 0x80, 0xE0

        // Register 0x20: [AM][VIB][EG-TYP][KSR][ MULTI (4 bits) ]
        uint8_t reg20 = (op.multi   & 0x0F) | // Multiplikator (0-15)
        (op.ksr     ? 0x10 : 0x00) | // Key Scale Rate
        (op.egTyp   ? 0x20 : 0x00) | // EG-Typ (Sustain-Modus)
        (op.vib     ? 0x40 : 0x00) | // Vibrato
        (op.am      ? 0x80 : 0x00);  // Amplitude Modulation (Tremolo)

        write(opOffset + 0x20, reg20);

        // write(opOffset + 0x20, op.multi | (op.ksr << 4) | (op.egTyp << 5) | (op.vib << 6) | (op.am << 7));
        write(opOffset + 0x40, op.tl | (op.ksl << 6));
        write(opOffset + 0x60, op.decay | (op.attack << 4));
        write(opOffset + 0x80, op.release | (op.sustain << 4));
        write(opOffset + 0xE0, op.wave & 0x07);
    }

void OPL3Controller::writeChannelReg(uint16_t baseReg, uint8_t channel, uint8_t value){
       uint16_t bankOffset = (channel <= 8) ? 0x000 : 0x100;
       write(bankOffset + baseReg + (channel % 9), value);
   }
