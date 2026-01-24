//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#include "OPL3Controller.h"
#include "ymfmGlue.h"
#include "opl3_base.h"
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
    m_lastSampleL = 0.f;
    m_lastSampleR = 0.f;

    mF32Buffer.resize(MAX_FRAMES * 2);

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
void SDLCALL OPL3Controller::audio_callback(void* userdata, SDL_AudioStream *stream, int additional_amount, int total_amount) {
    auto* controller = static_cast<OPL3Controller*>(userdata);
    if (!controller || additional_amount <= 0) return;

    // Calculate frames (F32 Stereo = 8 bytes per frame)
    int framesNeeded = additional_amount / 8;
    const int MAX_FRAMES = 2048;
    if (framesNeeded > MAX_FRAMES) framesNeeded = MAX_FRAMES;
    int totalSamples = framesNeeded * 2;

    // Use the pre-allocated member buffer from your class
    // This avoids creating 16KB-24KB on the stack every callback
    float* f32Buffer = controller->mF32Buffer.data();

    // Fill Buffer
    {
        std::lock_guard<std::recursive_mutex> lock(controller->mDataMutex);
        // Note: We call the NEW float version of fillBuffer
        controller->fillBuffer(f32Buffer, framesNeeded);
    }


    // DSP Effects
    if (controller->isAnyVoiceActive())
    {
        for (auto& effect : controller->mDspEffects) {
            effect->process(f32Buffer, totalSamples);
        }
    }

    // Send to SDL3
    SDL_PutAudioStreamData(stream, f32Buffer, framesNeeded * 8);

}

// void SDLCALL OPL3Controller::audio_callback(void* userdata, SDL_AudioStream *stream, int additional_amount, int total_amount) {
//     auto* controller = static_cast<OPL3Controller*>(userdata);
//     if ( !controller || additional_amount == 0 ) return;
//
//     // 1. SDL3 additional_amount is in BYTES.
//     // For F32 Stereo: 1 frame = 8 bytes.
//     int framesNeeded = additional_amount / 8;
//
//     // Use a fixed size or std::vector for the intermediate buffers
//     // 2048 frames * 2 channels = 4096 samples
//     const int MAX_FRAMES = 2048;
//     if (framesNeeded > MAX_FRAMES) framesNeeded = MAX_FRAMES;
//
//     int16_t s16Buffer[MAX_FRAMES * 2];
//     float f32Buffer[MAX_FRAMES * 2];
//
//     {
//         std::lock_guard<std::recursive_mutex> lock(controller->mDataMutex);
//         // Fill using your legacy int16_t logic
//         controller->fillBuffer(s16Buffer, framesNeeded);
//     }
//
//     // 2. Convert S16 legacy data to F32 for the DSP effects
//     int totalSamples = framesNeeded * 2;
//     const float inv32768 = 1.0f / 32768.0f;
//     for (int i = 0; i < totalSamples; ++i) {
//         f32Buffer[i] = s16Buffer[i] * inv32768;
//     }
//
//     // DSP
//     for (auto& effect : controller->mDspEffects) {
//         effect->process(f32Buffer, totalSamples);
//     }
//
//     // 4. Put the processed FLOAT data into the stream
//     // The byte size is framesNeeded * 8 (or totalSamples * 4)
//     SDL_PutAudioStreamData(stream, f32Buffer, framesNeeded * 8);
// }
//

