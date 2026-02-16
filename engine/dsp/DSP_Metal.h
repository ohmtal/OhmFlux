//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Metal Distortion
//-----------------------------------------------------------------------------
// * using ISettings
//-----------------------------------------------------------------------------

#pragma once

#include <cstdint>
#include <cstring>
#include <atomic>
#include <mutex>
#include <string>

#include <algorithm>
#include <cmath>
#include <vector>
#include <array>

#ifdef FLUX_ENGINE
#include <imgui.h>
#include <imgui_internal.h>
#include <gui/ImFlux.h>
#endif

#include "DSP_Effect.h"

namespace DSP {

    struct MetalData {
        float gain;
        float tight;
        float level;
    };

    struct MetalSettings: public ISettings {
        AudioParam<float> gain  { "Gain",  50.f , 1.f, 500.f, "%.2f" };
        AudioParam<float> tight { "Tight", 0.5f , 0.f, 1.f, "%.2f" };
        AudioParam<float> level { "Level", 0.5f , 0.f, 1.f, "%.2f" };



        MetalSettings() = default;
        REGISTER_SETTINGS(MetalSettings, &gain,&tight, &level )

        MetalData getData() const {
            return { gain.get(), tight.get(), level.get() };
        }
        void setData( const MetalData& data ){
            gain.set(data.gain);
            tight.set(data.tight);
            level.set(data.level);
        }

        std::vector<std::shared_ptr<IPreset>> getPresets() const override {
            return {
                std::make_shared<Preset<MetalSettings, MetalData>>(
                    "Custom", MetalData{ 200.f, 0.5f, 0.5f}
                ),

                std::make_shared<Preset<MetalSettings, MetalData>>(
                    "Low gain", MetalData{ 50.f, 0.5f, 0.5f}
                ),
                std::make_shared<Preset<MetalSettings, MetalData>>(
                    "Med gain", MetalData{ 200.f, 0.5f, 0.5f}
                ),
                std::make_shared<Preset<MetalSettings, MetalData>>(
                    "Hi gain", MetalData{ 400.f, 0.5f, 0.5f}
                ),
            };
        }


    };

    class Metal : public DSP::Effect {


    public:
        IMPLEMENT_EFF_CLONE(Metal)

        Metal(bool switchOn = false) : DSP::Effect(DSP::EffectType::Metal, switchOn) {

            mStates.assign(2, FilterState()); // Default to stereo

        }
        virtual std::string getName() const override { return "Metal Distortion"; }
        //----------------------------------------------------------------------
        const MetalSettings& getSettings() { return mSettings; }
        void setSettings(const MetalSettings& s) { mSettings = s; }
        //----------------------------------------------------------------------
        void save(std::ostream& os) const override {
            Effect::save(os);              // Save mEnabled
            mSettings.save(os);       // Save Settings
        }
        //----------------------------------------------------------------------
        bool load(std::istream& is) override {
            if (!Effect::load(is)) return false; // Load mEnabled
            return mSettings.load(is);      // Load Settings
            return true;
        }
        //----------------------------------------------------------------------
        virtual void process(float* buffer, int numSamples, int numChannels) override {
            float level = mSettings.level.get();
            if (!isEnabled() || level <= 0.001f) return;

            if (mStates.size() != static_cast<size_t>(numChannels)) {
                mStates.assign(numChannels, FilterState());
            }

            float dt = 1.0f / mSampleRate;

            // Pre-calculate filter parameters
            float hpf_freq = 80.0f + mSettings.tight.get() * 800.0f;
            float rc_hpf = 1.0f / (2.0f * (float)M_PI * hpf_freq);
            float alpha_hpf = rc_hpf / (rc_hpf + dt);

            float lpf_freq = 4500.0f;
            float rc_lpf = 1.0f / (2.0f * (float)M_PI * lpf_freq);
            float alpha_lpf = dt / (rc_lpf + dt);

            float gain = mSettings.gain.get();

            for (int i = 0; i < numSamples; i++) {
                int ch = i % numChannels;
                FilterState& s = mStates[ch];
                float input = buffer[i];

                // 1. Pre-filtering (Tightness)
                float hpf_out = alpha_hpf * (s.last_out_hpf + input - s.last_in_hpf);
                s.last_in_hpf = input;
                s.last_out_hpf = hpf_out;

                // 2. High Gain Distortion
                float drive = hpf_out * gain;
                float distorted = std::tanh(drive);
                // Soft clipping / refinement
                if (distorted > 0.9f) distorted = 0.9f + (distorted - 0.9f) * 0.1f;
                if (distorted < -0.9f) distorted = -0.9f + (distorted + 0.9f) * 0.1f;

                // 3. Post-filtering (Cabinet-ish)
                float out = s.last_out_lpf + alpha_lpf * (distorted - s.last_out_lpf);
                s.last_out_lpf = out;

                buffer[i] = out * level;
            }
        }

    private:
        MetalSettings mSettings;

        // States for filtering
        struct FilterState {
            float last_in_hpf = 0.0f;
            float last_out_hpf = 0.0f;
            float last_out_lpf = 0.0f;
        };
        std::vector<FilterState> mStates;

    #ifdef FLUX_ENGINE
    public:
        virtual ImVec4 getColor() const override { return ImVec4(0.7f, 0.0f, 0.0f, 1.0f); }

        virtual void renderPaddle() override {
            DSP::MetalSettings currentSettings = this->getSettings();
            currentSettings.gain.setKnobSettings(ImFlux::ksRed); // NOTE only works here !
            if (currentSettings.DrawPaddle(this)) {
                this->setSettings(currentSettings);
            }
        }

        virtual void renderUIWide() override {
            DSP::MetalSettings currentSettings = this->getSettings();
            if (currentSettings.DrawUIWide(this)) {
                this->setSettings(currentSettings);
            }
        }
        virtual void renderUI() override {
            DSP::MetalSettings currentSettings = this->getSettings();
            if (currentSettings.DrawUI(this)) {
                this->setSettings(currentSettings);
            }
        }


        #endif
    };

} // namespace DSP
