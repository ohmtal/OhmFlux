//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Effect Factory
//-----------------------------------------------------------------------------
#pragma once

#include <memory>
#include "DSP.h"

namespace DSP {

    class EffectFactory {
    public:

        static std::unique_ptr<Effect> Create(EffectType type) {
            dLog("EffectFactory: Trying to create effect ID: %d\n", (uint32_t)type);

            switch(type) {
                #define X_FACTORY(name, id) \
                case EffectType::name: \
                    dLog("EffectFactory: Match found for %s\n", #name); \
                    return std::make_unique<name>();
                    EFFECT_LIST(X_FACTORY)
                    #undef X_FACTORY

                default:
                    Log("[error] EffectFactory: NO MATCH found for ID %d\n", (uint32_t)type);
                    return nullptr;
            }
        }
        // static std::unique_ptr<DSP::Effect> Create(EffectType type) {
        //     switch (type) {
        //         case EffectType::Bitcrusher:
        //             return std::make_unique<Bitcrusher>();
        //
        //         case EffectType::Chorus:
        //             return std::make_unique<Chorus>();
        //
        //         case EffectType::Equalizer:
        //             return std::make_unique<Equalizer>();
        //
        //         case EffectType::Equalizer9Band:
        //             return std::make_unique<Equalizer9Band>();
        //
        //         case EffectType::Limiter:
        //             return std::make_unique<Limiter>();
        //
        //         case EffectType::Reverb:
        //             return std::make_unique<Reverb>();
        //
        //         case EffectType::SoundCardEmulation:
        //             return std::make_unique<SoundCardEmulation>();
        //
        //         case EffectType::SpectrumAnalyzer:
        //             return std::make_unique<SpectrumAnalyzer>();
        //
        //         case EffectType::Warmth:
        //             return std::make_unique<Warmth>();
        //
        //         case EffectType::VisualAnalyzer:
        //             return std::make_unique<VisualAnalyzer>();
        //
        //         case EffectType::Delay:
        //             return std::make_unique<Delay>();
        //
        //         case EffectType::VoiceModulator:
        //             return std::make_unique<VoiceModulator>();
        //
        //         case EffectType::RingModulator:
        //             return std::make_unique<RingModulator>();
        //
        //         case EffectType::OverDrive:
        //             return std::make_unique<OverDrive>();
        //
        //         case EffectType::NoiseGate:
        //             return std::make_unique<NoiseGate>();
        //
        //         case EffectType::DistortionBasic:
        //             return std::make_unique<DistortionBasic>();
        //
        //         case EffectType::Metal:
        //             return std::make_unique<Metal>();
        //
        //         case EffectType::ChromaticTuner:
        //             return std::make_unique<ChromaticTuner>();
        //
        //
        //         case EffectType::NONE:
        //         default:
        //             return nullptr;
        //     }
        // }
    };

} // namespace DSP

