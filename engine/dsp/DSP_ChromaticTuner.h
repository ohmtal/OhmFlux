//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Chromatic Tuner
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
    struct ChromaticTunerData {
        int channel = 0;
        bool    fastMode = false;
    };

    struct ChromaticTunerSettings : public ISettings {
        AudioParam<int> channel { "Channel", 0, 0,  8, "%d" };
        AudioParam<bool> fastMode { "Fast mode", 0, 0, 1, "%d" };

        ChromaticTunerSettings () = default;
        REGISTER_SETTINGS(ChromaticTunerSettings , &channel, &fastMode)

        ChromaticTunerData getData() const {
            return { channel.get(), fastMode.get()};
        }

        void setData(const ChromaticTunerData& data) {
            channel.set(data.channel);
            fastMode.set(data.fastMode);
        }
    };

    static const char* NOTE_NAMES[] = { "C", "C#", "D", "D#", "E", "F", "F#", "G", "G#", "A", "A#", "B" };


    class ChromaticTuner : public DSP::Effect {
    private:
        ChromaticTunerSettings mSettings;
        uint8_t mChannelCount = 2;


        static const int BUFFER_SIZE = 4096;
        float mBuffer[BUFFER_SIZE] = {0};
        int mWritePos = 0;
        int analysis_counter = 0;

        float mTunerFreq = 0.0f;
        int   mTunerThreshold = 2000;


    public:
        IMPLEMENT_EFF_CLONE(ChromaticTuner)

        ChromaticTuner(bool switchOn = false) :
            DSP::Effect(DSP::EffectType::ChromaticTuner, switchOn),
            mSettings() {}
        //----------------------------------------------------------------------
        virtual std::string getName() const override { return "Chromatic Tuner"; }
        //----------------------------------------------------------------------
        void setSettings(const ChromaticTunerSettings& s) {mSettings = s;}
        //----------------------------------------------------------------------
        ChromaticTunerSettings& getSettings() { return mSettings; }
        //----------------------------------------------------------------------
        void save(std::ostream& os) const override {
            Effect::save(os);              // Save mEnabled
            mSettings.save(os);       // Save Settings
        }
        //----------------------------------------------------------------------
        bool load(std::istream& is) override {
            if (!Effect::load(is)) return false; // Load mEnabled
            return mSettings.load(is);      // Load Settings
        }
        //----------------------------------------------------------------------
        float getFrequence() {return mTunerFreq;}
        //----------------------------------------------------------------------
        bool fetchNoteAndCents(int& note_index, float& cents) {
            if (mTunerFreq == 0.f) return false;
            float n = 12.0f * std::log2(mTunerFreq / 440.0f) + 69.0f;
            note_index = (int)std::round(n);
            cents = (n - note_index) * 100.0f;
            return true;
        }
        //----------------------------------------------------------------------
        void analyzeAccurate() {
            analysis_counter++;
            if (analysis_counter < mTunerThreshold) return;
            analysis_counter = 0;

            const int window_size = 1024;
            const int buffer_size = 4096;

            // 1. RMS Gate
            float rms = 0;
            for (int i = 0; i < window_size; ++i) {
                float s = mBuffer[(mWritePos - i + buffer_size) % buffer_size];
                rms += s * s;
            }
            if (rms < 0.0005f) { mTunerFreq = 0.0f; return; }

            // 2. Difference Function (AMDF - Average Magnitude Difference Function)
            // This is much sharper for high frequencies than correlation.
            auto get_diff = [&](int lag) {
                float diff = 0;
                for (int i = 0; i < window_size; ++i) {
                    float s1 = mBuffer[(mWritePos - i + buffer_size) % buffer_size];
                    float s2 = mBuffer[(mWritePos - i - lag + buffer_size) % buffer_size];
                    // Squared difference instead of multiplication
                    float d = s1 - s2;
                    diff += d * d;
                }
                return diff;
            };

            int min_lag =  (int)( mSampleRate / 1000.f );
            int max_lag = (int)(mSampleRate / 50.f);
            float min_diff = 1e10f;
            int best_lag = -1;

            // 3. Search for the absolute Minimum (instead of Maximum)
            for (int lag = min_lag; lag <= max_lag; ++lag) {
                float diff = get_diff(lag);
                if (diff < min_diff) {
                    min_diff = diff;
                    best_lag = lag;
                }
            }

            if (best_lag > min_lag && best_lag < max_lag) {
                // 4. Refinement with parabolic interpolation on the "Valley"
                float v1 = get_diff(best_lag - 1);
                float v2 = min_diff;
                float v3 = get_diff(best_lag + 1);

                float denom = v1 - 2.0f * v2 + v3;
                float offset = 0.0f;
                if (std::abs(denom) > 1e-6f) {
                    offset = (v1 - v3) / (2.0f * denom);
                }

                float raw_freq = mSampleRate / (static_cast<float>(best_lag) + offset);

                // 5. Intelligent Smoothing
                // Use a smaller alpha for a more stable reading
                float alpha = 0.15f;
                if (std::abs(mTunerFreq - raw_freq) > 50.0f) alpha = 1.0f; // Fast jump if note changes

                mTunerFreq = mTunerFreq * (1.0f - alpha) + raw_freq * alpha;
            }
        }
        //----------------------------------------------------------------------
        void analyzeFast() {
            // Perform analysis roughly 20 times per second
            analysis_counter++;
            if (analysis_counter < mTunerThreshold) return;
            analysis_counter = 0;

            int min_lag = (int)(mSampleRate / 1000.f); // 1000 Hz
            int max_lag = (int)(mSampleRate / 50.f);   // 50 Hz

            float best_corr = -1e10f;
            int best_lag = -1;
            const int window_size = 1024;

            float rms = 0;
            for(int i=0; i<window_size; ++i) {
                float s = mBuffer[(mWritePos - i + 4096) % 4096];
                rms += s*s;
            }
            if (rms < 0.001f) {
                mTunerFreq = 0.0f;
                return;
            }

            for (int lag = min_lag; lag <= max_lag; ++lag) {
                float corr = 0;
                for (int i = 0; i < window_size; ++i) {
                    float s1 = mBuffer[(mWritePos - i + 4096) % 4096];
                    float s2 = mBuffer[(mWritePos - i - lag + 4096) % 4096];
                    corr += s1 * s2;
                }
                if (corr > best_corr) {
                    best_corr = corr;
                    best_lag = lag;
                }
            }
            if (best_lag > 0) {
                mTunerFreq = mSampleRate / (float)best_lag;
            }
        }
        //----------------------------------------------------------------------
        virtual void process(float* buffer, int numSamples, int numChannels) override {
            if (!isEnabled()) return;
            mChannelCount = numChannels;

            int channel = mSettings.channel.get();
            bool fastMode = mSettings.fastMode.get();


            if ( channel > numChannels ) channel = 0; //reset to zero
            for (int i = 0; i < numSamples; i++) {
                int ch = i % numChannels;


                if (ch == channel) {
                    // --- add a sample ---
                    mBuffer[mWritePos] = buffer[i];
                    mWritePos = (mWritePos + 1) % BUFFER_SIZE;
                    // analyse
                    if ( fastMode ) analyzeFast();
                    else analyzeAccurate();
                }
            }
        }
        //----------------------------------------------------------------------
        #ifdef FLUX_ENGINE
        virtual ImVec4 getColor() const  override { return  ImVec4(0.5f, 0.7f, 0.7f, 1.0f);}

        virtual void renderPaddle() override {
            ImGui::PushID("ChromaticTuner_Effect_PADDLE");
            paddleHeader(getName().c_str(), ImGui::ColorConvertFloat4ToU32(getColor()), mEnabled);
            ChromaticTunerSettings currentSettings = this->getSettings();
            bool changed = false;
            renderFancyTuner();
            if (changed) this->setSettings(currentSettings);
            ImGui::PopID();
        }

        virtual void renderUIWide() override {
            ImGui::PushID("ChromaticTuner_Effect_Row_WIDE");
            if (ImGui::BeginChild("ChromaticTuner_W_BOX", ImVec2(-FLT_MIN,65.f) )) {

                ImFlux::GradientBox(ImVec2(-FLT_MIN, -FLT_MIN),0.f);
                ImGui::Dummy(ImVec2(2,0)); ImGui::SameLine();
                ImGui::BeginGroup();
                bool isEnabled = this->isEnabled();
                if (ImFlux::LEDCheckBox(getName(), &isEnabled, getColor())){
                    this->setEnabled(isEnabled);
                }
                ImGui::Separator();

                renderBasicTuner();

                // bool changed = false;
                // ChromaticTunerSettings currentSettings = this->getSettings();
                // if (!isEnabled) ImGui::BeginDisabled();
                //
                //
                // // Engine Update
                // if (changed) {
                //     if (isEnabled) {
                //         this->setSettings(currentSettings);
                //     }
                // }
                //
                // if (!isEnabled) ImGui::EndDisabled();
                ImGui::EndGroup();
            }
            ImGui::EndChild();
            ImGui::PopID();

        }

        virtual void renderUI() override {
            ImGui::PushID("ChromaticTuner_Effect");
            ChromaticTunerSettings currentSettings = this->getSettings();
            bool changed = false;
            bool enabled = this->isEnabled();

            if (ImFlux::LEDCheckBox(getName(), &enabled, getColor())) setEnabled(enabled);

            if (enabled) {
                if (ImGui::BeginChild("ChromaticTuner_BOX", ImVec2(-FLT_MIN, 55.f), ImGuiChildFlags_Borders)) {
                    // changed |= ImFlux::LEDCheckBox("Fast mode", &currentSettings.fastMode, ImVec4(0.4f,0.4f,0.8f,1.f));
                    // ImGui::SameLine();
                    // changed |= ImFlux::MiniKnobInt("Channel", &currentSettings.channel, 0, mChannelCount -1);

                    renderBasicTuner();

                    // if (changed) setSettings(currentSettings);
                }
                ImGui::EndChild();
            } else {
                ImGui::Separator();
            }
            ImGui::PopID();
        }


        void renderBasicTuner(int ledCount = 48) {
            float cents = 0.f; int note_index = 0;
            bool haveSignal = fetchNoteAndCents(note_index, cents);
            ImU32 led_col = IM_COL32(16, 16, 16, 255);
            if (haveSignal) led_col = std::abs(cents) < 3.0f ? ImFlux::COL32_NEON_GREEN : ImFlux::COL32_NEON_RED;

            ImGui::SetWindowFontScale(2.8f);
            ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.4f, 1.0f), "%2s", haveSignal ?  NOTE_NAMES[note_index % 12] : "--");
            ImGui::SetWindowFontScale(1.5f);
            ImGui::SameLine();
            if (haveSignal) ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.3f, 1.0f), "%d", (note_index / 12) - 1);
            else ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.3f, 1.0f), "-");
            ImGui::SetWindowFontScale(1.0f);
            ImGui::SameLine();
            ImFlux::LEDNeedleBar((cents + 50.0f) / 100.0f, ledCount, ImVec2(5.f,32.f), led_col,IM_COL32(16, 16, 16, 255), 0.5f );
        }