//------------------------------------------------------------------------------
bool OPL3Controller::initController()
{
    SDL_AudioSpec spec;
    spec.format = SDL_AUDIO_F32;  // SDL_AUDIO_S16;
    spec.channels = 2;
    spec.freq = 44100;
    mStream = SDL_CreateAudioStream(&spec, &spec);


    //-----

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

    // Digital post processing
    //------------------------
    // warmth:
    auto warmth = std::make_unique<DSP::Warmth>(false);
    mDSPWarmth = warmth.get();
    mDspEffects.push_back(std::move(warmth));
    //------------------------

    // bitcrusher:
    auto bitcrusher = std::make_unique<DSP::Bitcrusher>(false);
    mDSPBitCrusher = bitcrusher.get();
    mDspEffects.push_back(std::move(bitcrusher));
    //------------------------
    // Chrous:
    auto chorus = std::make_unique<DSP::Chorus>(false);
    mDSPChorus = chorus.get();
    mDspEffects.push_back(std::move(chorus));

    //------------------------
    // Reverb:
    auto reverb = std::make_unique<DSP::Reverb>(false);
    mDSPReverb = reverb.get();
    mDspEffects.push_back(std::move(reverb));
    //------------------------
    // 9Band:
    auto eq9band = std::make_unique<DSP::Equalizer9Band>(false);
    mEquilzer9Band = eq9band.get();
    mDspEffects.push_back(std::move(eq9band));

    //------------------------
    //Limiter Last !
    auto limiter = std::make_unique<DSP::Limiter>(false);
    mLimiter = limiter.get();
    mDspEffects.push_back(std::move(limiter));
    // ------------------------

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
bool OPL3Controller::checkAnyVoiceActive(float* buffer, int total_frames) {
    // 1. If sequencer is playing, we are ALWAYS active.
    if (mSeqState.playing) return true;

    // 2. Wake-up override (from playNoteByFNumHW)
    if (isAnyVoiceActive()) {

        // 3. Scan the buffer we JUST filled to see if there is any sound
        bool bufferHasSound = false;
        const float EPSILON = 0.00001f; // Ignore digital floor noise

        for (int i = 0; i < total_frames * 2; ++i) {
            if (std::abs(buffer[i]) > EPSILON) {
                bufferHasSound = true;
                break;
            }
        }

        if (bufferHasSound) {
            mSilenceCounter = 0;
            return true;
        }

        // 4. If the buffer was silent, increment counter
        mSilenceCounter += total_frames;

        if (mSilenceCounter > SILENCE_THRESHOLD) {
            mIsSilent = true;
            dLog("OPL3Controller enter Sleep Mode");
        }
        return true;
    }
    return false;
}
//------------------------------------------------------------------------------
void OPL3Controller::generate(float* buffer, int frames) {
    const float inv32768 = 1.0f / 32768.0f;
    for (int i = 0; i < frames; ++i) {
        mChip->generate(&mOutput);

        // Convert to float while mixing
        float L = (mOutput.data[0] + mOutput.data[2]) * inv32768;
        float R = (mOutput.data[1] + mOutput.data[3]) * inv32768;

        // Direct write to float buffer
        *buffer++ = L;
        *buffer++ = R;
    }
}
//------------------------------------------------------------------------------
void OPL3Controller::fillBuffer(float* buffer, int total_frames)
{
    const float inv32768 = 1.0f / 32768.0f;
    int buffer_offset = 0;

    if (!isAnyVoiceActive() && !mSeqState.playing) {
        std::memset(buffer, 0, total_frames * sizeof(float) * 2);
        return;
    }

    // If not playing a song, but manual notes are active
    if (!mSeqState.playing) {
        this->generate(buffer, total_frames); // Refactored generate() below
        this->checkAnyVoiceActive(buffer, total_frames);
        return;
    }

    int frames_left = total_frames;
    double step = this->getStep();

    while (frames_left > 0) {
        // 1. Calculate how many samples until the next sequencer tick
        double samples_until_tick = mSeqState.samples_per_tick - mSeqState.sample_accumulator;
        int chunk = std::min(frames_left, (int)std::max(1.0, samples_until_tick));

        // 2. Generate and Resample this chunk
        for (int i = 0; i < chunk; i++) {
            m_opl3_accumulator += step;

            // Generate new OPL3 data only when needed
            while (m_opl3_accumulator >= 1.0) {
                mChip->generate(&mOutput);
                // Mix and convert to float IMMEDIATELY
                // OPL3 uses 4-channel output usually mapped: 0+2 = Left, 1+3 = Right
                m_lastSampleL = (mOutput.data[0] + mOutput.data[2]) * inv32768;
                m_lastSampleR = (mOutput.data[1] + mOutput.data[3]) * inv32768;
                m_opl3_accumulator -= 1.0;
            }
            // Write the stored float values to the buffer
            buffer[(buffer_offset + i) * 2]     = m_lastSampleL;
            buffer[(buffer_offset + i) * 2 + 1] = m_lastSampleR;
        }

        // 3. Update Sequencer state
        mSeqState.sample_accumulator += chunk;
        if (mSeqState.sample_accumulator >= mSeqState.samples_per_tick) {
            this->tickSequencer();
            mSeqState.sample_accumulator -= mSeqState.samples_per_tick;
        }

        buffer_offset += chunk;
        frames_left -= chunk;
    }
}


//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
// void OPL3Controller::generate(int16_t* buffer, int frames) {
//     for (int i = 0; i < frames; ++i) {
//         // 1. Ask ymfm to compute one sample of data
//         // This populates mOutput.data[] with the current chip state
//         mChip->generate(&mOutput);
//
//         // 2. Interleave the Left and Right channels into your int16_t buffer
//         // OPL3 standard output: data[0] = Left, data[1] = Right
//         for (int chan = 0; chan < 2; ++chan) {
//             int32_t sample = mOutput.data[chan];
//             //raise volume distortion sound bad :P
//             // sample = mOutput.data[chan] * 256;
//
//             // 3. Optional: Manual Clamping for 16-bit safety
//             if (sample > 32767)  sample = 32767;
//             if (sample < -32768) sample = -32768;
//
//             *buffer++ = static_cast<int16_t>(sample);
//         }
//     }
// }
// //------------------------------------------------------------------------------
// void OPL3Controller::fillBuffer(int16_t* buffer, int total_frames)
// {
//     //for playTone without a song:
//     if (!mSeqState.playing) {
//         this->generate(buffer, total_frames);
//         return;
//     }
//
//     int frames_left = total_frames;
//     int buffer_offset = 0;
//
//     // The ratio of OPL3 native rate to your output rate (49715 / 44100)
//     double step = this->getStep();
//
//     while (frames_left > 0) {
//         if (mSeqState.playing) {
//             // Check if we need to tick the sequencer
//             double samples_needed = mSeqState.samples_per_tick - mSeqState.sample_accumulator;
//
//             if (samples_needed <= 0.0) {
//                 this->tickSequencer(); // Effects/Notes processed here
//                 mSeqState.sample_accumulator -= mSeqState.samples_per_tick;
//                 samples_needed = mSeqState.samples_per_tick;
//             }
//
//             // Chunk calculation for your effects (e.g., 6 ticks per row)
//             int chunk = std::min((int)frames_left, (int)std::max(1.0, samples_needed));
//
//             // Generate OPL3 samples with resampling for this chunk
//             for (int i = 0; i < chunk; i++) {
//                 m_opl3_accumulator += step;
//
//                 // If we've accumulated enough "time" for an OPL3 sample, generate it
//                 while (m_opl3_accumulator >= 1.0) {
//
//                     mChip->generate(&mOutput);
//                     // Mix 4-channel OPL3 to Stereo
//                     m_lastSampleL = mOutput.data[0] + mOutput.data[2];
//                     m_lastSampleR = mOutput.data[1] + mOutput.data[3];
//
//                     m_opl3_accumulator -= 1.0;
//                 }
//
//                 // Write the resampled/last-known sample to the 44.1k buffer
//                 buffer[(buffer_offset + i) * 2]     = m_lastSampleL;
//                 buffer[(buffer_offset + i) * 2 + 1] = m_lastSampleR;
//             }
//
//             mSeqState.sample_accumulator += chunk;
//             buffer_offset += chunk;
//             frames_left -= chunk;
//         } else {
//             // Not playing: Just generate silent/idle OPL3 samples
//             this->generate(&buffer[buffer_offset * 2], frames_left);
//             break;
//         }
//     }
// }
//------------------------------------------------------------------------------
void OPL3Controller::tickSequencer() {
    if (!mSeqState.playing || !mSeqState.current_song) return;

    const SongData& song = *mSeqState.current_song;

    // 1. Safety check: Is the order index valid?
    if (mSeqState.orderIdx >= song.orderList.size()) {
        Log("[error] sequence index exceeded sequence list!");
        mSeqState.playing = false; // Stop playback if we've run out of orders
        return;
    }

    // 2. Safety check: Is the pattern index in the order list valid?
    uint32_t patternIdx = song.orderList[mSeqState.orderIdx];
    if (patternIdx >= song.patterns.size()) {
        LogFMT("[error] Invalid pattern index {} at order {}", patternIdx, mSeqState.orderIdx);
        mSeqState.playing = false;
        return;
    }

    const Pattern& pat = song.patterns[song.orderList[mSeqState.orderIdx]];

    if (mSeqState.rowIdx >= pat.getRowCount()) {
        mSeqState.rowIdx = 0;
        return;
    }


    //
    for (uint8_t softChan = 0; softChan < SOFTWARE_CHANNEL_COUNT; ++softChan) {
        const SongStep& step = pat.getStep(mSeqState.rowIdx, softChan);

        if (mSeqState.current_tick == 0) {
            // --- Tick 0: New Row Trigger ---

            // Logic: Trigger if there is a note, OR if the step contains
            // data (volume/pan) that differs from the current channel state.
            bool noteTrigger = (step.note <= LAST_NOTE || step.note == STOP_NOTE);
            bool volumeChange = (step.volume != mSeqState.last_steps[softChan].volume);
            bool panningChange = (step.panning != mSeqState.last_steps[softChan].panning);

            if (noteTrigger || volumeChange || panningChange) {
                // playNote handles Note 0 (modulation only) vs Note 1-255 (trigger/stop)
                this->playNote(softChan, step);

            } else if (step.note == NONE_NOTE ) {
                mSeqState.last_steps[getHardWareChannel(softChan)]=step;
                mSeqState.ui_dirty = true;
            }

            // Process non-continuous effects that start on Tick 0 (like Position Jump)
            if (step.effectType != 0) {
                this->processStepEffects(getHardWareChannel(softChan), step);
            }
        } else {
            // --- Ticks 1+: Continuous Effects ---
            if (step.effectType != 0) {
                this->processStepEffects(getHardWareChannel(softChan), step);
            }
        }
    }

    //pre softwareChannel
//     for (int ch = 0; ch < MAX_HW_CHANNELS; ++ch) {
//         const SongStep& step = pat.getStep(mSeqState.rowIdx, ch);    //mSteps[mSeqState.rowIdx * MAX_HW_CHANNELS + ch];
//
//         if (mSeqState.current_tick == 0) {
//             // --- Tick 0: New Row Trigger ---
//
//             // Logic: Trigger if there is a note, OR if the step contains
//             // data (volume/pan) that differs from the current channel state.
//             bool noteTrigger = (step.note <= LAST_NOTE || step.note == STOP_NOTE);
//             bool volumeChange = (step.volume != mSeqState.last_steps[ch].volume);
//             bool panningChange = (step.panning != mSeqState.last_steps[ch].panning);
//
//             if (noteTrigger || volumeChange || panningChange) {
//                 // playNote handles Note 0 (modulation only) vs Note 1-255 (trigger/stop)
//                 this->playNoteHW(ch, step);
//
//
//             } else if (step.note == NONE_NOTE ) {
//                 mSeqState.last_steps[ch]=step;
//                 mSeqState.ui_dirty = true;
//             }
//
//             // Process non-continuous effects that start on Tick 0 (like Position Jump)
//             if (step.effectType != 0) {
//                 this->processStepEffects(ch, step);
//             }
//         } else {
//             // --- Ticks 1+: Continuous Effects ---
//             if (step.effectType != 0) {
//                 this->processStepEffects(ch, step);
//             }
//         }
//     }

    // --- Timing Advancement ---
    mSeqState.current_tick++;

    if (mSeqState.current_tick >= mSeqState.ticks_per_row) {
        mSeqState.current_tick = 0;
        mSeqState.rowIdx++;

        if (mSeqState.rowIdx >= pat.getRowCount()) {
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
                    for (int i = 0; i < MAX_HW_CHANNELS; ++i) {
                        mSeqState.last_steps[i] = {};
                    }
                    mSeqState.ui_dirty = true;
                }
            }
        }
    }
}

