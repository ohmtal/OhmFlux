//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// 2026-01-08
// * overrite void OplController::tickSequencer() to mute channels
//-----------------------------------------------------------------------------

#pragma once

#include <OplController.h>

static const int  OPL_MIN_OCTAVE = 1;
static const int  OPL_MAX_OCTAVE = 8;

class FluxEditorOplController : public OplController
{
    typedef OplController Parent ;
public:

    virtual ~FluxEditorOplController() = default;

    bool mSyncInstrumentChannel = true;

    void setLoop( bool value ) {
        mSeqState.loop = value;
    }



    struct ChannelSettings {
        int octave = 4;
        int step = 1;
        bool active = true;
    };

    // Size is exactly the number of channels (e.g., 9)
    ChannelSettings mChannelSettings[FMS_MAX_CHANNEL+1];
    //--------------------------------------------------------------------------

    void setChannelActive( int channel , bool value )
    {
         mChannelSettings[std::clamp(channel, FMS_MIN_CHANNEL, FMS_MAX_CHANNEL)].active = value;
         if (!value)
             stopNote(channel);
    }

    void setAllChannelActive( bool value )
    {
        for ( int ch = FMS_MIN_CHANNEL; ch <= FMS_MAX_CHANNEL; ch++ )
            setChannelActive(ch, value);
    }


    bool getChannelActive( int channel  )
    {
        if (channel < FMS_MIN_CHANNEL || channel > FMS_MAX_CHANNEL)
            return false;
        return mChannelSettings[channel].active;
    }

    //--------------------------------------------------------------------------
    int getStepByChannel(int channel) {

        return mChannelSettings[std::clamp(channel, FMS_MIN_CHANNEL, FMS_MAX_CHANNEL)].step;
    }
    void setStepByChannel(int channel, int step) {
        step = std::clamp(step,0,99);
        mChannelSettings[std::clamp(channel, FMS_MIN_CHANNEL, FMS_MAX_CHANNEL)].step = step;
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
        return mChannelSettings[std::clamp(channel, FMS_MIN_CHANNEL, FMS_MAX_CHANNEL)].octave;
    }

    void setOctaveByChannel(int channel, int val) {
        mChannelSettings[std::clamp(channel, FMS_MIN_CHANNEL, FMS_MAX_CHANNEL)].octave = std::clamp(val, OPL_MIN_OCTAVE, OPL_MAX_OCTAVE);
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
    int getNoteWithOctave(int channel, const char* noteBase, int lOctaveAdd = 0) {
        char noteBuf[OPL_MAX_OCTAVE];
        snprintf(noteBuf, sizeof(noteBuf), "%s%d", noteBase, getOctaveByChannel(channel)+lOctaveAdd);
        // Use the base class OplController::getIdFromNoteName
        return this->getIdFromNoteName(noteBuf);
    }

    //--------------------------------------------------------------------------
    // Insert Row Logic (Inside OplController)
    void insertRowAt(SongDataFMS& sd, uint16_t start)
    {

        sd.song_length++;

        //  Move all rows below targetSeq down by one
        for (int i = sd.song_length; i > start; --i) {
            for (int ch = FMS_MIN_CHANNEL; ch <= FMS_MAX_CHANNEL ; ++ch)
            {
                if (getChannelActive(ch))
                    sd.song[i][ch] = sd.song[i-1][ch];
            }
        }
        // Clear the newly inserted row
        for (int ch = FMS_MIN_CHANNEL; ch <= FMS_MAX_CHANNEL; ++ch){
            if (getChannelActive(ch))
                sd.song[start][ch] = 0;
        }

    }

    //--------------------------------------------------------------------------
    // Delete Range Logic
    void deleteSongRange(SongDataFMS& sd, uint16_t start, uint16_t end) {
        int rangeLen = (end - start) + 1;
        // Shift data up
        for (int i = start; i < sd.song_length - rangeLen; ++i) {
            for (int ch = FMS_MIN_CHANNEL; ch <= FMS_MAX_CHANNEL; ++ch) {
                if (getChannelActive(ch))
                    sd.song[i][ch] = sd.song[i + rangeLen][ch];
            }
        }
        // Clear remaining rows at end
        for (int i = sd.song_length - rangeLen; i < sd.song_length; ++i) {
            for (int ch = FMS_MIN_CHANNEL; ch <= FMS_MAX_CHANNEL; ++ch)
                if (getChannelActive(ch))
                    sd.song[i][ch] = 0;
        }
        sd.song_length -= rangeLen;
    }
    //--------------------------------------------------------------------------
    bool clearSongRange(SongDataFMS& sd, uint16_t start, uint16_t end)
    {
        if (end > FMS_MAX_SONG_LENGTH )
            return false;

        for (int i = start; i <= end; ++i)
        {
            for (int ch = FMS_MIN_CHANNEL; ch <= FMS_MAX_CHANNEL; ++ch) {
                if (getChannelActive(ch))
                    sd.song[i][ch] = 0;
            }
        }
        return true;
    }
    //--------------------------------------------------------------------------
    bool clearSong(SongDataFMS& sd)
    {
        sd.song_length = 0;
        return clearSongRange(sd, 0,FMS_MAX_SONG_LENGTH);
    }
    //--------------------------------------------------------------------------
    bool copySongRange(SongDataFMS& fromSD, uint16_t fromStart,  SongDataFMS& toSD, uint16_t toStart, uint16_t len)
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
            for (int ch = FMS_MIN_CHANNEL; ch <= FMS_MAX_CHANNEL; ++ch)
                if (getChannelActive(ch))
                    toSD.song[i+toStart][ch] = fromSD.song[i+fromStart][ch];
        }
        return true;
    }
    //--------------------------------------------------------------------------
    // override for only playing active channel
    void tickSequencer() override
    {
        // testing after changes:
        // Parent::tickSequencer();
        // return ;

        const SongDataFMS& s = *mSeqState.current_song;

        if ( mSeqState.song_stopAt > s.song_length )
        {
            Log("ERROR: song stop is greater than song_length. thats bad!!!");
            mSeqState.song_stopAt = s.song_length;
        }
        // if (mSeqState.song_counter < s.song_length)
        if (mSeqState.song_needle < mSeqState.song_stopAt)
        {
            for (int ch = FMS_MIN_CHANNEL; ch <= FMS_MAX_CHANNEL; ch++) {
                int16_t raw_note = s.song[mSeqState.song_needle][ch];

                // Update UI/Debug state
                mSeqState.last_notes[ch + 1] = raw_note;
                mSeqState.note_updated = true;

                if (getChannelActive(ch))
                {
                    if (raw_note == -1) {
                        this->stopNote(ch);
                    } else if (raw_note > 0) {
                        this->playNoteDOS(ch, (uint8_t)raw_note);
                    }
                }
            }
            mSeqState.song_needle++;
        }
        if (mSeqState.song_needle >= s.song_length || mSeqState.song_needle >= mSeqState.song_stopAt){
            if (mSeqState.loop)
            {
                if (mSeqState.song_startAt > mSeqState.song_stopAt)
                {
                    Log("ERROR: song start is greater than song stop. thats bad!!!");
                    mSeqState.song_startAt = 0;

                }
                mSeqState.song_needle = mSeqState.song_startAt;
            }
            else setPlaying(false);
        }

    } //tickSequencer
    //--------------------------------------------------------------------------

};
