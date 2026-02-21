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
#include <utils/errorlog.h>
#endif


namespace DSP {


    // for presets file
    constexpr uint16_t MAX_RACKS_IN_PRESET = 256;

    constexpr uint32_t DSP_AXE_MAGIC    =  DSP_STREAM_TOOLS::MakeMagic("AXE!");
    constexpr uint32_t DSP_AXE_VERSION  =  3;

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
    void setName(std::string name) {   mName = name.substr(0,255);}  //LIMIT 255 chars
    // void setName(std::string name) {mName = name;}

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
    int mSwitchRack = -1; //VERSION 2
    std::string mName = ""; //VERSION 3


    bool mEnabled = true;
    std::string mErrors = "";
    std::recursive_mutex mEffectMutex;

    int mFrequence = 0;


public:
    //--------------------------------------------------------------------------
    const std::string getName() {return mName;}
    // no long stories :P limited to 64 chars!
    void setName(std::string name) {   mName = name.substr(0,255);}
    //--------------------------------------------------------------------------
    std::vector<std::unique_ptr<EffectsRack>>& getPresets()  {return  mPresets; }


    EffectsRack* getActiveRack() {return  mActiveRack; }



    uint16_t getPresetsCount() const { return (uint16_t)mPresets.size(); };

    EffectsRack* getRackByIndex(uint16_t idx) {
        if (idx > getPresetsCount()) return nullptr;
        return mPresets[idx].get();
    }


