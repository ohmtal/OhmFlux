# Digital Sound Processing and Guitar Effects 
## by T.Hühn (XXTH) 2026
https://github.com/ohmtal

I have some optional ImGui / OhmFlux UI-Stuff included. 
But it basically can be used in every Project. 

## 🎸 Effects
- Analog Glow - Guitar Tube AMP Effect 
- AutoWah 
- Bit Crusher "Lo-Fi" Filter
- Stereo Chorus 
- Chromatic Tuner 
- Delay 
- Basic Distortion 
- Single Band Equalizer
- 9 Band Equalizer
- Limiter
- Metal Distortion 
- Noise Gate with low-/high-pass filter
- OverDrive 
- Reverb
- Ring Modulator
- Spectrum Analyzer (basic)
- Tone and Volume 
- Tremolo
- Visual Analyzer for VU Meter and Oszi
- Voice Modulator 
- Warmth a One-Pole Low-Pass Filter

## 🥁 Drums 
- Drum Kit and Looper 
    -- 4/4 one bar drum sequence 
    -- Looper with BPM sync. Up to 3 minutes - the sound data is stored in the drumkit effect
- DrumSynth: The Drum Classes
- Kick Drum Effect 
- TODO other drums should be also defined as Effects 

## 🚂 DSP_Effect.h, DSP_Effect_Manager.h, DSP_EffectFactory.h
Effects are organized in a Rack and the Racks are stored in Presets.
- Effect Manager classes: EffectsRack and EffectsManager
- Effect Factory is for creating and managing the Effects.
- The core is in DSP_Effect.h it's a bit a mess since it grows over time:
    - Effect Categories
    - Macro EFFECT_LIST for identifying the Effects
    - Some more handy macros 
    - IParamter virtual class for effect parameters 
    - AudioParams extents IParamter is used to handle the effect parameters and some rendering if OhmFlux is used. 
    - Effect Base Class also with rendering if using OhmFlux


## 🔨 Helper
- Normalizer for export to Wav 
- Tools:
    - Stream Tools for loading / saving binary data
    - Some UI Stuff using ImGui
- Math:
    - FastMath sin/cos look up table (LUT)
    - Softclipping 
    - and some more handy functions
