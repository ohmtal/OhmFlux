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
        static std::unique_ptr<DSP::Effect> Create(EffectType type) {
            switch (type) {
                case EffectType::Bitcrusher:
                    return std::make_unique<Bitcrusher>();

                case EffectType::Chorus:
                    return std::make_unique<Chorus>();

                case EffectType::Equalizer:
                    return std::make_unique<Equalizer>();

                case EffectType::Equalizer9Band:
                    return std::make_unique<Equalizer9Band>();

                case EffectType::Limiter:
                    return std::make_unique<Limiter>();

                case EffectType::Reverb:
                    return std::make_unique<Reverb>();

                case EffectType::SoundCardEmulation:
                    return std::make_unique<SoundCardEmulation>();

                case EffectType::SpectrumAnalyzer:
                    return std::make_unique<SpectrumAnalyzer>();

                case EffectType::Warmth:
                    return std::make_unique<Warmth>();

                case EffectType::VisualAnalyzer:
                    return std::make_unique<VisualAnalyzer>();

                case EffectType::Delay:
                    return std::make_unique<Delay>();

                case EffectType::VoiceModulator:
                    return std::make_unique<VoiceModulator>();

                case EffectType::RingModulator:
                    return std::make_unique<RingModulator>();

                case EffectType::NONE:
                default:
                    // Hier landen wir, wenn der Typ 0 ist oder vergessen wurde
                    return nullptr;
            }
        }
    };

} // namespace DSP

