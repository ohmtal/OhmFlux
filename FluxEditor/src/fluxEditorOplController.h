//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// FIXME use the channel also in piano and instrument editor!!
//-----------------------------------------------------------------------------

#pragma once

#include <OplController.h>

static const int  OPL_MIN_OCTAVE = 1;
static const int  OPL_MAX_OCTAVE = 8;

class FluxEditorOplController : public OplController
{
public:


    struct ChannelSettings {
        int octave = 4;
        int step = 1;
    };

    // Size is exactly the number of channels (e.g., 9)
    ChannelSettings mChannelSettings[FMS_MAX_CHANNEL+1];

    int getStepByChannel(int channel) {
        // Clamp between 0 and MAX-1 (e.g., 0 to 8)
        return mChannelSettings[std::clamp(channel, 0, FMS_MAX_CHANNEL)].step;
    }
    void setStepByChannel(int channel, int step) {
        // Clamp between 0 and MAX-1 (e.g., 0 to 8)
        mChannelSettings[std::clamp(channel, 0, FMS_MAX_CHANNEL)].step = step;
    }




    int getOctaveByChannel(int channel) {
        return mChannelSettings[std::clamp(channel, 0, FMS_MAX_CHANNEL)].octave;
    }

    void setOctaveByChannel(int channel, int val) {
        mChannelSettings[std::clamp(channel, 0, FMS_MAX_CHANNEL)].octave = std::clamp(val, OPL_MIN_OCTAVE, OPL_MAX_OCTAVE);
    }

    void incOctaveByChannel(int channel)
    {
        int lNext = getOctaveByChannel(channel) + 1;
        if ( lNext  > OPL_MAX_OCTAVE )
            lNext  = OPL_MIN_OCTAVE;
        setOctaveByChannel(channel , lNext);
    }

    void decOctaveByChannel(int channel)
    {
        int lNext = getOctaveByChannel(channel) - 1;
        if ( lNext  < OPL_MIN_OCTAVE )
            lNext  = OPL_MAX_OCTAVE;
        setOctaveByChannel(channel , lNext);
    }

    // Helper function moved inside the class
    int getNoteWithOctave(int channel, const char* noteBase) {
        char noteBuf[OPL_MAX_OCTAVE];
        snprintf(noteBuf, sizeof(noteBuf), "%s%d", noteBase, getOctaveByChannel(channel));
        // Use the base class OplController::getIdFromNoteName
        return this->getIdFromNoteName(noteBuf);
    }
};
