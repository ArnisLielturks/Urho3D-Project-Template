#include <Urho3D/Core/Object.h>

#include "AudioManager.h"
#include "AudioManagerDefs.h"
#include "../Console/ConsoleHandlerEvents.h"
#include "AudioEvents.h"

using namespace Urho3D;
using namespace AudioEvents;

AudioManager::AudioManager(Context* context) :
    Object(context)
{
    Init();
}

AudioManager::~AudioManager()
{
}

void AudioManager::Init()
{
    using namespace AudioDefs;
    soundEffects_[SOUND_EFFECTS::HIT] = "Sounds/PlayerFistHit.wav";
    soundEffects_[SOUND_EFFECTS::THROW] = "Sounds/NutThrow.wav";
    soundEffects_[SOUND_EFFECTS::BUTTON_CLICK] = "Sounds/click.wav";
    soundEffects_[SOUND_EFFECTS::ACHIEVEMENT] = "Sounds/achievement.wav";
    soundEffects_[SOUND_EFFECTS::PLACE_BLOCK] = "Sounds/place_block.wav";

    music_[MUSIC::GAME] = "Sounds/music.wav";
    music_[MUSIC::MENU] = "Sounds/menu.wav";

    ambientSounds_[AMBIENT_SOUNDS::LEVEL] = "Sounds/ambient.wav";

    SubscribeToEvents();
}

void AudioManager::SubscribeToEvents()
{
    SubscribeToEvent(E_PLAY_SOUND, URHO3D_HANDLER(AudioManager, HandlePlaySound));
    SubscribeToEvent(E_STOP_SOUND, URHO3D_HANDLER(AudioManager, HandleStopSound));
    SubscribeToEvent(E_STOP_ALL_SOUNDS, URHO3D_HANDLER(AudioManager, HandleStopAllSounds));

    SubscribeToEvent(E_PRESSED, URHO3D_HANDLER(AudioManager, HandleButtonClick));
    SubscribeToEvent(E_ITEMSELECTED, URHO3D_HANDLER(AudioManager, HandleButtonClick));
    SubscribeToEvent(E_TOGGLED, URHO3D_HANDLER(AudioManager, HandleButtonClick));

    SubscribeToEvent("ConsolePlaySound", URHO3D_HANDLER(AudioManager, HandleConsolePlaySound));
    SubscribeConsoleCommands();
}

void AudioManager::SubscribeConsoleCommands()
{
    using namespace ConsoleHandlerEvents::ConsoleCommandAdd;

    VariantMap& data = GetEventDataMap();
    data[P_NAME] = "play_sound";
    data[P_EVENT] = "ConsolePlaySound";
    data[P_DESCRIPTION] = "Play sound effect";
    SendEvent(ConsoleHandlerEvents::E_CONSOLE_COMMAND_ADD, data);
}

void AudioManager::HandlePlaySound(StringHash eventType, VariantMap& eventData)
{
    using namespace PlaySound;
    int index = -1;
    if (eventData.Contains(P_INDEX)) {
        index = eventData[P_INDEX].GetInt();
    }
    String type = eventData[P_TYPE].GetString();

    String filename;
    if (index >= 0) {
        if (type == SOUND_EFFECT) {
            filename = soundEffects_[index];
        }
        if (type == SOUND_MASTER) {
            // filename = soundEffects_[index];
        }
        if (type == SOUND_AMBIENT) {
            filename = ambientSounds_[index];
        }
        if (type == SOUND_VOICE) {
            // filename = soundEffects_[index];
        }
        if (type == SOUND_MUSIC) {
            filename = music_[index];
        }
    } else {
        filename = eventData[P_SOUND_FILE].GetString();
    }
    PlaySound(filename, type, index);
}

void AudioManager::HandleConsolePlaySound(StringHash eventType, VariantMap& eventData)
{
    URHO3D_LOGINFO("Handle console play sound");
    StringVector params = eventData["Parameters"].GetStringVector();
    if (params.Size() == 2) {
        PlaySound(params[1], SOUND_EFFECT);
    }

    else {
        URHO3D_LOGERROR("Invalid number of parameters");
    }
}

