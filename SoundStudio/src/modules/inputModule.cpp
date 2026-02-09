

// #include <mutex>
// #include <fstream>
// #include <vector>
// #include <string>
// #include <stdexcept>
// #include <type_traits>
#include <atomic>

#include <audio/fluxAudio.h>
#include "inputModule.h"
//------------------------------------------------------------------------------

void SDLCALL PipeCallback(void* userdata, SDL_AudioStream* stream, int additional_amount, int total_amount) {

    auto* inMod = static_cast<InputModule*>(userdata);

    if (additional_amount <= 0 || !inMod || !inMod->isOpen()) return;
    SDL_AudioStream* pStream = inMod->getStream();
    if (!pStream) {
        dLog("[error] NO STREAM!!!");
        inMod->close();
        return;
    }
    int bytesRead = 0;
    int available = SDL_GetAudioStreamAvailable(stream);
    int max_bytes = sizeof(inMod->getBuffer());
    int to_read = std::min(available, max_bytes);

    if (to_read > 0) bytesRead = SDL_GetAudioStreamData(stream, inMod->getBuffer(), to_read);

    if (bytesRead > 0) {
        // DO   NOT USE A EFFECT HERE !!!!!
        SDL_PutAudioStreamData(inMod->getStream(), inMod->getBuffer(), bytesRead);
    }
}
//------------------------------------------------------------------------------
void InputModule::DrawInputModuleUI(){
    ImGui::SetNextWindowSize(ImVec2(200, 200), ImGuiCond_FirstUseEver);
    ImGui::Begin("Input Stream");

    ImGui::Text("Audio Status: %s", (isOpen()) ? "Running" : "Closed");
    ImGui::Separator();
    if (!mOpen) {
        if (ImGui::Button("OPEN!")) open();
    } else {
        if (ImGui::Button("CLOSE")) close();
    }


    ImGui::End();
}
//------------------------------------------------------------------------------
bool InputModule::open(SDL_AudioSpec dstSpec) {

    if (mOpen) {
        Log("[error] Input Module already open!");
        return false;
    }

    SDL_AudioSpec hardwareSpec;
    if (SDL_GetAudioDeviceFormat(SDL_AUDIO_DEVICE_DEFAULT_RECORDING, &hardwareSpec, nullptr)) {
        int channels = hardwareSpec.channels;
        int frequency = hardwareSpec.freq;

        mInputSpec = hardwareSpec;
        Log("[info] Input Module Hardware: %d Hz, %d Channels", frequency, channels);
    }

    mInStream  = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_RECORDING, &mInputSpec, nullptr, nullptr);

    mStream = SDL_CreateAudioStream(&mInputSpec, &dstSpec);

    if ( mInStream  && mStream ) {
        AudioManager.bindStream(mStream);
        if (
            SDL_ResumeAudioStreamDevice(mInStream )
            &&
            SDL_SetAudioStreamPutCallback(mInStream , PipeCallback, this)

        )
        {
            SDL_ResumeAudioStreamDevice(mStream);
            Log("[info] Input Module: stream ready.");
            mOpen = true;
        } else {
            Log("[error] Input Module: failed! %s", SDL_GetError());
        }
    } else {
        Log("[error] Input Module: Failed to open audio devices: %s", SDL_GetError());
    }
    return mOpen;
}
//------------------------------------------------------------------------------
bool InputModule::close() {
    if (!mOpen) {
        Log("[error] Imput Module. cant close device not marked as open!");
        return false;
    }
    AudioManager.unBindStream(mStream);
    SDL_PauseAudioStreamDevice(mInStream);
    SDL_SetAudioStreamPutCallback(mInStream, nullptr, nullptr);
    SDL_DestroyAudioStream(mInStream);

    mInStream = nullptr;

    mOpen = false;
    return true;
}



