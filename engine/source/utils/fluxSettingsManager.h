//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Usage Examples:
// ===============
//
// void ApplyGameSettings() {
//     // 1. Get parameters (Lazy Addition)
//     // If "v_sync" isn't in settings.json, it's created with 'true'
//     bool vsync = SettingsManager().get("v_sync", true);
//
//     // If "player_name" isn't there, it's created with "Guest"
//     std::string name = SettingsManager().get("player_name", std::string("Guest"));
//
//     // 2. Modify parameters
//     // Update the volume based on some game logic
//     int currentVolume = 85;
//     SettingsManager().set("audio_volume", currentVolume);
//
//     // 3. Save to disk (Call this when settings change or on exit)
//     if (!SettingsManager().save()) {
//         SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Failed to write settings to disk!");
//     }
// }
// -----------------------------------------------------------------------------
// Full example :
// ==============
// bool EditorGui::Initialize()
// {
//     std::string lSettingsFile =
//     getGame()->mSettings.getPrefsPath()
//     .append(getGame()->mSettings.getSafeCaption())
//     .append("_prefs.json");
//     if (SettingsManager().Initialize(lSettingsFile))
//     {
//
//     } else {
//         LogFMT("Error: Can not open setting file: {}", lSettingsFile);
//     }
//     ....
//
// void EditorGui::Deinitialize()
// {
//     if (SettingsManager().IsInitialized()) {
//         SettingsManager().save();
//     }
//    ....
//
// mInsertMode   = SettingsManager().get("fluxComposer::mInsertMode", true);
// ----
// SettingsManager().set("fluxComposer::mInsertMode", mInsertMode);
//
// -----------------------------------------------------------------------------
// Struct example:
// ===============
//
// struct WindowConfig {
//     int width = 1920;
//     int height = 1080;
//     bool maximized = false;
// };
//
// //  Mapping Macro (MUST be in the same namespace as the struct!)
// // This generates the hidden from_json / to_json functions needed
// NLOHMANN_DEFINE_TYPE_NON_INTRUSIVE_WITH_DEFAULT(WindowConfig, width, height, maximized)
//
// Usage
// WindowConfig defaultConfig = { .width = 1920, .height = 1080, .maximized = false };
// WindowConfig currentCfg = SettingsManager().get("window_cfg", defaultConfig);


//-----------------------------------------------------------------------------
#pragma once
#include <SDL3/SDL.h>
#include <nlohmann/json.hpp>
#include <fstream>
#include <string>

using json = nlohmann::json;

class FluxSettingsManager {
private:
    json mData;
    std::string mFilePath;
    bool mIsInitialized = false;

    // Private constructor for Singleton
    FluxSettingsManager() = default;

public:
    // Delete copy and assignment to prevent duplicates
    FluxSettingsManager(const FluxSettingsManager&) = delete;
    FluxSettingsManager& operator=(const FluxSettingsManager&) = delete;

    static FluxSettingsManager& GetInstance() {
        static FluxSettingsManager instance; // Thread-safe "Meyers Singleton"
        return instance;
    }

    bool IsInitialized() { return mIsInitialized; }

    /**
     * Explicitly loads the settings file.
     * @return true if initialization was successful (even if file didn't exist).
     */
    bool Initialize(const std::string& filePath) {
        if (mIsInitialized) return true;
        mFilePath = filePath;

        if (mFilePath.empty()) {
            SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "FluxSettings: Path is empty");
            return false;
        }

        std::ifstream f(mFilePath);
        if (f.is_open()) {
            try {
                f >> mData;
            } catch (const json::parse_error& e) {
                SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "FluxSettings: Parse error in %s: %s", mFilePath.c_str(), e.what());
                return false;
            }
        }

        mIsInitialized = true;
        return true;
    }

    template<typename T>
    T get(const std::string& key, T def) {
        if (!mData.contains(key)) mData[key] = def;
        return mData[key].get<T>();
    }

    void set(const std::string& key, const json& val) { mData[key] = val; }

    bool save() {
        if (!mIsInitialized || mFilePath.empty()) return false;

        std::ofstream f(mFilePath, std::ios::out | std::ios::trunc);
        if (!f.is_open()) return false;

        f << mData.dump(4);
        return f.good(); // Validate write success
    }
};

// Shorthand alias to access the singleton instance
inline FluxSettingsManager& SettingsManager() {
    return FluxSettingsManager::GetInstance();
}