void AudioManager::PlaySound(String filename, String type, int index)
{
    //URHO3D_LOGINFO("Playing sound: " + filename + " [" + type + "]");
    StringHash filenameHash(filename);
    if (type == SOUND_EFFECT && effectsTimer_.Contains(filenameHash) && effectsTimer_[filename].GetMSec(false) < 10) {
        // Safeguard to disable same sound effect overlapping
        return;
    }
    effectsTimer_[filename].Reset();

     // Get the sound resource
    auto* cache = GetSubsystem<ResourceCache>();
    auto* sound = cache->GetResource<Sound>(filename);

    if (sound)
    {
        Node* node = new Node(context_);
        // Create a SoundSource component for playing the sound. The SoundSource component plays
        // non-positional audio, so its 3D position in the scene does not matter. For positional sounds the
        // SoundSource3D component would be used instead
        auto* soundSource = node->CreateComponent<SoundSource>();
        if (type == SOUND_EFFECT || type == SOUND_VOICE) {
            // Component will automatically remove itself when the sound finished playing
            soundSource->SetAutoRemoveMode(REMOVE_NODE);
        } else {
            sound->SetLooped(true);
            if (type == SOUND_MUSIC) {
                if (!multipleMusicTracks_) {
                    musicNodes_.Clear();
                }
                musicNodes_[index] = node;
            }
            if (type == SOUND_AMBIENT) {
                if (!multipleMusicTracks_) {
                    ambientNodes_.Clear();
                }
                ambientNodes_[index] = node;
            }
        }

        soundSource->SetSoundType(type);
        soundSource->Play(sound);
        // In case we also play music, set the sound volume below maximum so that we don't clip the output
        //soundSource->SetGain(0.75f);
    }
}

void AudioManager::HandleStopSound(StringHash eventType, VariantMap& eventData)
{
    using namespace StopSound;
    int index = eventData[P_INDEX].GetInt();
    String type = eventData[P_TYPE].GetString();

    if (type == SOUND_EFFECT) {

    }
    if (type == SOUND_MASTER) {

    }
    if (type == SOUND_AMBIENT) {
        // Disable only specific music
        if (ambientNodes_[index]) {
            ambientNodes_.Erase(index);
        }

        // Disable all music
        if (index == -1) {
            ambientNodes_.Clear();
        }
    }
    if (type == SOUND_VOICE) {

    }
    if (type == SOUND_MUSIC) {
        // Disable only specific music
        if (musicNodes_[index]) {
            musicNodes_.Erase(index);
        }

        // Disable all music
        if (index == -1) {
            musicNodes_.Clear();
        }
    }
}

void AudioManager::AllowMultipleMusicTracks(bool enabled)
{
    multipleMusicTracks_ = enabled;
}

void AudioManager::AllowMultipleAmbientTracks(bool enabled)
{
    multipleMusicTracks_ = enabled;
}

void AudioManager::HandleStopAllSounds(StringHash eventType, VariantMap& eventData)
{
    musicNodes_.Clear();
    ambientNodes_.Clear();
}

void AudioManager::HandleButtonClick(StringHash eventType, VariantMap& eventData)
{
    using namespace AudioDefs;
    using namespace PlaySound;
    VariantMap& data = GetEventDataMap();
    data[P_INDEX] = SOUND_EFFECTS::BUTTON_CLICK;
    data[P_TYPE] = SOUND_EFFECT;
    SendEvent(E_PLAY_SOUND, data);
}

SoundSource3D* AudioManager::AddEffectToNode(Node* node, unsigned int index)
{
    return CreateNodeSound(node, soundEffects_[index], SOUND_EFFECT);
}

SoundSource3D* AudioManager::AddMusicToNode(Node* node, unsigned int index)
{
    return CreateNodeSound(node, music_[index], SOUND_MUSIC);
}

SoundSource3D* AudioManager::CreateNodeSound(Node* node, const String& filename, const String& type)
{
    if (filename.Empty()) {
        return nullptr;
    }
    // Get the sound resource
    auto* cache = GetSubsystem<ResourceCache>();
    Sound* sound = cache->GetResource<Sound>(filename);

    if (sound)
    {
        URHO3D_LOGINFOF("Adding sound [%s] to node [%i], type [%s]", filename.CString(), node->GetID(), type.CString());
        auto* soundSource = node->CreateComponent<SoundSource3D>();
        soundSource->SetSoundType(type);
        soundSource->Play(sound);

        return soundSource;
    }

    return nullptr;
}
