//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Main include
//-----------------------------------------------------------------------------
#pragma once

#include <memory>
#include <atomic>
#include <fstream>


#ifdef FLUX_ENGINE
#include <imgui.h>
#include <imgui_internal.h>
#include <gui/ImFlux.h>
#endif

namespace DSP {
    //-----------------------------------------------------------------------------
    // lazy usage inside: auto writeVal = [&](const auto& atomicVal) {...}
    inline void writeAtomicVal(std::ostream& os,  const auto& atomicVal) {
        auto v = atomicVal.load();
        os.write(reinterpret_cast<const char*>(&v), sizeof(v));
    };
    // lazy usage inside: auto readVal = [&](auto& atomicDest) {...}
    inline void readAtomicVal(std::istream& is, auto& atomicDest) {
        typename std::remove_reference_t<decltype(atomicDest)>::value_type temp;
        is.read(reinterpret_cast<char*>(&temp), sizeof(temp));
        atomicDest.store(temp);
    };
    //-----------------------------------------------------------------------------
    #ifdef FLUX_ENGINE

    inline ImFlux::KnobSettings ksBlack = {.radius=25.f, .active=IM_COL32(200,200,0,255)   };
    inline ImFlux::KnobSettings ksRed   = {.radius=25.f, .bg_outer = IM_COL32(45, 5, 4, 255), .bg_inner = IM_COL32(65, 5, 4, 255),.active=IM_COL32(200,200,0,255)  };
    inline ImFlux::KnobSettings ksBlue  = {.radius=25.f, .bg_outer = IM_COL32(4, 5, 45, 255), .bg_inner = IM_COL32(4, 5, 65, 255),.active=IM_COL32(200,200,0,255)  };
    inline ImFlux::KnobSettings ksGreen = {.radius=25.f, .bg_outer = IM_COL32(4, 45, 5, 255), .bg_inner = IM_COL32(4, 65, 5, 255),.active=IM_COL32(200,200,0,255)  };


    inline bool rackKnob( const char* caption, float* value,
                         ImVec2 minMax, ImFlux::KnobSettings ks,
                         ImFlux::GradientParams gp = ImFlux::DEFAULT_GRADIENPARAMS
                        )
    {
        ImFlux::GradientBox(ImVec2(70.f,80.f),gp);
        ImGui::BeginGroup();
        ImFlux::ShadowText(caption, IM_COL32(132,132,132,255));
        ImGui::Dummy(ImVec2(5.f, 5.f)); ImGui::SameLine();
        bool result = ImFlux::LEDMiniKnob(caption, value, minMax.x, minMax.y, ks);
        ImGui::EndGroup();
        return result;
    };

    inline void paddleHeader(const char* caption, ImU32 baseColor, bool& enabled,
                      ImFlux::LedParams lp = ImFlux::LED_GREEN_ANIMATED_GLOW,
                      ImFlux::GradientParams gp = ImFlux::DEFAULT_GRADIENPARAMS
    ) {



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
        if (ImGui::IsWindowHovered() && ImGui::IsMouseHoveringRect(min, max)) {
            if (ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
                enabled = !enabled;
            }
        }

        ImFlux::GradientBox(ImVec2(0.f,0.f),gp);

    };

#endif

}
