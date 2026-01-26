#pragma once
#include <fluxMain.h>
#include <gui/fluxGuiGlue.h>
#include <imgui_internal.h>



class FluxGuiTest : public FluxMain
{
    typedef FluxMain Parent;
private:

    FluxGuiGlue* mGuiGlue = nullptr;
    FluxTexture* mBackgroundTex = nullptr;
    FluxRenderObject* mBackground = nullptr;


    struct TableConfig {
        int TargetRow = 1;
        int TargetCol = 1;
        int TotalCols = 20;
        int TotalRows = 200;
        bool ScrollRequest = false; // Flag to trigger scroll logic
    };

    struct PatternEditorState {
        int currentPatternIdx = 0;
        int cursorRow = 0;
        int cursorCol = 0;
        bool scrollToSelected = false;
        bool following = false;

        // Selection (start and end points for range-based operations)
        int selectStartRow = -1, selectStartCol = -1;
        int selectEndRow = -1, selectEndCol = -1;

        int rowCount = 256;
        int colCount = 12;



        bool isSelected(int r, int c) const {
            if (selectStartRow == -1) return false;
            int minR = std::min(selectStartRow, selectEndRow);
            int maxR = std::max(selectStartRow, selectEndRow);
            int minC = std::min(selectStartCol, selectEndCol);
            int maxC = std::max(selectStartCol, selectEndCol);
            return (r >= minR && r <= maxR && c >= minC && c <= maxC);
        }

        //------------------------------------------------------------------------------
        void moveCursorPosition(int rowAdd, int colAdd) {
            setCursorPosition(cursorRow + rowAdd, cursorCol+colAdd);
        }
        //------------------------------------------------------------------------------
        void setCursorPosition(int row, int col) {
            row = std::clamp(row, 0, rowCount -1 );
            col = std::clamp(col, 0, colCount -1 );
            if ( row == cursorRow && col == cursorCol )
                return ;
            cursorRow = row;
            cursorCol = col;
            scrollToSelected = true;
        }
        //------------------------------------------------------------------------------

    };



    TableConfig mTableConfig;
    PatternEditorState mPatternEditorState;


public:
    FluxGuiTest() {}
    ~FluxGuiTest() {}

    bool Initialize() override
    {
        if (!Parent::Initialize()) return false;

        mGuiGlue = new FluxGuiGlue(true);
        if (!mGuiGlue->Initialize())
            return false;

        mBackgroundTex = loadTexture("assets/background.bmp");

        // not centered ?!?!?! i guess center is not in place yet ?
        mBackground = new FluxRenderObject(mBackgroundTex);
        if (mBackground) {
            mBackground->setPos(getScreen()->getCenterF());
            mBackground->setSize(getScreen()->getScreenSize());
            queueObject(mBackground);
        }

        return true;
    }
    //--------------------------------------------------------------------------------------
    void Deinitialize() override
    {
        SAFE_DELETE(mGuiGlue);

        Parent::Deinitialize();
    }
    //--------------------------------------------------------------------------------------
    void onKeyEvent(SDL_KeyboardEvent event) override
    {
        bool isKeyUp = (event.type == SDL_EVENT_KEY_UP);
        if (event.key == SDLK_ESCAPE && isKeyUp)
            TerminateApplication();
    }
    //--------------------------------------------------------------------------------------
    void onMouseButtonEvent(SDL_MouseButtonEvent event) override    {    }
    //--------------------------------------------------------------------------------------
    void onEvent(SDL_Event event) override
    {
        mGuiGlue->onEvent(event);
    }
    //--------------------------------------------------------------------------------------
    void Update(const double& dt) override
    {
        Parent::Update(dt);
    }
    //--------------------------------------------------------------------------------------
    void onDrawTopMost() override; //Draw
    void DrawFullscreenTable(TableConfig& config);

