//-----------------------------------------------------------------------------
// Copyright (c) 2025 Thomas Hühn (XXTH) 
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once
#include <SDL3/SDL.h>

#include <unordered_map>
#include <vector>
#include <string>

#include "fluxGlobals.h"

// Define your Input Types
enum class FluxInputType { KEYBOARD, MOUSE, GAMEPAD, JOYSTICK };

struct FluxBinding {
    FluxInputType type;
    int code; // This will hold either SDL_Scancode or SDL_MouseButton
};

class FluxInput {
private:
    std::unordered_map<std::string, std::vector<FluxBinding>> mBindings;

public:
    //----------------------------------------------------------------------
    // STATIC :
    static const char* getKeyName( SDL_Scancode scancode) {
        retrun SDL_GetScancodeName(scancode);
    }

    static bool isKeyDown( SDL_Scancode scancode ) {
        int numkeys;
        const bool* kState = SDL_GetKeyboardState(&numkeys);
        return kState[scancode];
    }

    //----------------------------------------------------------------------
    // Bind Keyboard
    void bindKey(const std::string& action, SDL_Scancode code)
    {
        mBindings[action].push_back({FluxInputType::KEYBOARD, (int)code});
    }
    //----------------------------------------------------------------------
    // Bind Mouse (e.g., SDL_BUTTON_LEFT)
    void bindMouse(const std::string& action, int button)
    {
        mBindings[action].push_back({FluxInputType::MOUSE, button});
    }
    //----------------------------------------------------------------------
    void bindGamePad(const std::string& action, int button)
    {
        mBindings[action].push_back({FluxInputType::GAMEPAD, button});
    }
    //----------------------------------------------------------------------
    void bindJoyStick(const std::string& action, int button)
    {
        mBindings[action].push_back({FluxInputType::JOYSTICK, button});
    }
    //----------------------------------------------------------------------
    bool isActionActive(const std::string& action)
    {

        // Get Keyboard State
        int numkeys;
        const bool* kState = SDL_GetKeyboardState(&numkeys);

        // Get Mouse State
        float msX, msY;
        Uint32 mState = SDL_GetMouseState(&msX, &msY);


        //FIXME scaled pos ?!
        // if (g_CurrentScreen)
        // g_CurrentScreen.MousePos

        auto it = mBindings.find(action);
        if (it == mBindings.end()) return false;

        for (const auto& b : it->second) {
            if (b.type == FluxInputType::KEYBOARD) {
                if (b.code >= 0 && b.code < numkeys && kState[b.code]) return true;
            }
            else if (b.type == FluxInputType::MOUSE) {
                // Use the SDL_BUTTON_MASK macro to check the specific bit
                if (mState & SDL_BUTTON_MASK(b.code)) return true;
            }
            else if (b.type == FluxInputType::GAMEPAD) {
                // b.code should be a SDL_GamepadButton (e.g., SDL_GAMEPAD_BUTTON_SOUTH)
                for (auto* gamepad : gAppStatus.Gamepads) {
                    if (gamepad && SDL_GetGamepadButton(gamepad, (SDL_GamepadButton)b.code)) {
                        return true;
                    }
                }
            }
            else if (b.type == FluxInputType::JOYSTICK) {
                // For older Joysticks without Gamepad-Mapping
                for (auto* joystick : gAppStatus.JoySticks) {
                    if (joystick && SDL_GetJoystickButton(joystick, b.code)) {
                        return true;
                    }
                }
            }

        }
        return false;
    }
    //----------------------------------------------------------------------
}; //Class