//------------------------------------------------------------------------------
uint16_t OPL3Controller::get_modulator_offset(uint8_t channel) {
    static const uint8_t op_offsets[] = {
        0x00, 0x01, 0x02, 0x08, 0x09, 0x0A, 0x10, 0x11, 0x12
    };

    if (channel >= MAX_HW_CHANNELS) return 0;

    // IMPORTANT: Only return the relative offset (0x00 - 0x12)
    // Do NOT add 0x100 here if you are also adding it in applyInstrumentHW
    uint8_t relative_chan = channel % 9;
    return op_offsets[relative_chan];
}

// uint16_t OPL3Controller::get_modulator_offset(uint8_t channel) {
//     static const uint8_t op_offsets[] = {
//         0x00, 0x01, 0x02, 0x08, 0x09, 0x0A, 0x10, 0x11, 0x12
//     };
//
//     if (channel >= MAX_HW_CHANNELS) return 0;
//
//     // Use the BANK_LIMIT to decide between $000 and $100
//     uint16_t base = (channel <= opl3::BANK_LIMIT) ? 0x000 : 0x100;
//     uint8_t relative_chan = channel % 9;
//
//     return base + op_offsets[relative_chan];
// }

uint16_t OPL3Controller::get_carrier_offset(uint8_t channel) {
    if (channel >= MAX_HW_CHANNELS) return 0; // Out of range
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
    for (int i = 0; i < MAX_HW_CHANNELS; ++i) {
        stopNoteHW(i); // Sends Key-Off to 0xB0 + channel_offset

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
    for (int i = 0; i < SOFTWARE_CHANNEL_COUNT; i++)
        mChannelToNote[i] = -1;
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

    // // 2. Enable OPL3 extensions (Bank 1 access)
    // // write(0x105, 0x01); // OPL3 Mode enabled ?!
    // write(0x005, 0x01); // OPL3 Mode enabled ?!
    //
    // // 3. Enable Waveform Select (Global for all 18 channels)
    // write(0x01, 0x20);
    //
    // // 4. Disable 4-Operator modes initially
    // write(0x104, 0x00);
    //
    // // 5. Set Global Depth and Mode
    // write(0xBD, 0xC0);
    //
    // OP4 Mode by default :
    // 1. Reset / Clear Registers (Standard OPL practice)

    // Enable OPL3 extensions (BANK 1, Register 0x05)
    write(0x105, 0x01);

    // Enable Waveform Select (Bank 0, Register 0x01)
    write(0x01, 0x20);

    // Configure 4-Operator modes (Bank 1, Register 0x04)
    write(0x104, 0x00);

    // Set Global Depth and Mode (Bank 0, Register 0xBD)
    write(0xBD, 0xC0);

    // 6. Clear Waveforms (0xE0 range) for BOTH banks
    for (int i = 0x00; i < 22; i++) {
        write(0xE0 + i, 0x00);  // Bank 0
        write(0x1E0 + i, 0x00); // Bank 1
    }

    // Silence
    for (int i = 0; i < 0x16; i++) { // Clear 22 registers per block
        // Total Level (0x40 range)
        write(0x40 + i, 0x3F);  write(0x140 + i, 0x3F);
        // Attack/Decay (0x60 range)
        write(0x60 + i, 0x00);  write(0x160 + i, 0x00);
    }

    for (int i = 0; i < 9; i++) {
        write(0xB0 + i, 0x00);  write(0x1B0 + i, 0x00); // Key-Off
        // Set Default Output to L+R for all 18 channels
        write(0xC0 + i, 0x30);  write(0x1C0 + i, 0x30);
    }



    // 6. Silence Hardware
    // This sets TL=63 for all 18 channels
    this->silenceAll(true);

    this->initDefaultBank();



    Log("OPL3 Controller Reset: 18 Channels enabled, Shadows cleared, Sequencer at Start.");
}
//------------------------------------------------------------------------------
void OPL3Controller::setChannelVolume(uint8_t channel, uint8_t oplVolume) {
    if (channel >= MAX_HW_CHANNELS) return;
    uint8_t vol = oplVolume & 0x3F;
    uint16_t bank = (channel < 9) ? 0x000 : 0x100;

    // 1. Identify 4-OP state using your mapping
    bool isFourOpMaster = ChannelMapping[channel].isOP4Master;
    bool isFourOpSlave = ChannelMapping[channel].isOP4Slave;
    uint8_t fourOpReg = readShadow(0x104);

    if (isFourOpSlave) {
        uint8_t masterChannel = channel - 3;
        uint8_t bit = (masterChannel < 3) ? masterChannel : (masterChannel - 9 + 3);
        if (fourOpReg & (1 << bit)) return; // Controlled by master
    }

    if (isFourOpMaster) {
        uint8_t bit = (channel < 3) ? channel : (channel - 9 + 3);
        if (fourOpReg & (1 << bit)) {
            // Logic for 4-OP algorithm scaling...
            // Ensure updateOpVolume handles bank offsets!
            updateOpVolume(channel + 3, true, vol);
            // ... (rest of your algorithm logic)
            return;
        }
    }

    // 2-OP Logic (Now with fixed Bank addressing)
    uint16_t mod_off = get_modulator_offset(channel);
    uint16_t car_off = get_carrier_offset(channel);

    // Update Carrier (Always)
    uint8_t carReg = readShadow(bank + 0x40 + car_off); // Added bank!
    write(bank + 0x40 + car_off, (carReg & 0xC0) | vol);

    // Update Modulator (Only if Additive)
    uint16_t c0Addr = bank + 0xC0 + (channel % 9);
    if (readShadow(c0Addr) & 0x01) {
        uint8_t modReg = readShadow(bank + 0x40 + mod_off); // Added bank!
        write(bank + 0x40 + mod_off, (modReg & 0xC0) | vol);
    }
}

// void OPL3Controller::setChannelVolume(uint8_t channel, uint8_t oplVolume) {
//     if (channel >= MAX_HW_CHANNELS) return;
//     uint8_t vol = oplVolume & 0x3F;
//
//     // If this is Channel 3, 4, 5, 12, 13, or 14, check if its master is in 4-OP mode
//     if ((channel >= 3 && channel <= 5) || (channel >= 12 && channel <= 14)) {
//         uint8_t master = channel - 3;
//         if (readShadow(0x104) & (1 << (master % 9))) {
//             return; // Skip volume update for slave channel!
//         }
//     }
//
//     // Determine if this channel is part of a 4-OP pair
//     // 4-OP pairs are enabled via register 0x104 (bits 0-5 for channels 0,1,2,9,10,11)
//     uint8_t fourOpEnabled = readShadow(0x104);
//
//     // bool isFourOpMaster = (channel < 3 || (channel >= 9 && channel < 12)) && (fourOpEnabled & (1 << (channel % 9)));
//     // bool isFourOpSlave = (channel >= 3 && channel < 6) || (channel >= 12 && channel < 15);
//
//     bool isFourOpMaster = ChannelMapping[channel].isOP4Master;
//     bool isFourOpSlave = ChannelMapping[channel].isOP4Slave;
//
//     // If this is a slave channel (3,4,5,12,13,14) in 4-OP mode, do nothing.
//     // The master channel volume call will handle all 4 operators.
//     if (isFourOpSlave) {
//         uint8_t masterChannel = channel - 3;
//         if (fourOpEnabled & (1 << (masterChannel % 9))) return;
//     }
//
//     if (isFourOpMaster) {
//         // 4-OP Volume Logic
//         // You must scale the output operators based on the 4-OP connection bits in C0 and C3
//         uint16_t c0Addr = (channel < 9 ? 0x000 : 0x100) + 0xC0 + (channel % 9);
//         uint16_t c3Addr = c0Addr + 3; // The paired channel is always +3
//
//         uint8_t conn0 = readShadow(c0Addr) & 0x01;
//         uint8_t conn1 = readShadow(c3Addr) & 0x01;
//
//         // There are 4 possible 4-OP configurations (algorithms)
//         // Rule: Only operators that directly output to the mixer should be scaled.
//
//         // Always scale Carrier 2 (The final operator in the chain)
//         updateOpVolume(channel + 3, true, vol); // Operator Pair 1, Carrier
//
//         if (conn0 == 1 && conn1 == 1) { // 4-OP Additive: Scale all 4
//             updateOpVolume(channel, false, vol); // Mod 1
//             updateOpVolume(channel, true, vol);  // Car 1
//             updateOpVolume(channel + 3, false, vol); // Mod 2
//         } else if (conn0 == 1 && conn1 == 0) { // 2-OP FM + 2-OP FM
//             updateOpVolume(channel, true, vol);  // Car 1
//         }
//         // ... add other algorithm cases as needed
//     } else {
//
//         // 2 OP
//         uint16_t mod_off = get_modulator_offset(channel);
//         uint16_t car_off = get_carrier_offset(channel);
//
//         // 2. Update Carrier (Operator 1)
//         // Preservation of KSL (bits 6-7) is critical to maintain instrument scaling
//         uint8_t carReg = readShadow(0x40 + car_off);
//         write(0x40 + car_off, (carReg & 0xC0) | vol);
//
//         // 3. Update Modulator (Operator 0)
//         // Only applied if in Additive mode (Connection Bit 0 of $C0 is set)
//         uint16_t bank = (channel < 9) ? 0x000 : 0x100;
//         uint16_t c0Addr = bank + 0xC0 + (channel % 9);
//         bool isAdditive = (readShadow(c0Addr) & 0x01);
//
//         if (isAdditive) {
//             uint8_t modReg = readShadow(0x40 + mod_off);
//             write(0x40 + mod_off, (modReg & 0xC0) | vol);
//         }
//
//     }
// }
//------------------------------------------------------------------------------
// bool OPL3Controller::applyInstrumentSW(uint8_t softwareChannel, uint8_t instrumentIndex) {
//     if ( instrumentIndex >= mSoundBank.size())
//         return false;
//
//     return this->applyInstrumentHW(getHardWareChannel(softwareChannel), instrumentIndex);
// }
//------------------------------------------------------------------------------
bool OPL3Controller::applyInstrumentHW(uint8_t channel, uint8_t instrumentIndex) {
    if (channel >= MAX_HW_CHANNELS || instrumentIndex >= mSoundBank.size()) return false;


    // 1. Get Mapping and Instrument
    const auto& mapping = ChannelMapping[channel];
    // if (!mapping || mapping->isOP4Slave) return false;

    const auto& ins = mSoundBank[instrumentIndex];
    uint16_t bank = (channel < 9) ? 0x000 : 0x100;

    // 2. Handle 4-OP Register (0x104)
    if (mapping.isOP4Master) {
        uint8_t bit = (channel < 3) ? channel : (channel - 9 + 3);
        uint8_t reg = readShadow(0x104);

        if (ins.isFourOp) {
            write(0x104, reg | (1 << bit));
        } else {
            // CRITICAL: You must clear this bit for 2-OP instruments
            // or the slave channels (like 4 or 13) will be "captured"
            write(0x104, reg & ~(1 << bit));
        }
    }

    // 3. Pair 0 (Primary Operators)
    // Make sure your offset helpers return 0x00, 0x01, 0x02... NOT 9, 10, 11
    uint8_t modOff = get_modulator_offset(channel);
    uint8_t carOff = get_carrier_offset(channel);

    // IMPORTANT: setOperatorRegisters MUST add 'bank' to the base (0x20, 0x40, etc)
    setOperatorRegisters(bank, modOff, ins.pairs[0].ops[0]);
    setOperatorRegisters(bank, carOff, ins.pairs[0].ops[1]);

    uint8_t c0Val = (ins.pairs[0].panning << 4) | (ins.pairs[0].feedback << 1) | ins.pairs[0].connection;
    writeChannelRegHW(0xC0, channel, c0Val);

    // 4. Pair 1 (Slave Operators)
    if (mapping.isOP4Master) {
        uint8_t linkedChannel = channel + 3;
        uint16_t linkedBank = (linkedChannel < 9) ? 0x000 : 0x100;

        if (ins.isFourOp || ins.isDoubleVoice) {
            setOperatorRegisters(linkedBank, get_modulator_offset(linkedChannel), ins.pairs[1].ops[0]);
            setOperatorRegisters(linkedBank, get_carrier_offset(linkedChannel),   ins.pairs[1].ops[1]);
            uint8_t c1Val = (ins.pairs[1].panning << 4) | (ins.pairs[1].feedback << 1) | ins.pairs[1].connection;
            writeChannelRegHW(0xC0, linkedChannel, c1Val);
        } else {
            // SILENCE the Slave to prevent interference
            // was disabled for 9,10,11 tests
            write(linkedBank + 0x40 + get_modulator_offset(linkedChannel), 0x3F);
            write(linkedBank + 0x40 + get_carrier_offset(linkedChannel),   0x3F);
            writeChannelRegHW(0xB0, linkedChannel, 0x00);
        }
    }

    // dLog("ApplyInstrument: channel: %d, ins: %d/%d channel is master: %d", channel,  ins.isFourOp, ins.isDoubleVoice, mapping.isOP4Master);

    return true;
}

// //------------------------------------------------------------------------------
bool OPL3Controller::playNote(uint8_t softwareChannel, SongStep songStep) {

    if (softwareChannel >= SOFTWARE_CHANNEL_COUNT)
        return false;

    mChannelToNote[softwareChannel] = songStep.note;
    return this->playNoteHW(getHardWareChannel(softwareChannel), songStep);
}
//------------------------------------------------------------------------------
bool OPL3Controller::playNoteHW(uint8_t channel, SongStep step) {
    if (channel >= MAX_HW_CHANNELS) return false;

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

    // --- NOTE OFF ---
    if (step.note == STOP_NOTE) {
        stopNoteHW(channel);
        return true;
    }

    // --- TRIGGER NEW NOTE  also with nuked ---
    stopNoteHW(channel);


    if (step.instrument >= mSoundBank.size()) return false;
    if (!applyInstrumentHW(channel, step.instrument)) {
        Log("[error] Apply Instrument fail!!! ");
        return false;
    }
    const auto& currentIns = mSoundBank[step.instrument];


    setChannelVolume(channel, getOplVol(step.volume));
    setChannelPanning(channel, step.panning);


    // ---  NOTE OVERRIDE ---
    uint8_t targetNote = (currentIns.fixedNote != 0) ? currentIns.fixedNote : step.note;

    // --- INTERNAL NOTE CALCULATION ---
    int internalNote = (int)targetNote + currentIns.noteOffset ;

    // --- BLOCK/INDEX CALCULATION ---
    // octaves 0..7
    int block = (internalNote / 12 ) - 1;
    int noteIndex = internalNote % 12;

    // --- FREQUENCY CALCULATION ---
    uint32_t fnum = f_numbers[noteIndex];
    // Ensure we don't out-of-bounds the fineTuneTable
    int tuneIdx = std::clamp((int)currentIns.fineTune + 128, 0, 255);
    fnum = static_cast<uint32_t>(fnum * fineTuneTable[tuneIdx]);

    // --- NORMALIZATION ---
    // while (fnum >= 0x400 && block < 7) {
    //     fnum >>= 1;
    //     block++;
    // }
    // while (fnum < 0x200 && block > 0) {
    //     fnum <<= 1;
    //     block--;
    // }


    // --- 6. HARDWARE SAFETY ---
    if (block < 0) block = 0;
    if (block > 7) block = 7;
    if (fnum > 1023) fnum = 1023;



    playNoteByFNumHW(channel, (uint16_t)fnum, (uint8_t)block);

    // <<<<<<<<<<<<<<<<<<<<<<<<<<<<

    return true;
}

//------------------------------------------------------------------------------
bool OPL3Controller::stopNote(uint8_t softwareChannel) {

    if (softwareChannel >= SOFTWARE_CHANNEL_COUNT)
        return false;

    mChannelToNote[softwareChannel] = -1;

    return this->stopNoteHW(getHardWareChannel(softwareChannel));

}
//------------------------------------------------------------------------------
bool OPL3Controller::stopNoteHW(uint8_t channel) {
    if (channel >= MAX_HW_CHANNELS) return false;
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
    uint8_t fourOpReg = readShadow(0x104);
    bool isFourOpMaster = false;
    int bit = -1;
    if (channel >= 0 && channel <= 2) bit = channel;         // Bits 0, 1, 2
    else if (channel >= 9 && channel <= 11) bit = channel - 6; // Bits 3, 4, 5 (9-6=3, 10-6=4, 11-6=5)

    if (bit != -1 && (fourOpReg & (1 << bit))) {
        isFourOpMaster = true;
        keyOff(channel + 3); // Key-off the "slave" channel
    }
    if (isFourOpMaster) {
        // Apply Key-Off to the linked channel (+3) as well
        keyOff(channel + 3);
    }

    // crtitical:
    mChip->generate(&mOutput);

    return true;
}


//------------------------------------------------------------------------------
void OPL3Controller::write(uint16_t reg, uint8_t val, bool doLog)
{
    if (reg > 512 ) {
        Log("[error] OPL3Controller::write reg it out of bounds! %d", reg);
        return;
    }
    mShadowRegs[reg] = val;


    uint32_t portBase = (reg & 0x100) ? 2 : 0;
    uint8_t index = reg & 0xFF;
    mChip->write(portBase + 0, index);
    mChip->write(portBase + 1, val);
    // mChip->write_address(reg);
    // mChip->write_data(val);
    if (doLog) {
        Log("OPL_WRITE Bank:%d portBase:%d Reg:%02X Data:%02X", reg >> 8, portBase,  reg & 0xFF, val);
    }
}

// was a test for channel 9,10,11 but did not help
// void OPL3Controller::write(uint16_t reg, uint8_t val, bool doLog) {
//     if (reg > 512 ) {
//         Log("[error] OPL3Controller::write reg it out of bounds! %d", reg);
//         return;
//     }
//
//     mShadowRegs[reg] = val;
//
//     uint32_t port = (reg < 0x100) ? 0 : 2;
//     mChip->write(port, reg & 0xFF);     // Address
//     mChip->write(port + 1, val);        // Data
//
// }

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
    if (channel >= MAX_HW_CHANNELS) return false;

    // Determine the base address for the C0 register
    // Bank 1: 0xC0-0xC8 | Bank 2: 0x1C0-0x1C8
    uint16_t bankOffset = (channel < 9) ? 0x000 : 0x100;
    uint8_t relChan = channel % 9;
    uint16_t regAddr = bankOffset + 0xC0 + relChan;

    // Bit 0 is the Connection bit (0 = FM, 1 = Additive/AM)
    return (readShadow(regAddr) & 0x01);
}
//------------------------------------------------------------------------------
// void OPL3Controller::setChannelPanning(uint8_t channel, uint8_t pan) {
//     uint16_t c0Addr = ((channel < 9) ? 0x000 : 0x100) + 0xC0 + (channel % 9);
//     uint8_t c0Val = readShadow(c0Addr) & 0xCF; // Keep feedback/connection bits
//
//
//     if (pan < 21)      c0Val |= 0x10; // Left
//     else if (pan > 43) c0Val |= 0x20; // Right
//     else               c0Val |= 0x30; // Center
//
//     write(c0Addr, c0Val);
// }

