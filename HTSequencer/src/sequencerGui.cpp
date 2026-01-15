#include "sequencerGui.h"
#include "sequencerMain.h"
#include <gui/ImFileDialog.h>
#include <imgui_internal.h>
#include <utils/fluxSettingsManager.h>
#include <opl3_bridge_op2.h>

#include <algorithm>
#include <string>
#include <cctype>
//------------------------------------------------------------------------------

void SDLCALL ConsoleLogFunction(void *userdata, int category, SDL_LogPriority priority, const char *message)
{

    char lBuffer[512];
    if (priority == SDL_LOG_PRIORITY_ERROR)
    {
        // snprintf ensures you don't overflow lBuffer
        snprintf(lBuffer, sizeof(lBuffer), "[ERROR] %s", message);
    }
    else if (priority == SDL_LOG_PRIORITY_WARN)
    {
        snprintf(lBuffer, sizeof(lBuffer), "[WARN] %s", message);
    }
    else
    {
        snprintf(lBuffer, sizeof(lBuffer), "%s", message);
    }

    // bad if we are gone !!
    getMain()->getGui()->mConsole.AddLog("%s", message);


}
//------------------------------------------------------------------------------

void SequencerGui::Update(const double& dt)
{
    getMain()->getController()->consoleSongOutput(false);

}
//------------------------------------------------------------------------------
bool SequencerGui::Initialize()
{
    std::string lSettingsFile =
        getMain()->mSettings.getPrefsPath()
        .append(getMain()->mSettings.getSafeCaption())
        .append("_prefs.json");
    if (SettingsManager().Initialize(lSettingsFile))
    {

    } else {
        LogFMT("Error: Can not open setting file: {}", lSettingsFile);
    }

    mGuiSettings = SettingsManager().get("EditorGui::mEditorSettings", mDefaultGuiSettings);



    mGuiGlue = new FluxGuiGlue(true, false, nullptr);
    if (!mGuiGlue->Initialize())
        return false;



    // mSfxEditor = new FluxSfxEditor();
    // if (!mSfxEditor->Initialize())
    //     return false;
    //
    // mFMEditor = new FluxFMEditor();
    // if (!mFMEditor->Initialize())
    //     return false;
    //
    // mFMComposer = new FluxComposer( mFMEditor->getController());
    // if (!mFMComposer->Initialize())
    //     return false;

    // not centered ?!?!?! i guess center is not in place yet ?
    mBackground = new FluxRenderObject(getMain()->loadTexture("assets/background.png"));
    if (mBackground) {
        mBackground->setPos(getMain()->getScreen()->getCenterF());
        mBackground->setSize(getMain()->getScreen()->getScreenSize());
        getMain()->queueObject(mBackground);
    }

    // FileManager
    g_FileDialog.init( getGamePath(), {  ".op2", ".fmi", ".fms", ".wav", ".ogg" });
    // Console
    mConsole.OnCommand =  [&](ImConsole* console, const char* cmd) { OnConsoleCommand(console, cmd); };
    SDL_SetLogOutputFunction(ConsoleLogFunction, nullptr);



    return true;
}
//------------------------------------------------------------------------------
void SequencerGui::Deinitialize()
{

    //FIXME unbind to null!
    SDL_SetLogOutputFunction(nullptr, nullptr);

    // SAFE_DELETE(mFMComposer); //Composer before FMEditor !!!
    // SAFE_DELETE(mFMEditor);
    // SAFE_DELETE(mSfxEditor);
    SAFE_DELETE(mGuiGlue);

    if (SettingsManager().IsInitialized()) {
        SettingsManager().set("EditorGui::mEditorSettings", mGuiSettings);
        SettingsManager().save();
    }



}
//------------------------------------------------------------------------------
void SequencerGui::onEvent(SDL_Event event)
{
    mGuiGlue->onEvent(event);

    // if (mSfxEditor)
    //     mSfxEditor->onEvent(event);
    // if (mFMComposer)
    //     mFMComposer->onEvent(event);
    // if (mFMEditor)
    //     mFMEditor->onEvent(event);

}
//------------------------------------------------------------------------------
void SequencerGui::DrawMsgBoxPopup() {

    if (POPUP_MSGBOX_ACTIVE) {
        ImGui::OpenPopup(POPUP_MSGBOX_CAPTION.c_str());
        POPUP_MSGBOX_ACTIVE = false;
    }

    // 2. Always attempt to begin the modal
    // (ImGui only returns true here if the popup is actually open)
    ImVec2 center = ImGui::GetMainViewport()->GetCenter();
    ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

    if (ImGui::BeginPopupModal(POPUP_MSGBOX_CAPTION.c_str(), NULL, ImGuiWindowFlags_AlwaysAutoResize)) {
        ImGui::Text("%s",POPUP_MSGBOX_TEXT.c_str());
        ImGui::Separator();

        if (ImGui::Button("OK", ImVec2(120, 0))) {
            ImGui::CloseCurrentPopup();
        }
        ImGui::EndPopup();
    }
}

