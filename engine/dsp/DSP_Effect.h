//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Digital Sound Processing : Base class Effect
//-----------------------------------------------------------------------------
#pragma once
#include <cstdint>

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



    // enum class EffectType : uint32_t {
    //     NONE               = 0,
    //     Bitcrusher         = 1,
    //     Chorus             = 2,
    //     Equalizer          = 3, //UNUSED: only one band so i would be attached multiple times ...
    //     Equalizer9Band     = 4,
    //     Limiter            = 5,
    //     Reverb             = 6,
    //     SoundCardEmulation = 7,
    //     SpectrumAnalyzer   = 8, //no extra settings only analysing for visual effect
    //     Warmth             = 9,
    //     VisualAnalyzer     = 10,  //no extra settings only analysing for visual effect
    //     Delay              = 11,
    //     VoiceModulator     = 12,
    //     RingModulator      = 13,
    //     OverDrive          = 14,
    //     NoiseGate          = 15,
    //     DistortionBasic    = 16,
    //     Metal              = 17,
    //     ChromaticTuner     = 18
    //     // NOTE  don't forget to add this to the Effect Factory !!!
    //
    // };
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
