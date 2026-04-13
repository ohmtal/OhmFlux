## FireTV

- Gamepad buttons (integer read with test programm):

| action     | id | SDL | imgui | 
|------------|----|------|------|
| ok (enter) | 0  | SDL_GAMEPAD_BUTTON_SOUTH | ImGuiKey_GamepadFaceDown |
| back       | 1  | SDL_GAMEPAD_BUTTON_BACK | ImGuiKey_GamepadBack |
| menu       | 6  | SDL_GAMEPAD_BUTTON_START | ImGuiKey_GamepadStart |
| up         | 11 | SDL_GAMEPAD_BUTTON_DPAD_UP | ImGuiKey_GamepadDpadUp |
| down       | 12 | SDL_GAMEPAD_BUTTON_DPAD_DOWN | ImGuiKey_GamepadDpadDown |
| left       | 13 | SDL_GAMEPAD_BUTTON_DPAD_LEFT | ImGuiKey_GamepadDpadLeft |
| right      | 14 | SDL_GAMEPAD_BUTTON_DPAD_RIGHT | ImGuiKey_GamepadDpadRight |


- Multimedia  key:


| action     | id | SDL | imgui | 
|------------|----|------|------|


- FireTV Keys when detected as Keyboard:

```
#pragma once

#include "imgui.h"
#include <SDL3/SDL.h>

// Key definition
struct KeyDef {
    const char* action;
    ImGuiKey imgui_key;
    SDL_Scancode scancode = SDL_SCANCODE_UNKNOWN;
};

static KeyDef kKeys[] = {


    { "UP",        ImGuiKey_UpArrow   ,     SDL_SCANCODE_UP},
    { "DOWN",      ImGuiKey_DownArrow ,     SDL_SCANCODE_DOWN },
    { "LEFT",      ImGuiKey_LeftArrow,      SDL_SCANCODE_LEFT },
    { "RIGHT",     ImGuiKey_RightArrow,     SDL_SCANCODE_RIGHT },

    { "ENTER",     ImGuiKey_KeypadEnter,    SDL_SCANCODE_KP_ENTER },
    { "BACK",      ImGuiKey_AppBack ,       SDL_SCANCODE_AC_BACK},
    { "MENU",      ImGuiKey_None,           SDL_SCANCODE_MENU},
    { "HOME",      ImGuiKey_None ,          SDL_SCANCODE_AC_HOME},

    { "FORWARD",ImGuiKey_None, SDL_SCANCODE_MEDIA_FAST_FORWARD},
    { "PLAY",   ImGuiKey_None, SDL_SCANCODE_MEDIA_PLAY_PAUSE},
    { "REWIND", ImGuiKey_None, SDL_SCANCODE_MEDIA_REWIND},

};

inline void TestKeysWindow()
{
    ImGui::Begin("FireTV Key Tester (SDL3)");


    ImGui::Columns(3, "key_table");
    ImGui::Text("Action"); ImGui::NextColumn();
    ImGui::Text("ImGui"); ImGui::NextColumn();
    ImGui::Text("SDL"); ImGui::NextColumn();
    ImGui::Separator();

    int numkeys;
    const bool* kState = SDL_GetKeyboardState(&numkeys);


    for (int i = 0; i < IM_ARRAYSIZE(kKeys); i++)
    {
        const KeyDef& key = kKeys[i];
        bool isDown =  (key.imgui_key != ImGuiKey_None) ? ImGui::IsKeyDown(key.imgui_key) : false;

        ImGui::Text("%s", key.action); ImGui::NextColumn();
        if (isDown)
            ImGui::TextColored(ImVec4(0, 1, 0, 1), "%s", ImGui::GetKeyName(key.imgui_key));
        else
            ImGui::TextDisabled("---");

        ImGui::NextColumn();
        isDown = kState[key.scancode];
        if (isDown)
            ImGui::TextColored(ImVec4(0, 1, 0, 1), "%s", SDL_GetScancodeName(key.scancode));
        else
            ImGui::TextDisabled("---");


        ImGui::NextColumn();
    }
    ImGui::Columns(1);
    ImGui::Separator();


    ImGui::SeparatorText("ImGui Keys Keys:");

    for (ImGuiKey key = ImGuiKey_NamedKey_BEGIN; key < ImGuiKey_NamedKey_END; key = (ImGuiKey)(key + 1))
    {
        if (ImGui::IsKeyDown(key))
        {
            ImGui::TextColored(ImVec4(1, 1, 0, 1), "AKTIV: %s (ID: %d)", ImGui::GetKeyName(key), key);
        }
    }


    ImGui::SeparatorText("SDL3 Keys:");
    for ( int i = 0; i < numkeys; i++ ) {
        if (kState[i])  ImGui::TextColored(ImVec4(0, 1, 1, 1), "%d:%s", i, SDL_GetScancodeName((SDL_Scancode)i));
    }

    ImGui::End();
}

```
