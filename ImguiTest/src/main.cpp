//-----------------------------------------------------------------------------
// ohmFlux ImguiTest
//-----------------------------------------------------------------------------
// FIXME scaling on size changed...
//      => FluxScreen::updateWindowSize need a event for this
//-----------------------------------------------------------------------------


#include <fluxMain.h>
#include <SDL3/SDL_main.h> //<<< Android! and Windows

#include <imgui.h>
#include <backends/imgui_impl_sdl3.h>
#include <backends/imgui_impl_opengl3.h>

class ImGuiTest : public FluxMain
{
    typedef FluxMain Parent;
private:
    ImGuiIO* mGuiIO = nullptr;
    ImGuiStyle mBaseStyle;
    bool mScaleImGui = false;

public:
    ImGuiTest() {}
    ~ImGuiTest() {}

    bool Initialize() override
    {
        if (!Parent::Initialize()) return false;

        // Setup Dear ImGui context
        IMGUI_CHECKVERSION();
        ImGui::CreateContext();
        mGuiIO = &ImGui::GetIO();
        mGuiIO->ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;
        mGuiIO->DisplaySize = ImVec2(getScreen()->getHeight(), getScreen()->getWidth());

        mBaseStyle = ImGui::GetStyle();

        // Setup Platform/Renderer backends
        ImGui_ImplSDL3_InitForOpenGL(getScreen()->getWindow(), getScreen()->getGLContext());

        #if defined(__EMSCRIPTEN__) || defined(__ANDROID__)
        const char* glsl_version = "#version 300 es";
        #elif defined(__APPLE__)
        const char* glsl_version = "#version 150"; // Required for GL 3.2+ Core on macOS
        #else
        const char* glsl_version = "#version 130"; // Standard for GL 3.0+ on Desktop
        #endif

        ImGui_ImplOpenGL3_Init(glsl_version);
        return true;
    }
    //--------------------------------------------------------------------------------------
    void Deinitialize() override
    {
        ImGui_ImplOpenGL3_Shutdown();
        ImGui_ImplSDL3_Shutdown();
        ImGui::DestroyContext();

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
        if (event.type ==  FLUX_EVENT_SCALE_CHANGED)
        {

            mGuiIO->DisplaySize = ImVec2(getScreen()->getHeight(), getScreen()->getWidth());
            if ( mScaleImGui )
            {
                float lMasterScale = getScreen()->getScaleX(); // Usually better to scale UI based on height
                ImGui::GetStyle() = mBaseStyle; // Reset to the clean copy
                ImGui::GetStyle().ScaleAllSizes(lMasterScale);
                ImGui::GetStyle().FontScaleDpi = lMasterScale;
            }
        }


        ImGui_ImplSDL3_ProcessEvent(&event);
    }
    //--------------------------------------------------------------------------------------
    void Update(const double& dt) override
    {

        Parent::Update(dt);
    }
    //--------------------------------------------------------------------------------------
    void onDraw() override
    {
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        // overwritten in ShowDemoWindow
        // // Set position (x: 50, y: 50)
        // ImGui::SetNextWindowPos(ImVec2(0, 20), ImGuiCond_Always ); //ImGuiCond_FirstUseEver);
        // // Set size if desired
        // ImGui::SetNextWindowSize(ImVec2(550, 200), ImGuiCond_FirstUseEver);

        ImGui::ShowDemoWindow();

        //>>>>>>>>>>>>>>>>>>>>>>>>>> EDIT TABLE DEMO:
        ImGui::Begin("My Editable Settings");

            static char data[3][32] = { "Item 1", "Item 2", "Item 3" };
            static float values[3] = { 10.0f, 20.0f, 30.0f };

            if (ImGui::BeginTable("EditableTable", 2, ImGuiTableFlags_Borders | ImGuiTableFlags_Resizable))
            {
                ImGui::TableSetupColumn("Name");
                ImGui::TableSetupColumn("Value");
                ImGui::TableHeadersRow();

                for (int row = 0; row < 3; row++)
                {
                    ImGui::TableNextRow();

                    // Column 0: Text Input
                    ImGui::TableNextColumn();
                    ImGui::PushID(row); // Unique ID for each row's widgets
                    ImGui::SetNextItemWidth(-FLT_MIN); // Fill the cell width
                    ImGui::InputText("##name", data[row], IM_ARRAYSIZE(data[row]));

                    // Column 1: Float Drag
                    ImGui::TableNextColumn();
                    ImGui::SetNextItemWidth(-FLT_MIN);
                    ImGui::DragFloat("##value", &values[row]);

                    ImGui::PopID();
                }
                ImGui::EndTable();
            }
        ImGui::End();

        // -------------------------------
        // another window

        ImGui::SetNextWindowPos(ImVec2(300, 20), ImGuiCond_Always ); //ImGuiCond_FirstUseEver);

        ImGui::SetNextWindowSizeConstraints(ImVec2(200, 200), ImVec2(1000, 1000));
        ImGui::Begin("Resizable Window");
        // ... content ...
        ImGui::End();

        // -------------------------------

        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImGui::GetIO().DisplaySize);
        ImGuiWindowFlags flags = ImGuiWindowFlags_NoDecoration | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoSavedSettings | ImGuiWindowFlags_NoBringToFrontOnFocus;

        ImGui::Begin("BackgroundUI", NULL, flags);
            if (ImGui::Button("Left")) { }
            ImGui::SameLine();
            if (ImGui::Button("Middle")) { }
            ImGui::SameLine(150); // Optional: forces next widget to start at x=150
            if (ImGui::Button("Right")) { }

            if (ImGui::Button("quit"))
            {
                TerminateApplication();
            }

        ImGui::End();

        //<<<<<<<<<<<<<<<<<<<<<<<<<

        ImGui::Render();

        ImDrawData* drawData = ImGui::GetDrawData();
        if (drawData)
        {
            ImGui_ImplOpenGL3_RenderDrawData(drawData);
        }
    } //Draw
}; //classe ImguiTest
//------------------------------------------------------------------------------
// Main
//------------------------------------------------------------------------------
int main(int argc, char* argv[])
{
    (void)argc; (void)argv;
    ImGuiTest* game = new ImGuiTest();
    game->mSettings.Company = "Ohmflux";
    game->mSettings.Caption = "ImguiTest";
    game->mSettings.enableLogFile = true;
    // game->mSettings.IconFilename = "assets/particles/Skull2.bmp";
    // game->mSettings.CursorFilename = "assets/particles/BloodHand.bmp";
    // game->mSettings.cursorHotSpotX = 10;
    // game->mSettings.cursorHotSpotY = 10;

    // LogFMT("TEST: My pref path would be:{}", SDL_GetPrefPath(game->mSettings.Company, game->mSettings.Caption ));

    game->Execute();
    SAFE_DELETE(game);
    return 0;
}


