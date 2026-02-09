

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

    if (to_read > 0) {
        bytesRead = SDL_GetAudioStreamData(stream, inMod->getBuffer(), to_read);
    }
    if (bytesRead > 0) {
        // simple effect TEST >>>>
        int channel = 0;
        int num_samples = bytesRead / sizeof(float);
        float gate_threshold  = inMod->mSimpleEffectConfig.gate_threshold.load();
        float gate_release_ms = inMod->mSimpleEffectConfig.gate_release_ms.load();
        bool gate_enabled     = inMod->mSimpleEffectConfig.gate_enabled.load();

        float release_samples = (gate_release_ms / 1000.0f) * inMod->mInputSpec.freq;
        if (release_samples < 1.0f) release_samples = 1.0f;

        for (int i = 0; i < num_samples; ++i) {
            float raw_in = inMod->getBuffer()[i];
            float out = raw_in;

            channel = i % inMod->mInputSpec.channels;

            if ( gate_enabled )
                out = inMod->mNoiseGate.process(out, gate_threshold, release_samples, gate_enabled);

            // out *= 0.1f;
            inMod->lastInputValue = (float) gate_enabled; // out; //DEBUG

            inMod->getBuffer()[i] = out;
        }

        // <<<< TEST

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

        ImGui::TextDisabled("%f", lastInputValue.load());
        ImGui::TextDisabled("%d Hz channels:%d", mInputSpec.freq, mInputSpec.channels);

        ImGui::Spacing();

        if (ImGui::CollapsingHeader("Input Noise Filter", ImGuiTreeNodeFlags_DefaultOpen)) {
            ImGui::Text("Noise Gate");
            bool gate_enabled = mSimpleEffectConfig.gate_enabled.load();
            if (ImGui::Checkbox("Gate Toggle", &gate_enabled)) mSimpleEffectConfig.gate_enabled = gate_enabled;

            float threshold = mSimpleEffectConfig.gate_threshold.load();
            if (ImGui::SliderFloat("Gate Threshold", &threshold, 0.0f, 0.1f, "%.4f")) mSimpleEffectConfig.gate_threshold = threshold;

            float release = mSimpleEffectConfig.gate_release_ms.load();
            if (ImGui::SliderFloat("Gate Release (ms)", &release, 1.0f, 500.0f)) mSimpleEffectConfig.gate_release_ms = release;

            ImGui::Separator();
            ImGui::Text("Frequency Filters");

            bool hpf_enabled = mSimpleEffectConfig.hpf_enabled.load();
            if (ImGui::Checkbox("HPF Toggle (Hum Cut)", &hpf_enabled)) mSimpleEffectConfig.hpf_enabled = hpf_enabled;
            float hpf_a = mSimpleEffectConfig.hpf_alpha.load();
            if (ImGui::SliderFloat("HPF Alpha", &hpf_a, 0.900f, 1.0f, "%.4f")) mSimpleEffectConfig.hpf_alpha = hpf_a;
            ImGui::SetItemTooltip("1.0 = Bypass, lower = more bass cut");

            bool lpf_enabled = mSimpleEffectConfig.lpf_enabled.load();
            if (ImGui::Checkbox("LPF Toggle (Hiss Cut)", &lpf_enabled)) mSimpleEffectConfig.lpf_enabled = lpf_enabled;
            float lpf_a = mSimpleEffectConfig.lpf_alpha.load();
            if (ImGui::SliderFloat("LPF Alpha", &lpf_a, 0.001f, 1.0f, "%.4f")) mSimpleEffectConfig.lpf_alpha = lpf_a;
            ImGui::SetItemTooltip("1.0 = Bypass, lower = more treble cut");
        }




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
        mInputSpec = hardwareSpec;
        Log("[info] Input Module Hardware: %d Hz, %d channels formatid: %d", hardwareSpec.freq, hardwareSpec.channels, hardwareSpec.format);
    }

    mInputSpec.format=SDL_AUDIO_F32; //<< THIS!! override to float!!

    //TEST: use mono !!
    mInputSpec.channels = 1;



    mInStream  = SDL_OpenAudioDeviceStream(SDL_AUDIO_DEVICE_DEFAULT_RECORDING, &mInputSpec, nullptr, nullptr);
    mOutStream = SDL_CreateAudioStream(&mInputSpec, &dstSpec);

    if ( mInStream  && mOutStream ) {
        if (
            SDL_ResumeAudioStreamDevice(mInStream ) //<<< first!
            &&
            SDL_SetAudioStreamPutCallback(mInStream , PipeCallback, this)
        )
        {
            AudioManager.bindStream(mOutStream);

            // SDL_ResumeAudioStreamDevice(mStream);
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
    AudioManager.unBindStream(mOutStream);
    SDL_PauseAudioStreamDevice(mInStream);
    SDL_SetAudioStreamPutCallback(mInStream, nullptr, nullptr);
    SDL_DestroyAudioStream(mInStream);

    mInStream = nullptr;

    mOpen = false;
    return true;
}

