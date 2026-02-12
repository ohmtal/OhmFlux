//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
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
    class IParameter;
    template <typename T> class AudioParam;

    namespace DSP_STREAM_TOOLS {
        constexpr uint16_t  MAX_VECTOR_ELEMENTS = 1024;
        constexpr uint16_t  MAX_STRING_LENGTH = 1024;

        // i should have this in a dedicated header ... but since DSP is standalone ...


        //---------
        //--------- standard read write
        template <typename T>
        auto write_binary(std::ostream& os, const T& value)
        -> std::enable_if_t<!std::is_base_of_v<IParameter, T>>
        {
            static_assert(std::is_trivially_copyable_v<T>, "Type must be trivially copyable");
            os.write(reinterpret_cast<const char*>(&value), sizeof(T));
        }
        //---------
        template <typename T>
        auto read_binary(std::istream& is, T& value)
        -> std::enable_if_t<!std::is_base_of_v<IParameter, T>>
        {
            static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable for binary read");
            is.read(reinterpret_cast<char*>(&value), sizeof(T));
            if (!is) throw std::runtime_error("Binary read failed");
        }

        //---------
        // Overload AudioParam

        template <typename T>
        void write_binary(std::ostream& os, const AudioParam<T>& param) {
            T val = param.get();
            os.write(reinterpret_cast<const char*>(&val), sizeof(T));
        }

        // Overload AudioParam
        template <typename T>
        void read_binary(std::istream& is, AudioParam<T>& param) {
            T val;
            is.read(reinterpret_cast<char*>(&val), sizeof(T));
            if (!is) throw std::runtime_error("Binary read failed");
            param.set(val);
        }

        //---------


        inline void write_string(std::ostream& os, const std::string& str) {
            uint32_t size = static_cast<uint32_t>(str.size());
            if (size > MAX_STRING_LENGTH) {
                size = static_cast<uint32_t>(MAX_STRING_LENGTH);
            }
            write_binary(os, size);
            if (size > 0) {
                os.write(str.data(), size);
            }
        }
        //---------
        inline void read_string(std::istream& is, std::string& str) {
            uint32_t size = 0;
            read_binary(is, size);
            if (size > MAX_STRING_LENGTH) {
                throw std::runtime_error(std::format("String length {} exceeds limit of {}", size, MAX_STRING_LENGTH));
            }
            str.resize(size);
            if (size > 0) {
                is.read(str.data(), size);
            }
        }
        //---------
        // Helper to write std::vector of POD/Trivial types
        template <typename T>
        void write_vector(std::ostream& os, const std::vector<T>& vec) {
            static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable for binary I/O");
            uint32_t size = static_cast<uint32_t>(vec.size());
            if (size > MAX_VECTOR_ELEMENTS) {
                throw std::runtime_error(std::format("Vector size exceeds limit: read {} (max allowed: {})",
                                                     size, MAX_VECTOR_ELEMENTS));
            }
            write_binary(os, size);
            if (size > 0) {
                os.write(reinterpret_cast<const char*>(vec.data()), static_cast<size_t>(size) * sizeof(T));
            }
        }
        //---------

        template <typename T>
        void read_vector(std::istream& is, std::vector<T>& vec) {
            static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable for binary I/O");

            uint32_t size = 0;
            read_binary(is, size);
            if (size > MAX_VECTOR_ELEMENTS) {
                throw std::runtime_error(std::format("Vector size {} in file exceeds limit of {}",
                                                     size, MAX_VECTOR_ELEMENTS));
            }
            vec.resize(size);
            if (size > 0) {
                is.read(reinterpret_cast<char*>(vec.data()), static_cast<std::streamsize>(size) * sizeof(T));
            }
        }


        //----------------------------------------------------------------------
        // ~~~~~~~~~~~~ ofstream versions:  ~~~~~~~~~~~~~~~~~~
        template <typename T>
        void write_binary(std::ofstream& ofs, const T& value) {
            static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable for binary write");
            ofs.write(reinterpret_cast<const char*>(&value), sizeof(T));
        }

        template <typename T>
        void read_binary(std::ifstream& ifs, T& value) {
            static_assert(std::is_trivially_copyable_v<T>, "T must be trivially copyable for binary read");
            ifs.read(reinterpret_cast<char*>(&value), sizeof(T));
        }


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
    } ; // namespace DSP_STREAM_TOOLS

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
        if (ImFlux::DrawLED("on/off",enabled, lp)) enabled = !enabled;
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
