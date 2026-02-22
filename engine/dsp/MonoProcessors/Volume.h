//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once

#include <vector>
#include <iostream>
#include <cmath>
#include <algorithm>
#include <atomic>
#include <fstream>
#include <cstdlib>

#include "../DSP_Math.h"

namespace DSP {
namespace MonoProcessors {

    class Volume {
    public:
        float process(float input, float volume, bool softclip = true) {
            float out = input * volume;
            if (softclip) out = DSP::softClip(out);
            return out;
        }
    };


};};
