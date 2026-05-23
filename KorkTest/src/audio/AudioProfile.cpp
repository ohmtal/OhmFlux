#include "AudioProfile.h"
#include "appMain.h"
#include <platform/platformString.h>


namespace KorkFlux {
    IMPLEMENT_CONOBJECT(AudioProfile);

    bool AudioProfile::onAdd(){
        if (!*mFileName) return false;
        mAudioStream = new FluxAudioStream();
        if (!mAudioStream->LoadFile(mFileName)) return false;
        gMain->queueObject(mAudioStream);
        Log("[info] AudioProfile %d created.", getId());
        return Parent::onAdd();
    }

    void AudioProfile::onRemove(){
        gMain->unQueueObject(mAudioStream);
        Parent::onRemove();
    }

    void AudioProfile::initPersistFields(){
        Parent::initPersistFields();
        addGroup("Audio");
        addField("filename", TypeString, Offset(mFileName, AudioProfile));
        // fixme more ... :P we dont use description
        endGroup("Audio");
    }



    // --------------------------------------------------
    // alx compat

    ConsoleFunction(alxPlay, ConsoleBool, 2, 2, "AudioProfile") {
        AudioProfile* profile = dynamic_cast<AudioProfile*>(Sim::findObject(argv[1]));

        if (!profile || !profile->mAudioStream->IsInitialized()) return false;
        return profile->mAudioStream->play();
    }

    ConsoleFunction(alxStop, ConsoleBool, 2, 2, "AudioProfile") {
        AudioProfile* profile = dynamic_cast<AudioProfile*>(Sim::findObject(argv[1]));

        if (!profile || !profile->mAudioStream->IsInitialized()) return false;
        return profile->mAudioStream->stop();
    }
    ConsoleFunction(alxPause, ConsoleBool, 2, 2, "AudioProfile") {
        AudioProfile* profile = dynamic_cast<AudioProfile*>(Sim::findObject(argv[1]));

        if (!profile || !profile->mAudioStream->IsInitialized()) return false;
        return profile->mAudioStream->stop();
    }
    ConsoleFunction(alxUnPause, ConsoleBool, 2, 2, "AudioProfile") {
        AudioProfile* profile = dynamic_cast<AudioProfile*>(Sim::findObject(argv[1]));
        if (!profile || !profile->mAudioStream->IsInitialized()) return false;
        return profile->mAudioStream->resume();
    }

    ConsoleFunction(alxStopAll, void, 1, 1, "") {
        for (auto& obj:  gMain->getQueueObjects() ) {
            AudioProfile* profile = dynamic_cast<AudioProfile*>(obj);
            if (profile && profile->mAudioStream) profile->mAudioStream->stop();
        }
    }

    //     void alxPauseAll();
    //     void alxUnPauseAll();
    //     void alxStopAll();


}