void SequencerGui::ShowMenuBar()
{
    if (ImGui::BeginMainMenuBar())
    {
        if (ImGui::BeginMenu("File"))
        {
            if (ImGui::MenuItem("Exit")) { getMain()->TerminateApplication(); }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Window"))
        {
            ImGui::MenuItem("File Manager", NULL, &mGuiSettings.mShowFileManager);
            ImGui::MenuItem("Console", NULL, &mGuiSettings.mShowConsole);



            // ImGui::MenuItem("IMGui Demo", NULL, &mGuiSettings.mShowDemo);
            // ImGui::MenuItem("IMGui Demo", NULL, &mGuiSettings.mShowDemo);
            // ImGui::Separator();
            // ImGui::MenuItem("Sound Effects Generator", NULL, &mGuiSettings.mShowSFXEditor);
            // ImGui::Separator();
            // ImGui::MenuItem("FM Composer", NULL, &mGuiSettings.mShowFMComposer);
            // ImGui::MenuItem("FM Instrument Editor", NULL, &mGuiSettings.mShowFMInstrumentEditor);
            // ImGui::MenuItem("FM Full Scale", NULL, &mEditorSettings.mShowCompleteScale);

            ImGui::MenuItem("Test Menu", NULL);

            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Style"))
        {
            if (ImGui::MenuItem("Dark")) {ImGui::StyleColorsDark(); }
            if (ImGui::MenuItem("Light")) {ImGui::StyleColorsLight(); }
            if (ImGui::MenuItem("Classic")) {ImGui::StyleColorsClassic(); }
            ImGui::EndMenu();
        }

        // ----------- Master Volume
        float rightOffset = 230.0f;
        ImGui::SameLine(ImGui::GetWindowWidth() - rightOffset);

        ImGui::SetNextItemWidth(100);
        float currentVol = AudioManager.getMasterVolume();
        if (ImGui::SliderFloat("##MasterVol", &currentVol, 0.0f, 2.0f, "Vol %.1f"))
        {
            if (!AudioManager.setMasterVolume(currentVol))
                Log("Error: Failed to set SDL Master volume");
        }
        if (ImGui::IsItemHovered()) ImGui::SetTooltip("Master Volume");


        // -----------
        ImGui::EndMainMenuBar();
    }

}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void SequencerGui::DrawGui()
{

    mGuiGlue->DrawBegin();
    ShowMenuBar();


    // if ( mEditorSettings.mShowDemo )
    // {
    //     // ImGui::SetNextWindowDockID(mGuiGlue->getDockSpaceId(), ImGuiCond_FirstUseEver);
    //     ImGui::ShowDemoWindow();
    // }


    // if ( mEditorSettings.mShowFMComposer ) {
    //     // ImGui::SetNextWindowDockID(mGuiGlue->getDockSpaceId(), ImGuiCond_FirstUseEver);
    //     mFMComposer->DrawComposer();
    // }

    // if ( mEditorSettings.mShowFMInstrumentEditor ) {
    //     // ImGui::SetNextWindowDockID(mGuiGlue->getDockSpaceId(), ImGuiCond_FirstUseEver);
    //     mFMEditor->DrawInstrumentEditor();
    // }

    // if ( mParameter.mShowPianoScale ) {
    //     // ImGui::SetNextWindowDockID(mGuiGlue->getDockSpaceId(), ImGuiCond_FirstUseEver);
    //     mFMEditor->DrawPianoScale();
    // }

    // if ( mEditorSettings.mShowCompleteScale ) {
    //     mFMEditor->DrawScalePlayer();
    // }

    // if (mEditorSettings.mShowSFXEditor) {
    //     // ImGui::SetNextWindowDockID(mGuiGlue->getDockSpaceId(), ImGuiCond_FirstUseEver);
    //     mSfxEditor->Draw();
    // }


    DrawMsgBoxPopup();

    mConsole.Draw("Console", &mGuiSettings.mShowConsole);



    if (mGuiSettings.mShowFileManager)
    {
        if (g_FileDialog.Draw()) {
            // LogFMT("File:{} Ext:{}", g_FileDialog.selectedFile, g_FileDialog.selectedExt);

            if (g_FileDialog.mSaveMode)
            {
                if (!g_FileDialog.mCancelPressed)
                {
                    // if (g_FileDialog.mSaveExt == ".fms")
                    // {
                    //     if (g_FileDialog.selectedExt == "")
                    //         g_FileDialog.selectedFile.append(g_FileDialog.mSaveExt);
                    //     mFMComposer->saveSong(g_FileDialog.selectedFile);
                    // }
                    // else
                    // if (g_FileDialog.mSaveExt == ".fmi")
                    // {
                    //     if (g_FileDialog.selectedExt == "")
                    //         g_FileDialog.selectedFile.append(g_FileDialog.mSaveExt);
                    //     mFMEditor->saveInstrument(g_FileDialog.selectedFile);
                    // }
                    // else
                    // if (g_FileDialog.mSaveExt == ".fms.wav")
                    // {
                    //     if (g_FileDialog.selectedExt == "")
                    //         g_FileDialog.selectedFile.append(g_FileDialog.mSaveExt);
                    //     mFMComposer->exportSongToWav(g_FileDialog.selectedFile);
                    // }

                }


                //FIXME sfx
                //reset
                g_FileDialog.reset();
            } else {
                if ( g_FileDialog.selectedExt == ".op2" )
                {
                    if (!opl3_bridge_op2::ImportOP2(g_FileDialog.selectedFile, getMain()->getController()->mSoundBank) )
                        Log("[error] Failed to load %s",g_FileDialog.selectedFile.c_str() );
                    else
                         Log("Soundbank %s loaded! %zu instruments",g_FileDialog.selectedFile.c_str(), getMain()->getController()->mSoundBank.size() );
                }

                // else
                // if ( g_FileDialog.selectedExt == ".fmi" )
                //     mFMEditor->loadInstrument(g_FileDialog.selectedFile);
                // else
                // if ( g_FileDialog.selectedExt == ".fms" )
                //     mFMComposer->loadSong(g_FileDialog.selectedFile);

            }
        }

    }



    InitDockSpace();  

    mGuiGlue->DrawEnd();
}
//------------------------------------------------------------------------------
//------------------------------------------------------------------------------
void SequencerGui::onKeyEvent(SDL_KeyboardEvent event)
{
    // if ( mEditorSettings.mShowFMComposer )
    //     mFMComposer->onKeyEvent(event);
}
//------------------------------------------------------------------------------
void SequencerGui::InitDockSpace()
{
    if (mGuiSettings.mEditorGuiInitialized)
        return; 

    mGuiSettings.mEditorGuiInitialized = true;

    // ImGuiID dockspace_id = mGuiGlue->getDockSpaceId();
    //
    // // Clear any existing layout
    // ImGui::DockBuilderRemoveNode(dockspace_id);
    // ImGui::DockBuilderAddNode(dockspace_id, ImGuiDockNodeFlags_DockSpace);
    // ImGui::DockBuilderSetNodeSize(dockspace_id, ImVec2(1920, 990));
    //
    // // Replicate your splits (matches your ini data)
    // ImGuiID dock_main_id = dockspace_id;
    // ImGuiID dock_id_left, dock_id_right, dock_id_central;
    //
    // // First Split: Left (FM Instrument Editor) vs Right (Everything else)
    // // ratio ~413/1920 = 0.215f
    // dock_id_left = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Left, 0.215f, nullptr, &dock_main_id);
    //
    // // Second Split: Center (Composer/Generator) vs Right (File Browser)
    // // ratio ~259/1505 = 0.172f from the right
    // dock_id_right = ImGui::DockBuilderSplitNode(dock_main_id, ImGuiDir_Right, 0.172f, nullptr, &dock_id_central);
    //
    // // Dock the Windows to these IDs
    // ImGui::DockBuilderDockWindow("FM Instrument Editor", dock_id_left);
    // ImGui::DockBuilderDockWindow("File Browser", dock_id_right);
    // ImGui::DockBuilderDockWindow("Sound Effects Generator", dock_id_central);
    // ImGui::DockBuilderDockWindow("FM Song Composer", dock_id_central);
    //
    //
    // ImGui::DockBuilderFinish(dockspace_id);
}
//------------------------------------------------------------------------------
void SequencerGui::OnConsoleCommand(ImConsole* console, const char* cmdline)
{


    std::string cmd = fluxStr::getWord(cmdline,0);



    if (cmd == "play")
    {
        /*
         *     struct SongStep {
         *        uint8_t note = 0;        // 0=None, 1-96 (C-0 to B-7), 255=Off
         *        uint8_t instrument = 0;  // Index to Instrument table
         *        uint8_t volume = 63;     // 0-63 OPL range
         *        uint16_t effect = 0;     // Command (e.g., 0x0Axy for Volume Slide)
         * };
        */

        std::string tone = "C-3";
        if (fluxStr::getWord(cmdline,1) != "")
            tone = fluxStr::toUpper(fluxStr::getWord(cmdline,1));


        uint8_t instrument = fluxStr::strToInt(fluxStr::getWord(cmdline,2) , 1);


        SongStep step{opl3::NoteToValue(tone),instrument,63};

        getMain()->getController()->playNote(0,step);
    }
    else
    if (cmd == "stop")
    {
        getMain()->getController()->stopNote(0);
    }
    else
    if (cmd == "list")
    {
        for (int i = 0; i < getMain()->getController()->mSoundBank.size(); i++)
        {
            // Access individual instruments
            OplInstrument instrument = getMain()->getController()->mSoundBank[i];
            Log("#%d [%d] %s",i,instrument.isFourOp, instrument.name.c_str() );
        }
    }
    else
    if (cmd == "scale")
    {
        uint8_t instrument = fluxStr::strToInt(fluxStr::getWord(cmdline,1) , 1);
        Log ("Using Instrument %d",instrument);
        myTestSong = getMain()->getController()->createScaleSong(instrument);
        getMain()->getController()->playSong(myTestSong);
    }
    else
    if (cmd == "effects")
    {
        uint8_t instrument = fluxStr::strToInt(fluxStr::getWord(cmdline,1) , 1);
        Log ("Using Instrument %d",instrument);
        myTestSong = getMain()->getController()->createEffectTestSong(instrument);
        getMain()->getController()->playSong(myTestSong);
    }
    else
    if (cmd == "dump")
    {
        uint8_t instrument = fluxStr::strToInt(fluxStr::getWord(cmdline,1) , 1);
        Log ("Using Instrument %d",instrument);
        getMain()->getController()->dumpInstrument(instrument);
    }
    else
        if (cmd == "default")
        {
            getMain()->getController()->initDefaultBank();
        }
    else
    {
        console->AddLog("unknown command %s", cmd.c_str());
    }




}
//------------------------------------------------------------------------------
