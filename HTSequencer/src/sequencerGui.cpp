#include "sequencerGui.h"
#include "sequencerMain.h"
#include <gui/ImFileDialog.h>
#include <imgui_internal.h>
#include <utils/fluxSettingsManager.h>
#include <opl3_bridge_op2.h>
#include <opl3_bridge_wopl.h>
#include <opl3_bridge_fm.h>
#include <opl3_bridge_sbi.h>
#include "opl3_bridge_soundflux.h"
//
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
void SequencerGui::ShowFileManager(){
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


            g_FileDialog.reset();
        } else {
            if ( g_FileDialog.selectedExt == ".op2" )
            {
                if (!opl3_bridge_op2::importBank(g_FileDialog.selectedFile, getMain()->getController()->mSoundBank) )
                    Log("[error] Failed to load %s",g_FileDialog.selectedFile.c_str() );
                else
                    Log("Soundbank %s loaded! %zu instruments",g_FileDialog.selectedFile.c_str(), getMain()->getController()->mSoundBank.size() );
            }
            else
            if ( g_FileDialog.selectedExt == ".wopl" )
            {
                if (!opl3_bridge_wopl::importBank(g_FileDialog.selectedFile, getMain()->getController()->mSoundBank) )
                    Log("[error] Failed to load %s",g_FileDialog.selectedFile.c_str() );
                else
                    Log("Soundbank %s loaded! %zu instruments",g_FileDialog.selectedFile.c_str(), getMain()->getController()->mSoundBank.size() );
            }
            else
            if ( g_FileDialog.selectedExt == ".sbi" )
            {
                OplInstrument newIns;
                if (opl3_bridge_sbi::loadInstrument(g_FileDialog.selectedFile, newIns))
                {
                    getMain()->getController()->mSoundBank.push_back(newIns);
                    Log("Loaded %s to %zu", g_FileDialog.selectedFile.c_str(), getMain()->getController()->mSoundBank.size()-1);
                }  else {
                    Log("[error] failed to load:%s",g_FileDialog.selectedFile.c_str());
                }
            }
            else
            if ( g_FileDialog.selectedExt == ".fmi" ){
                std::array<uint8_t, 24> instrumentData;
                if (opl3_bridge_fm::loadInstrumentData(g_FileDialog.selectedFile,instrumentData)) {
                    OplInstrument newIns=opl3_bridge_fm::toInstrument(
                        std::string( fluxStr::extractFilename(g_FileDialog.selectedFile) ),
                        instrumentData
                    );
                    getMain()->getController()->mSoundBank.push_back(newIns);
                    Log("Loaded %s to %zu", g_FileDialog.selectedFile.c_str(), getMain()->getController()->mSoundBank.size()-1);

                } else {
                    Log("[error] failed to load:%s",g_FileDialog.selectedFile.c_str());
                }

            }
            else
            if ( g_FileDialog.selectedExt == ".fms" ) {
                if (opl3_bridge_fm::loadSongFMS(g_FileDialog.selectedFile, myTestSong)) {
                    getMain()->getController()->mSoundBank = myTestSong.instruments;

                } else {
                    Log("[error] failed to load:%s",g_FileDialog.selectedFile.c_str());
                }
            }


        }
    }
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


    auto* controller = getMain()->getController();
    controller->getDSPBitCrusher()->setEnabled(SettingsManager().get("DSP_BitCrusher_ON", false));
    controller->getDSPChorus()->setEnabled(SettingsManager().get("DSP_Chorus_ON", false));
    controller->getDSPReverb()->setEnabled(SettingsManager().get("DSP_Reverb_ON", false));
    controller->getDSPWarmth()->setEnabled(SettingsManager().get("DSP_Warmth_ON", false));
    controller->getDSPLimiter()->setEnabled(SettingsManager().get("DSP_LIMITER_ON", true));
    controller->getDSPEquilzer9Band()->setEnabled(SettingsManager().get("DSP_EQ9BAND_ON", true));


    controller->getDSPBitCrusher()->setSettings(SettingsManager().get<DSP::BitcrusherSettings>("DSP_BitCrusher", DSP::AMIGA_BITCRUSHER));
    controller->getDSPChorus()->setSettings(SettingsManager().get<DSP::ChorusSettings>("DSP_Chorus", DSP::LUSH80s_CHORUS));
    controller->getDSPReverb()->setSettings(SettingsManager().get<DSP::ReverbSettings>("DSP_Reverb", DSP::HALL_REVERB));
    controller->getDSPWarmth()->setSettings(SettingsManager().get<DSP::WarmthSettings>("DSP_Warmth", DSP::TUBEAMP_WARMTH));
    controller->getDSPEquilzer9Band()->setSettings( SettingsManager().get<DSP::Equalizer9BandSettings>("DSP_EQ9BAND", DSP::FLAT_EQ ));


    getScreenObject()->setWindowMaximized(SettingsManager().get("WINDOW_MAXIMIZED", getMain()->mSettings.WindowMaximized ));



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
    g_FileDialog.init( getGamePath(), { ".sbi", ".op2",".wopl", ".fmi", ".fms", ".wav", ".ogg" });
    // Console
    mConsole.OnCommand =  [&](ImConsole* console, const char* cmd) { OnConsoleCommand(console, cmd); };
    SDL_SetLogOutputFunction(ConsoleLogFunction, nullptr);


    // tests
    mOpl3Tests = std::make_unique<OPL3Tests>(getMain()->getController());



    return true;
}
//------------------------------------------------------------------------------
void SequencerGui::Deinitialize()
{

    SDL_SetLogOutputFunction(nullptr, nullptr);

    // SAFE_DELETE(mFMComposer); //Composer before FMEditor !!!
    // SAFE_DELETE(mFMEditor);
    // SAFE_DELETE(mSfxEditor);
    SAFE_DELETE(mGuiGlue);

    if (SettingsManager().IsInitialized()) {
        SettingsManager().set("EditorGui::mEditorSettings", mGuiSettings);

        auto* controller = getMain()->getController();
        SettingsManager().set("DSP_BitCrusher", controller->getDSPBitCrusher()->getSettings());
        SettingsManager().set("DSP_Chorus",     controller->getDSPChorus()->getSettings());
        SettingsManager().set("DSP_Reverb",     controller->getDSPReverb()->getSettings());
        SettingsManager().set("DSP_Warmth",     controller->getDSPWarmth()->getSettings());
        SettingsManager().set("DSP_EQ9BAND",     controller->getDSPEquilzer9Band()->getSettings());

        SettingsManager().set("DSP_BitCrusher_ON", controller->getDSPBitCrusher()->isEnabled());
        SettingsManager().set("DSP_Chorus_ON", controller->getDSPChorus()->isEnabled());
        SettingsManager().set("DSP_Reverb_ON", controller->getDSPReverb()->isEnabled());
        SettingsManager().set("DSP_Warmth_ON", controller->getDSPWarmth()->isEnabled());
        SettingsManager().set("DSP_LIMITER_ON", controller->getDSPLimiter()->isEnabled());
        SettingsManager().set("DSP_EQ9BAND_ON", controller->getDSPEquilzer9Band()->isEnabled());


        SettingsManager().set("WINDOW_MAXIMIZED", getScreenObject()->getWindowMaximized());

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
//------------------------------------------------------------------------------
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
            ImGui::MenuItem("Sound Bank", NULL, &mGuiSettings.mShowSoundBankEditor);
            ImGui::MenuItem("Scale Player", NULL, &mGuiSettings.mShowScalePlayer);
            ImGui::MenuItem("Digital Sound Processing", NULL, &mGuiSettings.mShowDSP);
            ImGui::Separator();
            ImGui::MenuItem("File Manager", NULL, &mGuiSettings.mShowFileManager);
            ImGui::MenuItem("Console", NULL, &mGuiSettings.mShowConsole);



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

    DrawMsgBoxPopup();

    mConsole.Draw("Console", &mGuiSettings.mShowConsole);

    if (mGuiSettings.mShowScalePlayer) RenderScalePlayerUI(true);

    ShowDSPWindow();
    ShowSoundBankWindow();


    if (mGuiSettings.mShowFileManager) ShowFileManager();



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

        std::string tone = "C-3";
        if (fluxStr::getWord(cmdline,1) != "")
            tone = fluxStr::toUpper(fluxStr::getWord(cmdline,1));

        uint8_t instrument = fluxStr::strToInt(fluxStr::getWord(cmdline,2) , 0);
        SongStep step{opl3::NoteToValue(tone),instrument,63};
        getMain()->getController()->playNote(0,step);
    }
    else
    if (cmd == "stop")
    {
        getMain()->getController()->stopNote(0);
        getMain()->getController()->silenceAll(false);
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
        int instrument = fluxStr::strToInt(fluxStr::getWord(cmdline,1) , -1);
        if ( instrument < 0 )
            instrument = mCurrentInstrumentId;
        Log ("Using Instrument %d",instrument);
        myTestSong = mOpl3Tests->createScaleSong(instrument);
        getMain()->getController()->playSong(myTestSong);
    }
    else
    if (cmd == "effects")
    {
        int instrument = fluxStr::strToInt(fluxStr::getWord(cmdline,1) , -1);
        if ( instrument < 0 )
            instrument = mCurrentInstrumentId;
        Log ("Using Instrument %d",instrument);
        myTestSong = mOpl3Tests->createEffectTestSong(instrument);
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
        if (cmd == "test")
        {

            mConsole.ClearLog();
            Log("TEST piano tone and sound like a piano (not as good as in opl3bankeeditor)");
            getMain()->getController()->stopNote(0);

            // 1. CHIP INITIALISIERUNG
            getMain()->getController()->write(0x01, 0x00);   // Test-Bits AUS (Wichtig für Ton)
            getMain()->getController()->write(0x05, 0x01);   // OPL3 Erweiterungen AN (Bank 0)
            getMain()->getController()->write(0x104, 0x01);  // 4-OP Modus für Kanal 0/3 AN (Bank 1)

            // 2. PAAR 0 (Kanal 0) - "Der Hammer"
            getMain()->getController()->write(0x20, 0x01);   // Multiplier
            getMain()->getController()->write(0x40, 0x10);   // TL (Lautstärke Modulator)
            getMain()->getController()->write(0x60, 0xF2);   // Attack (F), Decay (2)
            getMain()->getController()->write(0x80, 0x02);   // Sustain Level 0, Release 2 (EG-Type 0 = Halten)
            getMain()->getController()->write(0xE0, 0x00);   // Sinus

            getMain()->getController()->write(0x23, 0x01);   // Multiplier
            getMain()->getController()->write(0x43, 0x00);   // TL (Lautstärke Carrier - MAX)
            getMain()->getController()->write(0x63, 0xF2);   // Attack (F), Decay (2)
            getMain()->getController()->write(0x83, 0x02);   // Sustain Level 0, Release 2
            getMain()->getController()->write(0xE3, 0x00);   // Sinus

            // 3. PAAR 1 (Kanal 3) - "Der Körper"
            getMain()->getController()->write(0x28, 0x01);
            getMain()->getController()->write(0x48, 0x10);
            getMain()->getController()->write(0x68, 0xF2);
            getMain()->getController()->write(0x88, 0x02);
            getMain()->getController()->write(0xE8, 0x00);

            getMain()->getController()->write(0x2B, 0x01);
            getMain()->getController()->write(0x4B, 0x00);   // Zweiter Carrier (MAX Lautstärke)
            getMain()->getController()->write(0x6B, 0xF2);
            getMain()->getController()->write(0x8B, 0x02);
            getMain()->getController()->write(0xEB, 0x00);

            // 4. PANNING & ALGORITHMUS
            // Wir nutzen hier 4-OP Algorithmus 0 (FM -> FM -> FM -> FM) für maximale Wirkung
            getMain()->getController()->write(0xC0, 0x30);   // Links+Rechts, Anschlusstyp 0
            getMain()->getController()->write(0xC3, 0x30);   // Links+Rechts, Anschlusstyp 0

            // 5. FREQUENZ & KEY-ON
            getMain()->getController()->write(0xA0, 0x98);   // F-Number Low (Note A-4)
            getMain()->getController()->write(0xB0, 0x31);   // Block 4 + Key-On

            // getMain()->getController()->flush();
    }
    else
    if (cmd == "test2") {

        mConsole.ClearLog();
        Log("TEST2 piano ");
        getMain()->getController()->stopNote(0);

        // 1. HARD RESET & MODE ENABLE
        getMain()->getController()->write(0x01,  0x00); // Mute off
        getMain()->getController()->write(0x05,  0x01); // OPL3 On (Bank 0)
        getMain()->getController()->write(0x104, 0x01); // 4-OP Ch 0 On (Bank 1)

        // 2. PAIR 0 (Channel 0) - The Percussive Strike
        getMain()->getController()->write(0x20, 0x03);  // Multi 3, Sustain Bit 0 (Decay mode)
        getMain()->getController()->write(0x40, 0x10);  // TL (Medium Volume)
        getMain()->getController()->write(0x60, 0xF2);  // Attack F (Fast), Decay 2 (Slow fade)
        getMain()->getController()->write(0x80, 0x22);  // Sustain Level 2 (Audible!), Release 2
        getMain()->getController()->write(0xE0, 0x00);  // Sine

        getMain()->getController()->write(0x23, 0x01);  // Carrier Multi 1
        getMain()->getController()->write(0x43, 0x00);  // Carrier Volume MAX
        getMain()->getController()->write(0x63, 0xF2);
        getMain()->getController()->write(0x83, 0x22);  // Sustain Level 2 (Audible!)
        getMain()->getController()->write(0xE3, 0x04);  // Pulse Wave (Piano "Ping")

        // 3. PAIR 1 (Channel 3) - The Body
        getMain()->getController()->write(0x28, 0x01);
        getMain()->getController()->write(0x48, 0x10);
        getMain()->getController()->write(0x68, 0xF2);
        getMain()->getController()->write(0x88, 0x22);

        getMain()->getController()->write(0x2B, 0x01);
        getMain()->getController()->write(0x4B, 0x00);
        getMain()->getController()->write(0x6B, 0xF2);
        getMain()->getController()->write(0x8B, 0x22);

        // 4. PANNING & ALGORITHM
        getMain()->getController()->write(0xC0, 0x31);  // Pan L/R + FM Mode
        getMain()->getController()->write(0xC3, 0x30);  // Pan L/R

        // 5. TRIGGER (Middle C - Block 3)
        getMain()->getController()->write(0xA0, 0x69);
        getMain()->getController()->write(0xB0, 0x31);  // Key-On + Block 3 (Octave 4)
    }
    else
    if (cmd == "test6") {
        mConsole.ClearLog();
        Log("TEST6 still not as good as test more a organ then a piano");
        getMain()->getController()->stopNote(0);
        // 1. Hardware Init
        getMain()->getController()->write(0x01, 0x00);
        getMain()->getController()->write(0x05, 0x01);
        getMain()->getController()->write(0x104, 0x01);

        // 2. Pair 0 (Channel 0) - "The Attack"
        // Modulator 0
        getMain()->getController()->write(0x20, 0x23); // Multi 3, Sustain Bit ON (0x20)
        getMain()->getController()->write(0x40, 0x21); // Volume
        getMain()->getController()->write(0x60, 0xF3); // Attack F, Decay 3
        getMain()->getController()->write(0x80, 0xF2); // Sustain Level F, Release 2
        getMain()->getController()->write(0xE0, 0x00); // Wave 0 (Sine)
        // Carrier 0
        getMain()->getController()->write(0x23, 0x2E); // Multi 14, Sustain Bit ON
        getMain()->getController()->write(0x43, 0x00); // Volume (Loud)
        getMain()->getController()->write(0x63, 0xF1); // Attack F, Decay 1
        getMain()->getController()->write(0x83, 0xF4); // Sustain Level F, Release 4
        getMain()->getController()->write(0xE3, 0x04); // Wave 4 (Pulse - CRITICAL for "Piano" strike)

        // 3. Pair 1 (Channel 3) - "The Resonance"
        // Modulator 1
        getMain()->getController()->write(0x28, 0x22); // Multi 2, Sustain Bit ON
        getMain()->getController()->write(0x48, 0x21);
        getMain()->getController()->write(0x68, 0xF3);
        getMain()->getController()->write(0x88, 0xF2);
        getMain()->getController()->write(0xE8, 0x00); // Wave 0
        // Carrier 1
        getMain()->getController()->write(0x2B, 0x2E); // Multi 14, Sustain Bit ON
        getMain()->getController()->write(0x4B, 0x00); // Volume (Loud)
        getMain()->getController()->write(0x6B, 0xF1);
        getMain()->getController()->write(0x8B, 0xF4);
        getMain()->getController()->write(0xEB, 0x00); // Wave 0

        // 4. Algorithm & Panning
        getMain()->getController()->write(0xC0, 0x35); // Additive (Mixes Pair 0 and Pair 1)
        getMain()->getController()->write(0xC3, 0x34); // FM (Internal to Pair 1)

        // 5. Play Note (Middle C - C4)
        getMain()->getController()->write(0xA0, 0x69);
        getMain()->getController()->write(0xB0, 0x21); // Block 3 + Key-On

    }
    else
    if (cmd == "load")
    {
        std::string filename = "assets/op2/GENMIDI.op2";
        if (!opl3_bridge_op2::importBank(filename, getMain()->getController()->mSoundBank) )
            Log("[error] Failed to load %s", filename.c_str());
        else
            Log("Loaded %s", filename.c_str());

    }
    else
    if (cmd=="noteids")
    {
        std::string note_str;
        uint8_t note_id;
        for (uint16_t u = 0 ;u <= 255; u++ ) {

            note_str = opl3::ValueToNote(u);
            note_id  = opl3::NoteToValue(note_str);
            Log("%d => %s => %d", u, note_str.c_str(), note_id);
        }
        note_id  = opl3::NoteToValue("TOO LONG");
        Log("invalid test: %d", note_id);
        note_id  = opl3::NoteToValue("FOO");
        Log("invalid test2: %d", note_id);

    }
    else
    if (cmd == "t")
    {
        std::string filename = "assets/sbi/tumubar-bell-dmx.sbi";
        OplInstrument newIns;
        if (opl3_bridge_sbi::loadInstrument(filename,newIns)) {
                getMain()->getController()->mSoundBank.push_back(newIns);
                Log("Loaded %s to %zu", g_FileDialog.selectedFile.c_str(), getMain()->getController()->mSoundBank.size()-1);

                SongStep step;
                step.note = 48;
                step.instrument = getMain()->getController()->mSoundBank.size()-1;
                step.volume = 63;
                getMain()->getController()->playNote(0,step);

            } else {
                Log("[error] failed to load:%s",g_FileDialog.selectedFile.c_str());
            }
    }
    else
    if (cmd == "l")
    {
        std::string filename = "assets/wopl/fatman-4op.wopl";
        if (!opl3_bridge_wopl::importBank(filename, getMain()->getController()->mSoundBank) )
            Log("[error] Failed to load %s",filename.c_str() );
        else
            Log("Soundbank %s loaded! %zu instruments",filename.c_str(), getMain()->getController()->mSoundBank.size() );
    }
    else if (cmd=="chord") {
        getMain()->getController()->playChord(mCurrentInstrumentId, 60, opl3::CHORD_MAJOR);
    }
    else
    if (cmd == "savesong")
    {
        if (myTestSong.patterns.size() == 0)
        {
            int instrument = fluxStr::strToInt(fluxStr::getWord(cmdline,1) , -1);
            if ( instrument < 0 )
                instrument = mCurrentInstrumentId;
            Log ("Using Instrument %d",instrument);
            myTestSong = mOpl3Tests->createEffectTestSong(instrument);
        }

        // sync instruments from soundBank!
        myTestSong.instruments = getMain()->getController()->mSoundBank;

        LogFMT("\tPattern: {}\n\tInstruments: {}\n\tSequences: {}\n",
               myTestSong.patterns.size(), myTestSong.instruments.size(),
               myTestSong.orderList.size()
        );


        // Save the song data
        std::string testFilePath = "./test_song.htseq";
        bool saveSuccess = opl3_bridge_soundflux::saveSong(testFilePath, myTestSong);
        if (!saveSuccess) {
            LogFMT("[error] Failed to save song data to {}", testFilePath);
            Log("%s",opl3_bridge_soundflux::errors.c_str());
        }  else {
            LogFMT("Song successfully saved to {}", testFilePath);
        }

    }
    else
    if (cmd == "loadsong")
    {
        myTestSong.init();
        std::string testFilePath = "./test_song.htseq";
        bool success = opl3_bridge_soundflux::loadSong(testFilePath, myTestSong);
        if (!success) {
            LogFMT("[error] Failed to load song data to {}", testFilePath);
            Log("%s",opl3_bridge_soundflux::errors.c_str());
        }  else {
            LogFMT("Song successfully loaded {}", testFilePath);
            LogFMT("\tPattern: {}\n\tInstruments: {}\n\tSequences: {}\n",
                   myTestSong.patterns.size(), myTestSong.instruments.size(),
                   myTestSong.orderList.size()
                   );

        }
        // sync instruments to soundBank!
        getMain()->getController()->mSoundBank =  myTestSong.instruments;

    }
    else
    if (cmd == "clearsong")
    {
        getMain()->getController()->stopSong(true);
        myTestSong.init();
    }
    else
    if (cmd == "playsong")
    {
       Log("Playsong is: %d", getMain()->getController()->playSong(myTestSong, true));
    }
    else
    if (cmd == "stopsong")
    {
        getMain()->getController()->stopSong(true);
    }
    else
    {
        console->AddLog("unknown command %s", cmd.c_str());
    }




}
