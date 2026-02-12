//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : VisualAnalyzer
//-----------------------------------------------------------------------------
#pragma once

#include <vector>
#include <cstdint>
#include <algorithm>
#include <cstring>
#include <atomic>
#include <mutex>

#ifdef FLUX_ENGINE
#include <imgui.h>
#include <imgui_internal.h>
#include <gui/ImFlux.h>
#endif


#include "DSP_Effect.h"

namespace DSP {
    class VisualAnalyzer : public Effect {
    private:
        std::vector<float> mMirrorBuffer;
        std::mutex mDataMutex;

        // left right values for a VU-Meter
        // float mLeftLevel = 0.0f;
        // float mRightLevel = 0.0f;

        // float mRmsL = 0.0f;
        // float mRmsR = 0.0f;


        std::vector<float> mChannelLevels;
        std::vector<float> mChannelRms;
        std::vector<float> mSums;


    public:
        std::unique_ptr<Effect> clone() const override {
            // IMPLEMENT_EFF_CLONE: we do not need the clone macro here ... we have no data to copy
            return std::make_unique<VisualAnalyzer>();
        }


        VisualAnalyzer( bool switchOn = true) : Effect(switchOn) {
            mMirrorBuffer.resize(2048, 0.0f); // Size for the oscilloscope display
        }
        //----------------------------------------------------------------------
        virtual void process(float* buffer, int numSamples, int numChannels) override {
            if (!mEnabled) return;

            // 1. Initialize channel levels if count changed
            if (mChannelLevels.size() != static_cast<size_t>(numChannels)) {
                mChannelLevels.assign(numChannels, 0.0f);
                 mChannelRms.assign(numChannels, 0.0f);
                 mSums.assign(numChannels, 0.0f);
            }

            {
                std::lock_guard<std::mutex> lock(mDataMutex);

                // 2. Mirror Buffer (Oscilloscope View)
                // For visual clarity, we often mirror the "Mono Sum" or "Main Left"
                // to avoid a messy overlapping waveform. Here we mirror the interleaved data.
                int toCopy = std::min(numSamples, (int)mMirrorBuffer.size());
                if (toCopy > 0) {
                    std::move(mMirrorBuffer.begin() + toCopy, mMirrorBuffer.end(), mMirrorBuffer.begin());
                    std::copy(buffer + (numSamples - toCopy), buffer + numSamples, mMirrorBuffer.end() - toCopy);
                }
            }

            std::fill(mSums.begin(), mSums.end(), 0.0f);

            // 3. RMS Level Calculation per Channel
            int numFrames = numSamples / numChannels;
            if (numFrames > 0) {
                for (int i = 0; i < numSamples; i++) {
                    int channel = i % numChannels;
                    float sample = buffer[i];

                    // Safety check
                    if (!std::isfinite(sample)) sample = 0.0f;

                    mSums[channel] += (sample * sample);
                }

                for (int c = 0; c < numChannels; ++c) {
                    // Calculate RMS
                    float rms = std::sqrt(std::max(0.0f, mSums[c] / static_cast<float>(numFrames)));

                    // Visual Gain and Denormal Protection
                    float rawLevel = rms * 2.0f;
                    if (rawLevel < 1e-6f || !std::isfinite(rawLevel)) rawLevel = 0.0f;

                    mChannelRms[c] = rawLevel;
                    //denormal protection
                    if ( mChannelRms[c] < 1e-6f)  mChannelRms[c]= 0.0f;
                    if (!std::isfinite( mChannelRms[c]))  mChannelRms[c] = 0.0f;

                    // 4. Smoothing (Ballistics) per channel
                    mChannelLevels[c] = (mChannelLevels[c] * 0.8f) + (rawLevel * 0.2f);

                }
            }
        }


