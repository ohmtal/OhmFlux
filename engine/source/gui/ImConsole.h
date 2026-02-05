// dear imgui, v1.92.5
// (from demo code)
//-----------------------------------------------------------------------------
// : Redirct commands:
// mConsole.OnCommand =  [&](ImConsole* console, const char* cmd) { OnConsoleCommand(console, cmd); };
// ==>     void OnConsoleCommand(ImConsole* console, const char* cmd) {}
//
// To redirect use:  SDL_SetLogOutputFunction(MyLogCallback, optionalUserdataPointer)
//
//-----------------------------------------------------------------------------
#pragma once
#include "imgui.h"
#include <vector>
#include <string>
#include <algorithm>

struct ImConsole
{
private:
    char                  InputBuf[256];
    ImVector<char*>       Items;
    ImVector<char*>       History;
    int                   HistoryPos;    // -1: new line, 0..History.Size-1 browsing history.


    ImVec2 mButtonSize = ImVec2(60,20);
    bool   mCopyOnlyVisible = true;
    ImVector<int> mFilterIndices;
    bool mDirty = true;

public:
    std::function<void(ImConsole*, const char*)> OnCommand;

    ImVector<const char*> Commands;
    ImGuiTextFilter       Filter;
    bool                  AutoScroll;
    bool                  ScrollToBottom;

    bool mShowButtons = false; //FIXME save as option (and the other options too)



    //--------------------------------------------------------------------------
    ImConsole()
    {
        ClearLog();
        memset(InputBuf, 0, sizeof(InputBuf));
        HistoryPos = -1;

        // "CLASSIFY" is here to provide the test case where "C"+[tab] completes to "CL" and display multiple matches.
        // Commands.push_back("HELP");
        // Commands.push_back("HISTORY");
        // Commands.push_back("CLEAR");
        // Commands.push_back("CLASSIFY");
        AutoScroll = true;
        ScrollToBottom = false;
        // AddLog("Welcome to Dear ImGui!");
    }
    //--------------------------------------------------------------------------
    ~ImConsole()
    {
        ClearLog();
        for (int i = 0; i < History.Size; i++)
            ImGui::MemFree(History[i]);
    }
    //--------------------------------------------------------------------------
    // Portable helpers
    static int   Stricmp(const char* s1, const char* s2)         { int d; while ((d = toupper(*s2) - toupper(*s1)) == 0 && *s1) { s1++; s2++; } return d; }
    static int   Strnicmp(const char* s1, const char* s2, int n) { int d = 0; while (n > 0 && (d = toupper(*s2) - toupper(*s1)) == 0 && *s1) { s1++; s2++; n--; } return d; }
    static char* Strdup(const char* s)                           { IM_ASSERT(s); size_t len = strlen(s) + 1; void* buf = ImGui::MemAlloc(len); IM_ASSERT(buf); return (char*)memcpy(buf, (const void*)s, len); }
    static void  Strtrim(char* s)                                { char* str_end = s + strlen(s); while (str_end > s && str_end[-1] == ' ') str_end--; *str_end = 0; }
    //--------------------------------------------------------------------------
    void ClearLog()
    {
        for (int i = 0; i < Items.Size; i++)
            ImGui::MemFree(Items[i]);
        Items.clear();
        mDirty = true;
    }
    //--------------------------------------------------------------------------
    void    AddLog(const char* fmt, ...) IM_FMTARGS(2)
    {
        char buf[1024];
        va_list args;
        va_start(args, fmt);
        vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
        buf[IM_ARRAYSIZE(buf)-1] = 0;
        va_end(args);
        Items.push_back(Strdup(buf));
        mDirty = true;
    }
    //--------------------------------------------------------------------------
    bool SearchPopup() //return true if closed
    {
        bool result = false;


        // Set position to center of the current window
        ImVec2 center = ImGui::GetMainViewport()->GetCenter();
        ImGui::SetNextWindowPos(center, ImGuiCond_Appearing, ImVec2(0.5f, 0.5f));

        if (ImGui::BeginPopup("SearchPopup"))
        {
            // Focus the input field automatically when the popup appears
            if (ImGui::IsWindowAppearing())
                ImGui::SetKeyboardFocusHere();

            // The filter input
            // If Filter.Draw returns true, it means the text changed -> set Dirty
            if (Filter.Draw("##FilterInput", 200.0f))
            {
                mDirty = true;
            }

            // Close button
            ImGui::SameLine();
            if ( ImGui::Button("Close") ||
                 ImGui::IsKeyPressed(ImGuiKey_Enter)
                 || ImGui::IsKeyPressed(ImGuiKey_KeypadEnter)
            ) {
                ImGui::CloseCurrentPopup();
                result = true;
            }


            ImGui::EndPopup();
        }

        if (ImGui::IsKeyDown(ImGuiKey_LeftCtrl) && ImGui::IsKeyPressed(ImGuiKey_F))
            ImGui::OpenPopup("SearchPopup");

        return result;
    }
    //--------------------------------------------------------------------------

