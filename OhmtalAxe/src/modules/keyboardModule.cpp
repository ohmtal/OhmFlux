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
                case SDLK_F2:
                    getMain()->getAppGui()->getRackModule()->getManager()->prevRack();
                    break;
                case SDLK_F3:
                    getMain()->getAppGui()->getRackModule()->getManager()->nextRack();
                    break;

                // ... INPUT LINE
                case SDLK_F1:
                    getMain()->getAppGui()->getInputModule()->toggle();
                    break;



                case SDLK_ESCAPE:
                    //panic key!
                    getMain()->getAppGui()->getInputModule()->close();
                    getMain()->getAppGui()->getDrumKitLooperModule()->stop();
                    break;

                // ... DRUMKIT
                case SDLK_F5:  // SDLK_RETURN:
                    getMain()->getAppGui()->getDrumKitLooperModule()->toogleDrumKit();
                    break;


            }
        }
    }
