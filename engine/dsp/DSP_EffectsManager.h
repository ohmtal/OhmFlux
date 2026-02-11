//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Effect Manager
//-----------------------------------------------------------------------------
#pragma once

#include <vector>
#include <cstdint>
#include <string>
#include <algorithm>
#include <array>
#include <mutex>
#include <memory>

#include <fstream>
#include <stdexcept>

#include "DSP_Effect.h"
#include "DSP_EffectFactory.h"

#ifdef FLUX_ENGINE
#include <imgui.h>
#include <imgui_internal.h>
#include <gui/ImFlux.h>
#include <utils/errorlog.h>
#endif


namespace DSP {

namespace DSP_STREAM_TOOLS {
    // i should have this in a dedicated header ... but since DSP is standalone ...
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

} ;


class EffectsManager {
private:
    bool mEnabled = true;
    std::string mErrors = "";
    std::vector<std::unique_ptr<DSP::Effect>> mEffects;
    std::recursive_mutex mEffectMutex;

    int mFrequence = 0;


public:
    //--------------------------------------------------------------------------
    EffectsManager(bool switchOn = false) { mEnabled = switchOn;mFrequence = getSampleRateI();}


    //--------------------------------------------------------------------------
    void setSampleRate(float sampleRate) {
        std::lock_guard<std::recursive_mutex> lock(mEffectMutex);
        for (auto& effect : this->mEffects) {
            effect->setSampleRate(sampleRate);
        }

    }

    void checkFrequence( int freq) {
        if ( freq != mFrequence ) {
            mFrequence = freq;
#ifdef FLUX_DEBUG
            LogFMT("[info] Change frequence to {}", freq);
#endif
            setSampleRate(static_cast<float>(mFrequence));
        }
    }


    void setEnabled(bool value) { mEnabled = value;}

    bool isEnabled() const { return mEnabled; }

    std::vector<std::unique_ptr<DSP::Effect>>& getEffects() { return mEffects;    }

    void clear() { mEffects.clear();}


    //--------------------------------------------------------------------------
    // for visual calls. Only looks for the first one.... should be ok in 99%
    DSP::SpectrumAnalyzer* getSpectrumAnalyzer() {
        auto* fx = getEffectByType(DSP::EffectType::SpectrumAnalyzer);
        if (!fx) return nullptr;

        return static_cast<DSP::SpectrumAnalyzer*>(fx);
    }
    DSP::VisualAnalyzer* getVisualAnalyzer() {
        auto* fx = getEffectByType(DSP::EffectType::VisualAnalyzer);
        if (!fx) return nullptr;

        return static_cast<DSP::VisualAnalyzer*>(fx);
    }
    DSP::Equalizer9Band* getEqualizer9Band() {
        auto* fx = getEffectByType(DSP::EffectType::Equalizer9Band);
        if (!fx) return nullptr;

        return static_cast<DSP::Equalizer9Band*>(fx);
    }


    //--------------------------------------------------------------------------
    void addError(std::string error) {
        #ifdef FLUX_ENGINE
            LogFMT("[error] " + error);
        #endif
        mErrors += mErrors + error + "\n";

    }
    std::string getErrors(bool clear = false) {
        if ( clear ) clearErrors();
        return mErrors;
    }
    void clearErrors() { mErrors = "";}
    //--------------------------------------------------------------------------
    void lock() {
        mEffectMutex.lock();
    }

    void unlock() {
        mEffectMutex.unlock();
    }
    //--------------------------------------------------------------------------
    bool addEffect(std::unique_ptr<DSP::Effect> fx) {
        std::lock_guard<std::recursive_mutex> lock(mEffectMutex);
        if (!fx) return false;
        mEffects.push_back(std::move(fx));
        return true;
    }


    DSP::Effect* getEffectByType(DSP::EffectType type) {
        std::lock_guard<std::recursive_mutex> lock(mEffectMutex);
        for (auto& fx : mEffects) {
            if (fx->getType() == type) return fx.get();
        }
        return nullptr;
    }