        // // The process method just copies data, it doesn't change it
        // virtual void process(float* buffer, int numSamples, int numChannels) override {
        //     if (numChannels !=  2) { return;  }  //FIXME REWRITE from stereo TO variable CHANNELS
        //     if (!mEnabled) return;
        //
        //     std::lock_guard<std::mutex> lock(mDataMutex);
        //     // We store the last 'n' samples for the GUI to pick up
        //     int toCopy = std::min(numSamples, (int)mMirrorBuffer.size());
        //
        //     // Shift old data and add new (or use a ring buffer)
        //     std::move(mMirrorBuffer.begin() + toCopy, mMirrorBuffer.end(), mMirrorBuffer.begin());
        //     std::copy(buffer + (numSamples - toCopy), buffer + numSamples, mMirrorBuffer.end() - toCopy);
        //
        //     float sumL = 0.0f, sumR = 0.0f;
        //     int numFrames = numSamples / 2;
        //
        //     if (numFrames > 0) {
        //         for (int i = 0; i < numSamples; i += 2) {
        //             float sL = buffer[i];
        //             float sR = buffer[i+1];
        //
        //             // 1. Check for NaN/Inf in input to prevent poisoning the sum
        //             if (!std::isfinite(sL)) sL = 0.0f;
        //             if (!std::isfinite(sR)) sR = 0.0f;
        //
        //             sumL += (sL * sL);
        //             sumR += (sR * sR);
        //         }
        //
        //         // 2. Prevent division by zero and sqrt of negative (safety first)
        //         float rmsL = std::sqrt(std::max(0.0f, sumL / (float)numFrames));
        //         float rmsR = std::sqrt(std::max(0.0f, sumR / (float)numFrames));
        //
        //         // 3. Apply your visual gain (2.0f)
        //         mRmsL = rmsL * 2.0f;
        //         mRmsR = rmsR * 2.0f;
        //
        //         // 4. Denormal Protection & Cleaning
        //         if (mRmsL < 1e-6f) mRmsL = 0.0f;
        //         if (mRmsR < 1e-6f) mRmsR = 0.0f;
        //         if (!std::isfinite(mRmsL)) mRmsL = 0.0f;
        //         if (!std::isfinite(mRmsR)) mRmsR = 0.0f;
        //
        //         // 5. Smoothing (Ballistics)
        //         mLeftLevel  = (mLeftLevel  * 0.8f) + (mRmsL * 0.2f);
        //         mRightLevel = (mRightLevel * 0.8f) + (mRmsR * 0.2f);
        //     }
        //
        // }
        //----------------------------------------------------------------------
        // GUI calls this to get the data
        void getLatestSamples(std::vector<float>& outBuffer) {
            std::lock_guard<std::mutex> lock(mDataMutex);
            outBuffer = mMirrorBuffer;
        }
        //----------------------------------------------------------------------
        float getLevel(int channel) {
            if (channel >= 0 && channel < (int)mChannelLevels.size()) {
                return mChannelLevels[channel];
            }
            return 0.0f;
        }
        //----------------------------------------------------------------------
        float getDecible(int channel) {
            if (channel >= 0 && channel < (int)mChannelLevels.size()) {
                return 20.0f * std::log10(mChannelLevels[channel] + 1e-9f);
            }
            return -90.0f; // Silence floor
        }
        //----------------------------------------------------------------------
        float getRMS(int channel) {
            if (channel >= 0 && channel < (int)mChannelRms.size()) {
                return mChannelRms[channel];
            }
            return 0.0f;
        }
        //----------------------------------------------------------------------
        DSP::EffectType getType() const override { return DSP::EffectType::VisualAnalyzer; }


        virtual std::string getName() const override { return "VISUAL ANALYSER";}
#ifdef FLUX_ENGINE
        virtual ImVec4 getColor() const  override { return ImVec4(0.93f, 0.8f, 0.93f, 1.0f);}
        // i dont want to render UI here !
        virtual void renderUIWide() override {};
        virtual void renderUI() override {};




    // protected:
    //     ImFlux::VUMeterState mVUMeterStateLeft   = ImFlux::VUMETER_DEFAULT;
    //     ImFlux::VUMeterState mVUMeterStateRight  = ImFlux::VUMETER_DEFAULT;
    // public:

