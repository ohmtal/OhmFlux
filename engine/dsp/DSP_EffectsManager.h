//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
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
#include <filesystem>


#include <fstream>
#include <stdexcept>

#include "DSP_Effect.h"
#include "DSP_EffectFactory.h"

#ifdef FLUX_ENGINE
#include <imgui.h>
#include <imgui_internal.h>
#include <gui/ImFlux.h>
#endif


namespace DSP {


    // for presets file
    constexpr uint16_t MAX_RACKS_IN_PRESET = 256;

    constexpr uint32_t DSP_AXE_MAGIC    =  DSP_STREAM_TOOLS::MakeMagic("AXE!");
    constexpr uint32_t DSP_AXE_VERSION  =  1;

    //for rack file
    constexpr uint16_t MAX_EFFECTS_IN_RACKS = 64;

    // i use this in OPL too !
    // constexpr uint32_t DSP_RACK_MAGIC =  DSP_STREAM_TOOLS::MakeMagic("ROCK");
    // constexpr uint32_t DSP_RACK_VERSION = 1;


//------------------------------ EffectsRack -----------------------------------
class EffectsRack {
private:
    std::string mName = "";
    std::vector<std::unique_ptr<DSP::Effect>> mEffects;
public:
    EffectsRack() = default;

    //mutable getter
    std::vector<std::unique_ptr<DSP::Effect>>& getEffects() { return mEffects; }

    const uint16_t getEffectsCount() {return (uint16_t) mEffects.size();}
    const std::string getName() {return mName;}
    void setName(std::string name) {mName = name;}

    bool add(std::unique_ptr<DSP::Effect> fx) {
        if (!fx) return false;
        if ( getEffectsCount() >= MAX_EFFECTS_IN_RACKS ){
            // addError(std::format("[error] We cant have more then {} Effects in one Rack", MAX_RACKS_IN_PRESET));
            return false;
        }
        mEffects.push_back(std::move(fx));
        return true;
    }


    std::unique_ptr<EffectsRack> clone() const {
        auto newRack = std::make_unique<EffectsRack>();
        newRack->mName = this->mName + " *";
        for (const auto& fx : mEffects) {
            newRack->mEffects.push_back(fx->clone());
        }
        return newRack;
    }

    void reorderEffect(int from, int to) {
        if (from == to || from < 0 || to < 0 ||
            from >= (int)mEffects.size() || to >= (int)mEffects.size()) {
            return;
            }
        auto itFrom = mEffects.begin() + from;
        auto itTo = mEffects.begin() + to;

        if (from < to) {
            std::rotate(itFrom, itFrom + 1, itTo + 1);
        } else {
            std::rotate(itTo, itFrom, itFrom + 1);
        }
    }

    void save(std::ostream& os) const {
        DSP_STREAM_TOOLS::write_string(os, mName);
        uint32_t count = static_cast<uint32_t>(mEffects.size());

        if (count > MAX_EFFECTS_IN_RACKS) {
            throw std::runtime_error(std::format("Too many effects in rack! count={} (max allowed: {})", count, MAX_EFFECTS_IN_RACKS));
        }

        DSP_STREAM_TOOLS::write_binary(os, count);

        for (const auto& fx : mEffects) {
            EffectType type = fx->getType();
            DSP_STREAM_TOOLS::write_binary(os, type);
            fx->save(os);
        }
    }
    bool load(std::istream& is) {
            DSP_STREAM_TOOLS::read_string(is, mName);
            uint32_t count = 0;
            DSP_STREAM_TOOLS::read_binary(is, count);
            if (count > MAX_EFFECTS_IN_RACKS) {
                throw std::runtime_error(std::format("Too many effects in rack! count={} (max allowed: {})", count, MAX_EFFECTS_IN_RACKS));
            }

            mEffects.clear();
            mEffects.reserve(count);

            for (uint32_t i = 0; i < count; ++i) {
                EffectType type;
                DSP_STREAM_TOOLS::read_binary(is, type);
                auto fx = DSP::EffectFactory::Create(type);
                if (!fx) {
                    throw std::runtime_error(std::format("Unknown Effect Type: {}", (uint32_t)type));
                }
                if (!fx->load(is)) {
                    throw std::runtime_error(std::format("Failed to load settings for {}", fx->getName()));
                }
                mEffects.push_back(std::move(fx));
            }
            return true;
    }
};

//------------------------------ EffectsManager --------------------------------
class EffectsManager {
private:
    std::vector<std::unique_ptr<EffectsRack>> mPresets;
    EffectsRack* mActiveRack = nullptr;

