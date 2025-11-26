/**
 * @file AudioEngine.hpp
 * @author Fauzan(100%)
 * @brief Interface for the FMOD-based audio system
 *
 * Defines the AudioImplementation class for managing 2D/3D audio,
 * including sound events, parameter control, channel playback, and listener configuration.
 */

#ifndef AUDIOENGINE_HPP
#define AUDIOENGINE_HPP

#include <string>
#include <map>
#include <vector>
#include <iostream>
#include <cmath>
#include "fmod_studio.hpp"
#include "fmod.hpp"

using std::string;
using std::vector;

/**
 * @brief Simple 3D vector structure for audio positioning
 */
struct Vector3 {
    float x; ///< X coordinate
    float y; ///< Y coordinate
    float z; ///< Z coordinate
};

/**
 * @class AudioImplementation
 * @brief Internal implementation class for FMOD audio system
 *
 * This class manages the core FMOD systems, sounds, channels, banks, and events.
 * It is not meant to be used directly; use DKAudioEngine instead.
 */
class AudioImplementation {
public:
    /**
     * @brief Construct a new Audio Implementation object
     *
     * Initializes FMOD Studio System and core system
     */
    AudioImplementation();

    /**
     * @brief Destroy the Audio Implementation object
     *
     * Unloads all banks and releases FMOD systems
     */
    ~AudioImplementation();

    /**
     * @brief Update the audio system
     *
     * Should be called every frame to update FMOD and clean up stopped channels
     */
    void Update();

    FMOD::Studio::System* mpStudioSystem;  ///< FMOD Studio system instance
    FMOD::System* mpSystem;                ///< FMOD core system instance

    int mnNextChannelId;                   ///< Counter for generating unique channel IDs

    typedef std::map<string, FMOD::Sound*> SoundMap;
    typedef std::map<int, FMOD::Channel*> ChannelMap;
    typedef std::map<string, FMOD::Studio::EventInstance*> EventMap;
    typedef std::map<string, FMOD::Studio::Bank*> BankMap;

    SoundMap mSounds;    ///< Map of loaded sounds by name
    ChannelMap mChannels;///< Map of active channels by ID
    EventMap mEvents;    ///< Map of FMOD Studio events by name
    BankMap mBanks;      ///< Map of loaded FMOD Studio banks by name
};

/**
 * @namespace DKAudioEngine
 * @brief Main namespace for audio engine functionality
 *
 * Provides static methods for audio initialization, playback, 3D positioning,
 * and FMOD Studio event management.
 */
namespace DKAudioEngine {
    /**
     * @brief Initialize the audio engine
     *
     * Must be called before using any other audio functions
     */
    void Init();

    /**
     * @brief Update the audio engine
     *
     * Should be called every frame to update FMOD
     */
    void Update();

    /**
     * @brief Shutdown the audio engine
     *
     * Releases all resources and shuts down FMOD
     */
    void Shutdown();

    /**
     * @brief Load a sound file into memory
     *
     * @param strSoundName Path to the sound file
     * @param b3d Whether the sound should be 3D (true) or 2D (false)
     * @param bLooping Whether the sound should loop
     * @param bStream Whether to stream the sound (for large files) or load into memory
     */
    void LoadSound(const std::string& strSoundName, bool b3d = true, bool bLooping = false, bool bStream = false);

    /**
     * @brief Unload a sound from memory
     *
     * @param strSoundName Name/path of the sound to unload
     */
    void UnLoadSound(const std::string& strSoundName);

    /**
     * @brief Play a sound
     *
     * @param strSoundName Name/path of the sound to play
     * @param vPosition 3D position of the sound (only used for 3D sounds)
     * @param fVolumedB Volume in decibels (0 = normal, negative = quieter, positive = louder)
     * @return int Channel ID for controlling the sound playback
     */
    int PlaySounds(const string& strSoundName, const Vector3& vPosition = { 0, 0, 0 }, float fVolumedB = 0.0f);