    void renderVU(ImVec2 size, int century = 80)
    {
        // Calculate half width for stereo pairs
        float spacing = ImGui::GetStyle().ItemSpacing.x;
        ImVec2 halfSize = ImVec2((size.x - spacing) * 0.5f, size.y);

        if (century >= 80) { // 80s and 90s (LED Styles)
            float levL, levR;
            levL = this->getLevel(0);
            levR = this->getLevel(1);
            // this->getLevels(levL, levR);
            // this->getRMS(levL, levR);

            // 90s Style: Slim vertical-ish bars with dots
            if (century >= 90) {
                // ImFlux::VUMeter90th(levL, this->mVUMeterStateLeft);
                // ImFlux::VUMeter90th(levR, this->mVUMeterStateRight);

                ImFlux::VUMeter80th(levL, 24, ImVec2(4.f, 16.f));
                ImFlux::VUMeter80th(levR, 24, ImVec2(4.f, 16.f));

            }
            // 80s Style: Chunky horizontal blocks
            else {
                ImFlux::VUMeter80th(levL, 8, ImVec2(22.f, 8.f));
                ImFlux::VUMeter80th(levR, 8, ImVec2(22.f, 8.f));
            }
        }
        else { // 70s Style (Analog Needle)
            float dbL, dbR;
            dbL = this->getDecible(0);
            dbR = this->getDecible(1);

            // this->getDecible(dbL, dbR);

            auto mapDB = [](float db) {
                float minDB = -20.0f;
                return (db < minDB) ? 0.0f : (db - minDB) / (0.0f - minDB);
            };

            ImFlux::VUMeter70th(halfSize, mapDB(dbL));
            ImGui::SameLine();
            ImFlux::VUMeter70th(halfSize, mapDB(dbR));
        }
    }

