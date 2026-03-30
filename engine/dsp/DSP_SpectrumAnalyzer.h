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
        // static constexpr int FFT_SIZE = 512; //orig 512 Must be power of 2 .. 2048 would be better for FFT!
        uint16_t mFFT_SIZE = 512; //orig 512 Must be power of 2 .. 2048 would be better for FFT!
        std::vector<float> mCaptureBuffer;
        std::vector<float> mDisplayMagnitudes;
        int mWriteIdx = 0;

        void updateBuffers() {
            mCaptureBuffer.resize(mFFT_SIZE, 0.0f);
            mDisplayMagnitudes.resize(mFFT_SIZE / 2, 0.0f);
        }

    public:
        IMPLEMENT_EFF_CLONE_NO_SETTINGS(SpectrumAnalyzer)

        SpectrumAnalyzer(bool switchOn = false) : Effect(DSP::EffectType::SpectrumAnalyzer, switchOn) {
            mEffectName = "SPECTRUM ANALYSER";
            updateBuffers();
        }

        void setFFTSize(uint16_t fftSize) {
            mFFT_SIZE = fftSize;
            updateBuffers();
        }

        uint16_t getFFTSize() const { return mFFT_SIZE; }

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
                mWriteIdx = (mWriteIdx + 1) % mFFT_SIZE;
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
            static std::vector<std::complex<float>> fftData(mFFT_SIZE);

            for (int i = 0; i < mFFT_SIZE; i++) {
                // Hann Window to prevent spectral leakage
                float window = 0.5f * (1.0f - std::cos(2.0f * (float)M_PI * i / (mFFT_SIZE - 1)));
                // Align read pointer to the latest write position
                float sample = mCaptureBuffer[(mWriteIdx + i) % mFFT_SIZE];
                fftData[i] = std::complex<float>(sample * window, 0.0f);
            }


            performFFT(fftData);

            int numBars = mDisplayMagnitudes.size();

            constexpr float minFreq = 125.f; //63.0f;
            constexpr float maxFreq = 16000.0f;
            float ratio = maxFreq / minFreq;

            for (int i = 0; i < numBars; i++) {
                float fLow  = minFreq * std::pow(ratio, (float)i / numBars);
                float fHigh = minFreq * std::pow(ratio, (float)(i + 1) / numBars);


                int startBin = std::max(1, (int)(fLow * mFFT_SIZE / mSampleRate));
                int endBin = std::max(startBin + 1, (int)(fHigh * mFFT_SIZE / mSampleRate));

                float avgMag = 0.0f;
                for (int bin = startBin; bin < endBin && bin < mFFT_SIZE / 2; bin++) {
                    if (bin == 0 ) continue;
                    avgMag += std::abs(fftData[bin]);
                }

                // avgMag /= (endBin - startBin);
                float amplitude = avgMag / (mFFT_SIZE / 2.0f);
                // boost 40..150
                float visualVal = std::log10(amplitude * 60.0f + 1.0f);


                // Peak-Smoothing
                if (visualVal > mDisplayMagnitudes[i]) {
                    // Attack
                    mDisplayMagnitudes[i] = visualVal;
                } else {
                    // (Release) - 0.85f .. 0.95f
                    mDisplayMagnitudes[i] *= 0.85f; //0.85f;
                }
            } //for
            return mDisplayMagnitudes;
        }

        //----------------------------------------------------------------------
        const std::vector<float>& getMagnitudes() {
            // Determine how many samples to check per display bar
            // This ensures all captured data is represented in the visual
            int samplesPerBar = mFFT_SIZE / std::max(1, (int)mDisplayMagnitudes.size());

            for (int i = 0; i < (int)mDisplayMagnitudes.size(); ++i) {
                // Smoothing: Slow decay for a "fallback" effect
                mDisplayMagnitudes[i] *= 0.92f;

                // Peak Detection in the range assigned to this bar
                float peak = 0.0f;
                for (int j = 0; j < samplesPerBar; ++j) {
                    int idx = (i * samplesPerBar + j) % mFFT_SIZE;
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
        // lower powValue s (default was 1.5)
        std::vector<float> getLogarithmicBands( int numTargetBands, bool useFFT = false, float powValue = 0.85f) {
            SpectrumAnalyzer* analyzer = this;
            const auto& linearMags = useFFT ? analyzer->getMagnitudesFFT() : analyzer->getMagnitudes();

            std::vector<float> logBands(numTargetBands, 0.0f);
            int numLinear = static_cast<int>(linearMags.size());

            for (int i = 0; i < numTargetBands; ++i) {
                // Calculate start and end indices for this log-band
                // Use power function to make lower bands narrower and higher bands wider
                float startRel = pow(static_cast<float>(i) / numTargetBands, powValue);
                float endRel   = pow(static_cast<float>(i + 1) / numTargetBands, powValue);

                int startIdx = static_cast<int>(startRel * numLinear);
                int endIdx   = static_cast<int>(endRel * numLinear);

                // Ensure we at least pick one index
                if (endIdx <= startIdx) endIdx = startIdx + 1;

                // Average the magnitudes in this range
                float sum = 0.0f;
                int count = 0;
                for (int j = startIdx; j < endIdx && j < numLinear; ++j) {
                    sum += linearMags[j];
                    count++;
                }

                logBands[i] = (count > 0) ? (sum / count) : 0.0f;
            }
            return logBands;
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


        inline void DrawSpectrumAnalyzer(ImVec2 size, bool useFFT = true, uint16_t numBands = 64) {
            SpectrumAnalyzer* analyzer = this;
            if (!analyzer->isEnabled()) return;

            // const auto& mags = useFFT ? analyzer->getMagnitudesFFT() : analyzer->getMagnitudes();


            const auto& mags = analyzer->getLogarithmicBands(numBands, useFFT);


            ImDrawList* drawList = ImGui::GetWindowDrawList();
            ImVec2 p = ImGui::GetCursorScreenPos();

            // Background
            drawList->AddRectFilled(p, {p.x + size.x, p.y + size.y}, ImColor(15, 15, 15), 2.0f);

            // int numBars = (int)mags.size() / 4; // Group bins for cleaner look
            int numBars = numBands;
            float barWidth = size.x / numBars;
            float gap = 1.0f;

            for (int i = 0; i < numBars; i++) {
                // Average a few bins for each bar
                // float val = (mags[i*2] + mags[i*2+1]) * 0.5f;
                float val = mags[i];
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
