//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Base class Effect
//-----------------------------------------------------------------------------
// TODO: move templates out to a not file
// TODO: add color (U32) to params ( for buttons :D )
//-----------------------------------------------------------------------------
#pragma once
#include <cstdint>
#include <string>
#include <atomic>
#include <algorithm>
#include <vector>
#include <memory>


#ifdef FLUX_ENGINE
#include <imgui.h>
#include <imgui_internal.h>
#include <gui/ImFlux.h>
#include "DSP_tools.h"
#endif


namespace DSP {

    inline static float SAMPLE_RATE = 44100.f; // global setting for SampleRate
    inline int getSampleRateI()  { return static_cast<int>(SAMPLE_RATE); }
    inline float getSampleRateF() { return SAMPLE_RATE; }

    constexpr uint32_t DSP_RACK_MAGIC = 0x524F434B; // ASCII: 'R' 'O' 'C' 'K' -> 0x524F434B
    constexpr uint32_t DSP_RACK_VERSION = 1;

    //------------------------- EFFECT LIST  --------------------------------


    #define EFFECT_LIST(X) \
        X(Bitcrusher         ,1) \
        X(Chorus             ,2) \
        X(Equalizer          ,3) \
        X(Equalizer9Band     ,4) \
        X(Limiter            ,5) \
        X(Reverb             ,6) \
        X(SoundCardEmulation ,7) \
        X(SpectrumAnalyzer   ,8) \
        X(Warmth             ,9) \
        X(VisualAnalyzer     ,10) \
        X(Delay              ,11) \
        X(VoiceModulator     ,12) \
        X(RingModulator      ,13) \
        X(OverDrive          ,14) \
        X(NoiseGate          ,15) \
        X(DistortionBasic    ,16) \
        X(Metal              ,17) \
        X(ChromaticTuner     ,18)



    // macro power :
    enum class EffectType : uint32_t {
        NONE = 0,
        #define X_ENUM(name, id) name = id,
        EFFECT_LIST(X_ENUM)
        #undef X_ENUM
    };

    #define IMPLEMENT_EFF_CLONE(ClassName) \
    std::unique_ptr<Effect> clone() const override { \
        return std::make_unique<ClassName>(*this); \
    }
    //---------------------- PARAMETER DEFINITION --------------------------
    #define REGISTER_SETTINGS(ClassName, ...) \
    /* 1. Der Kopierkonstruktor (löst deinen Fehler) */ \
    ClassName(const ClassName& other) : ClassName() { \
        this->copyValuesFrom(other); \
    } \
    \
    /* 2. Der Zuweisungsoperator (für settings = other) */ \
    ClassName& operator=(const ClassName& other) { \
        if (this != &other) { \
            this->copyValuesFrom(other); \
        } \
        return *this; \
    } \
    \
    /* 3. Die restlichen Interface-Methoden */ \
    std::vector<IParameter*> getAll() override { return { __VA_ARGS__ }; } \
    std::vector<const IParameter*> getAll() const override { return { __VA_ARGS__ }; } \
    std::unique_ptr<ISettings> clone() const override { \
        auto copy = std::make_unique<ClassName>(); \
        copy->copyValuesFrom(*this); \
        return copy; \
    }


    // Parameter Interface
    class IParameter {
    public:
        virtual ~IParameter() = default;
        virtual std::string getName() const = 0;
        virtual float getNormalized() const = 0;

        virtual std::string getDisplayValue() const = 0;


        virtual void setDefaultValue() = 0;
        virtual std::string getDefaultValueAsString() const = 0;


        virtual bool isEqual(const IParameter* other) const = 0;

        virtual void saveToStream(std::ostream& os) const = 0;
        virtual void loadFromStream(std::istream& is) = 0;

        virtual void setFrom(const IParameter* other) = 0;


        #ifdef FLUX_ENGINE
        virtual bool  MiniKnobF() = 0;
        virtual bool FaderHWithText() = 0;
        #endif

    };

    //--------------------------------------------------------------------------
    // Preset Interface
    struct ISettings; // fwd
    struct IPreset {
        virtual ~IPreset() = default;
        virtual const char* getName() const = 0;
        virtual void apply(ISettings& settings) const = 0;
    };
    // Preset Params
    template <typename T_Settings, typename T_Data>
    struct Preset : public IPreset {
        const char* name;
        T_Data data;
        Preset(const char* n, T_Data d) : name(n), data(d) {}
        const char* getName() const override { return name; }
        void apply(ISettings& settings) const override {
            if (auto* s = dynamic_cast<T_Settings*>(&settings)) {
                s->setData(data);
            }
        }
    };
    //--------------------------------------------------------------------------
    // Settings Interface
    struct ISettings {
        virtual ~ISettings() = default;