    void renderPeakTest(bool withBackGround = true)
    {
        if (ImGui::BeginChild("RENDER_PEAK_TEST", ImVec2(0.f, 400.f))) {
            if (withBackGround) {
                ImFlux::GradientBox(ImVec2(-FLT_MIN, -FLT_MIN), 0.f);
                // ImGui::Dummy(ImVec2(2, 0)); ImGui::SameLine();
            }

            ImFlux::ShadowText("VU METERS ::: test rendering");


            // RMS (The raw power)
            float rmsL, rmsR;
            rmsL = this->getRMS(0);
            rmsR = this->getRMS(1);
            // this->getRMS(rmsL, rmsR);
            ImGui::TextDisabled("RMS L:%7.3f R:%7.3f", rmsL, rmsR);
            ImFlux::PeakMeter(rmsL); ImGui::SameLine(); ImFlux::PeakMeter(rmsR);

            // Levels
            float levL, levR;
            levL = this->getLevel(0);
            levR = this->getLevel(1);
            // this->getLevels(levL, levR);
            ImGui::TextDisabled("levels L:%7.3f R:%7.3f", levL, levR);
            ImFlux::PeakMeter(levL); ImGui::SameLine(); ImFlux::PeakMeter(levR);

            // 3. DECIBELS (The professional way)
            float dbL, dbR;
            dbL = this->getDecible(0);
            dbR = this->getDecible(1);

            // this->getDecible(dbL, dbR);
            ImGui::TextDisabled("dB  L:%7.1f R:%7.1f", dbL, dbR);

            // FIX for dB Meter: Map -60dB...0dB to 0.0...1.0 linear for the PeakMeter
            // We call this "Normalized dB"
            auto mapDB = [](float db) {
                float minDB = -60.0f; // Silence floor
                if (db < minDB) return 0.0f;
                return (db - minDB) / (0.0f - minDB); // Map to 0.0 -> 1.0 range
            };
            ImFlux::PeakMeter(mapDB(dbL)); ImGui::SameLine(); ImFlux::PeakMeter(mapDB(dbR));


            this->renderVU(ImVec2(240.f, 75.f), 70);
            this->renderVU(ImVec2(240.f, 75.f), 80);
            this->renderVU(ImVec2(240.f, 75.f), 90);
        }
        ImGui::EndChild();
    }
    //----------------------------------------------------------------------
    inline void DrawVisualAnalyzerOszi(ImVec2 size, int numChannels) {
        VisualAnalyzer* analyzer = this;
        if (!analyzer || !analyzer->isEnabled()) return;

        // 1. Setup Colors (Expand this array for more channels if needed)
        static const ImU32 channelColors[] = {
            IM_COL32(0, 255, 255, 220),   // Cyan (Ch 1 / L)
            IM_COL32(255, 255, 0, 180),   // Yellow (Ch 2 / R)
            IM_COL32(255, 0, 255, 180),   // Magenta (Ch 3)
            IM_COL32(0, 255, 0, 180),     // Green (Ch 4)
            IM_COL32(255, 128, 0, 180),   // Orange (Ch 5)
            IM_COL32(128, 128, 255, 180)  // Blue-ish (Ch 6)
        };
        const int maxColorCount = sizeof(channelColors) / sizeof(ImU32);

        if (size.x <= 0.0f) size.x = ImGui::GetContentRegionAvail().x;
        if (size.y <= 0.0f) size.y = ImGui::GetContentRegionAvail().y;

        static std::vector<float> samples;
        analyzer->getLatestSamples(samples);
        if (samples.empty() || numChannels <= 0) {
            ImGui::Dummy(size);
            return;
        }

        ImDrawList* dl = ImGui::GetWindowDrawList();
        ImVec2 pos = ImGui::GetCursorScreenPos();
        float mid_y = pos.y + size.y * 0.5f;

        // --- Background & Grid ---
        dl->AddRectFilled(pos, pos + size, IM_COL32(5, 12, 5, 255));
        // ... (Grid drawing logic same as before) ...

        // --- 2. Calculate Frames ---
        int num_frames = (int)samples.size() / numChannels;
        if (num_frames < 2) { ImGui::Dummy(size); return; }

        // Use a 2D-like point buffer approach to avoid reallocating many vectors
        static std::vector<ImVec2> points;
        points.resize(num_frames);

        float x_step = size.x / (float)(num_frames - 1);
        float yScale = size.y * 0.45f;

        // --- 3. Draw each channel separately ---
        for (int c = 0; c < numChannels; ++c) {
            // Calculate points for this specific channel
            for (int i = 0; i < num_frames; ++i) {
                float x = pos.x + i * x_step;
                // Access the sample for frame 'i' and channel 'c'
                float s = samples[i * numChannels + c];
                points[i] = ImVec2(x, mid_y - (s * yScale));
            }

            // Get color for this channel (wrap around if more channels than colors)
            ImU32 col = channelColors[c % maxColorCount];

            // Render current channel path
            dl->AddPolyline(points.data(), num_frames, col, 0, 1.2f);
        }

        // Border & Dummy
        dl->AddRect(pos, pos + size, IM_COL32(100, 100, 100, 255));
        ImGui::Dummy(size);
    }