    //--------------------------------------------------------------------------
    EffectsManager(bool switchOn = false) {
        mEnabled = switchOn;
        mFrequence = SAMPLE_RATE_I;

        setName("Presets");

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
    int addRack(std::string name = "Rack*" )
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
    int insertRackAbove( int fromIndex  ) {
        int result = cloneRack(fromIndex);
        if (result < 0) return result;
        reorderRack(result, fromIndex);
        return result;
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
        if ( mActiveRack == nullptr ) return -1;
        for (int i = 0; i < (int)mPresets.size(); ++i) {
            if (mPresets[i].get() == mActiveRack) {
                return i;
            }
        }
        addError("[error] Current Rack not found! Something is really wrong here!"); //maybe assert
        return -1; // should not happen!
    }
    //--------------------------------------------------------------------------
    int cloneCurrent()  {
        int currentIndex = getActiveRackIndex();
        if (currentIndex == -1) {
            addError("[error] No active rack to clone!");
            return -1;
        }
        return cloneRack(currentIndex);
    }
    //--------------------------------------------------------------------------
    bool prevRack() {
        int currentIdx = getActiveRackIndex();
        int count = getPresetsCount();
        currentIdx--;
        if (currentIdx < 0) currentIdx = count -1;
        return setActiveRack(currentIdx);
    }
    //--------------------------------------------------------------------------
    bool nextRack() {
        int currentIdx = getActiveRackIndex();
        int count = getPresetsCount();
        currentIdx++;
        if (currentIdx >= count) currentIdx = 0;
        return setActiveRack(currentIdx);
    }
    //--------------------------------------------------------------------------
    bool setActiveRack(int index) {
        if (index >= 0 && index < (int)mPresets.size()) {
            std::lock_guard<std::recursive_mutex> lock(mEffectMutex);
            mActiveRack = mPresets[index].get();
            return true;
        }
        return false;
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
        if (mActiveRack == nullptr ) return;
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
    //--------------------------------------------------------------------------
    void setEnabled(bool value) { mEnabled = value;}
    //--------------------------------------------------------------------------
    bool isEnabled() const { return mEnabled; }
    //--------------------------------------------------------------------------
    std::vector<std::unique_ptr<DSP::Effect>>& getEffects() {
        static std::vector<std::unique_ptr<DSP::Effect>> emptyVector;
        if (!mActiveRack) {
            return emptyVector;
        }
        return mActiveRack->getEffects();
    }

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
        if (!fx || !mActiveRack) return false;
        if ( mActiveRack->getEffectsCount() >= MAX_EFFECTS_IN_RACKS ){
            addError(std::format("[error] We cant have more then {} Effects in one Rack", MAX_RACKS_IN_PRESET));
            return false;
        }
        // mActiveRack->getEffects().push_back(std::move(fx));
        mActiveRack->add(std::move(fx));
        return true;
    }
    //--------------------------------------------------------------------------
    DSP::Effect* getEffectByType(DSP::EffectType type) {
        if ( !mActiveRack ) return nullptr;
        std::lock_guard<std::recursive_mutex> lock(mEffectMutex);
        for (auto& fx : mActiveRack->getEffects()) {
            if (fx->getType() == type) return fx.get();
        }
        return nullptr;
    }

    //--------------------------------------------------------------------------
    bool removeEffect( size_t effectIndex  ) {
        if ( !mActiveRack ) return false;
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

        if (mActiveRack == nullptr) return;
#ifdef FLUX_ENGINE
        bool isEnabled = mEnabled;

        if (!isEnabled) ImGui::BeginDisabled();
        for (auto& effect : this->mActiveRack->getEffects()) {
            if (effect)
            {
                switch ( mode )
                {
                    case 1: effect->renderUIWide();break;
                    case 2: effect->renderPaddle();break;
                    case 3: effect->renderCustomUI();break;
                    default: effect->renderUI(); break;
                }
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

        std::lock_guard<std::recursive_mutex> lock(mEffectMutex);

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
                if (!mActiveRack) return false;
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
    void SavePresetsStream(std::ostream& ofs) const {
        ofs.exceptions(std::ios::badbit | std::ios::failbit);
        DSP_STREAM_TOOLS::write_binary(ofs, DSP::DSP_AXE_MAGIC);
        DSP_STREAM_TOOLS::write_binary(ofs, uint32_t(DSP_AXE_VERSION));

        int32_t presetCount = getPresetsCount();
        DSP_STREAM_TOOLS::write_binary(ofs, presetCount);

        for (int32_t rackIdx = 0; rackIdx < presetCount; rackIdx++) {
            SaveRackStream(mPresets[rackIdx].get(), ofs );
        }

        // version 2 ...
        DSP_STREAM_TOOLS::write_binary(ofs, mSwitchRack);

        // version 3 ( 3 days 3 version ^^ )
        DSP_STREAM_TOOLS::write_string(ofs, mName);


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
        std::lock_guard<std::recursive_mutex> lock(mEffectMutex);

        mPresets.clear();
        mPresets.reserve(presetCount);

        for (int32_t rackIdx = 0; rackIdx < presetCount; rackIdx++) {
            if (!LoadRackStream(ifs, RackLoadMode::AppendToPresets , false)) {
                addError(std::format("LoadPresetStream: preset load failed at rack {}! see previous error.", rackIdx));
                return false;
            }
        }
        if (!mPresets.empty()) {
            mActiveRack = mPresets.front().get();
        } else {
            mActiveRack = nullptr;
        }
        //... version 2
        if (version > 1) {
            DSP_STREAM_TOOLS::read_binary(ifs, mSwitchRack);
        }

        //... version 3
        mName = "no name";
        if (version > 2) {
             DSP_STREAM_TOOLS::read_string(ifs, mName);
        }


        //... final check
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
    // --------- process -------------
    void process(float* buffer, int numSamples, int numChannels) {
        if (!mEnabled || !mActiveRack) return;
        std::lock_guard<std::recursive_mutex> lock(mEffectMutex);
        for (auto& effect : this->mActiveRack->getEffects()) {
            effect->process(buffer, numSamples, numChannels);
        }

    }
    //--------------------------------------------------------------------------
    // Preset switch
    void setSwitchRack(int idx) {
        if (idx >= (int)mPresets.size()) idx = -1; //reset
        mSwitchRack = idx;
    }
    int getSwitchRack() const { return mSwitchRack; }
    bool switchRack() {
        if (mSwitchRack < 0 || mSwitchRack >= (int)mPresets.size()) return false;
        int lastRack = getActiveRackIndex();
        if (setActiveRack(mSwitchRack)) {
            mSwitchRack = lastRack;
            return true;
        }
        return false;
    }


    //--------------------------------------------------------------------------
    #ifdef FLUX_ENGINE
    void DrawPresetList(float colorSeed = 0.f) {
        // because i coded it in my project and moved it here when done:
        DSP::EffectsManager* lManager = this;

        // default size
        ImVec2 controlSize = {0,0};
        // magic pointer movement
        static int move_from = -1, move_to = -1;
        int delete_idx = -1, insert_idx = -1;
        int clone_idx = -1;
        // init pointers

        // --- Compact Header ---

        ImFlux::LEDCheckBox("Active",&mEnabled, ImVec4(0.2f,0.8f,0.2f,1.f));

        ImFlux::SameLineBreak(140.f);
        ImGui::SetNextItemWidth(140.f);
        char presetNameBuff[64];
        strncpy(presetNameBuff, getName().c_str(), sizeof(presetNameBuff));

        if (ImGui::InputText("##preset Name", presetNameBuff, sizeof(presetNameBuff))) {
            lManager->setName(presetNameBuff);
        }
        ImFlux::Hint("Presets List Name");


        // Start a child region for scrolling if the list gets long
        ImGui::BeginChild("PresetListScroll", controlSize, false);



        int currentIdx = lManager->getActiveRackIndex();

        for (int rackIdx = 0; rackIdx < lManager->getPresetsCount(); rackIdx++) {
            const bool is_selected = (currentIdx == rackIdx);
            const bool is_switchRack = ( mSwitchRack >=0 ) && (mSwitchRack == rackIdx);
            ImGui::PushID(lManager);ImGui::PushID(rackIdx);

            // 1. Draw Index
            ImGui::AlignTextToFramePadding();
            // ImGui::TextDisabled("%02d", rackIdx);
            ImGui::TextDisabled("%02X", rackIdx);
            ImGui::SameLine();

            // 2. Button Dimensions & Interaction
            ImVec2 pos = ImGui::GetCursorScreenPos();
            ImVec2 size = ImVec2(ImGui::GetContentRegionAvail().x - 10.0f, ImGui::GetFrameHeight());


            float coloredWidth = 30.0f;
            ImVec2 coloredSize(coloredWidth, size.y);


            // InvisibleButton acts as the interaction hit-box for DragDrop and Clicks
            ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
            bool pressed = ImGui::InvisibleButton("rack_btn", size);
            if (pressed) lManager->setActiveRack(rackIdx);

            bool is_hovered = ImGui::IsItemHovered();
            bool is_active = ImGui::IsItemActive();
            ImDrawList* draw_list = ImGui::GetWindowDrawList();

            // ----------- rendering

            // 1. Logic for enhanced selection visibility
            ImU32 col32 = ImFlux::getColorByIndex(rackIdx, colorSeed);
            ImU32 colMiddle32 =  IM_COL32(20, 20, 20, 255);
            ImU32 border_col = IM_COL32_WHITE;
            float border_thickness = 1.0f;

            if (is_selected || is_switchRack) {
                col32 = (col32 & 0x00FFFFFF) | 0xFF000000;
                border_col = ImFlux::COL32_NEON_YELLOW;
                border_thickness = 2.0f;
                colMiddle32 = is_switchRack ? IM_COL32(60, 40, 60, 255) : IM_COL32(40, 40, 40, 255);

            } else {
                col32 = (col32 & 0x00FFFFFF) | 0x66000000;
            }

            // left
            draw_list->AddRectFilled(pos, pos + coloredSize, col32, 3.0f);

            // Middle part: Starts after left bar, width is (total - 2 * side bars)
            ImVec2 midPos = ImVec2(pos.x + coloredWidth, pos.y);
            ImVec2 midSize = ImVec2(size.x - (2.0f * coloredWidth), size.y);
            draw_list->AddRectFilled(midPos, midPos + midSize, colMiddle32, 0.0f); // No rounding for middle to avoid gaps

            // Right part: Starts at the end minus the side bar width
            ImVec2 rightPos = ImVec2(pos.x + size.x - coloredWidth, pos.y);
            draw_list->AddRectFilled(rightPos, rightPos + coloredSize, col32, 3.0f);


            // Selection "Glow" / Outline
            if (is_selected) {
                // Outer Glow Effect: Draw a slightly larger, transparent rect behind/around
                // draw_list->AddRect(pos - ImVec2(2, 2), pos + size + ImVec2(2, 2),
                //                    IM_COL32(255, 255, 0, 100), 3.0f, 0, 4.0f);



                // Solid Inner Border
                draw_list->AddRect(pos, pos + size, border_col, 3.0f, 0, border_thickness);

                // OPTIONAL: Add a small white "active" indicator circle on the left
                draw_list->AddCircleFilled(ImVec2(pos.x - 5, pos.y + size.y * 0.5f), 3.0f, border_col);
            } else if (is_hovered) {
                draw_list->AddRect(pos, pos + size, IM_COL32(255, 255, 255, 180), 3.0f);
            }

            // Text Contrast
            // For the selected item, use Black text if the background is very bright
            // (or stay with White+Shadow for consistency)
            std::string rackNameStr = lManager->getRackByIndex(rackIdx)->getName();

            char rackName[64];
            strncpy(rackName, rackNameStr.c_str(), sizeof(rackName));


            ImVec2 text_size = ImGui::CalcTextSize(rackName);
            ImVec2 text_pos = ImVec2(pos.x + (size.x - text_size.x) * 0.5f, pos.y + (size.y - text_size.y) * 0.5f);

            ImU32 text_col = is_selected ? IM_COL32_WHITE : IM_COL32(200, 200, 200, 255);
            draw_list->AddText(text_pos + ImVec2(1, 1), IM_COL32(0, 0, 0, 255), rackName);
            draw_list->AddText(text_pos, text_col, rackName);


            // 6. Context Menu
            if (ImGui::BeginPopupContextItem("row_menu")) {
                ImGui::SeparatorText(rackName);
                if (ImGui::MenuItem("Set as Switch Rack")) setSwitchRack(rackIdx);
                ImGui::SeparatorText("Actions");
                if (ImGui::MenuItem("Clone")) clone_idx = rackIdx;
                if (ImGui::MenuItem("Clone Above")) insert_idx = rackIdx;
                if (ImGui::MenuItem("Remove")) delete_idx = rackIdx;
                ImGui::SeparatorText("Name:");
                ImGui::SetNextItemWidth(150.f);
                if (ImGui::InputText("##Rack Name", rackName, sizeof(rackName))) {
                    lManager->getRackByIndex(rackIdx)->setName(rackName);
                }

                ImGui::EndPopup();
            }

            // 7. Drag and Drop (Attached to the InvisibleButton)
            if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID)) {
                ImGui::SetDragDropPayload("DND_ORDER", &rackIdx, sizeof(int));
            ImGui::Text("Moving %s", rackName);
            ImGui::EndDragDropSource();
            }
            if (ImGui::BeginDragDropTarget()) {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("DND_ORDER")) {
                    move_from = *(const int*)payload->Data;
                    move_to = rackIdx;
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::PopStyleVar();
            ImGui::PopID();ImGui::PopID();
        }
        ImGui::EndChild();

        // Deferred move/delete/add
        if (delete_idx != -1) lManager->removeRack(delete_idx);
        if ( clone_idx != -1 ) {
            int newId = lManager->cloneRack(clone_idx);
            if (newId >=0) lManager->setActiveRack(newId);
        }
        if (insert_idx != -1) {
            int newId = lManager->insertRackAbove(insert_idx);
            if (newId >=0) lManager->setActiveRack(newId);
        }
        if (move_from != -1 && move_to != -1) {
            // dLog("[info] move rack from %d to %d", move_from, move_to) ;
            lManager->reorderRack(move_from, move_to);
            move_from = -1;
            move_to = -1;
        }
    } //DrawPresetList

#endif

}; //class Effects
}; //namespace