    bool mEnabled = true;
    std::string mErrors = "";
    // std::vector<std::unique_ptr<DSP::Effect>> mEffects; replaced by mActiveRack->mEffects
    std::recursive_mutex mEffectMutex;

    int mFrequence = 0;


public:
    //--------------------------------------------------------------------------
    std::vector<std::unique_ptr<EffectsRack>>& getPresets()  {return  mPresets; }
    EffectsRack* getActiveRack() {return  mActiveRack; }
    std::vector<std::unique_ptr<DSP::Effect>>& getActiveRackEffects() { return mActiveRack->getEffects();}

    uint16_t getPresetsCount() const { return (uint16_t)mPresets.size(); };

    //--------------------------------------------------------------------------
    EffectsManager(bool switchOn = false) {
        mEnabled = switchOn;
        mFrequence = SAMPLE_RATE_I;


        auto defaultRack = std::make_unique<EffectsRack>();
        defaultRack->setName("Rack n Roll");
        mPresets.push_back(std::move(defaultRack));
        mActiveRack = mPresets.front().get();
    }

    ~EffectsManager() {
        mEnabled = false;
        mActiveRack = nullptr;
        mPresets.clear();
    }


    //--------------------------------------------------------------------------
    void reorderEffectInActiveRack(int from, int to) {
        if (mActiveRack) {
            std::lock_guard<std::recursive_mutex> lock(mEffectMutex);
            mActiveRack->reorderEffect(from, to);

        }
    }
    //--------------------------------------------------------------------------
    // return the new rack index
    int addRack(std::string name = "empty rack" )
    {
        if (getPresetsCount() >= MAX_RACKS_IN_PRESET){
            addError(std::format("[error] We cant have more then {} racks in a Preset", MAX_RACKS_IN_PRESET));
            return -1;
        }

        auto newRack = std::make_unique<EffectsRack>();
        newRack->setName(name);
        mPresets.push_back(std::move(newRack));
        return (int)mPresets.size() - 1;
    }
    //--------------------------------------------------------------------------
    int cloneRack( int fromIndex  )
    {
        if (fromIndex < 0 || fromIndex >= (int)mPresets.size()) {
            addError(std::format("[error] Cant clone rack. index out of bounds! idx:{} max:{}", fromIndex, (int)mPresets.size()));
            return -1;
        }
        if (getPresetsCount() >= MAX_RACKS_IN_PRESET){
            addError(std::format("[error] We cant have more then {} racks in a Preset", MAX_RACKS_IN_PRESET));
            return -1;
        }

        auto newRack = mPresets[fromIndex]->clone();
        mPresets.push_back(std::move(newRack));
        return (int)mPresets.size() - 1;
    }
    //--------------------------------------------------------------------------
    int getActiveRackIndex()  {
        for (int i = 0; i < (int)mPresets.size(); ++i) {
            if (mPresets[i].get() == mActiveRack) {
                return i;
            }
        }
        addError("[error] Current Rack not found! Something is really wrong here!"); //maybe assert
        return -1; // should not happen!
    }
    //--------------------------------------------------------------------------
    int cloneCurrent() {
        int currentIndex = getActiveRackIndex();
        if (currentIndex == -1) {
            addError("[error] No active rack to clone!");
            return -1;
        }
        return cloneRack(currentIndex);
    }
    //--------------------------------------------------------------------------
    void setActiveRack(int index) {
        if (index >= 0 && index < (int)mPresets.size()) {
            std::lock_guard<std::recursive_mutex> lock(mEffectMutex);
            mActiveRack = mPresets[index].get();
        }
    }
    //--------------------------------------------------------------------------
    bool removeRack(int index) {
        if (index < 0 || index >= (int)mPresets.size()) return false;
        if (mPresets.size() <= 1) {
            addError("[warn] Cannot remove the last remaining rack.");
            return false;
        }
        int currentIndex = getActiveRackIndex();
        int nextActiveIndex = currentIndex;

        if (index == currentIndex) {
            nextActiveIndex = (index > 0) ? index - 1 : 0;
        } else if (index < currentIndex) {
            nextActiveIndex = currentIndex - 1;
        }
        std::lock_guard<std::recursive_mutex> lock(mEffectMutex);
        mActiveRack = nullptr;
        mPresets.erase(mPresets.begin() + index);
        mActiveRack = mPresets[nextActiveIndex].get();


        return true;
    }
    //--------------------------------------------------------------------------
    void reorderRack(int from, int to) {
        if (from == to || from < 0 || to < 0 ||
            from >= (int)mPresets.size() || to >= (int)mPresets.size()) return;

        auto itFrom = mPresets.begin() + from;
        auto itTo = mPresets.begin() + to;

        if (from < to) {
            std::rotate(itFrom, itFrom + 1, itTo + 1);
        } else {
            std::rotate(itTo, itFrom, itFrom + 1);
        }
    }
    //--------------------------------------------------------------------------
    void setSampleRate(float sampleRate) {
        std::lock_guard<std::recursive_mutex> lock(mEffectMutex);
        for (auto& effect : this->mActiveRack->getEffects()) {
            effect->setSampleRate(sampleRate);
        }

    }
    //--------------------------------------------------------------------------
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