//         void renderFancyTuner(ImVec2 size) {
//             float freq = 0.f;
//             float cents = 0.f; int note_idx = 0;
//             bool haveSignal = fetchNoteAndCents(note_idx, cents);
//
//             {
//                 ImDrawList* draw_list = ImGui::GetWindowDrawList();
//                 ImVec2 cursor_pos = ImGui::GetCursorScreenPos();
//
//                 // If width (size.x) is 0, use the full available width of the window
//                 float avail_width = ImGui::GetContentRegionAvail().x;
//                 float final_width = (size.x <= 0.0f) ? avail_width : size.x;
//                 float final_height = size.y;
//
//                 ImFlux::GradientBoxDL(ImFlux::DEFAULT_GRADIENPARAMS, draw_list);
//
//                 // --- 2. Dynamic Centering ---
//                 // Base all drawing on this center point
//                 ImVec2 center = ImVec2(cursor_pos.x + final_width * 0.5f, cursor_pos.y + final_height * 0.85f);
//                 float radius = final_height * 0.75f;
//
//                 // Colors
//                 ImU32 col_white = ImGui::GetColorU32(ImGuiCol_Text);
//                 ImU32 col_accent = ImGui::GetColorU32(ImGuiCol_PlotLinesHovered);
//                 ImU32 col_ok = IM_COL32(0, 255, 0, 255);
//                 ImU32 needle_col = (haveSignal && fabsf(cents) < 5.0f) ? IM_COL32(0, 255, 0, 255) : col_accent;
//
//                 // LED's
//
//
//
//                 int ledCount = 30;
//
//
//
//                 ImU32 led_col = IM_COL32(60,0,0,255);
//                 ImVec2 ledSquare = { 10.f, 10.f};
//                 float spacing = 2.f;
//                 ImVec2 p_min;
//                 ImVec2 p_max;
//
//                 for(int i=0; i<=ledCount; ++i) {
//                     p_min = ImVec2(cursor_pos.x + i * (ledSquare.x + spacing), cursor_pos.y);
//                     p_max = ImVec2(p_min.x + ledSquare.x, p_min.y + ledSquare.y);
//
//
//                     if (isLit) {
//                         draw_list->AddRectFilled(p_min, p_max, led_col, 1.0f);
//                         // Add a small "inner light" highlight to make it look like a physical LED
//                         draw_list->AddLine(p_min, ImVec2(p_max.x, p_min.y), IM_COL32(255, 255, 255, 100));
//                     } else {
//                         draw_list->AddRectFilled(p_min, p_max, led_col, 1.0f);
//                     }
//
//                 }
//
//
//
//                 // --- Text Centering ---
//                 char buf_note[8];
//                 char buf_oct[4];
//                 snprintf(buf_note, sizeof(buf_note), "%s", haveSignal ? NOTE_NAMES[note_idx % 12] : "--");
//                 snprintf(buf_oct, sizeof(buf_oct), "%d", haveSignal ? (note_idx / 12) - 1 : 0);
//                 ImGui::SetWindowFontScale(3.0f);
//                 ImVec2 size_note = ImGui::CalcTextSize(buf_note);
//                 ImGui::SetWindowFontScale(2.0f);
//                 ImVec2 size_oct = ImGui::CalcTextSize(buf_oct);
//                 float spacing = 8.0f; // Gap between Note and Octave
//                 float total_block_width = size_note.x + spacing + size_oct.x;
//                 float start_x = center.x - (total_block_width * 0.5f);
//                 float text_y = cursor_pos.y + 15.f;
//                 ImGui::SetWindowFontScale(3.0f);
//                 draw_list->AddText(ImVec2(start_x, text_y), needle_col, buf_note);
//                 ImGui::SetWindowFontScale(2.0f);
//                 draw_list->AddText(ImVec2(start_x + size_note.x + spacing, text_y ), needle_col, buf_oct);
//                 ImGui::SetWindowFontScale(1.0f);
//
//
//
//                 char buf_info[64];
//                 snprintf(buf_info, sizeof(buf_info), "FREQ: %.2f Hz   CENTS: %+.1f", freq, cents);
//                 draw_list->AddText(ImVec2(cursor_pos.x, cursor_pos.y + final_height  - 15.f), col_accent, buf_info);
//
//
//                 // draw_list->AddText(ImVec2(start_x + size_note.x + spacing, text_y , "FREQ: %.2f Hz   CENTS: %+.1f", freq, cents);
//
//                 // Essential: Advancing the cursor so ImGui knows we used this space
//                 ImGui::Dummy(ImVec2(final_width, final_height));
//             }
//         }


        void renderFancyTuner( ){
                if ( !isEnabled() ) {
                    ImGui::SetWindowFontScale(2.f);
                    ImGui::TextColored(ImVec4(0.2f, 0.2f, 0.2f, 1.0f), "--- DISABLED ---");
                    ImGui::SetWindowFontScale(1.0f);
                    ImGui::Dummy(ImVec2(0.f, 89.f)); //96??
                    return;
                }

                float freq = 0.f;
                float cents = 0.f; int note_idx = 0;
                if (fetchNoteAndCents( note_idx, cents )) {
                    freq = mTunerFreq;
                    ImDrawList* draw_list = ImGui::GetWindowDrawList();
                    ImVec2 region = ImGui::GetContentRegionAvail();

                    const char* current_note = NOTE_NAMES[note_idx % 12];
                    int octave = (note_idx / 12) - 1;

                    // Large Note Display
                    ImGui::SetWindowFontScale(4.0f);
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.4f, 1.0f), "%s", current_note);
                    ImGui::SetWindowFontScale(1.5f);
                    ImGui::SameLine();
                    ImGui::TextColored(ImVec4(0.0f, 0.8f, 0.3f, 1.0f), "%d", octave);
                    ImGui::SetWindowFontScale(1.0f);

                    // LCD Meter
                    // ImVec2 c_pos = ImGui::GetCursorScreenPos();
                    ImVec2 m_pos = ImGui::GetCursorScreenPos();
                    m_pos.y += 10;
                    float m_w = region.x;
                    if (m_w <= 0.0f) return;
                    float m_h = 30;

                    draw_list->AddRectFilled(m_pos, ImVec2(m_pos.x + m_w, m_pos.y + m_h), IM_COL32(20, 40, 20, 255));

                    // Scale marks
                    for(int i=0; i<=10; ++i) {
                        float x = m_pos.x + (m_w * i / 10.0f);
                        draw_list->AddLine(ImVec2(x, m_pos.y), ImVec2(x, m_pos.y + (i%5==0 ? 10 : 5)), IM_COL32(0, 200, 0, 100));
                    }

                    float center_x = m_pos.x + m_w * 0.5f;
                    float indicator_x = center_x + (cents / 50.0f) * (m_w * 0.5f);
                    indicator_x = std::clamp(indicator_x, m_pos.x, m_pos.x + m_w);

                    ImU32 color = std::abs(cents) < 3.0f ? IM_COL32(0, 255, 0, 255) : IM_COL32(255, 50, 0, 255);
                    draw_list->AddRectFilled(ImVec2(indicator_x - 3, m_pos.y - 2), ImVec2(indicator_x + 3, m_pos.y + m_h + 2), color);
                    draw_list->AddLine(ImVec2(center_x, m_pos.y - 5), ImVec2(center_x, m_pos.y + m_h + 5), IM_COL32(255, 255, 255, 255), 2.0f);

                    ImGui::Dummy(ImVec2(m_w, m_h + 15));
                    ImGui::TextColored(ImVec4(0.0f, 1.0f, 0.0f, 0.7f), "FREQ: %.2f Hz   CENTS: %+.1f", freq, cents);
                    // ImGui::TextDisabled("HEIGHT :%f", ImGui::GetCursorScreenPos().y - c_pos.y);;

                } else {
                    // ImVec2 m_pos = ImGui::GetCursorScreenPos();
                    ImGui::SetWindowFontScale(2.f);
                    ImGui::TextColored(ImVec4(0.2f, 0.2f, 0.2f, 1.0f), "--- NO SIGNAL ---");
                    ImGui::SetWindowFontScale(1.0f);
                    // ImGui::TextDisabled("HEIGHT :%f", ImGui::GetCursorScreenPos().y - m_pos.y);;
                    ImGui::Dummy(ImVec2(0.f, 89.f)); //96??


                }
        }

        #endif
    }; //CLASS!


} // namespace