    void Draw(const char* title, bool* p_open)
    {
        ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);

        if (!ImGui::Begin(title, p_open))
        {
            ImGui::End();
            return;
        }

        bool loDoCopyToClipboard = false;

        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Close Console"))
                *p_open = false;


            ImGui::TextDisabled("Menu");
            ImGui::Separator();
            ImGui::TextDisabled("Hint:double click line => copy to clipboard");
            ImGui::Separator();
            if (ImGui::Selectable("Clear")) ClearLog();
            loDoCopyToClipboard = ImGui::Selectable("Copy");
            if (ImGui::Selectable("Show Menu Buttons", mShowButtons)) mShowButtons = !mShowButtons;


            ImGui::EndPopup();
        }


        // Options menu
        if (ImGui::BeginPopup("Options"))
        {
            ImGui::Checkbox("Auto-scroll", &AutoScroll);
            ImGui::Checkbox("Copy only visible Log entries", &mCopyOnlyVisible);

            ImGui::Checkbox("Show Buttons", &mShowButtons);
            ImGui::EndPopup();
        }


        if ( mShowButtons )
        {
            if (ImGui::Button("Clear", mButtonSize)){ ClearLog(); }
            ImGui::SameLine();
            bool loDoCopyToClipboard = ImGui::Button("Copy", mButtonSize);

            ImGui::SameLine();

            // Options, Filter
            ImGui::SetNextItemShortcut(ImGuiMod_Ctrl | ImGuiKey_O, ImGuiInputFlags_Tooltip);
            if (ImGui::Button("Options", mButtonSize)) ImGui::OpenPopup("Options");

            ImGui::SameLine(ImGui::GetWindowWidth() - 110.f);
            if (Filter.Draw("##FilterMain", 100)) mDirty = true;
            ImGui::Separator();

        }





        // Reserve enough left-over height for 1 separator + 1 input text
        const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing();
        bool openOpt = false;
        bool openSearch = false;
        if (ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), ImGuiChildFlags_NavFlattened, ImGuiWindowFlags_HorizontalScrollbar))
        {
            if (ImGui::BeginPopupContextWindow())
            {
                ImGui::TextDisabled("Menu");
                ImGui::Separator();
                ImGui::TextDisabled("Hint:double click line => copy to clipboard");
                ImGui::Separator();
                if (ImGui::Selectable("Clear")) ClearLog();
                loDoCopyToClipboard = ImGui::Selectable("Copy");
                if (ImGui::Selectable("Show Menu Buttons", mShowButtons)) mShowButtons = !mShowButtons;

                ImGui::Separator();

                ImGui::EndPopup();
            }


            if ( mDirty  )
            {
                mFilterIndices.clear();
                mFilterIndices.reserve(Items.Size);
                for (int i = 0; i < Items.Size; i++) {
                    if (Filter.PassFilter(Items[i])) {
                        mFilterIndices.push_back(i);
                    }
                }
                mDirty = false;
            }

            if (loDoCopyToClipboard) {
                if (!mCopyOnlyVisible) {
                    ImGui::LogToClipboard();
                    for (int idx : mFilterIndices) ImGui::TextUnformatted(Items[idx]);
                    ImGui::LogFinish();
                    loDoCopyToClipboard = false;
                } else {
                    ImGui::LogToClipboard();
                }
            }

            ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing

            ImGuiListClipper clipper;
            clipper.Begin(mFilterIndices.Size); // Nutze die Anzahl der GEFILTERTEN Items
            while (clipper.Step())
            {
                for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
                {
                    const char* item = Items[mFilterIndices[i]];
                    ImVec4 color;
                    bool has_color = false;
                    if (strstr(item, "[error]")) { color = ImVec4(1.0f, 0.4f, 0.4f, 1.0f); has_color = true; }
                    else if (strstr(item, "[warn]")) { color = ImVec4(0.4f, 1.0f, 0.4f, 1.0f); has_color = true; }
                    else if (strstr(item, "[info]")) { color = ImVec4(0.4f, 0.4f, 1.0f, 1.0f); has_color = true; }
                    else if (strncmp(item, "# ", 2) == 0) { color = ImVec4(1.0f, 0.8f, 0.6f, 1.0f); has_color = true; }
                    if (has_color)
                        ImGui::PushStyleColor(ImGuiCol_Text, color);
                    ImGui::TextUnformatted(item);

                    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                    {
                        ImGui::SetClipboardText(item);
                        // ImGui::SetTooltip("Copied to clipboard!");
                    }

                    if (has_color)
                        ImGui::PopStyleColor();
                }
            }

            ImGui::PopStyleVar();

            if (loDoCopyToClipboard ) {
                ImGui::LogFinish();
                loDoCopyToClipboard = false;
            }

            if (ScrollToBottom || (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
                ImGui::SetScrollHereY(1.0f);
            ScrollToBottom = false;

        } //ScrollingRegion
        ImGui::EndChild();
        ImGui::Separator();

        // Command-line && search PopUp
        if (SearchPopup())
        {
            // next one is focused :D
            ImGui::SetKeyboardFocusHere();
        }


        bool reclaim_focus = false;
        ImGuiInputTextFlags input_text_flags = ImGuiInputTextFlags_EnterReturnsTrue | ImGuiInputTextFlags_EscapeClearsAll | ImGuiInputTextFlags_CallbackCompletion | ImGuiInputTextFlags_CallbackHistory;
        ImGui::SetNextItemWidth(-FLT_MIN);
        if (ImGui::InputText("##Input", InputBuf, IM_ARRAYSIZE(InputBuf), input_text_flags, &TextEditCallbackStub, (void*)this))
        {
            char* s = InputBuf;
            Strtrim(s);
            if (s[0])
                ExecCommand(s);
            strcpy(s, "");
            reclaim_focus = true;
        }

        // Auto-focus on window apparition
        ImGui::SetItemDefaultFocus();
        if (reclaim_focus)
            ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget



        ImGui::End();
    }
    //--------------------------------------------------------------------------
    void ExecCommand(const char* command_line)
    {
        AddLog("# %s\n", command_line);
        // Insert into history. First find match and delete it so it can be pushed to the back.
        // This isn't trying to be smart or optimal.
        HistoryPos = -1;
        for (int i = History.Size - 1; i >= 0; i--)
            if (Stricmp(History[i], command_line) == 0)
            {
                ImGui::MemFree(History[i]);
                History.erase(History.begin() + i);
                break;
            }
        History.push_back(Strdup(command_line));

        if (OnCommand)
            OnCommand(this, command_line);

        // On command input, we scroll to bottom even if AutoScroll==false
        ScrollToBottom = true;
    }
    //--------------------------------------------------------------------------
    // In C++11 you'd be better off using lambdas for this sort of forwarding callbacks
    static int TextEditCallbackStub(ImGuiInputTextCallbackData* data)
    {
        ImConsole* console = (ImConsole*)data->UserData;
        return console->TextEditCallback(data);
    }
    //--------------------------------------------------------------------------
    int     TextEditCallback(ImGuiInputTextCallbackData* data)
    {
        //AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
        switch (data->EventFlag)
        {
            case ImGuiInputTextFlags_CallbackCompletion:
            {
                // Example of TEXT COMPLETION

                // Locate beginning of current word
                const char* word_end = data->Buf + data->CursorPos;
                const char* word_start = word_end;
                while (word_start > data->Buf)
                {
                    const char c = word_start[-1];
                    if (c == ' ' || c == '\t' || c == ',' || c == ';')
                        break;
                    word_start--;
                }

                // Build a list of candidates
                ImVector<const char*> candidates;
                for (int i = 0; i < Commands.Size; i++)
                    if (Strnicmp(Commands[i], word_start, (int)(word_end - word_start)) == 0)
                        candidates.push_back(Commands[i]);

                if (candidates.Size == 0)
                {
                    // No match
                    AddLog("No match for \"%.*s\"!\n", (int)(word_end - word_start), word_start);
                }
                else if (candidates.Size == 1)
                {
                    // Single match. Delete the beginning of the word and replace it entirely so we've got nice casing.
                    data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
                    data->InsertChars(data->CursorPos, candidates[0]);
                    data->InsertChars(data->CursorPos, " ");
                }
                else
                {
                    // Multiple matches. Complete as much as we can..
                    // So inputting "C"+Tab will complete to "CL" then display "CLEAR" and "CLASSIFY" as matches.
                    int match_len = (int)(word_end - word_start);
                    for (;;)
                    {
                        int c = 0;
                        bool all_candidates_matches = true;
                        for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
                            if (i == 0)
                                c = toupper(candidates[i][match_len]);
                        else if (c == 0 || c != toupper(candidates[i][match_len]))
                            all_candidates_matches = false;
                        if (!all_candidates_matches)
                            break;
                        match_len++;
                    }

                    if (match_len > 0)
                    {
                        data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
                        data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
                    }

                    // List matches
                    AddLog("Possible matches:\n");
                    for (int i = 0; i < candidates.Size; i++)
                        AddLog("- %s\n", candidates[i]);
                }

                break;
            }
            case ImGuiInputTextFlags_CallbackHistory:
            {
                // Example of HISTORY
                const int prev_history_pos = HistoryPos;
                if (data->EventKey == ImGuiKey_UpArrow)
                {
                    if (HistoryPos == -1)
                        HistoryPos = History.Size - 1;
                    else if (HistoryPos > 0)
                        HistoryPos--;
                }
                else if (data->EventKey == ImGuiKey_DownArrow)
                {
                    if (HistoryPos != -1)
                        if (++HistoryPos >= History.Size)
                            HistoryPos = -1;
                }

                // A better implementation would preserve the data on the current input line along with cursor position.
                if (prev_history_pos != HistoryPos)
                {
                    const char* history_str = (HistoryPos >= 0) ? History[HistoryPos] : "";
                    data->DeleteChars(0, data->BufTextLen);
                    data->InsertChars(0, history_str);
                }
            }
        }
        return 0;
    }
};