    std::vector<std::unique_ptr<DSP::Effect>>& getEffects() { return mActiveRack->getEffects();    }

    void clear() { mActiveRack->getEffects().clear();}


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
        if ( mActiveRack->getEffectsCount() >= MAX_EFFECTS_IN_RACKS ){
            addError(std::format("[error] We cant have more then {} Effects in one Rack", MAX_RACKS_IN_PRESET));
            return false;
        }
        // mActiveRack->getEffects().push_back(std::move(fx));
        mActiveRack->add(std::move(fx));
        return true;
    }


    DSP::Effect* getEffectByType(DSP::EffectType type) {
        std::lock_guard<std::recursive_mutex> lock(mEffectMutex);
        for (auto& fx : mActiveRack->getEffects()) {
            if (fx->getType() == type) return fx.get();
        }
        return nullptr;
    }

    //--------------------------------------------------------------------------
    bool removeEffect( size_t effectIndex  ) {
        if (effectIndex >= mActiveRack->getEffects().size() )
        {
            addError(std::format("Remove Effect failed index out of bounds! {}", effectIndex));
            return false;
        }
        mActiveRack->getEffects().erase(mActiveRack->getEffects().begin() + effectIndex);
        return true;
    }
    //--------------------------------------------------------------------------
    // modes:0 = renderUI,  1=  renderUIWide, 2 = renderPaddle, 3 = customUI
    void renderUI(int mode = 0 ) {

#ifdef FLUX_ENGINE
        bool isEnabled = mEnabled;

        if (!isEnabled) ImGui::BeginDisabled();
        for (auto& effect : this->mActiveRack->getEffects()) {
            switch ( mode )
            {
                case 1: effect->renderUIWide();break;
                case 2: effect->renderPaddle();break;
                case 3: effect->renderCustomUI();break;
                default: effect->renderUI(); break;
            }

        }
        if (!isEnabled) ImGui::EndDisabled();
#endif
    }
    //--------------------------------------------------------------------------
    void SaveRackStream( EffectsRack* rack,   std::ostream& ofs) const {
        ofs.exceptions(std::ios::badbit | std::ios::failbit);
        DSP_STREAM_TOOLS::write_binary(ofs, DSP::DSP_RACK_MAGIC);
        DSP_STREAM_TOOLS::write_binary(ofs, uint32_t(DSP_RACK_VERSION));
        rack->save(ofs);
    }
    //--------------------------------------------------------------------------
    bool SaveActiveRack(std::string filePath) {
        if (!mActiveRack) return false;

        clearErrors();
        std::lock_guard<std::recursive_mutex> lock(mEffectMutex);
        try {
            std::ofstream ofs(filePath, std::ios::binary);
            SaveRackStream(mActiveRack, ofs);
            ofs.close();
            return true;
        } catch (const std::exception& e) {
            addError(std::format("Save failed for {}: {}", filePath, e.what()));
            return false;
        }
    }
    //--------------------------------------------------------------------------
    enum RackLoadMode {
        ReplacePresets = 0,
        AppendToPresets = 1,
        AppendToPresetsAndSetActive = 2,
        OnlyUpdateExistingSingularity = 3
    };
    bool LoadRackStream(std::istream& ifs, RackLoadMode loadMode = RackLoadMode::ReplacePresets, bool EOFcheck = true) {
        uint32_t magic = 0;
        DSP_STREAM_TOOLS::read_binary(ifs, magic);
        if (magic != DSP::DSP_RACK_MAGIC) {
            addError("LoadRackStream: Invalid File Format (Magic mismatch)");
            return false;
        }

        uint32_t version  = 0;
        DSP_STREAM_TOOLS::read_binary(ifs, version);
        if (version > DSP_RACK_VERSION) {
            addError(std::format("LoadRackStream - INVALID VERSION NUMBER:{}", version));
            return false;
        }


        auto loadedRack = std::make_unique<EffectsRack>();
        if (!loadedRack->load(ifs)) {
            addError("LoadRackStream Failed to parse Rack data.");
            return false;
        }

        ifs.exceptions(std::ifstream::badbit);
        if (EOFcheck) {
            ifs.get();
            if (!ifs.eof()) {
                addError("LoadRackStream File too long (unexpected trailing data)!");
                return false;
            }
        }
        std::lock_guard<std::recursive_mutex> lock(mEffectMutex);

        switch (loadMode) {
            case RackLoadMode::AppendToPresets:
                if (getPresetsCount() >= MAX_RACKS_IN_PRESET){
                    addError(std::format("LoadRackStream: We cant have more then {} racks in a Preset", MAX_RACKS_IN_PRESET));
                    return -1;
                }

                mPresets.push_back(std::move(loadedRack));
                break;
            case AppendToPresetsAndSetActive:
                if (getPresetsCount() >= MAX_RACKS_IN_PRESET){
                    addError(std::format("LoadRackStream: We cant have more then {} racks in a Preset", MAX_RACKS_IN_PRESET));
                    return -1;
                }

                mPresets.push_back(std::move(loadedRack));
                mActiveRack = mPresets.back().get();
                break;

            case OnlyUpdateExistingSingularity:
                mActiveRack->setName(loadedRack->getName());
                for (const auto& fx : loadedRack->getEffects()) {
                    EffectType type = fx->getType();
                    Effect* foundFX = getEffectByType(type);
                    if (foundFX) {
                        std::stringstream ss(std::ios::in | std::ios::out | std::ios::binary);
                        fx->save(ss);
                        ss.seekg(0);
                        foundFX->load(ss);
                    }
                }
                break;


            default: //ReplacePresets
                mPresets.clear();
                mPresets.push_back(std::move(loadedRack));
                mActiveRack = mPresets.front().get();
        }

        return true;
    }
    //--------------------------------------------------------------------------
    bool LoadRack(std::string filePath, RackLoadMode loadMode = RackLoadMode::ReplacePresets) {
        if (!std::filesystem::exists(filePath)) {
            addError(std::format("LoadRack: File {} not found.", filePath));
            return false;
        }
        clearErrors();
        std::ifstream ifs;
        ifs.exceptions(std::ifstream::badbit | std::ifstream::failbit);

        try {
            ifs.open(filePath, std::ios::binary);

            if (!LoadRackStream(ifs,loadMode))
                return false;

            return true;
        } catch (const std::ios_base::failure& e) {
            if (ifs.eof()) {
                addError("LoadRack: Unexpected End of File: The file is truncated.");
            } else {
                addError(std::format("LoadRack: I/O failure: {}", e.what()));
            }
            return false;
        } catch (const std::exception& e) {
            addError(std::format("LoadRack: General error: {}", e.what()));
            return false;
        }
    }
    //--------------------------------------------------------------------------
    // SAVE Presets (AXE!)
    //--------------------------------------------------------------------------
    void SavePresetsStream(std::ostream& ofs) const {
        ofs.exceptions(std::ios::badbit | std::ios::failbit);
        DSP_STREAM_TOOLS::write_binary(ofs, DSP::DSP_AXE_MAGIC);
        DSP_STREAM_TOOLS::write_binary(ofs, uint32_t(DSP_AXE_VERSION));

        int32_t presetCount = getPresetsCount();
        DSP_STREAM_TOOLS::write_binary(ofs, presetCount);

        for (int32_t rackIdx = 0; rackIdx < presetCount; rackIdx++) {
            SaveRackStream(mPresets[rackIdx].get(), ofs );
        }

    }
    //--------------------------------------------------------------------------
    bool SavePresets(std::string filePath) {
        if (getPresetsCount() < 1) return false;

        clearErrors();
        std::lock_guard<std::recursive_mutex> lock(mEffectMutex);
        try {
            std::ofstream ofs(filePath, std::ios::binary);
            SavePresetsStream(ofs);
            ofs.close();
            return true;
        } catch (const std::exception& e) {
            addError(std::format("SavePresets: Save failed for {}: {}", filePath, e.what()));
            return false;
        }
    }
    //--------------------------------------------------------------------------
    // LOAD Presets (AXE!)
    //--------------------------------------------------------------------------
    bool LoadPresetStream(std::istream& ifs) {
        uint32_t magic = 0;
        DSP_STREAM_TOOLS::read_binary(ifs, magic);
        if (magic != DSP::DSP_AXE_MAGIC) {
            addError("LoadPresetStream: Invalid File Format (Magic mismatch)");
            return false;
        }

        uint32_t version  = 0;
        DSP_STREAM_TOOLS::read_binary(ifs, version);
        if (version > DSP_AXE_VERSION) {
            addError(std::format("LoadPresetStream: Preset load - INVALID VERSION NUMBER:{}", version));
            return false;
        }

        int32_t presetCount = 0;
        DSP_STREAM_TOOLS::read_binary(ifs, presetCount);
        if ( presetCount < 1 || presetCount > MAX_EFFECTS_IN_RACKS) {
            addError(std::format("LoadPresetStream: preset count out ouf bounds: {}! max:{}", presetCount, MAX_EFFECTS_IN_RACKS));
            return false;
        }

        mPresets.clear();
        mPresets.reserve(presetCount);

        for (int32_t rackIdx = 0; rackIdx < presetCount; rackIdx++) {
            if (!LoadRackStream(ifs, RackLoadMode::AppendToPresets , false)) {
                addError(std::format("LoadPresetStream: preset load failed at rack {}! see previous error.", rackIdx));
                return false;
            }
        }

        ifs.exceptions(std::ifstream::badbit);
        ifs.get();
        if (!ifs.eof()) {
            addError("LoadPresetStream: File too long (unexpected trailing data)!");
            return false;
        }

        return true;
    }
    //--------------------------------------------------------------------------
    bool LoadPresets(std::string filePath) {
        if (!std::filesystem::exists(filePath)) {
            addError(std::format("LoadPresets: File {} not found.", filePath));
            return false;
        }
        clearErrors();
        std::ifstream ifs;
        ifs.exceptions(std::ifstream::badbit | std::ifstream::failbit);
        //--------------------------------------------------------------------------

        try {
            ifs.open(filePath, std::ios::binary);

            if (!LoadPresetStream(ifs))
                return false;

            return true;
        } catch (const std::ios_base::failure& e) {
            if (ifs.eof()) {
                addError("LoadPresets: Unexpected End of File: The file is truncated.");
            } else {
                addError(std::format("LoadPresets: I/O failure: {}", e.what()));
            }
            return false;
        } catch (const std::exception& e) {
            addError(std::format("LoadPresets: General error: {}", e.what()));
            return false;
        }
    }


