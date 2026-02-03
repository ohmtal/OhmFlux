//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Base class Effect
//-----------------------------------------------------------------------------
#pragma once
#include <cstdint>

namespace DSP {

    enum class EffectType : uint32_t {
        NONE               = 0,
        Bitcrusher         = 1,
        Chorus             = 2,
        Equalizer          = 3, //UNUSED: only one band so i would be attached multiple times ...
        Equalizer9Band     = 4,
        Limiter            = 5,
        Reverb             = 6,
        SoundCardEmulation = 7,
        SpectrumAnalyzer   = 8, //no extra settings only analysing for visual effect
        Warmth             = 9,
        VisualAnalyzer     = 10  //no extra settings only analysing for visual effect

    };

    class Effect {
    protected:
        bool mEnabled = false;

    public:
        Effect(bool switchOn = false) { mEnabled = switchOn;}
        virtual ~Effect() {}
        virtual void process(float* buffer, int numSamples) {}

        virtual DSP::EffectType getType() const { return DSP::EffectType::NONE; }

        // Interface for serialization
        virtual void save(std::ostream& os) const {
            os.write(reinterpret_cast<const char*>(&mEnabled), sizeof(mEnabled));
        }

        virtual bool load(std::istream& is) {
            is.read(reinterpret_cast<char*>(&mEnabled), sizeof(mEnabled));
            return is.good();
        }

        void setEnabled(bool value) { mEnabled = value; }
        bool isEnabled() { return mEnabled; }

    }; //class
}; //namespace
