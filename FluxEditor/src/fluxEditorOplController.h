//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// FIXME use the channel also in piano and instrument editor!!
// FIXME overrite void OplController::tickSequencer() to mute channels
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
    //--------------------------------------------------------------------------

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

    //--------------------------------------------------------------------------
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

    //--------------------------------------------------------------------------
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

    //--------------------------------------------------------------------------
    // Helper function moved inside the class
    int getNoteWithOctave(int channel, const char* noteBase) {
        char noteBuf[OPL_MAX_OCTAVE];
        snprintf(noteBuf, sizeof(noteBuf), "%s%d", noteBase, getOctaveByChannel(channel));
        // Use the base class OplController::getIdFromNoteName
        return this->getIdFromNoteName(noteBuf);
    }

    //--------------------------------------------------------------------------
    // Insert Row Logic (Inside OplController)
    void insertRowAt(SongData& sd, uint16_t start, int onChannel = -1)
    {

        sd.song_length++;

        // 1. Move all rows below targetSeq down by one
        for (int i = sd.song_length; i > start; --i) {

            if (onChannel >= 0 && onChannel <= FMS_MAX_CHANNEL) {
                sd.song[i][onChannel] = sd.song[i-1][onChannel];
            } else {
                for (int ch = 0; ch < 9; ++ch) {
                    sd.song[i][ch] = sd.song[i-1][ch];
                }
            }
        }
        // 2. Clear the newly inserted row
        if (onChannel >= 0 && onChannel <= FMS_MAX_CHANNEL) {
            sd.song[start][onChannel] = 0;
        } else {
            for (int ch = 0; ch < 9; ++ch)
                sd.song[start][ch] = 0;
        }
    }

    //--------------------------------------------------------------------------
    // Delete Range Logic
    void deleteRange(SongData& sd, uint16_t start, uint16_t end) {
        //FIXME untested
        // int rangeLen = (end - start) + 1;
        // // Shift data up
        // for (int i = start; i < 1000 - rangeLen; ++i) {
        //     for (int ch = 0; ch < 9; ++ch) {
        //         sd.song[i][ch] = sd.song[i + rangeLen][ch];
        //     }
        // }
        // // Clear remaining rows at end
        // for (int i = 1000 - rangeLen; i < 1000; ++i) {
        //     for (int ch = 0; ch < 9; ++ch) sd.song[i][ch] = 0;
        // }
    }
    //--------------------------------------------------------------------------
    bool clearSongRange(SongData& sd, uint16_t start, uint16_t end)
    {
        if (end > FMS_MAX_SONG_LENGTH )
            return false;

        for (int i = start; i <= end; ++i)
        {
            for (int ch = 0; ch <= FMS_MAX_CHANNEL; ++ch) {
                sd.song[i][ch] = 0;
            }
        }
        return true;
    }
    //--------------------------------------------------------------------------
    bool clearSong(SongData& sd)
    {
        sd.song_length = 0;
        return clearSongRange(sd, 0,FMS_MAX_SONG_LENGTH);
    }
    //--------------------------------------------------------------------------
    bool copySongRange(SongData& fromSD, uint16_t fromStart,  SongData& toSD, uint16_t toStart, uint16_t len)
    {

        dLog("copySongRange len:%d", len );

        if (fromStart + len > FMS_MAX_SONG_LENGTH + 1 || toStart + len > FMS_MAX_SONG_LENGTH + 1 )
        {
            Log("Cant copy SongData out of range! from %d, to %d", fromStart + len,toStart + len  );
            return false;
        }

        if (toStart + len > toSD.song_length)
            toSD.song_length = toStart + len;


        for ( int i = 0 ; i <= len; i++)
        {
            for (int ch = 0; ch <= FMS_MAX_CHANNEL; ++ch)
                toSD.song[i+toStart][ch] = fromSD.song[i+fromStart][ch];
        }
        return true;
    }


};