        virtual std::vector<IParameter*> getAll() = 0;
        virtual std::vector<const IParameter*> getAll() const = 0;

         // virtual std::vector<std::shared_ptr<IPreset>> getPresets() const = 0;
        //----------------------------------------------------------------------
        void resetToDefaults() {
            for (auto* p : getAll()) {
                p->setDefaultValue();
            }
        }
        //----------------------------------------------------------------------
        virtual std::vector<std::shared_ptr<IPreset>> getPresets() const {
            return {}; // defaults to none
        }
        //----------------------------------------------------------------------
        void save(std::ostream& os) const {
            uint8_t ver = 1; //fake version and sanity check on load
            DSP_STREAM_TOOLS::write_binary(os, ver);
            for (auto* p : getAll()) {
                 p->saveToStream(os);
            }
        }
        bool load(std::istream& is) {
            // read fake version
            uint8_t ver = 0;
            DSP_STREAM_TOOLS::read_binary(is, ver);
            if (ver != 1) return false;

            for (auto* p : getAll()) {
                p->loadFromStream(is);
            }
            return is.good();
        }
        //----------------------------------------------------------------------
        bool operator==(const ISettings& other) const {
            auto paramsMe = this->getAll();
            auto paramsOther = other.getAll();
            if (paramsMe.size() != paramsOther.size()) return false;

            for (size_t i = 0; i < paramsMe.size(); ++i) {
                if (!paramsMe[i]->isEqual(paramsOther[i])) return false;
            }
            return true;
        }
        //----------------------------------------------------------------------
         virtual std::unique_ptr<ISettings> clone() const = 0;
        //----------------------------------------------------------------------
         void apply(const IPreset* preset) {
             if (preset) preset->apply(*this);
         }
        //----------------------------------------------------------------------
        bool isMatchingPreset(const IPreset* preset) {
            auto clone = this->clone();
            preset->apply(*clone);
            return *this == *clone;
        }
        //----------------------------------------------------------------------
        void copyValuesFrom(const ISettings& other) {
            auto myParams = getAll();
            auto otherParams = other.getAll();
            for (size_t i = 0; i < myParams.size(); ++i) {
                myParams[i]->setFrom(otherParams[i]);
            }
        }
        //----------------------------------------------------------------------
#ifdef FLUX_ENGINE
        bool drawStepper(ISettings& settings) {
            bool changed = false;
            auto presets = settings.getPresets();
            if (presets.empty()) return changed;
            int currentIdx = 0; // Default: "Custom" oder erster Eintrag
            for (int i = 0; i < (int)presets.size(); ++i) {
                if (settings.isMatchingPreset(presets[i].get())) {
                    currentIdx = i;
                    break;
                }
            }
            std::vector<const char*> names;
            for (auto& p : presets) names.push_back(p->getName());

            int displayIdx = currentIdx;
            ImGui::SameLine(ImGui::GetWindowWidth() - 260.f);

            if (ImFlux::ValueStepper("##Preset", &displayIdx, names.data(), (int)names.size()))
            {
                if (displayIdx >= 0 && displayIdx < (int)presets.size()) {
                    presets[displayIdx]->apply(settings);
                    changed = true;
                }
            }
            return changed;
        }
#endif
    }; //ISettings


    //--------------------------------------------------------------------------
    //--------------------------------------------------------------------------
    // Parameter Template-Class Thread safe
    template <typename T>
    class AudioParam : public IParameter {
    public:
        AudioParam(std::string name, T def, T min, T max, std::string unit = "")
        : name(name), minVal(min), maxVal(max), unit(unit) {
            value.store(def);
            defaultValue = def;
        }

        operator T() const { return value.load(std::memory_order_relaxed); }

        AudioParam& operator=(T newValue) {
            set(newValue);
            return *this;
        }

        T get() const { return value.load(std::memory_order_relaxed); }

        void set(T newValue) {
            value.store(std::clamp(newValue, minVal, maxVal));
        }

        void setDefaultValue() override { set(defaultValue);}
        T getDefaultValue() { return defaultValue; }
        std::string getDefaultValueAsString() const override {
            if constexpr (std::is_same_v<T, float>) {
                return std::to_string(defaultValue);
            } else if constexpr (std::is_same_v<T, bool>) {
                return defaultValue ? "true" : "false";
            } else {
                return std::to_string(defaultValue);
            }
        }

