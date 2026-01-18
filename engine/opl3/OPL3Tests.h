//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once
// #include "ymfm.h"
#include "ymfm_opl.h"
#include "opl3.h"
#include "OPL3Controller.h"
#include <SDL3/SDL_audio.h>


class OPL3Tests {
private:
    OPL3Controller* mController;
public:
    OPL3Tests(OPL3Controller* controller) {
        mController = controller;
    }



    void TESTChip() {

        if (!mController) return;

        SDL_AudioStream* stream = mController->getAudioStream();
        if (!stream) return;

        // using OplChip = ymfm::ymf262; //OPL3
        ymfm::ymf262::output_data& output = mController->getOutPut();

        SDL_PauseAudioStreamDevice(stream);
        SDL_SetAudioStreamGetCallback(stream, NULL, NULL);

        // 1. Hard Reset
        // mChip->reset();


        // 2. Enable OPL3 (Bank 2, Reg 0x05, Bit 0)
        mController->write(0x105, 0x01);

        // 3. Setup a Channel (Channel 0)
        // $20: Multiplier/Vibrato ($20=Mod, $23=Car)
        mController->write(0x20, 0x01); mController->write(0x23, 0x01);
        // $40: Total Level ($40=Mod, $43=Car). 0 is LOUDEST.
        mController->write(0x40, 0x10); mController->write(0x43, 0x00);
        // $60: Attack/Decay. 0xF is INSTANT ATTACK.
        mController->write(0x60, 0xF0); mController->write(0x63, 0xF0);
        // $80: Sustain/Release.
        mController->write(0x80, 0xFF); mController->write(0x83, 0xFF);
        // $C0: Panning (Bits 4-5). 0x30 is BOTH (Center).<
        mController->write(0xC0, 0x31); // 0x30 (Pan) | 0x01 (Connection)

        // 4. Trigger Note ($A0/$B0)
        mController->write(0xA0, 0x6B); // F-Number Low
        mController->write(0xB0, 0x31); // 0x20 (Key-On) | 0x10 (Octave 4) | 0x01 (F-Num High)

        // 5. Generate
        mController->getChip()->generate(&output);
        // mOutput.data[0...3] should now contain non-zero values.
        LogFMT("TEST OPL3 Chip data: {} {} {} {}",
               output.data[0],
               output.data[1],
               output.data[2],
               output.data[3]);

        // rebind the audio stream!
        SDL_SetAudioStreamGetCallback(stream, OPL3Controller::audio_callback, this);
        SDL_ResumeAudioStreamDevice(stream);


    }

    opl3::SongData createScaleSong(uint8_t instrumentIndex) {
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

    opl3::SongData createEffectTestSong(uint8_t ins) {
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

};  // namespace OPL3