    // inline void DrawVisualAnalyzerOszi(ImVec2 size) {
    //     VisualAnalyzer* analyzer = this;
    //     if (!analyzer || !analyzer->isEnabled()) return;
    //
    //     // 1. Resolve Auto-Size
    //     if (size.x <= 0.0f) size.x = ImGui::GetContentRegionAvail().x;
    //     if (size.y <= 0.0f) size.y = ImGui::GetContentRegionAvail().y;
    //
    //     static std::vector<float> samples;
    //     analyzer->getLatestSamples(samples);
    //     if (samples.empty()) {
    //         ImGui::Dummy(size);
    //         return;
    //     }
    //
    //     ImDrawList* dl = ImGui::GetWindowDrawList();
    //     ImVec2 pos = ImGui::GetCursorScreenPos();
    //     float mid_y = pos.y + size.y * 0.5f;
    //
    //     // 2. Draw Background & Grid (Keep it retro)
    //     dl->AddRectFilled(pos, pos + size, IM_COL32(5, 12, 5, 255));
    //     ImU32 gridCol = IM_COL32(30, 60, 30, 120);
    //     for (int i = 1; i < 10; ++i) {
    //         dl->AddLine(ImVec2(pos.x + size.x * (i / 10.0f), pos.y), ImVec2(pos.x + size.x * (i / 10.0f), pos.y + size.y), gridCol);
    //         dl->AddLine(ImVec2(pos.x, pos.y + size.y * (i / 10.0f)), ImVec2(pos.x + size.x, pos.y + size.y * (i / 10.0f)), gridCol);
    //     }
    //     dl->AddLine(ImVec2(pos.x, mid_y), ImVec2(pos.x + size.x, mid_y), IM_COL32(80, 160, 80, 180), 1.5f);
    //
    //     // 3. Prepare Points for Polyline
    //     int num_frames = (int)samples.size() / 2;
    //     static std::vector<ImVec2> pointsL, pointsR;
    //     pointsL.resize(num_frames);
    //     pointsR.resize(num_frames);
    //
    //     float x_step = size.x / (float)(num_frames - 1);
    //     float yScale = size.y * 0.45f;
    //
    //     for (int i = 0; i < num_frames; ++i) {
    //         float x = pos.x + i * x_step;
    //         // Left Channel
    //         pointsL[i] = ImVec2(x, mid_y - (samples[i * 2] * yScale));
    //         // Right Channel
    //         pointsR[i] = ImVec2(x, mid_y - (samples[i * 2 + 1] * yScale));
    //     }
    //
    //     // 4. Render with Polyline (Much faster for many segments)
    //     dl->AddPolyline(pointsL.data(), num_frames, IM_COL32(0, 255, 255, 220), 0, 1.5f);
    //     dl->AddPolyline(pointsR.data(), num_frames, IM_COL32(255, 255, 0, 180), 0, 1.5f);
    //
    //     // Border
    //     dl->AddRect(pos, pos + size, IM_COL32(100, 100, 100, 255));
    //     ImGui::Dummy(size);
    // }

    // inline void DrawVisualAnalyzerOszi(ImVec2 size) {
    //     VisualAnalyzer* analyzer = this;
    //     if (!analyzer || !analyzer->isEnabled()) return;
    //
    //     // 1. Resolve Auto-Size (Handle -1.0 or FLT_MIN)
    //     if (size.x <= 0.0f) size.x = ImGui::GetContentRegionAvail().x;
    //     if (size.y <= 0.0f) size.y = ImGui::GetContentRegionAvail().y;
    //
    //     static std::vector<float> samples;
    //     analyzer->getLatestSamples(samples);
    //     if (samples.empty()) {
    //         ImGui::Dummy(size);
    //         return;
    //     }
    //
    //     ImDrawList* dl = ImGui::GetWindowDrawList();
    //     ImVec2 pos = ImGui::GetCursorScreenPos();
    //
    //     // 2. Draw Background & Grid
    //     // Retro Dark Green Background
    //     dl->AddRectFilled(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(5, 12, 5, 255));
    //
    //     // Draw Grid Lines (10x10 divisions)
    //     ImU32 gridCol = IM_COL32(30, 60, 30, 120);
    //     for (int i = 1; i < 10; ++i) {
    //         // Vertical lines
    //         float x = pos.x + size.x * (i / 10.0f);
    //         dl->AddLine(ImVec2(x, pos.y), ImVec2(x, pos.y + size.y), gridCol);
    //         // Horizontal lines
    //         float y = pos.y + size.y * (i / 10.0f);
    //         dl->AddLine(ImVec2(pos.x, y), ImVec2(pos.x + size.x, y), gridCol);
    //     }
    //
    //     // Midline (Zero Cross)
    //     float mid_y = pos.y + size.y * 0.5f;
    //     dl->AddLine(ImVec2(pos.x, mid_y), ImVec2(pos.x + size.x, mid_y), IM_COL32(80, 160, 80, 180), 1.5f);
    //
    //     // 3. Plot Waveforms
    //     int num_frames = (int)samples.size() / 2;
    //     float x_step = size.x / (float)(num_frames - 1);
    //
    //     for (int i = 0; i < num_frames - 1; ++i) {
    //         float x1 = pos.x + i * x_step;
    //         float x2 = pos.x + (i + 1) * x_step;
    //
    //         // Left Channel (Cyan)
    //         float l_y1 = mid_y - (samples[i * 2] * size.y * 0.45f);
    //         float l_y2 = mid_y - (samples[(i + 1) * 2] * size.y * 0.45f);
    //         dl->AddLine(ImVec2(x1, l_y1), ImVec2(x2, l_y2), IM_COL32(0, 255, 255, 220), 1.2f);
    //
    //         // Right Channel (Yellow)
    //         float r_y1 = mid_y - (samples[i * 2 + 1] * size.y * 0.45f);
    //         float r_y2 = mid_y - (samples[(i + 1) * 2 + 1] * size.y * 0.45f);
    //         dl->AddLine(ImVec2(x1, r_y1), ImVec2(x2, r_y2), IM_COL32(255, 255, 0, 180), 1.2f);
    //     }
    //
    //     // Border
    //     dl->AddRect(pos, ImVec2(pos.x + size.x, pos.y + size.y), IM_COL32(100, 100, 100, 255));
    //
    //     // 4. Register in ImGui Layout
    //     ImGui::Dummy(size);
    // }

