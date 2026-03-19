//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : SpectrumAnalyzer
//-----------------------------------------------------------------------------
// - no need for ISettings
//-----------------------------------------------------------------------------

#pragma once

#include <vector>
#include <cstdint>
#include <algorithm>
#include <cstring>
#include <atomic>
#include <complex>

#ifdef FLUX_ENGINE
#include <imgui.h>
#include <imgui_internal.h>
#include <gui/ImFlux.h>
#endif


#include "DSP_Effect.h"
#include "DSP_Math.h"

namespace DSP {
    class SpectrumAnalyzer : public Effect {
    private:
        static constexpr int FFT_SIZE = 2048; //orig 512 Must be power of 2 .. 2048 would be better for FFT!
        std::vector<float> mCaptureBuffer;
        std::vector<float> mDisplayMagnitudes;
        int mWriteIdx = 0;

    public:
        IMPLEMENT_EFF_CLONE_NO_SETTINGS(SpectrumAnalyzer)

        SpectrumAnalyzer(bool switchOn = false) : Effect(DSP::EffectType::SpectrumAnalyzer, switchOn) {
            mEffectName = "SPECTRUM ANALYSER";

            mCaptureBuffer.resize(FFT_SIZE, 0.0f);
            mDisplayMagnitudes.resize(FFT_SIZE / 2, 0.0f);
        }


        //----------------------------------------------------------------------
        virtual void process(float* buffer, int numSamples, int numChannels) override {
            if (!mEnabled) return;

            // Process in frames to handle interleaving correctly
            for (int i = 0; i < numSamples; i += numChannels) {
                float monoSum = 0.0f;
                // 1. Sum all channels in the current frame
                for (int c = 0; c < numChannels; ++c) {
                    //NOTE: Added * 0.85f to match limiter
                    monoSum += buffer[i + c];// * 0.85f;
                }
                // 2. Average the sum to keep the level consistent
                monoSum /= static_cast<float>(numChannels);
                // 3. Capture into the circular buffer
                mCaptureBuffer[mWriteIdx] = monoSum;
                mWriteIdx = (mWriteIdx + 1) % FFT_SIZE;
            }
        }
        //----------------------------------------------------------------------
        // Helper to reorder the array for FFT (Bit-reversal)
        void bitReverse(std::vector<std::complex<float>>& x) {
            int n = (int)x.size();
            for (int i = 1, j = 0; i < n; i++) {
                int bit = n >> 1;
                for (; j & bit; bit >>= 1) j ^= bit;
                j ^= bit;
                if (i < j) std::swap(x[i], x[j]);
            }
        }
        //----------------------------------------------------------------------
        // Efficient In-Place FFT implementation
        void performFFT(std::vector<std::complex<float>>& x) {
            int n = (int)x.size();
            bitReverse(x);

            // FAST MATH VERSION
            for (int len = 2; len <= n; len <<= 1) {
                float phase01 = -1.0f / (float)len;
                while (phase01 < 0.0f) phase01 += 1.0f;
                std::complex<float> wlen(
                    DSP::FastMath::fastCos(phase01),
                    DSP::FastMath::fastSin(phase01)
                );

                for (int i = 0; i < n; i += len) {
                    std::complex<float> w(1, 0);
                    for (int j = 0; j < len / 2; j++) {
                        std::complex<float> u = x[i + j];
                        std::complex<float> v = x[i + j + len / 2] * w;
                        x[i + j] = u + v;
                        x[i + j + len / 2] = u - v;
                        w *= wlen;
                    }
                }
            }

            // for (int len = 2; len <= n; len <<= 1) {
            //     float angle = -2.0f * M_PI / len;
            //     std::complex<float> wlen(std::cos(angle), std::sin(angle));
            //     for (int i = 0; i < n; i += len) {
            //         std::complex<float> w(1, 0);
            //         for (int j = 0; j < len / 2; j++) {
            //             std::complex<float> u = x[i + j];
            //             std::complex<float> v = x[i + j + len / 2] * w;
            //             x[i + j] = u + v;
            //             x[i + j + len / 2] = u - v;
            //             w *= wlen;
            //         }
            //     }
            // }



        }
        //----------------------------------------------------------------------
        const std::vector<float>& getMagnitudesFFT() {
            // Prepare data for FFT (Windowing)
            // Reuse a static vector to avoid heap allocation every frame
            static std::vector<std::complex<float>> fftData(FFT_SIZE);

            for (int i = 0; i < FFT_SIZE; i++) {
                // Hann Window to prevent spectral leakage
                float window = 0.5f * (1.0f - std::cos(2.0f * (float)M_PI * i / (FFT_SIZE - 1)));
                // Align read pointer to the latest write position
                float sample = mCaptureBuffer[(mWriteIdx + i) % FFT_SIZE];
                fftData[i] = std::complex<float>(sample * window, 0.0f);
            }


            performFFT(fftData);

            int numBars = mDisplayMagnitudes.size();

            //~~~~~~~~~~~~~~~~~~~~~

            constexpr float minFreq = 63.f; //63.0f;
            constexpr float maxFreq = 16000.0f;
            float ratio = maxFreq / minFreq;

            for (int i = 0; i < numBars; i++) {
                float fLow  = minFreq * std::pow(ratio, (float)i / numBars);
                float fHigh = minFreq * std::pow(ratio, (float)(i + 1) / numBars);


                int startBin = std::max(1, (int)(fLow * FFT_SIZE / mSampleRate));
                int endBin = std::max(startBin + 1, (int)(fHigh * FFT_SIZE / mSampleRate));

                float avgMag = 0.0f;
                for (int bin = startBin; bin < endBin && bin < FFT_SIZE / 2; bin++) {
                    if (bin == 0 ) continue;
                    avgMag += std::abs(fftData[bin]);
                }

                // avgMag /= (endBin - startBin);
                float amplitude = avgMag / (FFT_SIZE / 2.0f);
                // boost 40..150
                float visualVal = std::log10(amplitude * 60.0f + 1.0f);


            // better but still not good
            // FFT_SIZE to 2048
            // float binStep = (float)(FFT_SIZE / 2) / std::pow(numBars, 1.5f);

            // for (int i = 0; i < numBars; i++) {
            //
            //     float lowF  = std::pow((float)i / numBars, 2.0f) * (FFT_SIZE / 2);
            //     float highF = std::pow((float)(i + 1) / numBars, 2.0f) * (FFT_SIZE / 2);
            //
            //     if (highF - lowF < 1.0f) highF = lowF + 1.0f;
            //
            //     float avgMag = 0.0f;
            //     int count = 0;
            //
            //     for (int bin = (int)lowF; bin < (int)highF && bin < FFT_SIZE / 2; bin++) {
            //         if (bin < 2 ) continue; // DC Offset
            //         avgMag += std::abs(fftData[bin]);
            //         count++;
            //     }
            //
            //     // if (count > 0) avgMag /= count;
            //
            //     float amplitude = avgMag / (FFT_SIZE / 2);
            //     float visualVal = std::log10(amplitude * 100.0f + 1.0f);


            // ~~~~~~~~~~~~~~~~~~~~~~~
            // // 3. Map FFT Bins to UI Bars
            // int numBars = (int)mDisplayMagnitudes.size();
            // for (int i = 0; i < numBars; i++) {
            //     // Smooth decay (fallback)
            //     mDisplayMagnitudes[i] *= 0.88f;
            //
            //     // Logarithmic frequency mapping
            //     float normX = (float)i / numBars;
            //
            //     int lowBin = (int)(std::pow(2.0f, (normX * 7.0f) + 1.5f));
            //     int highBin = (int)(std::pow(2.0f, (i + 1.0f) / numBars * 8.0f));
            //
            //     lowBin = std::clamp(lowBin, 0, FFT_SIZE / 2 - 1);
            //     highBin = std::clamp(highBin, lowBin + 1, FFT_SIZE / 2);
            //
            //     float avgMag = 0.0f;
            //     for (int bin = lowBin; bin < highBin; bin++) {
            //         float amplitude = std::abs(fftData[bin]) / (FFT_SIZE / 2);
            //
            //         if (bin < 6) {
            //             amplitude *= (bin / 6.0f);
            //         }
            //         avgMag += amplitude;
            //
            //     }
            //     avgMag /= (highBin - lowBin); // Average energy in this frequency band
            //
            //     float visualVal = std::log10(avgMag * 60.0f + 1.0f);
            //     // visualVal *= 0.8f;

                // Peak-Smoothing
                if (visualVal > mDisplayMagnitudes[i]) {
                    // Attack
                    mDisplayMagnitudes[i] = visualVal;
                } else {
                    // (Release) - 0.85f .. 0.95f
                    mDisplayMagnitudes[i] *= 0.85f;
                }
            } //for
            return mDisplayMagnitudes;
        }

