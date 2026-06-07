#include "AudioProfile.h"
#include "appMain.h"
#include <platform/platformString.h>


namespace ElfFlux {
    IMPLEMENT_CONOBJECT(AudioProfile);

    bool AudioProfile::onAdd(){
        if (!*mFileName) return false;
        // mAudioStream = new FluxAudioStream();
        if (!mAudioStream.LoadFile(mFileName)) return false;
        gMain->queueObject(&mAudioStream);
        Log("[info] AudioProfile %d created. (file:%s, looping:%d)", getId(), mFileName, mAudioStream.mLooping);
        return Parent::onAdd();
    }

    void AudioProfile::onRemove(){
        gMain->unQueueObject(&mAudioStream);
        Parent::onRemove();
    }

    void AudioProfile::initPersistFields(){
        Parent::initPersistFields();
        addGroup("Audio");
        addField("filename", TypeString, Offset(mFileName, AudioProfile));
        addField("Volume", TypeF32, Offset(mAudioStream.mGain, AudioProfile));
        addField("looping", TypeBool, Offset(mAudioStream.mLooping, AudioProfile));
        // usePosition will become better when i finished the the audio system with panning
        addField("usePosition", TypeBool, Offset(mAudioStream.mUsePosition, AudioProfile));
        addField("x", TypeF32, Offset(mAudioStream.mPosition.x, AudioProfile));
        addField("y", TypeF32, Offset(mAudioStream.mPosition.y, AudioProfile));
        addField("z", TypeF32, Offset(mAudioStream.mPosition.z, AudioProfile));

        endGroup("Audio");
    }



    // --------------------------------------------------
    // alx compat

    // return the id of the profile
    ConsoleFunction(alxPlay, ConsoleInt, 2, 2, "AudioProfile") {
        AudioProfile* profile = dynamic_cast<AudioProfile*>(Sim::findObject(argv[1]));

        if (!profile || !profile->mAudioStream.IsInitialized()) return false;
        if (profile->mAudioStream.play()) return profile->getId();

        return 0;
    }

    ConsoleFunction(alxIsPlaying, ConsoleBool, 2, 2, "AudioProfile") {
        AudioProfile* profile = dynamic_cast<AudioProfile*>(Sim::findObject(argv[1]));

        if (!profile || !profile->mAudioStream.IsInitialized()) return false;
        return profile->mAudioStream.isPlaying();
    }


    ConsoleFunction(alxStop, ConsoleBool, 2, 2, "AudioProfile") {
        AudioProfile* profile = dynamic_cast<AudioProfile*>(Sim::findObject(argv[1]));

        if (!profile || !profile->mAudioStream.IsInitialized()) return false;
        return profile->mAudioStream.stop();
    }
    ConsoleFunction(alxPause, ConsoleBool, 2, 2, "AudioProfile") {
        AudioProfile* profile = dynamic_cast<AudioProfile*>(Sim::findObject(argv[1]));

        if (!profile || !profile->mAudioStream.IsInitialized()) return false;
        return profile->mAudioStream.stop();
    }
    ConsoleFunction(alxUnPause, ConsoleBool, 2, 2, "AudioProfile") {
        AudioProfile* profile = dynamic_cast<AudioProfile*>(Sim::findObject(argv[1]));
        if (!profile || !profile->mAudioStream.IsInitialized()) return false;
        return profile->mAudioStream.resume();
    }

    ConsoleFunction(alxStopAll, void, 1, 1, "") {
        for (auto& obj:  gMain->getQueueObjects() ) {
            FluxAudioStream* as = dynamic_cast<FluxAudioStream*>(obj);
            if (as) {
                as->stop();
            }
        }
    }

    //same as stop
    ConsoleFunction(alxPauseAll, void, 1, 1, "") {
        for (auto& obj:  gMain->getQueueObjects() ) {
            FluxAudioStream* as = dynamic_cast<FluxAudioStream*>(obj);
            if (as) {
                as->stop();
            }
        }
    }
    ConsoleFunction(alxUnPauseAll, void, 1, 1, "") {
        for (auto& obj:  gMain->getQueueObjects() ) {
            FluxAudioStream* as = dynamic_cast<FluxAudioStream*>(obj);
            if (as) {
                as->resume();
            }
        }
    }



}


