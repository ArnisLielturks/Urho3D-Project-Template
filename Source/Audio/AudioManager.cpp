#include <Urho3D/Urho3DAll.h>
#include "AudioManager.h"
#include "../MyEvents.h"
#include "AudioManagerDefs.h"

/// Construct.
AudioManager::AudioManager(Context* context) :
    Object(context),
    _multipleMusicTracks(true),
    _multipleAmbientTracks(true)
{
    Init();
}

AudioManager::~AudioManager()
{
}

void AudioManager::Init()
{
    using namespace AudioDefs;
    _soundEffects[SOUND_EFFECTS::HIT] = "Sounds/PlayerFistHit.wav";
    _soundEffects[SOUND_EFFECTS::THROW] = "Sounds/NutThrow.wav";
    _soundEffects[SOUND_EFFECTS::BUTTON_CLICK] = "Sounds/click.wav";
    _soundEffects[SOUND_EFFECTS::ACHIEVEMENT] = "Sounds/achievement.wav";

    _music[MUSIC::GAME] = "Sounds/music.wav";
    _music[MUSIC::MENU] = "Sounds/menu.wav";

    _ambientSounds[AMBIENT_SOUNDS::LEVEL] = "Sounds/ambient.wav";

    SubscribeToEvents();
}

void AudioManager::SubscribeToEvents()
{
    SubscribeToEvent(MyEvents::E_PLAY_SOUND, URHO3D_HANDLER(AudioManager, HandlePlaySound));
    SubscribeToEvent(MyEvents::E_STOP_SOUND, URHO3D_HANDLER(AudioManager, HandleStopSound));
    SubscribeToEvent(MyEvents::E_STOP_ALL_SOUNDS, URHO3D_HANDLER(AudioManager, HandleStopAllSounds));

    SubscribeToEvent(E_RELEASED, URHO3D_HANDLER(AudioManager, HandleButtonClick));
    SubscribeToEvent(E_ITEMSELECTED, URHO3D_HANDLER(AudioManager, HandleButtonClick));
    SubscribeToEvent(E_TOGGLED, URHO3D_HANDLER(AudioManager, HandleButtonClick));

    SubscribeToEvent("ConsolePlaySound", URHO3D_HANDLER(AudioManager, HandleConsolePlaySound));
	SubscribeConsoleCommands();
}

void AudioManager::SubscribeConsoleCommands()
{
    using namespace MyEvents::ConsoleCommandAdd;

	VariantMap data = GetEventDataMap();
	data[P_NAME] = "play_sound";
	data[P_EVENT] = "ConsolePlaySound";
	data[P_DESCRIPTION] = "Play sound effect";
	SendEvent(MyEvents::E_CONSOLE_COMMAND_ADD, data);
}

void AudioManager::HandlePlaySound(StringHash eventType, VariantMap& eventData)
{
    using namespace MyEvents::PlaySound;
    int index = -1;
    if (eventData.Contains(P_INDEX)) {
        index = eventData[P_INDEX].GetInt();
    }
    String type = eventData[P_TYPE].GetString();

    String filename;
    if (index >= 0) {
        if (type == SOUND_EFFECT) {
            filename = _soundEffects[index];
        }
        if (type == SOUND_MASTER) {
            // filename = _soundEffects[index];
        }
        if (type == SOUND_AMBIENT) {
            filename = _ambientSounds[index];
        }
        if (type == SOUND_VOICE) {
            // filename = _soundEffects[index];
        }
        if (type == SOUND_MUSIC) {
            filename = _music[index];
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
    if (type == SOUND_EFFECT && _effectsTimer.Contains(filenameHash) && _effectsTimer[filename].GetMSec(false) < 10) {
        // Safeguard to disable same sound effect overlapping
        return;
    }
    _effectsTimer[filename].Reset();

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
                if (!_multipleMusicTracks) {
                    _musicNodes.Clear();
                }
                _musicNodes[index] = node;
            }
            if (type == SOUND_AMBIENT) {
                if (!_multipleAmbientTracks) {
                    _ambientNodes.Clear();
                }
                _ambientNodes[index] = node;
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
    using namespace MyEvents::StopSound;
    int index = eventData[P_INDEX].GetInt();
    String type = eventData[P_TYPE].GetString();

    if (type == SOUND_EFFECT) {

    }
    if (type == SOUND_MASTER) {

    }
    if (type == SOUND_AMBIENT) {
        // Disable only specific music
        if (_ambientNodes[index]) {
            _ambientNodes.Erase(index);
        }

        // Disable all music
        if (index == -1) {
            _ambientNodes.Clear();
        }
    }
    if (type == SOUND_VOICE) {

    }
    if (type == SOUND_MUSIC) {
        // Disable only specific music
        if (_musicNodes[index]) {
            _musicNodes.Erase(index);
        }

        // Disable all music
        if (index == -1) {
            _musicNodes.Clear();
        }
    }
}

void AudioManager::AllowMultipleMusicTracks(bool enabled)
{
    _multipleMusicTracks = enabled;
}

void AudioManager::AllowMultipleAmbientTracks(bool enabled)
{
    _multipleAmbientTracks = enabled;
}

void AudioManager::HandleStopAllSounds(StringHash eventType, VariantMap& eventData)
{
    _musicNodes.Clear();
    _ambientNodes.Clear();
}

void AudioManager::HandleButtonClick(StringHash eventType, VariantMap& eventData)
{
    using namespace AudioDefs;
    using namespace MyEvents::PlaySound;
    VariantMap data = GetEventDataMap();
    data[P_INDEX] = SOUND_EFFECTS::BUTTON_CLICK;
    data[P_TYPE] = SOUND_EFFECT;
    SendEvent(MyEvents::E_PLAY_SOUND, data);
}

SoundSource3D* AudioManager::AddEffectToNode(Node* node, unsigned int index)
{
    return CreateNodeSound(node, _soundEffects[index], SOUND_EFFECT);
}

SoundSource3D* AudioManager::AddMusicToNode(Node* node, unsigned int index)
{
    return CreateNodeSound(node, _music[index], SOUND_MUSIC);
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