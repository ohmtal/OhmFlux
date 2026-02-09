#pragma once

#include <SDL3/SDL.h>
#include <string>
#include <imgui.h>

#include <core/fluxBaseObject.h>
#include <DSP.h>

namespace SimpleDSP {

    struct EffectsConfig {
        std::atomic<float> gate_threshold{0.01f};
        std::atomic<float> gate_release_ms{100.0f};
        std::atomic<bool> gate_enabled{false};

        std::atomic<float> lpf_alpha{1.0f}; // 1.0 = bypass, < 1.0 = filtering
        std::atomic<bool> lpf_enabled{false};

        std::atomic<float> hpf_alpha{1.0f}; // 1.0 = bypass
        std::atomic<bool> hpf_enabled{false};

        std::atomic<float> distortion_gain{10.0f};
        std::atomic<float> distortion_level{0.5f};
        std::atomic<bool> distortion_enabled{false};

        std::atomic<float> delay_time_ms{300.0f};
        std::atomic<float> delay_feedback{0.4f};
        std::atomic<float> delay_mix{0.3f};
        std::atomic<bool> delay_enabled{false};

        // Tuner state
        std::atomic<float> tuner_freq{0.0f};
        std::atomic<bool> tuner_enabled{true};

        // Visualization
        static constexpr int scope_size = 512;
        float scope_buffer[scope_size] = {0};
        std::atomic<int> scope_pos{0};
    };



struct NoiseGate {
    float current_gain = 1.0f;
    float process(float input, float threshold, float release_samples, bool enabled) {
        if (!enabled) return input;
        float abs_input = std::abs(input);
        if (abs_input >= threshold) {
            current_gain = 1.0f;
        } else {
            current_gain -= 1.0f / release_samples;
            if (current_gain < 0.0f) current_gain = 0.0f;
        }
        return input * current_gain;
    }
};
}

class InputModule : public FluxBaseObject {
public:
    static const int BUFFER_SIZE = 4096;


private:

    bool mOpen = false;
    bool mInitialized = false;

    float buffer[BUFFER_SIZE] = {0};
    SDL_AudioStream *mOutStream = nullptr;
    SDL_AudioStream* mInStream   = nullptr;

    // NO ..effects here !! DSP::NoiseGate*  mNoiseGate = nullptr;



public:
    // set by hardware spec !!
    SDL_AudioSpec mInputSpec = { SDL_AUDIO_F32, 1, 48000 };


    InputModule() = default;
    ~InputModule() {
        close();
    }
    bool Initialize() override  {


        mInitialized = true;
        return true;
    }

//TEST >>>>>>>>>>>>>>>
    SimpleDSP::EffectsConfig mSimpleEffectConfig;

    SimpleDSP::NoiseGate mNoiseGate;


    std::atomic<float> lastInputValue = 0;
//<<<<<<<<<<<<<<<<<<<< TEST


    float (&getBuffer())[BUFFER_SIZE] {
        return buffer;
    }
    SDL_AudioStream *getStream()  { return mOutStream; }


    bool isOpen() const { return mOpen; }
    bool open(SDL_AudioSpec dstSpec = { SDL_AUDIO_F32, 2, 44100});
    bool close();


    virtual void Update(const double& dt) override {}


    void DrawInputModuleUI();
}; //CLASS
