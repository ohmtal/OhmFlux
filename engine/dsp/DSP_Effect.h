//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Base class Effect
//-----------------------------------------------------------------------------
#pragma once
#include <cstdint>

namespace DSP {

    class Effect {
    protected:
        bool mEnabled = false;

    public:
        Effect(bool switchOn = false) { mEnabled = switchOn;}
        virtual ~Effect() {}
        virtual void process(float* buffer, int numSamples) {}

        void on() { mEnabled = true;}
        void off() { mEnabled = false;}
        bool inOn() { return mEnabled; }

    }; //class
}; //namespace
