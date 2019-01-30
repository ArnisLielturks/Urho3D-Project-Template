#pragma once

#include <Urho3D/Urho3DAll.h>

using namespace Urho3D;

class AudioManager : public Object
{
    URHO3D_OBJECT(AudioManager, Object);

public:
    /// Construct.
    AudioManager(Context* context);

    virtual ~AudioManager();

    void Create();

    void Dispose();

    /**
     * When enabled, multiple music tracks can be played simultaneously
     * If disabled, previous music track will be stopped when new music
     * starts to play
     */
    void AllowMultipleMusicTracks(bool enabled);

    /**
     * When enabled, multiple ambient tracks can be played simultaneously
     * If disabled, previous ambient track will be stopped when new ambient sound
     * starts to play
     */
    void AllowMultipleAmbientTracks(bool enabled);

    /**
     * Add sound effect to specific node
     */
    SoundSource3D* AddEffectToNode(Node* node, unsigned int index);

    /**
     * Add music to specific node
     */
    SoundSource3D* AddMusicToNode(Node* node, unsigned int index);

protected:
    virtual void Init();

private:

    SoundSource3D* CreateNodeSound(Node* node, const String& filename, const String& type);
    void SubscribeToEvents();
	void SubscribeConsoleCommands();

	/**
	 * Listen for any button clicks
	 */
    void HandleButtonClick(StringHash eventType, VariantMap& eventData);

    /**
     * Handle E_PLAY_SOUND event
     */
    void HandlePlaySound(StringHash eventType, VariantMap& eventData);

    /**
     * Handle E_STOP_SOUND event
     */
    void HandleStopSound(StringHash eventType, VariantMap& eventData);

    /**
     * Handle E_STOP_ALL_SOUNDS event
     */
    void HandleStopAllSounds(StringHash eventType, VariantMap& eventData);

    /**
     * Handle "ConsolePlaySound" event
     */
    void HandleConsolePlaySound(StringHash eventType, VariantMap& eventData);

    /**
     * Play the actual sound with configured settings
     */
    void PlaySound(String filename, String type = "Effect", int index = -1);

    /**
     * All sound effects map
     */
    HashMap<unsigned int, String> _soundEffects;

    /**
     * All music tracks map
     */
    HashMap<unsigned int, String> _music;

    /**
     * All active music track nodes
     */
    HashMap<int, SharedPtr<Node>> _musicNodes;

    /**
     * All ambient sound tracks map
     */
    HashMap<unsigned int, String> _ambientSounds;

    /**
     * All active ambient sound nodes
     */
    HashMap<int, SharedPtr<Node>> _ambientNodes;

    /**
     * Allow multiple music tracks to play
     */
    bool _multipleMusicTracks;

    /**
     * Allow multiple ambient tracks to play
     */
    bool _multipleAmbientTracks;

    HashMap<StringHash, Timer> _effectsTimer;
};