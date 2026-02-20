#include <atomic>

#include <audio/fluxAudio.h>
#include "src/appMain.h"
#include "inputModule.h"
#include "rackModule.h"


//------------------------------------------------------------------------------
static int INPUT_MODULE_CHANNELS = 2;
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


        int num_samples = bytesRead / sizeof(float);

        //preprocessed
        inMod->mInputEffects->process(
            inMod->getBuffer(),
            num_samples,
            inMod->mInputSpec.channels );


        for (int i = 0; i < num_samples; ++i) {
            float raw_in = inMod->getBuffer()[i];
            float out = raw_in;
            // channel = i % inMod->mInputSpec.channels;
            inMod->mVisuallizer.add_sample(raw_in);
            inMod->getBuffer()[i] = out;
        }

        // run effects !!
        getMain()->getAppGui()->getRackModule()->process(inMod->getBuffer(), num_samples, INPUT_MODULE_CHANNELS);


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

        // ImGui::TextDisabled("%f", lastInputValue.load());
        ImGui::TextDisabled("%d Hz channels:%d", mInputSpec.freq, mInputSpec.channels);

        ImGui::Spacing();
        // input Osci::
        ImGui::BeginChild("InputOsci", ImVec2(0.f,140.f));
        static float scope_zoom = 1.0f;
        ImGui::SliderFloat("Zoom", &scope_zoom, 0.1f, 10.0f);
        ImGui::PlotLines("##Scope", mVisuallizer.scope_buffer,
                         SimpleDSP::Visualizer::scope_size,
                         mVisuallizer.scope_pos.load(),
                         NULL,
                         -1.0f/scope_zoom,
                         1.0f/scope_zoom,
                         ImVec2(-1, -1));
        //<<<<
        ImGui::EndChild();
        mInputEffects->renderUI(2);




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

    // mInputSpec.freq = 44100 ; //<< test 44.1k

    // nailed it to stereo !!
    mInputSpec.channels = INPUT_MODULE_CHANNELS;

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


    //setup sample rate
    getMain()->getAppGui()->getRackModule()->setSampleRate(mInputSpec.freq);

    return mOpen;
}
//------------------------------------------------------------------------------
bool InputModule::close() {
    if (!mOpen) {
        // silent Log("[error] Imput Module. cant close device not marked as open!");
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

