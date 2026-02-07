//-----------------------------------------------------------------------------
// Copyright (c) 2026 Ohmtal Game Studio
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
#pragma once

#include <imgui.h>
#include <imgui_internal.h>
#include <cmath>
#include <algorithm>

#include "../ImFlux.h"

namespace ImFlux {


//-------------- Demo ShowButtonFancyGallery
inline void ShowCase_ButtonFancyGallery() {
    // ImGui::Begin("Fancy Button Gallery");

    // Standard styling for the whole gallery
    static ButtonParams p;
    p.size = ImVec2(120, 40);
    p.rounding = 4.0f;

    // --- ROW 1: Mouse Over Effects ---
    ImGui::Text("Mouse Over Effects:");

    p.color = IM_COL32(50, 100, 200, 255); // Blueish
    p.mouseOverEffect = BUTTON_MO_HIGHLIGHT;
    ButtonFancy("Highlight", p); ImGui::SameLine();

    p.mouseOverEffect = BUTTON_MO_PULSE;
    ButtonFancy("Pulse", p); ImGui::SameLine();

    p.mouseOverEffect = BUTTON_MO_GLOW;
    ButtonFancy("Glow", p); ImGui::SameLine();

    p.mouseOverEffect = BUTTON_MO_GLOW_PULSE;
    ButtonFancy("Glow and Pulse", p); ImGui::SameLine();

    p.mouseOverEffect = BUTTON_MO_SHAKE;
    ButtonFancy("Shake", p); ImGui::SameLine();

    p.mouseOverEffect = BUTTON_MO_BOUNCE;
    ButtonFancy("Bounce", p);

    ImGui::Separator();

    // ImGui::Text("Animation Speed");
    // p.mouseOverEffect = BUTTON_MO_GLOW_PULSE;
    // p.color = IM_COL32(200, 40, 40, 255);
    //
    // p.animationSpeed = 5.0f;
    // ButtonFancy("Slow Pulse", p); ImGui::SameLine();
    //
    // p.animationSpeed = 25.0f;
    // ButtonFancy("URGENT", p);


    ImGui::Separator();

    // --- ROW 2: Shapes & Rounding ---
    ImGui::Text("Shapes & Rounding:");

    p.mouseOverEffect = BUTTON_MO_HIGHLIGHT;
    p.color = IM_COL32(100, 100, 100, 255); // Grey

    p.rounding = 0.0f;
    ButtonFancy("Square", p); ImGui::SameLine();

    p.rounding = 8.0f;
    ButtonFancy("Rounded", p); ImGui::SameLine();

    p.rounding = p.size.y * 0.5f; // Perfect Capsule
    ButtonFancy("Capsule", p);

    ImGui::Separator();

    // --- ROW 3: Toggle Features ---
    ImGui::Text("Feature Toggles:");

    p.rounding = 4.0f;
    p.color = IM_COL32(40, 150, 40, 255); // Green

    p.bevel = true; p.gloss = true;
    ButtonFancy("Bevel+Gloss", p); ImGui::SameLine();

    p.bevel = false; p.gloss = false;
    ButtonFancy("Flat Style", p); ImGui::SameLine();

    p.shadowText = false;
    ButtonFancy("No Shadow", p);

    // ImGui::End();
}


inline void ShowCase_MISC()
{

    ImGui::TextDisabled("Bit Editor");
    static uint8_t foobit = 0xff;
    ImFlux::BitEditor("foo", &foobit);ImGui::SameLine();ImGui::Text("FooBit %02X [%d]", foobit, foobit);

    ImGui::Separator();
    ImGui::TextDisabled("LCDDisplay");
    ImFlux::LCDDisplay("##FooBit4711", (float)foobit, 3, 0, 16.0f, Color4FIm(cl_Red));

    ImGui::Separator();
    ImGui::TextDisabled("LCDNumber");
    ImFlux::LCDNumber((float)foobit, 3, 0, 24.0f, Color4FIm(cl_Yellow));

    ImGui::Separator();
    float value = foobit / 255.f;
    ImGui::TextDisabled("PeakMeter");
    ImFlux::PeakMeter( value);

    ImGui::Separator();
    ImGui::TextDisabled("VU Meter");
    ImFlux::VUMeter70th(ImVec2(160,70),value, "VU");
    ImGui::SameLine();
    ImGui::BeginGroup();
    ImFlux::VUMeter80th(value);
    ImFlux::VUMeter80th(value, 5, ImVec2(30.f, 8.f));
    ImFlux::VUMeter80th(value, 30, ImVec2(4.f, 8.f));
    ImFlux::VUMeter80th(value, 100, ImVec2(2.f, 8.f));
    ImGui::EndGroup();
    ImGui::SameLine();
    ImFlux::VUMeter70th(ImVec2(160,70),value, "VU", IM_COL32(200, 200, 200, 255), IM_COL32(5, 5, 5, 5));
    ImGui::SameLine();

    static VUMeterState myState1;
    myState1.ledCount = 16;
    myState1.ledSquare = ImVec2(12.f, 3.f);
    VUMeter90th(value, myState1);

    ImGui::SameLine();

    static VUMeterState myState2;
    myState2.holdTime = 0.f;
    VUMeter90th(value, myState2);

    ImGui::Separator();
    ImGui::TextDisabled("Text");

    ImFlux::ShadowText("Shadow Text");

    ImGui::Separator();
    ImGui::TextDisabled("Knobs, with scroll wheel action (hold shift for small changes)");

    static float barFloat = 0.5f;
    static int   barInt   = 128;
    ImFlux::LEDRingKnob("barKnob", &barFloat, 0.f, 1.f);
    ImGui::SameLine();

    ImFlux::SeparatorVertical(4.f);
    KnobSettings ks = DARK_KNOB;
    ImFlux::MiniKnobF("MiniKnobF Dark", &barFloat, 0.f, 1.f, ks);
    ImGui::SameLine();

    KnobSettings ls = DARK_KNOB;
    ls.active = IM_COL32(0,255,0,255);
    ls.radius = 30.f;
    ImFlux::LEDMiniKnob("LEDMiniKnob Dark", &barFloat, 0.f, 1.f, ls);

    ImGui::SameLine();
    ImFlux::MiniKnobFloat("MiniKnobFloat", &barFloat, 0.f, 1.f); // float radius = 12.f,  float speed = 0.01f) {
    ImGui::SameLine();
    ImFlux::MiniKnobFloat("MiniKnobFloat20", &barFloat, 0.f, 1.f, 20.f); // float radius = 12.f,  float speed = 0.01f) {
    ImGui::SameLine();
    ImFlux::LCDNumber(barFloat, 5, 2, 32.0f, Color4FIm(cl_Lime));
    ImFlux::SeparatorVertical(4.f);
    ImFlux::MiniKnobInt("MiniKnobInt", &barInt, 0,255);
    ImGui::SameLine();
    ImFlux::LCDDisplay("##BarInt1", (float)barInt, 3, 0, 10.0f, Color4FIm(cl_Yellow));

}
//---------------------------------------------------------------------------------------
inline void ShowCase_LED() {

    // Predefined Basic Params
    LedParams pStatic = { 8.f, false, false, ImColor(0, 255, 0) };
    LedParams pGlow = { 8.f, false, true, ImColor(0, 255, 255) }; // Cyan

    // --- STATIC SECTION ---
    ImGui::TextDisabled("STATIC LEDS");
    DrawLED("Green Off", false, pStatic); ImGui::SameLine();
    DrawLED("Green On", true, pStatic);   ImGui::SameLine();

    pStatic.colorOn = ImColor(255, 0, 0);
    DrawLED("Red On", true, pStatic);     ImGui::SameLine();

    DrawLED("Cyan Glow", true, pGlow);

    ImGui::Separator();

    // --- ANIMATED SECTION ---
    ImGui::TextDisabled("ANIMATED LEDS (ALIVE)");

    // 1. FADE (Breathing)
    LedParams pFade = { 8.f, true, true, ImColor(0, 255, 0) };
    pFade.animationType = LED_Ani_FADE;
    pFade.aniSpeed = 4.0f;
    DrawLED("Fade Green", true, pFade); ImGui::SameLine();

    // 2. PULSE (Growing/Shrinking)
    LedParams pPulse = { 8.f, true, true, ImColor(255, 255, 0) };
    pPulse.animationType = LED_Ani_PULSE;
    pPulse.aniRadius = 3.0f;
    pPulse.aniSpeed = 6.0f;
    DrawLED("Pulse Yellow", true, pPulse); ImGui::SameLine();

    // 3. LINEAR (Blinking)
    LedParams pLinear = { 8.f, true, false, ImColor(255, 0, 0) };
    pLinear.animationType = LED_Ani_Linear;
    pLinear.aniSpeed = 10.0f;
    DrawLED("Blink Red", true, pLinear); ImGui::SameLine();

    // 4. INTERACTIVE LED
    LedParams pInteract = { 10.f, true, true, ImColor(255, 0, 255) };
    pInteract.renderMouseEvents = true;
    pInteract.animationType = LED_Ani_FADE;
    if (DrawLED("Click me!", true, pInteract)) {
        // LED reacts to clicks like a button
    }
    ImGui::SameLine();
    ImGui::Text("<- Clickable LED");

    ImGui::Separator();
    ImGui::TextDisabled("LED CheckBox (wrapped)");
    static bool foo = false;
    LEDCheckBox("Enable foo", &foo, ImColor(255,128,128));ImGui::SameLine();
}
//---------------------------------------------------------------------------------------
inline void ShowCase_StepperAndSlider()
{
    ImGui::TextDisabled("Stepper (Combobox)");

    const char* modeNames[] = {
        "Blended (Smooth)", "Modern LPF (Warm)",
        "Sound Blaster Pro", "Sound Blaster", "AdLib Gold", "Sound Blaster Clone"
    };

    // The preview label shown when combo is closed

    static int lRenderModeInt  = 0;
    if (ImFlux::ValueStepper("##Preset", &lRenderModeInt, modeNames, IM_ARRAYSIZE(modeNames))) {
        //....
    }

    ImGui::Separator();
    ImGui::TextDisabled("FaderVertical");

    const char* labels[] = { "63", "125", "250", "500", "1k", "2k", "4k", "8k", "16k" };
    const float minGain = -12.0f;
    const float maxGain = 12.0f;
    static float gains[9] = {0.f};

    for (int i = 0; i < 9; i++) {
        ImGui::PushID(i);

        // Get current gain from the DSP engine (you may need a getGain method in your EQ class)
        float currentGain = gains[i];

        // Draw the vertical slider
        ImGui::BeginGroup();


        // if (ImGui::VSliderFloat("##v", ImVec2(sliderWidth, sliderHeight), &currentGain, minGain, maxGain, "")) {
        if (ImFlux::FaderVertical("##v", ImVec2(30.f, 100.f), &currentGain, minGain, maxGain)) {
            gains[i] = currentGain;
        }
        ImFlux::FaderButton("ON", ImVec2(30.f, 18.f));

        // Frequency label centered under slider
        float textWidth = ImGui::CalcTextSize(labels[i]).x;
        ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (30.f - textWidth) * 0.5f);
        ImGui::TextUnformatted(labels[i]);
        ImGui::EndGroup();

        if (i < 8) ImGui::SameLine(0, 12.f); // Spacing between sliders

        ImGui::PopID();
    }


