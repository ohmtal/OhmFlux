//-----------------------------------------------------------------------------
// Copyright (c) 2026 Thomas Hühn (XXTH)
// SPDX-License-Identifier: MIT
//-----------------------------------------------------------------------------
// Effect Render for Backgrounds
//-----------------------------------------------------------------------------
#pragma once

#include <string>

#include "core/fluxGlue.h"
#include "core/fluxBaseObject.h"
#include "render/fluxShader.h"
#include "utils/fluxFile.h"
#include "DSP_SpectrumAnalyzer.h"

namespace IronTuner {

    class BackGroundEffects : public FluxBaseObject{
    private:
         FluxShader* mShader = nullptr;

         float mTotalTime = 0.0f;
         float mRmsL = 0.0f;
         float mRmsR = 0.0f;
         GLuint mVAO = 0, mVBO = 0;



         static constexpr int mFreqCount = 32; //MAX 32!

         DSP::SpectrumAnalyzer* mAnalyzer = nullptr;
         std::vector<float> mSmoothedMags;

         std::string mShaderPath = "";
         const std::string mVertShaderFile = "quad.vert";
         const std::vector<std::string> mFragShaderFiles = {
             "glowAndBars.frag"         // 0
             , "liquidTerrain.frag"     // 1
             , "rain.frag"              // 2
             , "rainGlow.frag"          // 3
             , "glow.frag"              // 4
             , "glowLightning.frag"     // 5
             , "bars.frag"              // 6
             , "liquidAura.frag"        // 7
             , "liquidAura2.frag"        // 8

             , "test.frag" //9
        };





    public:
        //------------------
        // shader options:
        bool mScanLines = false;

        bool mShaderESTesting = false;
        //-------------------
        const std::vector<std::string> mFragShaderCaptions = {
            "Glow and Bars"         // 0
            , "Liquid Pulse"        // 1
            , "Rain"                // 2
            , "Rain and Glow"       // 3
            , "Glow and Bars II"    // 4
            , "Glow and Lightning"  // 5
            , "Bars"                // 6
            , "Liquid Aura"         // 7
            , "Liquid Aura II"         // 8

            , "TEST!"
        };




        BackGroundEffects() = default;
        ~BackGroundEffects() = default;
        //----------------------------------------------------------------------
        bool LoadShader(uint8_t fragShaderId = 0, bool enableScanLines = false) {
            FluxFile textFile;

            if (fragShaderId >= mFragShaderFiles.size() ) fragShaderId = 0;

            std::string fragSrc = "";
            std::string vertSrc = "";
            if (!textFile.LoadTextFile(mShaderPath+mFragShaderFiles[fragShaderId], fragSrc)) {
                Log("[error] failed to load Fragment Shader!! %s", SDL_GetError());
                return false;
            }
            if (!textFile.LoadTextFile(mShaderPath+mVertShaderFile , vertSrc)) {
                Log("[error] failed to load Fragment Shader!! %s", SDL_GetError());
                return false;
            }

            if (isAndroidBuild() || mShaderESTesting) {
                fragSrc = "#version 300 es\n" + fragSrc;
                vertSrc = "#version 300 es\n" + vertSrc;
            } else {
                fragSrc = "#version 330 core\n" + fragSrc;
                vertSrc = "#version 330 core\n" + vertSrc;
            }

            mShader = new FluxShader();
            if (!mShader->load(vertSrc.c_str(), fragSrc.c_str())) {
                Log("[error] failed to compile Shaders!!");
                return false;
            }

            mScanLines = enableScanLines;

            Log("Background: Fragment Shader %s loaded. Scanlines: %d", mFragShaderFiles[fragShaderId].c_str(), (int)mScanLines);

            return true;
        }
        //----------------------------------------------------------------------
        virtual bool Initialize() override {
            mShaderPath = getGamePath()+"/assets/shader/";

            if (!LoadShader()) return false;
            dLog("[info] BackGroundEffects initialized.");

            float vertices[] = {
                -1.0f,  1.0f, 0.0f,
                -1.0f, -1.0f, 0.0f,
                1.0f,  1.0f, 0.0f,
                1.0f, -1.0f, 0.0f
            };

            glGenVertexArrays(1, &mVAO);
            glGenBuffers(1, &mVBO);
            glBindVertexArray(mVAO);
            glBindBuffer(GL_ARRAY_BUFFER, mVBO);
            glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 3 * sizeof(float), (void*)0);
            glEnableVertexAttribArray(0);
            glBindVertexArray(0);
            return true;

        }
        //----------------------------------------------------------------------
        void setAnalyzer(DSP::SpectrumAnalyzer* analyzer) {
            mAnalyzer = analyzer;
            //default 512! mAnalyzer->setFFTSize(256); //lower load worse fft
        }
        //----------------------------------------------------------------------
        virtual void Deinitialize() override{

            if (mVAO) glDeleteVertexArrays(1, &mVAO);
            if (mVBO) glDeleteBuffers(1, &mVBO);
            SAFE_DELETE(mShader);
        }
        //----------------------------------------------------------------------
        void UpdateLevels(const double& dt, const Point2F audioLevels)  {
            mTotalTime += static_cast<float>(dt / 1000.f );
            mRmsL = audioLevels.x;
            mRmsR = audioLevels.y;



            if ( mAnalyzer )
            {
                auto currentBands = mAnalyzer->getLogarithmicBands(mFreqCount, true, 0.5f);
                if (!currentBands.empty()) {
                    // Initialize smoothed vector if needed
                    if (mSmoothedMags.size() != currentBands.size()) {
                        mSmoothedMags = currentBands;
                    }
                    // lower first bar bass;
                    if (mFreqCount == 16) currentBands[0] *= 0.65f;

                    // Ballistics constants
                    constexpr float attack = 0.2f;  //0.8 How fast it rises (0.0 to 1.0)
                    constexpr float decay = 0.96f;  //0.92 How slow it falls (0.0 to 1.0)

                    for (size_t i = 0; i < mSmoothedMags.size(); ++i) {
                        // If new value is higher, rise quickly (Attack)


                        if (currentBands[i] > mSmoothedMags[i]) {
                            mSmoothedMags[i] = (mSmoothedMags[i] * (1.0f - attack)) + (currentBands[i] * attack);
                        } else {
                            // If new value is lower, fall slowly (Decay)
                            mSmoothedMags[i] *= decay;
                        }

                        // Prevent denormals / tiny values
                        if (mSmoothedMags[i] < 0.001f) mSmoothedMags[i] = 0.0f;
                    }
                }
            }


        }
        //----------------------------------------------------------------------
        virtual void Draw() override {
            if (!mShader) return;

            mShader->use();
            mShader->setFloat("u_time", mTotalTime);
            mShader->setFloat("u_rmsL", mRmsL);
            mShader->setFloat("u_rmsR", mRmsR);
            Point2I size =  getScreenObject()->getRealScreenSize();
            mShader->setVec2("u_res", (float)size.x, (float)size.y);

            if (mScanLines) {
                mShader->setBool("u_scanlines", mScanLines);
            }


            mShader->setFloat("u_freqCount", mFreqCount);
            if (mAnalyzer && !mSmoothedMags.empty()) {

                mShader->setFloatArray("u_freqs", mSmoothedMags.data(), mFreqCount);
            }



            glDisable(GL_DEPTH_TEST);
            glBindVertexArray(mVAO);
            glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
            glBindVertexArray(0);
            glEnable(GL_DEPTH_TEST);
        }
        //----------------------------------------------------------------------

    }; //Class
}; //Namespace

