//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// App Globals
//-----------------------------------------------------------------------------
#pragma once

#include <imgui.h>
#include <string>
#include <gui/ImFileDialog.h>


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