    //--------------------------------------------------------------------------------------
    //------------------------------------------------------------------------------
    void DrawPatternRow(ImDrawList* drawList, int row, float startX, float rowY, float rowH, float colW, PatternEditorState& state) {
        for (int col = 0; col < state.colCount; col++) {
            ImVec2 p1 = { startX + (col * colW), rowY };
            ImVec2 p2 = { p1.x + colW, p1.y + rowH };

            bool isCursor = (state.cursorRow == row && state.cursorCol == col);
            bool isSelected = state.isSelected(row, col);

            ImU32 bgCol = IM_COL32(30, 30, 30, 255);
            if (isCursor)      bgCol = IM_COL32(100, 100, 255, 200); // Blue cursor
            else if (isSelected) bgCol = IM_COL32(60, 60, 90, 255);   // Muted blue selection
            if (!isCursor && !isSelected && (row % 4 == 0)) bgCol = IM_COL32(40, 40, 40, 255);


            drawList->AddRectFilled(p1, p2, bgCol);
            drawList->AddRect(p1, p2, IM_COL32(60, 60, 60, 255));

            char testData[16];
            snprintf(testData, 16, "%03d %03d", row, col);
            drawList->AddText(ImVec2(p1.x + 4, p1.y + 2), IM_COL32_WHITE, testData);
        }
    }
    //------------------------------------------------------------------------------
    void PatternEditorKeyBoardAction(PatternEditorState& state) {
        // Note: ImGui::IsKeyPressed handles repeat rates automatically in 2026
        if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))    state.moveCursorPosition(-1, 0);
        if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))  state.moveCursorPosition( 1, 0);
        if (ImGui::IsKeyPressed(ImGuiKey_LeftArrow))  state.moveCursorPosition( 0,-1);
        if (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) state.moveCursorPosition( 0, 1);

        if (ImGui::IsKeyPressed(ImGuiKey_PageUp))     state.moveCursorPosition(-16, 0);
        if (ImGui::IsKeyPressed(ImGuiKey_PageDown))   state.moveCursorPosition( 16, 0);

        // Using 2026 Modifier Constants
        bool ctrl = ImGui::GetIO().KeyCtrl; // or ImGui::IsKeyDown(ImGuiMod_Ctrl)
        if (ImGui::IsKeyPressed(ImGuiKey_Home))       state.setCursorPosition(ctrl ? 0 : state.cursorRow, 0);
        if (ImGui::IsKeyPressed(ImGuiKey_End))        state.setCursorPosition(ctrl ? state.rowCount - 1 : state.cursorRow, state.colCount - 1);
    }

    void RenderStablePatternEditor(PatternEditorState& state) {
        const float rowH = 20.0f;
        const float colW = 80.0f;

        ImGuiChildFlags cf = ImGuiChildFlags_Borders | ImGuiChildFlags_NavFlattened;
        ImGuiWindowFlags wf = ImGuiWindowFlags_AlwaysVerticalScrollbar;

        if (ImGui::BeginChild("PatternTable", ImVec2(0, 0), cf, wf)) {
            // 1. CAPTURE ANCHOR (Crucial: Do this BEFORE Dummy)
            // This is the absolute screen position of Row 0, Column 0
            ImVec2 tableStartPos = ImGui::GetCursorScreenPos();

            float scrollY = ImGui::GetScrollY();
            float winH = ImGui::GetWindowHeight();

            // 2. INPUT HANDLING
            if (ImGui::IsWindowFocused(ImGuiFocusedFlags_ChildWindows)) {
                PatternEditorKeyBoardAction(state);
            }

            // 3. LAYOUT (Reserve space)
            // This moves the internal ImGui cursor to the end of the table
            ImGui::Dummy(ImVec2(colW * state.colCount, state.rowCount * rowH));

            // 4. CLIPPING & RENDERING
            ImGuiWindow* window = ImGui::GetCurrentWindow();
            ImDrawList* drawList = ImGui::GetWindowDrawList();

            // Use the official ClipRect to prevent edge bleeding
            drawList->PushClipRect(window->ClipRect.Min, window->ClipRect.Max, true);

            int firstVisible = (int)floorf(scrollY / rowH);
            int lastVisible  = (int)ceilf((scrollY + winH) / rowH);

            for (int r = std::max(0, firstVisible); r < std::min(state.rowCount, lastVisible); r++) {
                // MATH: Anchor + (Row * Height) - Scroll
                // This is now independent of where the ImGui "Cursor" currently is
                float rowScreenY = tableStartPos.y + (r * rowH) - scrollY;

                DrawPatternRow(drawList, r, tableStartPos.x, rowScreenY, rowH, colW, state);
            }
            drawList->PopClipRect();

            // 5. PROGRAMMATIC SCROLLING
            if (state.scrollToSelected) {
                float targetY = state.cursorRow * rowH;
                float centerOffset = (winH * 0.5f) - (rowH * 0.5f);

                // In 2026, SetScrollY handles its own clamping, but manual clamping is safer
                float maxScroll = std::max(0.0f, (state.rowCount * rowH) - winH);
                ImGui::SetScrollY(std::clamp(targetY - centerOffset, 0.0f, maxScroll));

                state.scrollToSelected = false;
            }
        }
        ImGui::EndChild();
    }

    //--------------------------------------------------------------------------------------

}; //class ImguiTest
