#include "sequencerGui.h"
#include "sequencerMain.h"
#include <imgui_internal.h>

#include <algorithm>
#include <string>
#include <cctype>
//------------------------------------------------------------------------------

bool SequencerGui::exportSongToWav(std::string filename)
{
    if (!getMain()->getController()->songValid(mCurrentSong)) {
        LogFMT("[error] current Song is invalid.. can't export!");
        return false;
    }

    dLog("[info] start export to wav filename:%s", filename.c_str());

    //getMain()->getController()->exportToWav(mCurrentSong, filename, nullptr);


    if (mCurrentExport || !getMain()->getController()->songValid(mCurrentSong)) return false; // Already exporting!

    mCurrentExport = new ExportTask();
    mCurrentExport->controller = getMain()->getController();
    mCurrentExport->song = mCurrentSong;
    mCurrentExport->filename = filename;
    mCurrentExport->applyEffects = mExportWithEffects;

    // Create the thread
    SDL_Thread* thread = SDL_CreateThread(ExportThreadFunc, "WavExportThread", mCurrentExport);

    if (!thread) {
        delete mCurrentExport;
        mCurrentExport = nullptr;
        return false;
    }

    // Detach the thread so it cleans itself up when finished
    SDL_DetachThread(thread);
    return true;
}
//------------------------------------------------------------------------------
void SequencerGui::callSaveSong() {
    if (!getMain()->getController()->songValid(mCurrentSong))
        return;

    std::string fileName = fluxStr::sanitizeFilenameWithUnderScores(mCurrentSong.title) + ".fms3";

    g_FileDialog.setFileName(fileName);
    g_FileDialog.mSaveMode = true;
    g_FileDialog.mSaveExt = ".fms3";
    g_FileDialog.mLabel = "Save Song (.fms3)";
}
//------------------------------------------------------------------------------
/*
 * Playmodes:
 *      0 (default)= With startat/endAt
 *      1 = only selection
 *      2 = full Song
 *      3 = autodetect
 *      4 = play from position
 */
void SequencerGui::playSong(U8 playMode)
{

    getMain()->getController()->playSong(mCurrentSong, mLoopSong);
    // FIXME
    // switch (playMode)
    // {
    //     case 1: mController->playSong(mSongData, mLoop, getSelectionMin(), getSelectionMax() + 1);break;
    //     case 2: mController->playSong(mSongData, mLoop); break;
    //     case 3:
    //         if (getSelectionLen() > 1)
    //             playSong(1);
    //     else
    //         playSong(0);
    //     break;
    //     case 4: mController->playSong(mSongData, mLoop, mSelectedRow);break;
    //     default: mController->playSong(mSongData, mLoop, mStartAt, mEndAt); break;
    // }
}
//------------------------------------------------------------------------------
void SequencerGui::callExportSong(){
    stopSong();
    std::string fileName = fluxStr::sanitizeFilenameWithUnderScores(mCurrentSong.title) + ".wav";
    g_FileDialog.setFileName(fileName);
    g_FileDialog.mSaveMode = true;
    g_FileDialog.mSaveExt = ".wav";
    g_FileDialog.mLabel = "Export Song (.wav)";
}
//------------------------------------------------------------------------------
void SequencerGui::newSong(){
    stopSong();
    mCurrentSong.init();
}
//------------------------------------------------------------------------------
void SequencerGui::stopSong(){
    getMain()->getController()->stopSong();
}
//------------------------------------------------------------------------------
