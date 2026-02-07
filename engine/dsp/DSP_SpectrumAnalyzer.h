//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : SpectrumAnalyzer
//-----------------------------------------------------------------------------
#pragma once

#include <vector>
#include <cstdint>
#include <algorithm>
#include <cstring>
#include <atomic>

#ifdef FLUX_ENGINE
#include <imgui.h>
#include <imgui_internal.h>
#include <gui/ImFlux.h>
#endif


#include "DSP_Effect.h"

namespace DSP {
    class SpectrumAnalyzer : public Effect {
    private:
        static constexpr int FFT_SIZE = 512; // Must be power of 2
        std::vector<float> mCaptureBuffer;
        std::vector<float> mDisplayMagnitudes;
        int mWriteIdx = 0;

    public:
        SpectrumAnalyzer(bool switchOn = false) : Effect(switchOn) {
            mCaptureBuffer.resize(FFT_SIZE, 0.0f);
            mDisplayMagnitudes.resize(FFT_SIZE / 2, 0.0f);
        }

        DSP::EffectType getType() const override { return DSP::EffectType::SpectrumAnalyzer; }

        // The audio callback calls this
        virtual void process(float* buffer, int numSamples) override {
            if (!mEnabled) return;

            // Capture samples into our circular buffer
            for (int i = 0; i < numSamples; ++i) {
                mCaptureBuffer[mWriteIdx] = buffer[i];
                mWriteIdx = (mWriteIdx + 1) % FFT_SIZE;
            }
        }

        // The GUI thread calls this to get the data
        const std::vector<float>& getMagnitudes() {
            // TODO: Replace with a FFT logic (e.g., KissFFT)
            for (int i = 0; i < mDisplayMagnitudes.size(); ++i) {
                // Smoothing logic: slowly decay the bars
                mDisplayMagnitudes[i] *= 0.92f;

                // Peak detection from the captured buffer
                float sample = std::abs(mCaptureBuffer[i % FFT_SIZE]);
                if (sample > mDisplayMagnitudes[i]) mDisplayMagnitudes[i] = sample;
            }
            return mDisplayMagnitudes;
        }
        virtual std::string getName() const override { return "SPECTRUM ANALYSER";}
        #ifdef FLUX_ENGINE
        virtual ImVec4 getColor() const  override { return ImVec4(0.73f, 0.8f, 0.73f, 1.0f);}
        // i dont want to render UI here !
        virtual void renderUIWide() override {};
        virtual void renderUI() override {};


        inline void DrawSpectrumAnalyzer(ImVec2 size) {
            SpectrumAnalyzer* analyzer = this;
            if (!analyzer->isEnabled()) return;

            const auto& mags = analyzer->getMagnitudes();
            ImDrawList* drawList = ImGui::GetWindowDrawList();
            ImVec2 p = ImGui::GetCursorScreenPos();

            // Background
            drawList->AddRectFilled(p, {p.x + size.x, p.y + size.y}, ImColor(15, 15, 15), 2.0f);

            int numBars = (int)mags.size() / 4; // Group bins for cleaner look
            float barWidth = size.x / numBars;
            float gap = 1.0f;

            for (int i = 0; i < numBars; i++) {
                // Average a few bins for each bar
                float val = (mags[i*2] + mags[i*2+1]) * 0.5f;
                float height = std::clamp(val * size.y * 2.0f, 2.0f, size.y);

                // Color Gradient: Green (bottom) to Red (top)
                ImU32 col = ImGui::ColorConvertFloat4ToU32(ImVec4(
                    std::clamp(val * 2.0f, 0.0f, 1.0f), // Red increases with height
                                                                  std::clamp(2.0f - val * 2.0f, 0.0f, 1.0f), // Green decreases
                                                                  0.2f, 1.0f
                ));

                ImVec2 barMin = {p.x + (i * barWidth) + gap, p.y + size.y - height};
                ImVec2 barMax = {p.x + ((i + 1) * barWidth) - gap, p.y + size.y};

                drawList->AddRectFilled(barMin, barMax, col, 1.0f);
            }

            ImGui::Dummy(size);
        }
        #endif

    }; //class


} //namespace