    ImGui::Separator();
    ImGui::TextDisabled("FaderVertical2");

    for (int i = 0; i < 9; i++) {
        ImGui::PushID( (i + 1) * 10);
        std::string tmpStr = std::format("Gain #{:02}",i);
        FaderVertical2(tmpStr.c_str(), ImVec2(30.f,100.f), &gains[i], minGain, maxGain, "%4.2f") ;
        ImGui::SameLine();
        ImGui::PopID();
    }

    ImGui::NewLine();
    ImGui::Separator();
    ImGui::TextDisabled("FaderHorizontal");
    FaderHorizontal("Gain FaderHorizontal", ImVec2(100.f,24.f), &gains[0], minGain, maxGain) ;

    ImGui::Separator();
    ImGui::TextDisabled("FaderHWithText");
    FaderHWithText("Gain FaderHWithText", &gains[1], minGain, maxGain,"%4.2f") ;

}


inline void ShowCase_LCD_Full()
{
    // ImGui::Begin("LCD Display Showcase");

    ImU32 lcdColor = Color4FImU32(cl_Coral);
    float lcdHeight = 26.0f;

    // --- Row 1: The Full Alphabet ---
    ImGui::TextDisabled("ALPHABET");
    ImFlux::LCDText("ABCDEFGHIJKLMNOPQRSTUVWXYZ", 26, lcdHeight, lcdColor, false);

    ImGui::Spacing();

    // --- Row 2: Numbers & Symbols ---
    ImGui::TextDisabled("NUMBERS & SYMBOLS");
    ImFlux::LCDText("0123456789 +-/*=\\|<> !?_[]", 26, lcdHeight, lcdColor, false);

    ImGui::Spacing();

    // --- Row 3: Your Test Case (Scrolling) ---
    ImGui::TextDisabled("SCROLLING TEST");
    ImFlux::LCDText("OhmFlux Engine ImFlux Widgets Showcase :)", 15, 32.0f, lcdColor, true, 3.0f);

    ImGui::Spacing();

    // --- Row 4: Comparison with LCDNumber ---
    ImGui::TextDisabled("NUMERIC LCD (FOR ALIGNMENT CHECK)");
    static float testVal = 123.45f;
    testVal += 0.01f;
    ImFlux::LCDNumber(testVal, 5, 2, 32.0f, lcdColor);

    // ImGui::End();
}

