#pragma once

#include <imgui.h>
#include <string>
#include <gui/ImFileDialog.h>

//SDL Events
inline U32 FLUX_EVENT_COMPOSER_OPL_CHANNEL_CHANGED = 0;
inline U32 FLUX_EVENT_INSTRUMENT_OPL_CHANNEL_CHANGED = 0;
inline U32 FLUX_EVENT_INSTRUMENT_OPL_INSTRUMENT_NAME_CHANGED = 0;

//File Dialog
inline ImFileDialog g_FileDialog;

//MessageBox
inline bool POPUP_MSGBOX_ACTIVE = false;
inline std::string POPUP_MSGBOX_CAPTION = "Msg";
inline std::string POPUP_MSGBOX_TEXT   = "..";

inline void showMessage(std::string caption, std::string text)
{
    POPUP_MSGBOX_CAPTION = caption;
    POPUP_MSGBOX_TEXT   = text;
    POPUP_MSGBOX_ACTIVE = true;
}

inline std::string_view extractFilename(std::string_view path) {
    size_t lastSlash = path.find_last_of("/\\");
    if (lastSlash == std::string_view::npos) {
        return path;
    }
    return path.substr(lastSlash + 1);
}