    /**
     * @brief Set the 3D position of a playing sound channel
     *
     * @param nChannelId Channel ID returned from PlaySounds
     * @param vPosition New 3D position
     */
    void SetChannel3dPosition(int nChannelId, const Vector3& vPosition);

    /**
     * @brief Set the volume of a playing sound channel
     *
     * @param nChannelId Channel ID returned from PlaySounds
     * @param fVolumedB Volume in decibels
     */
    void SetChannelVolume(int nChannelId, float fVolumedB);

    /**
     * @brief Stop a specific channel
     *
     * @param nChannelId Channel ID to stop
     */
    void StopChannel(int nChannelId);

    /**
     * @brief Stop all currently playing channels
     */
    void StopAllChannels();

    /**
     * @brief Check if a channel is currently playing
     *
     * @param nChannelId Channel ID to check
     * @return true if the channel is playing, false otherwise
     */
    bool IsPlaying(int nChannelId);

    /**
     * @brief Load an FMOD Studio bank file
     *
     * @param strBankName Path to the bank file
     * @param flags FMOD Studio load bank flags (default: FMOD_STUDIO_LOAD_BANK_NORMAL)
     */
    void LoadBank(const std::string& strBankName, FMOD_STUDIO_LOAD_BANK_FLAGS flags = FMOD_STUDIO_LOAD_BANK_NORMAL);

    /**
     * @brief Load an FMOD Studio event
     *
     * @param strEventName Event path (e.g., "event:/Music/MainTheme")
     */
    void LoadEvent(const std::string& strEventName);

    /**
     * @brief Play an FMOD Studio event
     *
     * @param strEventName Event path to play
     */
    void PlayEvent(const string& strEventName);

    /**
     * @brief Stop an FMOD Studio event
     *
     * @param strEventName Event path to stop
     * @param bImmediate If true, stop immediately; if false, allow fadeout
     */
    void StopEvent(const string& strEventName, bool bImmediate = false);

    /**
     * @brief Check if an FMOD Studio event is currently playing
     *
     * @param strEventName Event path to check
     * @return true if the event is playing, false otherwise
     */
    bool IsEventPlaying(const string& strEventName);

    /**
     * @brief Get the value of an event parameter
     *
     * @param strEventName Event path
     * @param strParameterName Name of the parameter
     * @param parameter Pointer to receive the parameter value
     */
    void GetEventParameter(const string& strEventName, const string& strParameterName, float* parameter);

    /**
     * @brief Set the value of an event parameter
     *
     * @param strEventName Event path
     * @param strParameterName Name of the parameter
     * @param fValue Value to set
     */
    void SetEventParameter(const string& strEventName, const string& strParameterName, float fValue);

    /**
     * @brief Set the 3D listener position and orientation
     *
     * @param vPos Position of the listener
     * @param fVolumedB Master volume in decibels
     */
    void Set3dListenerAndOrientation(const Vector3& vPos, float fVolumedB = 0.0f);

    /**
     * @brief Convert a Vector3 to FMOD_VECTOR
     *
     * @param vPosition Vector3 to convert
     * @return FMOD_VECTOR Converted vector
     */
    FMOD_VECTOR VectorToFmod(const Vector3& vPosition);

    /**
     * @brief Convert decibels to linear volume
     *
     * @param dB Volume in decibels
     * @return float Linear volume value
     */
    float dbToVolume(float dB);

    /**
     * @brief Convert linear volume to decibels
     *
     * @param volume Linear volume value
     * @return float Volume in decibels
     */
    float VolumeTodB(float volume);

    /**
     * @brief Check FMOD result and print error if failed
     *
     * @param result FMOD_RESULT to check
     * @return int 0 if success, 1 if error
     */
    int ErrorCheck(FMOD_RESULT result);
}

#endif // AUDIOENGINE_HPP