//---------------------------------------------------------------------------------------

inline void ShowCaseWidgets()
{
   ImGui::Begin("ImFlux ShowCase Widgets");
   //inline void LCDText(std::string text, int display_chars, float height, ImU32 color_on, bool scroll = true, float scroll_speed = 2.0f) {
   ImFlux::LCDText("OhmFlux Engine ImFlux Widgets Showcase :)", 12, 22.f, Color4FImU32(cl_Coral), true, 2.f);

   ImFlux::LCDText("OhmFlux Engine ImFlux Widgets Showcase :)", 12, 22.f, Color4FImU32(cl_Coral), true, 2.f);

   if (ImGui::BeginTabBar("ShowCaseTabs", ImGuiTabBarFlags_AutoSelectNewTabs /*| ImGuiTabBarFlags_Reorderable*/)) {

       if (ImGui::BeginTabItem("Knobs, LCD and more")) {
           ShowCase_MISC();
           ImGui::EndTabItem();
       }
       if (ImGui::BeginTabItem("LCD Text")) {
           ShowCase_LCD_Full();
           ImGui::EndTabItem();
       }



       if (ImGui::BeginTabItem("Stepper and Slider")) {
            ShowCase_StepperAndSlider();
            ImGui::EndTabItem();
       }

       if (ImGui::BeginTabItem("LED's")) {
           ShowCase_LED();
           ImGui::EndTabItem();
       }

       if (ImGui::BeginTabItem("ButtonFancy")) {
           ShowCase_ButtonFancyGallery();
           ImGui::EndTabItem();
       }




        ImGui::EndTabBar();
   } //BeginTabBar
   ImGui::End();
}
//---------------------------------------------------------------------------------------
};
