#include "sequencerGui.h"
#include "sequencerMain.h"
#include <imgui_internal.h>

#include <algorithm>
#include <string>
#include <cctype>

//------------------------------------------------------------------------------

bool SequencerGui::isPlaying() {
    return getMain()->getController()->getSequencerState().playing;
}
uint16_t SequencerGui::getPlayingRow() {
    return getMain()->getController()->getSequencerState().rowIdx;
}
uint16_t SequencerGui::getPlayingSequenceIndex() {
    return getMain()->getController()->getSequencerState().orderIdx;
}


bool SequencerGui::playSong() {
    getMain()->getController()->getSequencerStateMutable()->playRange.init();
   return  getMain()->getController()->playSong(mCurrentSong, mLoopSong);
}
//------------------------------------------------------------------------------
bool SequencerGui::playSelected(PatternEditorState& state, bool forcePatternPlay)
{
    OPL3Controller::PlaySequencePatternRange& lPlayRange = getMain()->getController()->getSequencerStateMutable()->playRange;

    lPlayRange.init();
    lPlayRange.active = true;
    if (!forcePatternPlay &&  state.selection.active &&  state.selection.getCount() > 0 )
    {
        state.selection.sort(); //make sure its in the right position
        lPlayRange.startPoint[0] = state.selection.startPoint[0];
        lPlayRange.startPoint[1] = state.selection.startPoint[1];
        lPlayRange.stopPoint[0]  = state.selection.endPoint[0];
        lPlayRange.stopPoint[1]  = state.selection.endPoint[1];

        // dLog("PlaySelected PlayRange: %d, %d - %d, %d",
        //      lPlayRange.startPoint[0], lPlayRange.startPoint[1],
        //      lPlayRange.stopPoint[0], lPlayRange.stopPoint[1]
        //      );

    }
    lPlayRange.patternIdx  = state.currentPatternIdx;
    return  getMain()->getController()->playSong(mCurrentSong, mLoopSong);
}
//------------------------------------------------------------------------------
bool SequencerGui::clearSelectedSteps(PatternEditorState& state) {
    if (state.selection.getCount() < 1)
        return false;
    for (int r = state.selection.minRow(); r <= state.selection.maxRow(); ++r) {
        for (int c = state.selection.minCol(); c <= state.selection.maxCol(); ++c) {
            state.pattern->getStep(r, c).init();
        }
    }
    return true;
}


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
//FIXME songdata or pattern ... not sure ....
//    - Pattern is less overhead
//    - SongData can also be saved!
//    - pattern can be created with the right size
//    i think pattern won :P //FIXME TODO !!!
// opl3::SongData SequencerGui::CreateTempSelection(const opl3::Pattern& activePattern, const PatternEditorState& state) {
//     SongData temp;
//     int minR = std::min(state.selectStartRow, state.selectEndRow);
//     int maxR = std::max(state.selectStartRow, state.selectEndRow);
//
//     Pattern subPattern;
//     subPattern.mName = "SelectionClip";
//
//     // Resize to accommodate the selected rows
//     auto& steps = subPattern.getStepsMutable();
//     int rowCount = (maxR - minR) + 1;
//     steps.resize(rowCount * SOFTWARE_CHANNEL_COUNT);
//
//     for (int r = minR; r <= maxR; r++) {
//         for (int c = 0; c < SOFTWARE_CHANNEL_COUNT; c++) {
//             if (state.isSelected(r, c)) {
//                 steps[(r - minR) * SOFTWARE_CHANNEL_COUNT + c] = activePattern.getStep(r, c);
//             } else {
//                 // Fill unselected channels in the range with empty notes
//                 steps[(r - minR) * SOFTWARE_CHANNEL_COUNT + c].note = 255;
//             }
//         }
//     }
//
//     temp.patterns.push_back(subPattern);
//     temp.orderList.push_back(0);
//     return temp;
// }
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
    p.getStepsMutable().resize(64 * opl3::SOFTWARE_CHANNEL_COUNT);
    for(auto& s : p.getStepsMutable()) {
        s.note = 255;
        s.volume = 63;
        s.panning = 32;
    }

    // set default instruments

    // UNUSED !!!
    // int maxCnt = (int)getMain()->getController()->getSoundBank().size();
    //
    // for (int i = 0;  i < SOFTWARE_CHANNEL_COUNT; i++)
    // {
    //     mCurrentSong.channelInstrument[i]= (i < maxCnt) ? i : 0;
    //
    // }

    mCurrentSong.patterns.push_back(std::move(p));
    uint8_t newPatternIdx = (uint8_t)mCurrentSong.patterns.size() - 1;
    mCurrentSong.orderList.push_back(newPatternIdx);

}
//------------------------------------------------------------------------------
void SequencerGui::stopSong(){
    getMain()->getController()->stopSong();
}
//------------------------------------------------------------------------------
void SequencerGui::deleteAndShiftDataUp(PatternEditorState& state) {
    if (!state.pattern) return;

    int minC = state.selection.active ? state.selection.minCol() : state.cursorCol;
    int maxC = state.selection.active ? state.selection.maxCol() : state.cursorCol;
    int startR = state.selection.active ? state.selection.minRow() : state.cursorRow;
    int lastRow = state.pattern->getRowCount() - 1;

    // How many rows are we removing? (1 if no selection)
    int shiftAmount = state.selection.active ? state.selection.getRowCount() : 1;

    for (int c = minC; c <= maxC; ++c) {
        // Move data up
        for (int r = startR; r <= lastRow; ++r) {
            if (r + shiftAmount <= lastRow) {
                state.pattern->getStep(r, c) = state.pattern->getStep(r + shiftAmount, c);
            } else {
                // Clear the remaining space at the bottom
                state.pattern->getStep(r, c).init();
            }
        }
    }
    // Clean up selection after a delete-shift
    if (state.selection.active) state.selection.init();
}
//------------------------------------------------------------------------------
void SequencerGui::insertAndshiftDataDown(PatternEditorState& state) {
    if (!state.pattern) return;

    // Determine range: Selection or single cell?
    int minC = state.selection.active ? state.selection.minCol() : state.cursorCol;
    int maxC = state.selection.active ? state.selection.maxCol() : state.cursorCol;
    int startR = state.selection.active ? state.selection.minRow() : state.cursorRow;
    int lastRow = state.pattern->getRowCount() - 1;

    for (int c = minC; c <= maxC; ++c) {
        // Shift from bottom up to avoid overwriting data
        for (int r = lastRow; r > startR; --r) {
            state.pattern->getStep(r, c) = state.pattern->getStep(r - 1, c);
        }
        // Clear the entry point
        state.pattern->getStep(startR, c).init();
    }
}
//------------------------------------------------------------------------------
void SequencerGui::setInstrumentSelection(PatternEditorState& state, uint16_t instrumentIndex)
{
    if (!state.selection.active || !state.pattern) return;
    if (instrumentIndex >= getMain()->getController()->getSoundBank().size()) {
        Log("[error] changeInstrumentSelection instrumentIndex out of bounds! %d", instrumentIndex);
        return;
    }

    state.selection.sort();
    for (int r = state.selection.startPoint[0]; r <= state.selection.endPoint[0]; ++r) {
        for (int c = state.selection.startPoint[1]; c <= state.selection.endPoint[1]; ++c) {
            auto& step = state.pattern->getStep(r, c);
            step.instrument = instrumentIndex;
        }
    }
}
//------------------------------------------------------------------------------
void SequencerGui::transposeSelection(PatternEditorState& state, int semitones) {
    if (!state.selection.active || !state.pattern) return;

    state.selection.sort();
    for (int r = state.selection.startPoint[0]; r <= state.selection.endPoint[0]; ++r) {
        for (int c = state.selection.startPoint[1]; c <= state.selection.endPoint[1]; ++c) {
            auto& step = state.pattern->getStep(r, c);

            // Only transpose if there is an actual note (not a STOP or EMPTY)
            if (step.note < 128) {
                int newNote = (int)step.note + semitones;
                step.note = (uint8_t)std::clamp(newNote, 0, 127);
            }
        }
    }
}
//------------------------------------------------------------------------------
void SequencerGui::selectPatternRow(PatternEditorState& state){
    state.selection.active = true;
    state.selection.startPoint = { (uint16_t)state.cursorRow , 0};
    state.selection.endPoint = {
        (uint16_t)state.cursorRow
        , static_cast<unsigned short>(getCurrentPattern()->getColCount() -1)
    };
}
//------------------------------------------------------------------------------
void SequencerGui::selectPatternCol(PatternEditorState& state){
    state.selection.active = true;
    state.selection.startPoint = {0, (uint16_t)state.cursorCol};
    state.selection.endPoint = {
        static_cast<unsigned short>(getCurrentPattern()->getRowCount() -1)
        ,(uint16_t)state.cursorCol
    };
}
//------------------------------------------------------------------------------
void SequencerGui::selectPatternAll(PatternEditorState& state){
    state.selection.active = true;
    state.selection.startPoint = {0, 0};
    state.selection.endPoint = {
        static_cast<unsigned short>(getCurrentPattern()->getRowCount() -1)
        , static_cast<unsigned short>(getCurrentPattern()->getColCount() -1)
    };
}
//------------------------------------------------------------------------------
void SequencerGui::pasteStepsFromClipboard(PatternEditorState& state, const PatternClipboard& cb, bool useContextPoint) {
    if (!cb.active || !state.pattern) return;



    int sourceIdx = 0;
    for (int r = 0; r < cb.rows; ++r) {
        for (int c = 0; c < cb.cols; ++c) {

            int targetR = useContextPoint ? state.contextRow + r : state.cursorRow + r;
            int targetC = useContextPoint ? state.contextCol + r :state.cursorCol + c;

            // 3. Boundary Check: Ensure we don't write outside the pattern
            // Assuming your pattern has a way to check max rows/cols
            if (targetR < state.pattern->getRowCount() && targetC < 12 /* channel count */) {
                state.pattern->getStep(targetR, targetC) = cb.data[sourceIdx];
            }
            sourceIdx++;
        }
    }
    dLog("[info] Clipboard: Pasted area at R%d C%d", state.cursorRow, state.cursorCol);
}
//------------------------------------------------------------------------------
void SequencerGui::copyStepsToClipboard(PatternEditorState& state, PatternClipboard& cb) {
    if (!state.selection.active || !state.pattern) return;
    dLog("[info] 1: selection count:%d", state.selection.getCount());
    state.selection.sort();
    dLog("[info] 2: selection count:%d", state.selection.getCount());
    cb.clear();
    cb.rows = state.selection.getRowCount();
    cb.cols = state.selection.getColCount();
    cb.data.reserve(cb.rows * cb.cols);

    // 2. Extract steps from the pattern
    for (int r = state.selection.startPoint[0]; r <= state.selection.endPoint[0]; ++r) {
        for (int c = state.selection.startPoint[1]; c <= state.selection.endPoint[1]; ++c) {
            cb.data.push_back(state.pattern->getStep(r, c));
        }
    }
    cb.active = true;
    dLog("[info] Clipboard:Copied %d x %d area", cb.rows, cb.cols);
}
//------------------------------------------------------------------------------

