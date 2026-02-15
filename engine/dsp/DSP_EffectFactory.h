//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Effect Factory
//-----------------------------------------------------------------------------
// NOTE: fetch category in gui with:
//          EffectCatId cat = EffectFactory::GetCategory(myEffect->getType());
//-----------------------------------------------------------------------------
#pragma once

#include <memory>
#include "DSP.h"

namespace DSP {

    // Example usage:
    // mDrumKit = cast_unique<DSP::DrumKit>(DSP::EffectFactory::Create(DSP::EffectType::DrumKit));

    template <typename T>
    std::unique_ptr<T> cast_unique(std::unique_ptr<DSP::Effect> source) {
        if (!source) return nullptr;
        // Release ownership from the base pointer and wrap it in the derived type
        return std::unique_ptr<T>(static_cast<T*>(source.release()));
    }

    // save version:
    // Example usage:
    //     auto tempEffect = DSP::EffectFactory::Create(DSP::EffectType::DrumKit);
    //     mDrumKit = dynamic_cast_unique<DSP::DrumKit>(tempEffect);
    template <typename T>
    std::unique_ptr<T> dynamic_cast_unique(std::unique_ptr<DSP::Effect>& source) {
        T* derived = dynamic_cast<T*>(source.get());
        if (derived) {
            source.release(); // Only release if the cast was successful
            return std::unique_ptr<T>(derived);
        }
        return nullptr;
    }



    class EffectFactory {
    public:
        static std::unique_ptr<Effect> Create(EffectType type) {
            switch(type) {
                #define X_FACTORY(name, id, cat) \
                case EffectType::name: \
                    return std::make_unique<name>();
                    EFFECT_LIST(X_FACTORY)
                    #undef X_FACTORY
                default: return nullptr;
            }
        }


        static EffectCatId GetCategory(EffectType type) {
            switch(type) {
                #define X_GET_CAT(name, id, cat) \
                case EffectType::name: return cat;
                EFFECT_LIST(X_GET_CAT)
                #undef X_GET_CAT
                default: return EffectCatId::Utility;
            }
        }

        static std::vector<EffectType> GetEffectsByCategory(EffectCatId category) {
            std::vector<EffectType> results;
            #define X_FILTER(name, id, cat) \
            if (cat == category) { results.push_back(EffectType::name); }
            EFFECT_LIST(X_FILTER)
            #undef X_FILTER
            return results;
        }

        // get the basic name (not the defined in class)
        static const char* GetName(EffectType type) {
            switch(type) {
                #define X_NAME(name, id, cat) \
                case EffectType::name: return #name;
                EFFECT_LIST(X_NAME)
                #undef X_NAME
                default: return "Unknown";
            }
        }

        // static std::unique_ptr<Effect> Create(EffectType type) {
        //     switch(type) {
        //         #define X_FACTORY(name, id) \
        //         case EffectType::name: \
        //             return std::make_unique<name>();
        //             EFFECT_LIST(X_FACTORY)
        //             #undef X_FACTORY
        //
        //         default:
        //             // Log("[error] EffectFactory: NO MATCH found for ID %d\n", (uint32_t)type);
        //             return nullptr;
        //     }
        // }
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

