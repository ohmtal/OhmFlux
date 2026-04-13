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
| backward  | 1073742090 | SDL_SCANCODE_MEDIA_REWIND | ImGuiKey_MediaRewind |
| play      | 1073742095 | SDL_SCANCODE_MEDIA_PLAYPAUSE | ImGuiKey_MediaPlayPause |
| forward   | 1073742089 | SDL_SCANCODE_MEDIA_FAST_FORWARD | ImGuiKey_MediaFastForward |


- FireTV Keys when detected as Keyboard:

| action     | id | SDL | imgui |
|------------|----|------|------|
| up         | 1073741906 | SDLK_UP | ImGuiKey_UpArrow |
| down       | 1073741905 | SDLK_DOWN | ImGuiKey_DownArrow |
| left       | 1073741904 | SDLK_LEFT | ImGuiKey_LeftArrow |
| right      | 1073741903 | SDLK_RIGHT | ImGuiKey_RightArrow |
|--|--|--|--|
| ok (enter) | 1073741912 | SDLK_RETURN | ImGuiKey_Enter |
| back       | 1073742106 | SDLK_AC_BACK (or SDLK_BACKSPACE if mapped as backspace) | ImGuiKey_Escape |
| menu       | 1073741942 | SDLK_MENU | ImGuiKey_Menu |
| home       | 1073742105 | SDLK_AC_HOME (or SDLK_HOME) | ImGuiKey_Home |
|--|--|--|--|
| backward  | 1073742090 | SDL_SCANCODE_MEDIA_REWIND | ImGuiKey_MediaRewind |
| play      | 1073742095 | SDL_SCANCODE_MEDIA_PLAYPAUSE | ImGuiKey_MediaPlayPause |
| forward   | 1073742089 | SDL_SCANCODE_MEDIA_FAST_FORWARD | ImGuiKey_MediaFastForward |