        //----------------------------------------------------------------------
        const std::vector<float>& getMagnitudes() {
            // Determine how many samples to check per display bar
            // This ensures all captured data is represented in the visual
            int samplesPerBar = FFT_SIZE / std::max(1, (int)mDisplayMagnitudes.size());

            for (int i = 0; i < (int)mDisplayMagnitudes.size(); ++i) {
                // Smoothing: Slow decay for a "fallback" effect
                mDisplayMagnitudes[i] *= 0.92f;

                // Peak Detection in the range assigned to this bar
                float peak = 0.0f;
                for (int j = 0; j < samplesPerBar; ++j) {
                    int idx = (i * samplesPerBar + j) % FFT_SIZE;
                    float val = std::abs(mCaptureBuffer[idx]);
                    if (val > peak) peak = val;
                }

                // Update the display bar if the new peak is higher
                if (peak > mDisplayMagnitudes[i]) {
                    mDisplayMagnitudes[i] = peak;
                }
            }

            return mDisplayMagnitudes;
        }
        //----------------------------------------------------------------------
        // virtual std::string getName() const override { return "SPECTRUM ANALYSER";}
        //----------------------------------------------------------------------
        //----------------------------------------------------------------------
        #ifdef FLUX_ENGINE
        virtual ImVec4 getDefaultColor() const  override { return ImVec4(0.73f, 0.8f, 0.73f, 1.0f);}
        // i dont want to render UI here !
        virtual void renderUIWide() override {};
        virtual void renderUI() override {};


        inline void DrawSpectrumAnalyzer(ImVec2 size, bool useFFT = false) {
            SpectrumAnalyzer* analyzer = this;
            if (!analyzer->isEnabled()) return;

            const auto& mags = useFFT ? analyzer->getMagnitudesFFT() : analyzer->getMagnitudes();

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
