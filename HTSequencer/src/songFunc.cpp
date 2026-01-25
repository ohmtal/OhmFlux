#include "sequencerGui.h"
#include "sequencerMain.h"
#include <imgui_internal.h>

#include <algorithm>
#include <string>
#include <cctype>
//------------------------------------------------------------------------------
void SequencerGui::InsertRow(opl3::Pattern& pat, int rowIndex){
    auto& steps = pat.getStepsMutable();
    auto pos = steps.begin() + (rowIndex * SOFTWARE_CHANNEL_COUNT);
    // Insert 12 empty steps
    steps.insert(pos, SOFTWARE_CHANNEL_COUNT, SongStep{});
}
//------------------------------------------------------------------------------
opl3::Pattern* SequencerGui::getCurrentPattern()
{
    PatternEditorState& state = mPatternEditorState;
    SongData& song = mCurrentSong;

    if (song.patterns.empty())
        return nullptr;

    // Safety clamping
    if (state.currentPatternIdx < 0)
        state.currentPatternIdx = 0;

    if (state.currentPatternIdx >= (int)song.patterns.size()) {
        state.currentPatternIdx = (int)song.patterns.size() - 1;
    }

    // Return the memory address of the pattern in the vector
    return &song.patterns[state.currentPatternIdx];
}
//------------------------------------------------------------------------------
opl3::SongData SequencerGui::CreateTempSelection(const opl3::Pattern& activePattern, const PatternEditorState& state) {
    SongData temp;
    int minR = std::min(state.selectStartRow, state.selectEndRow);
    int maxR = std::max(state.selectStartRow, state.selectEndRow);

    Pattern subPattern;
    subPattern.mName = "SelectionClip";

    // Resize to accommodate the selected rows
    auto& steps = subPattern.getStepsMutable();
    int rowCount = (maxR - minR) + 1;
    steps.resize(rowCount * SOFTWARE_CHANNEL_COUNT);

    for (int r = minR; r <= maxR; r++) {
        for (int c = 0; c < SOFTWARE_CHANNEL_COUNT; c++) {
            if (state.isSelected(r, c)) {
                steps[(r - minR) * SOFTWARE_CHANNEL_COUNT + c] = activePattern.getStep(r, c);
            } else {
                // Fill unselected channels in the range with empty notes
                steps[(r - minR) * SOFTWARE_CHANNEL_COUNT + c].note = 255;
            }
        }
    }

    temp.patterns.push_back(subPattern);
    temp.orderList.push_back(0);
    return temp;
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
bool SequencerGui::exportSongToWav(std::string filename)
{
    if (!getMain()->getController()->songValid(mCurrentSong)) {
        LogFMT("[error] current Song is invalid.. can't export!");
        return false;
    }

    dLog("[info] start export to wav filename:%s", filename.c_str());


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
    g_FileDialog.mFilters = {".fms3"};
    g_FileDialog.mDirty  = true;
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
    if (!getMain()->getController()->songValid(mCurrentSong))
        return;

    stopSong();

    std::string fileName = fluxStr::sanitizeFilenameWithUnderScores(mCurrentSong.title) + ".wav";
    g_FileDialog.setFileName(fileName);
    g_FileDialog.mSaveMode = true;
    g_FileDialog.mSaveExt = ".wav";
    g_FileDialog.mLabel = "Export Song (.wav)";
    g_FileDialog.mFilters = {".wav"};
    g_FileDialog.mDirty  = true;

}
//------------------------------------------------------------------------------
void SequencerGui::newSong(){
    stopSong();
    mCurrentSong.init();

    // add a 128 row pattern
    // TODO: put this in a function? see also DrawNewPatternModal
    Pattern p;
    p.mName = "Pat 01";
    p.getStepsMutable().resize(128 * opl3::SOFTWARE_CHANNEL_COUNT);
    for(auto& s : p.getStepsMutable()) {
        s.note = 255;
        s.volume = 63;
        s.panning = 32;
    }

    mCurrentSong.patterns.push_back(std::move(p));
    uint8_t newPatternIdx = (uint8_t)mCurrentSong.patterns.size() - 1;
    mCurrentSong.orderList.push_back(newPatternIdx);

}
//------------------------------------------------------------------------------
void SequencerGui::stopSong(){
    getMain()->getController()->stopSong();
}
//------------------------------------------------------------------------------
