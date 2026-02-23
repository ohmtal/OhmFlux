#pragma once

#include <string>
#include <atomic>

#include <SDL3/SDL.h>
#include <DSP.h>
#include <DSP_EffectsManager.h>
#include <DSP_EffectFactory.h>

#include "drumKitLooper.h"

#include <imgui.h>
#include <gui/ImFlux.h>

#include <core/fluxBaseObject.h>
#include <audio/fluxAudio.h>

class SoundMixModule : public FluxBaseObject {
private:
    std::unique_ptr<DSP::EffectsManager> mDrumManager = nullptr;

    std::atomic<float> mMasterVolume = 1.f;
    bool mInitialized = false;



public:
    SoundMixModule() = default;
    ~SoundMixModule() {
        SDL_SetAudioPostmixCallback(AudioManager.getDeviceID(), NULL, NULL);
    }

    bool Initialize() override;
    bool close();


    void DrawVisualAnalyzer(bool* p_enabled);
    void DrawDrums(bool* p_enabled);


    //lazy add to public:
    std::unique_ptr<DSP::SpectrumAnalyzer> mSpectrumAnalyzer = nullptr;
    std::unique_ptr<DSP::VisualAnalyzer> mVisualAnalyzer = nullptr;

    // master volume
    float getMasterVolume() { return mMasterVolume.load(std::memory_order_relaxed); }
    void setMasterVolume(float vol) { mMasterVolume.store(DSP::clamp(vol, 0.f, 1.f));}


    //--------------------------------------------------------------------------
    // std::vector<std::unique_ptr<DSP::Effect>>& getEffects() const { return mEffectsManager->getEffects();    }

    DSP::EffectsManager* getDrumManager() const {
        return mDrumManager.get();
    }


    //--------------------------------------------------------------------------
    void WriteWavHeader(SDL_IOStream *io, SDL_AudioSpec *spec, Uint32 dataSize) {
        SDL_SeekIO(io, 0, SDL_IO_SEEK_SET);

        SDL_WriteIO(io, "RIFF", 4);
        SDL_WriteU32LE(io, 36 + dataSize); // Dateigröße - 8
        SDL_WriteIO(io, "WAVE", 4);

        SDL_WriteIO(io, "fmt ", 4);
        SDL_WriteU32LE(io, 16);
        SDL_WriteU16LE(io, 3); // 3 = IEEE Float (da Postmix in Float arbeitet)
        SDL_WriteU16LE(io, spec->channels);
        SDL_WriteU32LE(io, spec->freq);

        Uint32 bitsPerSample = SDL_AUDIO_BITSIZE(spec->format);
        SDL_WriteU32LE(io, (spec->freq * spec->channels * bitsPerSample) / 8); // ByteRate
        SDL_WriteU16LE(io, (spec->channels * bitsPerSample) / 8);             // BlockAlign
        SDL_WriteU16LE(io, bitsPerSample);

        SDL_WriteIO(io, "data", 4);
        SDL_WriteU32LE(io, dataSize);
    }

    // NOTE Example code:

    // recording_stream = SDL_CreateAudioStream(&spec, &spec);
    // ------
    // is_recording = true;
    // SDL_Thread *thread = SDL_CreateThread(RecordingThread, "Writer", &spec);
    // ------
    // int RecordingThread(void *ptr) {
    //     SDL_AudioSpec *spec = (SDL_AudioSpec*)ptr;
    //     SDL_IOStream *io = SDL_IOFromFile("output_mastered.wav", "wb");
    //     if (!io) return -1;
    //
    //     WriteWavHeader(io, spec, 0);
    //
    //     float buffer[4096];
    //     while (is_recording || SDL_GetAudioStreamAvailable(recording_stream) > 0) {
    //         int available = SDL_GetAudioStreamAvailable(recording_stream);
    //         if (available > 0) {
    //             int to_read = (available > sizeof(buffer)) ? sizeof(buffer) : available;
    //             int gotten = SDL_GetAudioStreamData(recording_stream, buffer, to_read);
    //             if (gotten > 0) {
    //                 SDL_WriteIO(io, buffer, gotten);
    //                 total_bytes_recorded += gotten;
    //             }
    //         } else {
    //             SDL_Delay(10); // Entlastung
    //         }
    //     }
    //
    //     WriteWavHeader(io, spec, total_bytes_recorded);
    //     SDL_CloseIO(io);
    //     SDL_Log("Aufnahme beendet. %u Bytes geschrieben.", total_bytes_recorded);
    //     return 0;
    // }


};