    // inline void DrawVisualAnalyzerOszi(ImVec2 size) {
    //     VisualAnalyzer* analyzer = this;
    //     if (!analyzer || !analyzer->isEnabled()) return;
    //
    //     static std::vector<float> samples;
    //     analyzer->getLatestSamples(samples); // Get interleaved L/R data
    //
    //     if (samples.empty()) return;
    //
    //     // Use ImGui Child Window for a contained drawing area
    //     if (ImGui::BeginChild("OsziArea", size, true, ImGuiWindowFlags_NoScrollbar)) {
    //         ImDrawList* draw_list = ImGui::GetWindowDrawList();
    //         ImVec2 canvas_pos = ImGui::GetCursorScreenPos();
    //         ImVec2 canvas_size = ImGui::GetContentRegionAvail();
    //
    //         // 1. Draw Grid (Retro Style)
    //         draw_list->AddRectFilled(canvas_pos, ImVec2(canvas_pos.x + canvas_size.x, canvas_pos.y + canvas_size.y), IM_COL32(10, 15, 10, 255));
    //
    //         float mid_y = canvas_pos.y + canvas_size.y * 0.5f;
    //         draw_list->AddLine(ImVec2(canvas_pos.x, mid_y), ImVec2(canvas_pos.x + canvas_size.x, mid_y), IM_COL32(50, 100, 50, 150));
    //
    //         // 2. Plot Channels
    //         // We iterate with i+=2 because data is [L, R, L, R...]
    //         int num_frames = (int)samples.size() / 2;
    //         float x_step = canvas_size.x / (float)num_frames;
    //
    //         for (int i = 0; i < num_frames - 1; ++i) {
    //             // Left Channel (Cyan)
    //             float l_y1 = mid_y - (samples[i * 2] * canvas_size.y * 0.45f);
    //             float l_y2 = mid_y - (samples[(i + 1) * 2] * canvas_size.y * 0.45f);
    //
    //             // Right Channel (Yellow or Red)
    //             float r_y1 = mid_y - (samples[i * 2 + 1] * canvas_size.y * 0.45f);
    //             float r_y2 = mid_y - (samples[(i + 1) * 2 + 1] * canvas_size.y * 0.45f);
    //
    //             float x1 = canvas_pos.x + i * x_step;
    //             float x2 = canvas_pos.x + (i + 1) * x_step;
    //
    //             draw_list->AddLine(ImVec2(x1, l_y1), ImVec2(x2, l_y2), IM_COL32(0, 255, 255, 200), 1.5f);
    //             draw_list->AddLine(ImVec2(x1, r_y1), ImVec2(x2, r_y2), IM_COL32(255, 255, 0, 150), 1.5f);
    //         }
    //     }
    //     ImGui::EndChild();
    // }
#endif

    }; //CLASS

} //namespace
