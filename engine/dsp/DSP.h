//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Main include
//-----------------------------------------------------------------------------
#pragma once

#include <memory>

#include "DSP_Effect.h"
#include "DSP_tools.h"

#include "DSP_Bitcrusher.h"
#include "DSP_Chorus.h"
#include "DSP_Limiter.h"
#include "DSP_Reverb.h"
#include "DSP_Warmth.h"
#include "DSP_Equilizer.h"
#include "DSP_Equilizer9Band.h"
#include "DSP_SpectrumAnalyzer.h"
#include "DSP_SoundCardEmulation.h"
#include "DSP_VisualAnalyzer.h"
#include "DSP_Delay.h"
#include "DSP_VoiceModulator.h"
#include "DSP_RingModulator.h"
#include "DSP_OverDrive.h"
#include "DSP_NoiseGate.h"
#include "DSP_DistortionBasic.h"
#include "DSP_Metal.h"
#include "DSP_ChromaticTuner.h"

#include "Drums/DSP_DrumKit.h" //(Wip)
#include "Drums/DSP_KickDrum.h"


namespace DSP {


    //-----------------------------------------------------------------------------
    // HELPER FUNCTION IF YOU NEED TO ADD A EFFECT TO A SHADOW POINTER:
    // Example:  mDSPBitCrusher = addEffectToChain<DSP::Bitcrusher>(mDspEffects, false);
    template<typename T>
    T* addEffectToChain(std::vector<std::unique_ptr<DSP::Effect>>& chain, bool enabled = false) {
        auto fx = std::make_unique<T>(enabled);
        T* ptr = fx.get();
        chain.push_back(std::move(fx));
        return ptr;
    }
    //-----------------------------------------------------------------------------
    // normalize a stream
    inline void normalizeBuffer(float* buffer, size_t count, float targetPeak = 1.0f) {
        float currentPeak = 0.0f;

        // Pass 1: Find the absolute maximum peak
        for (size_t i = 0; i < count; ++i) {
            float absValue = std::abs(buffer[i]);
            if (absValue > currentPeak) {
                currentPeak = absValue;
            }
        }

        // Pass 2: Scale all samples (only if the buffer isn't silent)
        if (currentPeak > 0.0f) {
            float factor = targetPeak / currentPeak;
            for (size_t i = 0; i < count; ++i) {
                buffer[i] *= factor;
            }
        }
    }
};
