#include "keyboardModule.h"
#include <src/appMain.h>

void KeyBoardModule::onKeyEvent(SDL_KeyboardEvent event)
    {

        if (event.repeat) return;

        // check imgui input >>>
        ImGuiIO& io = ImGui::GetIO();
        if (io.WantTextInput /*FIXME console does ? || io.WantCaptureKeyboard*/) {
            return;
        }
        if (ImGui::IsAnyItemActive()) {
            return;
        }
        // <<<

        bool isKeyUp = (event.type == SDL_EVENT_KEY_UP);
        bool isAlt =  event.mod & SDLK_LALT || event.mod & SDLK_RALT;

        if (isKeyUp) {
            switch (event.key) {
                // ... RACK
                case SDLK_SPACE:
                    getMain()->getAppGui()->getRackModule()->getManager()->switchRack();
                    break;
                // ... INPUT LINE
                case SDLK_F1:
                    getMain()->getAppGui()->getInputModule()->open();
                    break;
                case SDLK_ESCAPE:
                    getMain()->getAppGui()->getInputModule()->close();
                    break;

                // ... DRUMKIT
                case SDLK_F5:  // SDLK_RETURN:
                    getMain()->getAppGui()->getDrumKitLooperModule()->toogleDrumKit();
                    break;


            }
        }
    }