        std::string getName() const override { return name; }
        float getNormalized() const override {
            return static_cast<float>(get() - minVal) / static_cast<float>(maxVal - minVal);
        }
        std::string getDisplayValue() const override {
            return std::to_string(get()) + " " + unit;
        }

        T getMin() const { return minVal; }
        T getMax() const { return maxVal; }
        void setMin( T value ) { minVal = value; }
        void setMax( T value ) { maxVal = value; }
        std::string getUnit() const { return unit; }

        bool isEqual(const IParameter* other) const override {
            auto* typedOther = dynamic_cast<const AudioParam<T>*>(other);
            if (!typedOther) return false;
            return this->get() == typedOther->get();
        }

        void saveToStream(std::ostream& os) const override {
            DSP_STREAM_TOOLS::write_binary(os, *this);
        }

        void loadFromStream(std::istream& is) override {
            DSP_STREAM_TOOLS::read_binary(is, *this);
        }

        void setFrom(const IParameter* other) override {
            if (auto* typedOther = dynamic_cast<const AudioParam<T>*>(other)) {
                this->set(typedOther->get());
            }
        }

    private:
        std::string name;
        std::atomic<T> value;
        T defaultValue;
        T minVal, maxVal;
        std::string unit;

    public:
#ifdef FLUX_ENGINE
   virtual bool  MiniKnobF() override {
        float tmpValue = get();
        if (ImFlux::MiniKnobF(name.c_str(), &tmpValue, minVal, maxVal)) {
            set(tmpValue);
            return true;
        }
        return false;
    }
    virtual bool FaderHWithText() override {
        float tmpValue = get();
        if (ImFlux::FaderHWithText(name.c_str(), &tmpValue, minVal, maxVal, unit.c_str())) {
            set(tmpValue);
            return true;
        }
        return false;
    }

#endif
    };

    //------------------------- BASE CLASS --------------------------------

    class Effect {
    protected:
        bool mEnabled = false;

    public:
        Effect(bool switchOn = false) { mEnabled = switchOn;}
        virtual ~Effect() {}

        virtual std::unique_ptr<Effect> clone() const = 0;

        // process the samples and modify the buffer ... here is the beef :)
        virtual void process(float* buffer, int numSamples, int numChannels) {}

        // added for single float processing ...
        // you need to handle it manually since it's not supported on all
        // classes !!! We simply return the input again by default
        virtual float processFloat(float input) { return input; }


        virtual DSP::EffectType getType() const { return DSP::EffectType::NONE; }

        // Interface for serialization
        virtual void save(std::ostream& os) const {
            os.write(reinterpret_cast<const char*>(&mEnabled), sizeof(mEnabled));
        }

        virtual bool load(std::istream& is) {
            is.read(reinterpret_cast<char*>(&mEnabled), sizeof(mEnabled));
            return is.good();
        }

        virtual void reset() {}

        virtual void setSampleRate(float sampleRate) {}

        virtual void setEnabled(bool value) {
            mEnabled = value;
            reset();
        }
        virtual bool isEnabled() const { return mEnabled; }

        // this is required for export to wave on delayed effects
        virtual float getTailLengthSeconds() const { return 0.f; }


        virtual std::string getName() const { return "EFFECT # FIXME ";}


#ifdef FLUX_ENGINE
    virtual ImVec4 getColor() const { return ImVec4(0.5f,0.5f,0.5f,1.f);}

    virtual void renderUIWide() {
        char buf[32];
        snprintf(buf, sizeof(buf), "Effect_Row_W_%d", getType());
        ImGui::PushID(buf);

        ImFlux::GradientBox(ImVec2(-FLT_MIN, -FLT_MIN),0.f);
        ImGui::Dummy(ImVec2(2,0)); ImGui::SameLine();

        bool isEnabled = this->isEnabled();
        if (ImFlux::LEDCheckBox(getName(), &isEnabled, getColor())){
            this->setEnabled(isEnabled);
        }
        ImGui::PopID();
    }


    virtual void renderPaddle() {
        // paddleHeader(getName().c_str(), ImGui::ColorConvertFloat4ToU32(getColor()), mEnabled);
    }

    virtual void renderUI() {
          char buf[32];
          snprintf(buf, sizeof(buf), "Effect_Row_%d", getType());
          ImGui::PushID(buf);
          bool isEnabled = this->isEnabled();
          if (ImFlux::LEDCheckBox(getName(), &isEnabled, getColor())){
              this->setEnabled(isEnabled);
          }
          ImGui::PopID();
     }

#endif
    }; //class
}; //namespace