    // bool scanAndLoadPresetsFromFolder(const std::string& folderPath, bool createIfMissing = true) {
    //     namespace fs = std::filesystem;
    //     if (!fs::exists(folderPath) || !fs::is_directory(folderPath)) {
    //         if ( createIfMissing ) {
    //             if (!fs::create_directories(folderPath))
    //                 addError(std::format("Failed to create preset directory:{}", folderPath));
    //         }
    //         // we have no folder
    //         return true;
    //     }
    //     for (const auto& entry : fs::directory_iterator(folderPath)) {
    //         // let's rock ;)
    //         if (entry.is_regular_file() && entry.path().extension() == ".axe") {
    //             std::string path = entry.path().string();
    //             if (LoadRack(path, RackLoadMode::AppendToPresets)) {
    //             } else {
    //                 addError(std::format("[error] Failed to auto-load: {}", path.c_str()));
    //                 return false;
    //             }
    //         }
    //     }
    //     return true;
    // }
    //--------------------------------------------------------------------------
    void process(float* buffer, int numSamples, int numChannels) {
        if (!mEnabled) return;

        for (auto& effect : this->mActiveRack->getEffects()) {
            effect->process(buffer, numSamples, numChannels);
        }

    }
    //--------------------------------------------------------------------------


}; //class Effects
}; //namespace
