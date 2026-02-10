//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Main include
//-----------------------------------------------------------------------------
#pragma once

#include <memory>

#ifdef FLUX_ENGINE
#include <imgui.h>
#include <imgui_internal.h>
#include <gui/ImFlux.h>
#endif



namespace DSP {


    //-----------------------------------------------------------------------------
    // HELPER FUNCTION IF YOU NEED TO ADD A EFFECT TO A SHADOW POINTER:
    // Example:  mDSPBitCrusher = addEffectToChain<DSP::Bitcrusher>(mDspEffects, false);
    template<typename T>
    T* addEffectToChain(std::vector<std::unique_ptr<DSP::Effect>>& chain, bool enabled = false) {
        auto fx = std::make_unique<T>(enabled);
        T* ptr = fx.get();
        chain.push_back(std::move(fx));
        return ptr;
    }
    //-----------------------------------------------------------------------------
    // normalize a stream
    inline void normalizeBuffer(float* buffer, size_t count, float targetPeak = 1.0f) {
        float currentPeak = 0.0f;

        // Pass 1: Find the absolute maximum peak
        for (size_t i = 0; i < count; ++i) {
            float absValue = std::abs(buffer[i]);
            if (absValue > currentPeak) {
                currentPeak = absValue;
            }
        }

        // Pass 2: Scale all samples (only if the buffer isn't silent)
        if (currentPeak > 0.0f) {
            float factor = targetPeak / currentPeak;
            for (size_t i = 0; i < count; ++i) {
                buffer[i] *= factor;
            }
        }
    }
    //-----------------------------------------------------------------------------
    #ifdef FLUX_ENGINE

    void rackKnobAtomic( const char* caption, auto& atomicVal,
                         ImVec2 minMax, ImFlux::KnobSettings ks,
                         ImFlux::GradientParams gp = ImFlux::DEFAULT_GRADIENPARAMS
                        )
    {
        auto v = atomicVal.load();

        ImFlux::GradientBox(ImVec2(70.f,80.f),gp);
        ImGui::Dummy(ImVec2(5.f, 5.f)); ImGui::SameLine();
        ImGui::BeginGroup();
        ImFlux::ShadowText(caption, IM_COL32(132,132,132,255));
        if ( ImFlux::LEDMiniKnob(caption, &v, minMax.x, minMax.y, ks) ) {
            atomicVal = v;
        }
        ImGui::EndGroup();
        // ImGui::SameLine();ImGui::Dummy(ImVec2(2.f, 5.f));

    };

    void paddleHeader(const char* caption, ImU32 baseColor, auto& atomicEnabledVal,
                      ImFlux::LedParams lp = ImFlux::LED_GREEN_ANIMATED_GLOW,
                      ImFlux::GradientParams gp = ImFlux::DEFAULT_GRADIENPARAMS



    ) {
        auto enabled = atomicEnabledVal.load();


        // FOR HEADER CLICK :
        ImVec2 min = ImGui::GetCursorScreenPos();
        ImVec2 max = ImVec2(min.x + ImGui::GetContentRegionAvail().x, min.y + 50.f);

        ImFlux::GradientParams gpHeader = gp;
        gpHeader.col_top = baseColor;
        gpHeader.col_bot = ImFlux::ModifyRGB(baseColor, 0.8);
        ImFlux::GradientBox(ImVec2(0.f,50.f),gpHeader);
        ImGui::BeginGroup();
        ImGui::Dummy(ImVec2(0,8));ImGui::Dummy(ImVec2(4,0)); ImGui::SameLine();
        ImFlux::DrawLED("on/off",enabled, lp);
        ImGui::SameLine();
        ImGui::SetWindowFontScale(2.f);
        ImFlux::ShadowText(caption);
        ImGui::SetWindowFontScale(1.f);
        ImGui::Dummy(ImVec2(0,8));
        ImGui::EndGroup();

        // click magic:
        if (ImGui::IsMouseHoveringRect(min, max) && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
            atomicEnabledVal = !atomicEnabledVal;
        }

        ImFlux::GradientBox(ImVec2(0.f,0.f),gp);

    };

#endif

}
