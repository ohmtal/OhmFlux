#pragma once

#include <imgui.h>
#include <string>




static bool POPUP_MSGBOX_ACTIVE = false;
static std::string POPUP_MSGBOX_CAPTION = "Msg";
static std::string POPUP_MSGBOX_TEXT   = "..";

inline void showMessage(std::string caption, std::string text)
{
    POPUP_MSGBOX_CAPTION = caption;
    POPUP_MSGBOX_TEXT   = text;
    POPUP_MSGBOX_ACTIVE = true;
}