    //--------------------------------------------------------------------------
    bool removeEffect( size_t effectIndex  ) {
        if (effectIndex >= mEffects.size() )
        {
            addError(std::format("Remove Effect failed index out of bounds! {}", effectIndex));
            return false;
        }
        mEffects.erase(mEffects.begin() + effectIndex);
        return true;
    }
    //--------------------------------------------------------------------------
    // modes:0 = renderUI,  1=  renderUIWide, 2 = renderPaddle
    void renderUI(int mode = 0 ) {

#ifdef FLUX_ENGINE
        bool isEnabled = mEnabled;

        if (!isEnabled) ImGui::BeginDisabled();
        for (auto& effect : this->mEffects) {
            switch ( mode )
            {
                case 1: effect->renderUIWide();break;
                case 2: effect->renderPaddle();break;
                default: effect->renderUI(); break;
            }

        }
        if (!isEnabled) ImGui::EndDisabled();
#endif
    }
    //--------------------------------------------------------------------------
    bool Load(std::string filePath) {
        clearErrors();
        std::ifstream ifs;
        ifs.exceptions(std::ifstream::badbit | std::ifstream::failbit);
        try {
            ifs.open(filePath, std::ios::binary);

            uint32_t magic = 0;
            ifs.read(reinterpret_cast<char*>(&magic), sizeof(magic));

            if (magic != DSP::DSP_MAGIC) {
                addError("Invalid File Format (Magic mismatch)");
                return false;
            }

            uint32_t count = 0;
            ifs.read(reinterpret_cast<char*>(&count), sizeof(count));

            this->clear();

            std::lock_guard<std::recursive_mutex> lock(mEffectMutex);
            for (uint32_t i = 0; i < count; ++i) {
                DSP::EffectType type;
                ifs.read(reinterpret_cast<char*>(&type), sizeof(type));

                std::unique_ptr<DSP::Effect> fx = DSP::EffectFactory::Create(type);

                if (fx) {
                    if (!fx->load(ifs)) {
                        addError(std::format("Failed to load settings for effect type: {}", (uint32_t)type));
                        return false;
                    }
                    mEffects.push_back(std::move(fx));
                } else {
                    addError(std::format("Unknown Effect type: {}. Loading aborted to prevent corruption.", (uint32_t)type));
                    return false;
                }
            }
            ifs.exceptions(std::ifstream::badbit);
            ifs.clear();

            ifs.get();
            if (!ifs.eof()) {
                addError("File too long (unexpected trailing data)!");
                return false;
            }

            return true;

        } catch (const std::ios_base::failure& e) {
            if (ifs.eof()) {
                addError("Unexpected End of File: The file is truncated.");
            } else {
                addError(std::format("I/O failure: {}", e.what()));
            }
            return false;
        } catch (const std::exception& e) {
            addError(std::format("General error: {}", e.what()));
            return false;
        }
    }
    //--------------------------------------------------------------------------
    bool Save(std::string filePath) {
        clearErrors();
        std::ofstream ofs; // (filePath, std::ios::binary);
        ofs.exceptions(std::ofstream::badbit | std::ofstream::failbit);
        try {
            ofs.open(filePath, std::ios::binary);

            if (!ofs.is_open()) {
                addError(std::format("Can't open File {} for write.", filePath));
                return false;
            }
            DSP_STREAM_TOOLS::write_binary(ofs, DSP::DSP_MAGIC);
            uint32_t count = static_cast<uint32_t>(mEffects.size());
            std::lock_guard<std::recursive_mutex> lock(mEffectMutex);
            DSP_STREAM_TOOLS::write_binary(ofs, count);
            for (const auto& fx : mEffects) {
                DSP::EffectType type = fx->getType();
                DSP_STREAM_TOOLS::write_binary(ofs, type);
                fx->save(ofs);
            }
            ofs.close();
            return true;
        } catch (const std::ios_base::failure& e) {
            // Detailed system error (e.g., "No space left on device")
            addError(std::format("Disk I/O Error: {}", e.what()));
            return false;
        } catch (const std::exception& e) {
            addError(std::format("General write exception: {}", e.what()));
            return false;
        }

        return false;
    }
    //--------------------------------------------------------------------------
    void process(float* buffer, int numSamples, int numChannels) {
        if (!mEnabled) return;

        // int i = 0;
        for (auto& effect : this->mEffects) {
            // i++; dLog("processing %d => %s",i, effect->getName().c_str());
            effect->process(buffer, numSamples, numChannels);
        }

    }
    //--------------------------------------------------------------------------

}; //class Effects
}; //namespace
