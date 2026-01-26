//-----------------------------------------------------------------------------
// ohmFlux ImguiTest
//-----------------------------------------------------------------------------

#include <fluxMain.h>
#include <SDL3/SDL_main.h> //<<< Android! and Windows
#include <gui/fluxGuiGlue.h>


class FluxGuiTest : public FluxMain
{
    typedef FluxMain Parent;
private:

    FluxGuiGlue* mGuiGlue = nullptr;
    FluxTexture* mBackgroundTex = nullptr;
    FluxRenderObject* mBackground = nullptr;

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


    void onDrawTopMost() override
    {
        mGuiGlue->DrawBegin();
        ImGui::ShowDemoWindow();

        const ImGuiViewport* main_viewport = ImGui::GetMainViewport();
        ImGui::SetNextWindowPos(ImVec2(main_viewport->WorkPos.x + 650, main_viewport->WorkPos.y + 20), ImGuiCond_FirstUseEver);
        ImGui::SetNextWindowSize(ImVec2(550, 680), ImGuiCond_FirstUseEver);
        ImGuiWindowFlags window_flags = 0;
        // Main body of the Demo window starts here.
        if (!ImGui::Begin("TableTest", nullptr, window_flags))
        {
            // Early out if the window is collapsed, as an optimization.
            ImGui::End();
            return;
        }
        DrawFullscreenTable(mConfig);
        ImGui::End();


        mGuiGlue->DrawEnd();

    } //Draw

    struct TableConfig {
        int TargetRow = 1;
        int TargetCol = 1;
        int TotalCols = 20;
        int TotalRows = 200;
        bool ScrollRequest = false; // Flag to trigger scroll logic
    };

    TableConfig mConfig;


    void DrawFullscreenTable(TableConfig& config) {
        // 1. Calculate Page Size based on AVAILABLE window space
        float avail_height = ImGui::GetContentRegionAvail().y;
        float row_height = ImGui::GetTextLineHeightWithSpacing();

        // Determine how many rows fit in the current window view
        int page_size = std::max(1, (int)(avail_height / row_height) - 1);

        // 2. Keyboard Input Handling
        if (ImGui::IsWindowFocused(ImGuiFocusedFlags_RootAndChildWindows)) {
            int move_y = 0;
            if (ImGui::IsKeyPressed(ImGuiKey_UpArrow))    move_y = -1;
            if (ImGui::IsKeyPressed(ImGuiKey_DownArrow))  move_y = 1;
            if (ImGui::IsKeyPressed(ImGuiKey_PageUp))     move_y = -page_size;
            if (ImGui::IsKeyPressed(ImGuiKey_PageDown))   move_y = page_size;

            if (move_y != 0) {
                config.TargetRow = std::clamp(config.TargetRow + move_y, 0, config.TotalRows - 1);
                config.ScrollRequest = true;
            }

            int move_x = (ImGui::IsKeyPressed(ImGuiKey_RightArrow)) - (ImGui::IsKeyPressed(ImGuiKey_LeftArrow));
            if (move_x != 0) {
                config.TargetCol = std::clamp(config.TargetCol + move_x, 0, config.TotalCols - 1);
                config.ScrollRequest = true;
            }
        }

        // 3. Table Setup
        ImGuiTableFlags flags = ImGuiTableFlags_ScrollX | ImGuiTableFlags_ScrollY |
        ImGuiTableFlags_RowBg | ImGuiTableFlags_Borders |
        ImGuiTableFlags_NoSavedSettings;

        // Setting size to (0,0) makes it fill the remaining window space
        if (ImGui::BeginTable("FullWindowTable", config.TotalCols, flags, ImVec2(0.0f, 0.0f))) {

            // Freeze Header Row (1) and First Column (1)
            ImGui::TableSetupScrollFreeze(1, 1);

            for (int i = 0; i < config.TotalCols; i++) {
                // Fixed width for columns ensures horizontal scrolling is predictable
                ImGui::TableSetupColumn(i == 0 ? "Fixed ID" : "Data", ImGuiTableColumnFlags_WidthFixed, 100.0f);
            }
            ImGui::TableHeadersRow();

            // 4. Optimized Rendering with Clipper
            ImGuiListClipper clipper;
            clipper.Begin(config.TotalRows);
            while (clipper.Step()) {
                for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
                    ImGui::TableNextRow();
                    for (int col = 0; col < config.TotalCols; col++) {
                        if (!ImGui::TableSetColumnIndex(col)) continue;

                        bool is_target = (row == config.TargetRow && col == config.TargetCol);

                        // Fixed Column (Left) Styling
                        if (col == 0) {
                            ImGui::TextDisabled("[%03d]", row);
                        } else {
                            ImGui::Text("Cell %d,%d", row, col);
                        }

                        // 5. Scroll Management
                        if (is_target) {
                            // Highlight the active cell
                            ImGui::TableSetBgColor(ImGuiTableBgTarget_CellBg, ImGui::GetColorU32(ImGuiCol_HeaderActive));

                            if (config.ScrollRequest) {
                                // Brings the target cell into the center of the window
                                ImGui::SetScrollHereX(0.5f);
                                ImGui::SetScrollHereY(0.5f);
                                config.ScrollRequest = false;
                            }
                        }
                    }
                }
            }
            ImGui::EndTable();
        }
    }


}; //class ImguiTest
//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    (void)argc; (void)argv;
    FluxGuiTest* game = new FluxGuiTest();
    game->mSettings.Company = "Ohmflux";
    game->mSettings.Caption = "ImguiTest";
    game->mSettings.enableLogFile = true;
    game->mSettings.WindowMaximized = true;
    // game->mSettings.ScreenWidth  = 1920;
    // game->mSettings.ScreenHeight = 1080;
    // game->mSettings.IconFilename = "assets/particles/Skull2.bmp";
    // game->mSettings.CursorFilename = "assets/particles/BloodHand.bmp";
    // game->mSettings.cursorHotSpotX = 10;
    // game->mSettings.cursorHotSpotY = 10;

    // LogFMT("TEST: My pref path would be:{}", SDL_GetPrefPath(game->mSettings.Company, game->mSettings.Caption ));

    game->Execute();
    SAFE_DELETE(game);
    return 0;
}


