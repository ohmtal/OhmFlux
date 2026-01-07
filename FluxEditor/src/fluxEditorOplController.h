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

    void setLoop( bool value ) {
        mSeqState.loop = value;
    }


    struct ChannelSettings {
        int octave = 4;
        int step = 1;
        bool muted = false;
    };

    // Size is exactly the number of channels (e.g., 9)
    ChannelSettings mChannelSettings[FMS_MAX_CHANNEL+1];

    void setChannelMuted( int channel , bool value )
    {
         mChannelSettings[std::clamp(channel, 0, FMS_MAX_CHANNEL)].muted = value;
    }

    bool getChannelMuted( int channel  )
    {
        if (channel < 0 || channel > FMS_MAX_CHANNEL)
            return false;
        return mChannelSettings[channel].muted;
    }


    int getStepByChannel(int channel) {

        return mChannelSettings[std::clamp(channel, 0, FMS_MAX_CHANNEL)].step;
    }
    void setStepByChannel(int channel, int step) {
        step = std::clamp(step,0,99);
        mChannelSettings[std::clamp(channel, 0, FMS_MAX_CHANNEL)].step = step;
    }

    void incStepByChannel(int channel ) {
        int lNext = getStepByChannel(channel) + 1;
        setStepByChannel(channel , lNext);
    }

    void decStepByChannel(int channel ) {
        int lNext = getStepByChannel(channel) - 1;
        if (lNext < 1)
            lNext = 1;
        setStepByChannel(channel , lNext);
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