void OPL3Controller::setChannelPanning(uint8_t channel, uint8_t pan) {
    if (channel >= MAX_HW_CHANNELS) return;


    // 1. Calculate the correct register address
    uint16_t bankOffset = (channel <= 8) ? 0x000 : 0x100;
    uint16_t c0Addr = bankOffset + 0xC0 + (channel % 9);


    // 2. Read from shadow. Ensure mShadowRegs is large enough (at least 512 bytes)!
    uint8_t currentC0 = readShadow(c0Addr);

    // 3. Clear only the Panning bits (Bits 4 and 5)
    // 0xCF is 11001111 - this preserves Feedback (1-3) and Connection (0)
    uint8_t c0Val = currentC0 & 0xCF;

    // 4. Apply new Panning bits
    if (pan < 21)      c0Val |= 0x10; // Left (CHA)
    else if (pan > 43) c0Val |= 0x20; // Right (CHB)
    else               c0Val |= 0x30; // Center (CHA + CHB)


    // 5. Write back
    write(c0Addr, c0Val);
}

//------------------------------------------------------------------------------
void OPL3Controller::initDefaultBank(){
    mSoundBank.clear();
    for (int i = 0; i < 6; i++ )
        mSoundBank.push_back(OPL3InstrumentPresets::GetMelodicDefault(i));

}
//------------------------------------------------------------------------------
void OPL3Controller::dumpInstrument(uint8_t instrumentIndex) {
    if (instrumentIndex >= mSoundBank.size()) {
        LogFMT("Error: Instrument index {} out of bounds (max {}).", instrumentIndex, mSoundBank.size() - 1);
        return;
    }

    const auto& ins = mSoundBank[instrumentIndex];

    LogFMT("--- Instrument Dump: {} (Index {}) ---", ins.name, instrumentIndex);
    LogFMT("  Is Four-Op : {}", ins.isFourOp ? "Yes" : "No");
    LogFMT("  DoubleVoice: {}", ins.isDoubleVoice ? "Yes" : "No");
    LogFMT("  Fine Tune  : {}", (int)ins.fineTune);
    LogFMT("  Fixed Note : {}", (ins.fixedNote == NONE_NOTE) ? "None" : std::to_string(ins.fixedNote));
    LogFMT("  Note offset: {}", (int)ins.noteOffset);


    for (int pIdx = 0; pIdx < ((ins.isFourOp || ins.isDoubleVoice) ? 2 : 1); ++pIdx) {
        const auto& pair = ins.pairs[pIdx];
        LogFMT("  --- Operator Pair {} ---", pIdx);
        LogFMT("    Feedback: {}", pair.feedback);
        LogFMT("    Connection: {}", pair.connection == 0 ? "FM" : "Additive");
        LogFMT("    Panning: {}", pair.panning);

        for (int opIdx = 0; opIdx < 2; ++opIdx) {
            const auto& op = pair.ops[opIdx];
            LogFMT("    {} Operator:", opIdx == 0 ? "Modulator" : "Carrier");

            // Manual mapping to OplInstrument::OpParams members based on OPL_OP_METADATA order
            LogFMT("      Multi:   0x{:02X} ({:d})", op.multi, op.multi);
            LogFMT("      TL:      0x{:02X} ({:d})", op.tl, op.tl);
            LogFMT("      Attack:  0x{:02X} ({:d})", op.attack, op.attack);
            LogFMT("      Decay:   0x{:02X} ({:d})", op.decay, op.decay);
            LogFMT("      Sustain: 0x{:02X} ({:d})", op.sustain, op.sustain);
            LogFMT("      Release: 0x{:02X} ({:d})", op.release, op.release);
            LogFMT("      Wave:    0x{:02X} ({:d})", op.wave, op.wave);
            LogFMT("      KSR:     0x{:02X} ({:d})", op.ksr, op.ksr);
            LogFMT("      EGType:  0x{:02X} ({:d})", op.egTyp, op.egTyp);
            LogFMT("      Vib:     0x{:02X} ({:d})", op.vib, op.vib);
            LogFMT("      AM:      0x{:02X} ({:d})", op.am, op.am);
            LogFMT("      KSL:     0x{:02X} ({:d})", op.ksl, op.ksl);
        }
    }
    LogFMT("-----------------------------------");
}
//------------------------------------------------------------------------------
void OPL3Controller::setFrequencyLinear(uint8_t channel, float linearFreq) {
    if (channel >= MAX_HW_CHANNELS) return;

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
    if (channel >= MAX_HW_CHANNELS) return;

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
    if (channel >= MAX_HW_CHANNELS) return;

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


            // dLog("Slide...channel:%d row:%d step.volume:%d( oplVolume: %d)", channel , mSeqState.rowIdx, currentVol, getOplVol(currentVol));

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
void OPL3Controller::consoleSongOutput(bool useNumbers, bool showHWChannels)
{
    if (!mSeqState.playing)
        return;

    char buffer[1024]; // Increased size: 18 channels * ~15 chars each + padding
    char *ptr = buffer;


    if (mSeqState.ui_dirty) {
        // sprintf returns the number of characters written.
        // Use that to advance the pointer.
        ptr += sprintf(ptr, "[%02d:%03d] ", mSeqState.orderIdx, mSeqState.rowIdx);

        if (showHWChannels) {
            for (int ch = 0; ch < MAX_HW_CHANNELS; ch++) {
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

                // Separator between OP4/OP2 0..
                if (ch == 8) ptr += sprintf(ptr, " || ");
            }
        } else {
            for (int softwareChannel = 0; softwareChannel < SOFTWARE_CHANNEL_COUNT; softwareChannel++) {

                const SongStep& step = mSeqState.last_steps[getHardWareChannel(softwareChannel)];

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

                // Separator between OP4/OP2 0..
                if (softwareChannel == 5) ptr += sprintf(ptr, " || ");
            }
        }



        Log("%s", buffer);
        mSeqState.ui_dirty = false;
    }
}
//------------------------------------------------------------------------------
bool OPL3Controller::playSong(opl3::SongData& songData, bool loop )  {

    if (!songValid(songData))
        return false;


    std::lock_guard<std::recursive_mutex> lock(mDataMutex);

    // Assign Song Data
    mSeqState.current_song = &songData;

    // Reset Sequencer Positions
    mSeqState.orderIdx = 0;
    mSeqState.rowIdx = 0;
    mSeqState.current_tick = 0;
    mSeqState.sample_accumulator = 0.f;
    mSeqState.loop = loop;
    mSeqState.ticks_per_row = songData.ticksPerRow;

    // Reset Channel States using the new struct array
    // This clears all redundancy checks so the first row of the song
    // is guaranteed to write to the hardware registers.
    for (int i = 0; i < MAX_HW_CHANNELS; ++i) {
        mSeqState.last_steps[i] = {}; // Resets note, vol, pan, effect
        mSeqState.last_steps[i].volume = 255; // Force update on first row
    }

    // Calculate timing
    double hostRate = 44100.0;
    double ticks_per_sec = songData.bpm * 0.4; //songData.ticksPerSecond does nothing ?!?!?! TODO ?

    // Calculate samples per tick
    if (ticks_per_sec > 0) {
        // At 125 BPM: 44100 / 50 = 882 samples per tick
        mSeqState.samples_per_tick = hostRate / ticks_per_sec;
    } else {
        mSeqState.samples_per_tick = hostRate;
    }



    // Start Playback
    mSeqState.playing = true;
    mSeqState.ui_dirty = true;

    return true;
}
//------------------------------------------------------------------------------
void OPL3Controller::playNoteByFNumHW(uint8_t channel, uint16_t fnum, uint8_t octave) {
    if (channel >= MAX_HW_CHANNELS) return;

    // reset silent check
    mIsSilent = false;
    mSilenceCounter = 0;

    uint16_t bankOffset = (channel <= 8) ? 0x000 : 0x100;
    uint8_t relChan = (channel <= 8) ? channel : (channel - 9);

    // 2. Identify if this is a 4-OP Master channel
    uint8_t fourOpReg = readShadow(0x104);
    bool isFourOpMaster = false;
    if (channel < 3 && (fourOpReg & (1 << channel))) {
        isFourOpMaster = true;
    } else if (channel >= 9 && channel < 12 && (fourOpReg & (1 << (channel - 9 + 3)))) {
        isFourOpMaster = true;
    }

    // 3. Write Frequency and Octave to the primary (Master) channel
    // Register 0xA0: F-Number LSB
    write(bankOffset + 0xA0 + relChan, fnum & 0xFF);

    // Register 0xB0: Key-On (bit 5), Block (octave), F-Number MSB
    // We ALWAYS send Key-On to the Master channel.
    uint8_t b0Val = 0x20 | ((octave & 0x07) << 2) | ((fnum >> 8) & 0x03);
    write(bankOffset + 0xB0 + relChan, b0Val);

    // 4. Handle Slave Channel if in 4-OP Mode
    // was disabled for 9,10,11 tests
    if (isFourOpMaster) {
        // Linked channel is always Hardware Channel + 3
        uint8_t linkedChan = channel + 3;
        uint16_t linkedBankOffset = (linkedChan <= 8) ? 0x000 : 0x100;
        uint8_t linkedRelChan = (linkedChan <= 8) ? linkedChan : (linkedChan - 9);

        // SYNC the F-Number and Octave to the Slave channel
        write(linkedBankOffset + 0xA0 + linkedRelChan, fnum & 0xFF);

        // IMPORTANT: For 4-OP voices, the hardware ignores the Key-On bit (0x20)
        // on the SLAVE. You should write only the Block/F-Num for consistency.
        uint8_t b0SlaveVal = ((octave & 0x07) << 2) | ((fnum >> 8) & 0x03);
        write(linkedBankOffset + 0xB0 + linkedRelChan, b0SlaveVal);
    }


        // dLog("CHANNEL %d  isFourOpMaster= %d , 0x104 = %d", channel , isFourOpMaster,  readShadow(0x104));

}

//------------------------------------------------------------------------------
void OPL3Controller::setOperatorRegisters(uint16_t bank, uint8_t opOffset, const opl3::OplInstrument::OpPair::OpParams& op) {
    // 1. Calculate parameters
    uint8_t reg20 = (op.multi & 0x0F) |
    (op.ksr   ? 0x10 : 0x00) |
    (op.egTyp ? 0x20 : 0x00) |
    (op.vib   ? 0x40 : 0x00) |
    (op.am    ? 0x80 : 0x00);

    // 2. Write to BOTH banks using the provided bank offset
    // baseAddr = bank (0x000 or 0x100) + baseReg (0x20, 0x40, etc) + opOffset
    write(bank + 0x20 + opOffset, reg20);
    write(bank + 0x40 + opOffset, op.tl | (op.ksl << 6));
    write(bank + 0x60 + opOffset, op.decay | (op.attack << 4));
    write(bank + 0x80 + opOffset, op.release | (op.sustain << 4));
    write(bank + 0xE0 + opOffset, op.wave & 0x07);
}

// void OPL3Controller::setOperatorRegisters(uint16_t opOffset, const opl3::OplInstrument::OpPair::OpParams& op) {
//     // Standard OPL Register layout: 0x20, 0x40, 0x60, 0x80, 0xE0
//
//     // Register 0x20: [AM][VIB][EG-TYP][KSR][ MULTI (4 bits) ]
//     uint8_t reg20 = (op.multi   & 0x0F) | // Multiplikator (0-15)
//     (op.ksr     ? 0x10 : 0x00) | // Key Scale Rate
//     (op.egTyp   ? 0x20 : 0x00) | // EG-Typ (Sustain-Modus)
//     (op.vib     ? 0x40 : 0x00) | // Vibrato
//     (op.am      ? 0x80 : 0x00);  // Amplitude Modulation (Tremolo)
//
//     write(opOffset + 0x20, reg20);
//
//     // write(opOffset + 0x20, op.multi | (op.ksr << 4) | (op.egTyp << 5) | (op.vib << 6) | (op.am << 7));
//     write(opOffset + 0x40, op.tl | (op.ksl << 6));
//     write(opOffset + 0x60, op.decay | (op.attack << 4));
//     write(opOffset + 0x80, op.release | (op.sustain << 4));
//     write(opOffset + 0xE0, op.wave & 0x07);
// }
//------------------------------------------------------------------------------
void OPL3Controller::writeChannelRegHW(uint16_t baseReg, uint8_t channel, uint8_t value){
    uint16_t bankOffset = (channel <= 8) ? 0x000 : 0x100;
    write(bankOffset + baseReg + (channel % 9), value);
}
//------------------------------------------------------------------------------
void OPL3Controller::toggleDeepEffects(bool deepTremolo, bool deepVibrato) {
    uint8_t currentBD = readShadow(0xBD);

    if (deepTremolo) currentBD |= 0x80;  // Set Bit 7
    else             currentBD &= ~0x80; // Clear Bit 7

    if (deepVibrato) currentBD |= 0x40;  // Set Bit 6
    else             currentBD &= ~0x40; // Clear Bit 6

    write(0xBD, currentBD);
}
//------------------------------------------------------------------------------
bool OPL3Controller::songValid(const opl3::SongData& songData) {
    if (songData.patterns.empty()) {
        return false;
    }

    // 1. Validate Pattern Indices in OrderList
    for (uint8_t patternIdx : songData.orderList) {
        if (patternIdx >= songData.patterns.size()) {
            return false;
        }
    }

    // 2. Validate Instrument Indices within Patterns
    // This works because Pattern now has public begin() and end()
    for (const auto& pattern : songData.patterns) {
        for (const auto& step : pattern) {
            if (step.note != NONE_NOTE && step.instrument >= mSoundBank.size()) {
                return false;
            }
        }
    }

    return true;
}
//------------------------------------------------------------------------------
bool OPL3Controller::exportToWav(opl3::SongData& sd, const std::string& filename, float* progressOut, bool applyEffects) {
    detachAudio();
    dLog("[info] OPL3Controller::exportToWav (Optimized Float Pipeline)...");

    playSong(sd, false);

    // 1. Calculate dimensions
    uint32_t total_ticks = sd.getTotalRows() * sd.ticksPerRow;
    uint32_t totalFrames = static_cast<uint32_t>(total_ticks * mSeqState.samples_per_tick);
    int sampleRate = 44100;
    int chunkSize = 4096;

    // 2. Allocate FLOAT buffer for the entire export
    // This is the "Working Master" buffer
    std::vector<float> f32ExportBuffer(totalFrames * 2);
    int framesProcessed = 0;

    mSeqState.sample_accumulator = 0;
    m_pos = 0;

    // 3. Generation Loop
    while (framesProcessed < totalFrames) {
        int toWrite = std::min(chunkSize, (int)(totalFrames - framesProcessed));

        // Use the new FLOAT fillBuffer directly
        this->fillBuffer(&f32ExportBuffer[framesProcessed * 2], toWrite);

        framesProcessed += toWrite;

        if (progressOut) {
            float progressScale = applyEffects ? 0.70f : 1.0f;
            *progressOut = (float)framesProcessed / (float)totalFrames * progressScale;
        }
    }

    this->stopSong(true);

    // 4. Processing Phase
    if (applyEffects) {
        if (progressOut) *progressOut = 0.75f;

        // Apply DSP directly to the master float buffer
        for (auto& effect : this->mDspEffects) {
            effect->process(f32ExportBuffer.data(), f32ExportBuffer.size());
        }
        if (progressOut) *progressOut = 0.85f;

        // Normalize
        DSP::normalizeBuffer(f32ExportBuffer.data(), f32ExportBuffer.size(), 0.98f);
        if (progressOut) *progressOut = 0.90f;
    }

    if (progressOut) *progressOut = 1.0f;

    attachAudio();


    return saveWavFile(filename, f32ExportBuffer, sampleRate);
}
//------------------------------------------------------------------------------
bool OPL3Controller::saveWavFile(const std::string& filename, const std::vector<float>& data, int sampleRate)  {
    // Open the file for writing using SDL3's IO system
    SDL_IOStream* io = SDL_IOFromFile(filename.c_str(), "wb");
    if (!io) {
        LogFMT("ERROR:Failed to open file for writing: %s", SDL_GetError());
        return false;
    }

    dLog("[info] OPL3Controller::saveWavFile (32-bit Float).....");

    uint32_t numChannels = 2;
    uint32_t bitsPerSample = 32; // 32 bits for float
    uint32_t dataSize = (uint32_t)(data.size() * sizeof(float));
    uint32_t fileSize = 36 + dataSize;
    uint32_t byteRate = sampleRate * numChannels * (bitsPerSample / 8);
    uint16_t blockAlign = (uint16_t)(numChannels * (bitsPerSample / 8));

    // Write the WAV Header
    SDL_WriteIO(io, "RIFF", 4);
    SDL_WriteU32LE(io, fileSize);
    SDL_WriteIO(io, "WAVE", 4);
    SDL_WriteIO(io, "fmt ", 4);
    SDL_WriteU32LE(io, 16);

    // IMPORTANT: AudioFormat 3 is IEEE Float (instead of 1 for PCM)
    SDL_WriteU16LE(io, 3);

    SDL_WriteU16LE(io, (uint16_t)numChannels);
    SDL_WriteU32LE(io, (uint32_t)sampleRate);
    SDL_WriteU32LE(io, byteRate);
    SDL_WriteU16LE(io, blockAlign);
    SDL_WriteU16LE(io, (uint16_t)bitsPerSample);

    SDL_WriteIO(io, "data", 4);
    SDL_WriteU32LE(io, dataSize);

    // Write the raw float data directly - no conversion needed!
    SDL_WriteIO(io, data.data(), dataSize);

    SDL_CloseIO(io);
    LogFMT("Successfully exported WAV {}", filename);

    return true;
}
//------------------------------------------------------------------------------
void OPL3Controller::detachAudio(){
    SDL_PauseAudioStreamDevice(mStream);
    SDL_SetAudioStreamGetCallback(mStream, NULL, NULL);
}

void OPL3Controller::attachAudio(){
    SDL_SetAudioStreamGetCallback(mStream, OPL3Controller::audio_callback, this);
    SDL_ResumeAudioStreamDevice(mStream);
}
//------------------------------------------------------------------------------
