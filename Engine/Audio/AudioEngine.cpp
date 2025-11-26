/**
 * @file AudioEngine.cpp
 * @author Fauzan (100%)
 * @brief Implementation of the FMOD-based audio system for 3D sound management
 *
 * Implements the core FMOD Studio system setup, error checking, and
 * functions for loading banks, playing sounds, managing channels, and setting 3D listener properties.
 */

#include "AudioEngine.hpp"
#include "AudioErrorHandler.hpp"

 /**
  * @brief Construct a new Audio Implementation object
  *
  * Initializes the FMOD Studio System with the following steps:
  * 1. Creates the FMOD Studio System
  * 2. Initializes the system with 32 max channels
  * 3. Retrieves the core FMOD system for low-level operations
  */
AudioImplementation::AudioImplementation() {
    mpStudioSystem = NULL;
    mpSystem = NULL;
    mnNextChannelId = 0; // Initialize the channel ID counter

    std::cout << "Creating FMOD Studio System..." << std::endl;
    FMOD_RESULT result = FMOD::Studio::System::create(&mpStudioSystem);
    if (DKAudioEngine::ErrorCheck(result) != 0) {
        std::cout << "Failed to create FMOD Studio System!" << std::endl;
        return;
    }

    std::cout << "Initializing FMOD Studio System..." << std::endl;
    result = mpStudioSystem->initialize(32, FMOD_STUDIO_INIT_NORMAL, FMOD_INIT_NORMAL, NULL);
    if (DKAudioEngine::ErrorCheck(result) != 0) {
        std::cout << "Failed to initialize FMOD Studio System!" << std::endl;
        return;
    }

    std::cout << "Getting core system from studio system..." << std::endl;
    result = mpStudioSystem->getCoreSystem(&mpSystem);
    if (DKAudioEngine::ErrorCheck(result) != 0) {
        std::cout << "Failed to get core system!" << std::endl;
        return;
    }

    std::cout << "FMOD Audio system initialized successfully!" << std::endl;
}

/**
 * @brief Destroy the Audio Implementation object
 *
 * Unloads all FMOD Studio banks and releases the FMOD Studio System
 */
AudioImplementation::~AudioImplementation() {
    DKAudioEngine::ErrorCheck(mpStudioSystem->unloadAll());
    DKAudioEngine::ErrorCheck(mpStudioSystem->release());
}

/**
 * @brief Update the audio system
 *
 * Performs the following tasks each frame:
 * 1. Checks all active channels for stopped sounds
 * 2. Removes stopped channels from the active channel map
 * 3. Updates the FMOD Studio System
 */
void AudioImplementation::Update() {
    vector<ChannelMap::iterator> pStoppedChannels;
    for (auto it = mChannels.begin(), itEnd = mChannels.end(); it != itEnd; ++it)
    {
        bool bIsPlaying = false;
        it->second->isPlaying(&bIsPlaying);
        if (!bIsPlaying)
        {
            pStoppedChannels.push_back(it);
        }
    }
    for (auto& it : pStoppedChannels)
    {
        mChannels.erase(it);
    }
    DKAudioEngine::ErrorCheck(mpStudioSystem->update());
}

/// Global pointer to the audio implementation singleton
AudioImplementation* sgpImplementation = nullptr;

/**
 * @brief Initialize the audio engine
 *
 * Creates the AudioImplementation singleton instance
 */
void DKAudioEngine::Init() {
    sgpImplementation = new AudioImplementation;

    // Initialize error handler with supported formats
    std::vector<std::string> supportedFormats = { ".ogg", ".mp3", ".wav" };
    AudioErrorHandler::Initialize(supportedFormats);
    std::cout << "AudioErrorHandler initialized" << std::endl;
}

/**
 * @brief Update the audio engine
 *
 * Calls the update method on the implementation to process audio system updates
 */
void DKAudioEngine::Update() {
    if (sgpImplementation) {
        sgpImplementation->Update();
    }
}

/**
 * @brief Load a sound file into memory
 *
 * @param strSoundName Path to the sound file to load
 * @param b3d If true, sound is loaded as 3D; if false, as 2D
 * @param bLooping If true, sound will loop continuously
 * @param bStream If true, sound is streamed from disk; if false, loaded into memory
 *
 * The function performs validation checks and configures FMOD mode flags based on parameters.
 * If the sound is already loaded, the function returns early.
 */
void DKAudioEngine::LoadSound(const std::string& strSoundName, bool b3d, bool bLooping, bool bStream)
{
    if (!sgpImplementation) {
        std::cout << "ERROR: FMOD not initialized!" << std::endl;
        AudioErrorHandler::ShowErrorPopup(
            AudioErrorHandler::ErrorType::ENGINE_NOT_INITIALIZED,
            strSoundName
        );
        return;
    }

    if (!sgpImplementation->mpSystem) {
        std::cout << "ERROR: FMOD core system is null!" << std::endl;
        AudioErrorHandler::ShowErrorPopup(
            AudioErrorHandler::ErrorType::ENGINE_NOT_INITIALIZED,
            strSoundName
        );
        return;
    }

    AudioErrorHandler::ErrorType validationResult =
        AudioErrorHandler::ValidateAudioFile(strSoundName);

    if (validationResult != AudioErrorHandler::ErrorType::UNKNOWN_ERROR) {
        AudioErrorHandler::ShowErrorPopup(validationResult, strSoundName);
        std::cout << "ERROR: Audio file validation failed: " << strSoundName << std::endl;
        return;  // Don't try to load invalid file
    }

    // Check if already loaded
    auto tFoundIt = sgpImplementation->mSounds.find(strSoundName);
    if (tFoundIt != sgpImplementation->mSounds.end()) {
        std::cout << "Sound already loaded: " << strSoundName << std::endl;
        return;
    }

    std::cout << "Loading sound: " << strSoundName << std::endl;
    std::cout << "Parameters - 3D: " << b3d << ", Looping: " << bLooping << ", Stream: " << bStream << std::endl;

    FMOD_MODE eMode = FMOD_DEFAULT;
    eMode |= b3d ? FMOD_3D : FMOD_2D;
    eMode |= bLooping ? FMOD_LOOP_NORMAL : FMOD_LOOP_OFF;
    eMode |= bStream ? FMOD_CREATESTREAM : FMOD_CREATECOMPRESSEDSAMPLE;

    std::cout << "FMOD Mode flags: " << eMode << std::endl;

    FMOD::Sound* pSound = nullptr;
    std::cout << "Calling createSound..." << std::endl;
    FMOD_RESULT result = sgpImplementation->mpSystem->createSound(strSoundName.c_str(), eMode, nullptr, &pSound);

    if (DKAudioEngine::ErrorCheck(result) == 0 && pSound) {
        sgpImplementation->mSounds[strSoundName] = pSound;
        std::cout << "Sound loaded successfully!" << std::endl;
    }
    else {
        std::cout << "Failed to load sound!" << std::endl;
        // Show error popup for FMOD errors
        std::string errorInfo = "FMOD Error Code: " + std::to_string(static_cast<int>(result));
        AudioErrorHandler::ShowErrorPopup(
            AudioErrorHandler::ErrorType::FILE_CORRUPTED,
            strSoundName,
            errorInfo
        );
    }
}


/**
 * @brief Unload a sound from memory
 *
 * @param strSoundName Name/path of the sound to unload
 *
 * Releases the FMOD sound resource and removes it from the sound map
 */
void DKAudioEngine::UnLoadSound(const std::string& strSoundName)
{
    // Safety check: if audio engine is already shut down, return early
    if (!sgpImplementation) {
        return;
    }

    auto tFoundIt = sgpImplementation->mSounds.find(strSoundName);
    if (tFoundIt == sgpImplementation->mSounds.end())
        return;

    DKAudioEngine::ErrorCheck(tFoundIt->second->release());
    sgpImplementation->mSounds.erase(tFoundIt);
}

/**
 * @brief Play a sound and return a channel ID for control
 *
 * @param strSoundName Name/path of the sound to play
 * @param vPosition 3D world position (only used if sound is 3D)
 * @param fVolumedB Volume in decibels (0 = normal volume)
 * @return int Unique channel ID for controlling this sound instance
 *
 * If the sound is not already loaded, it will be loaded automatically.
 * The sound starts paused and is unpaused after all attributes are set.
 */
int DKAudioEngine::PlaySounds(const string& strSoundName, const Vector3& vPosition, float fVolumedB)
{
    // Safety check: if audio engine is already shut down, return invalid ID
    if (!sgpImplementation) {
        return -1;
    }

    int nChannelId = sgpImplementation->mnNextChannelId++;
    auto tFoundIt = sgpImplementation->mSounds.find(strSoundName);
    if (tFoundIt == sgpImplementation->mSounds.end())
    {
        LoadSound(strSoundName);
        tFoundIt = sgpImplementation->mSounds.find(strSoundName);
        if (tFoundIt == sgpImplementation->mSounds.end())
        {
            return nChannelId;
        }
    }

    // Safety check: ensure sound pointer is valid before using it
    if (!tFoundIt->second)
    {
        std::cout << "ERROR: Sound pointer is null for: " << strSoundName << std::endl;
        return nChannelId;
    }

    FMOD::Channel* pChannel = nullptr;
    DKAudioEngine::ErrorCheck(sgpImplementation->mpSystem->playSound(tFoundIt->second, nullptr, true, &pChannel));
    if (pChannel)
    {
        FMOD_MODE currMode;
        tFoundIt->second->getMode(&currMode);
        if (currMode & FMOD_3D) {
            FMOD_VECTOR position = VectorToFmod(vPosition);
            DKAudioEngine::ErrorCheck(pChannel->set3DAttributes(&position, nullptr));
        }
        DKAudioEngine::ErrorCheck(pChannel->setVolume(dbToVolume(fVolumedB)));
        DKAudioEngine::ErrorCheck(pChannel->setPaused(false));
        sgpImplementation->mChannels[nChannelId] = pChannel;
    }
    return nChannelId;
}

/**
 * @brief Set the 3D position of an active sound channel
 *
 * @param nChannelId Channel ID returned from PlaySounds
 * @param vPosition New 3D world position
 *
 * This function only affects 3D sounds. 2D sounds will ignore position changes.
 */
void DKAudioEngine::SetChannel3dPosition(int nChannelId, const Vector3& vPosition)
{
    // Safety check: if audio engine is already shut down, return early
    if (!sgpImplementation) {
        return;
    }

    auto tFoundIt = sgpImplementation->mChannels.find(nChannelId);
    if (tFoundIt == sgpImplementation->mChannels.end())
        return;

    FMOD_VECTOR position = VectorToFmod(vPosition);
    DKAudioEngine::ErrorCheck(tFoundIt->second->set3DAttributes(&position, NULL));
}

/**
 * @brief Set the volume of an active sound channel
 *
 * @param nChannelId Channel ID returned from PlaySounds
 * @param fVolumedB Volume in decibels (0 = normal, negative = quieter, positive = louder)
 */
void DKAudioEngine::SetChannelVolume(int nChannelId, float fVolumedB)
{
    // Safety check: if audio engine is already shut down, return early
    if (!sgpImplementation) {
        return;
    }

    auto tFoundIt = sgpImplementation->mChannels.find(nChannelId);
    if (tFoundIt == sgpImplementation->mChannels.end())
        return;

    DKAudioEngine::ErrorCheck(tFoundIt->second->setVolume(dbToVolume(fVolumedB)));
}

/**
 * @brief Load an FMOD Studio bank file
 *
 * @param strBankName Path to the .bank file
 * @param flags FMOD Studio load flags (default: FMOD_STUDIO_LOAD_BANK_NORMAL)
 *
 * Banks contain FMOD Studio events and must be loaded before their events can be used.
 * If the bank is already loaded, this function returns early.
 */
void DKAudioEngine::LoadBank(const std::string& strBankName, FMOD_STUDIO_LOAD_BANK_FLAGS flags) {
    auto tFoundIt = sgpImplementation->mBanks.find(strBankName);
    if (tFoundIt != sgpImplementation->mBanks.end())
        return;
    FMOD::Studio::Bank* pBank;
    DKAudioEngine::ErrorCheck(sgpImplementation->mpStudioSystem->loadBankFile(strBankName.c_str(), flags, &pBank));
    if (pBank) {
        sgpImplementation->mBanks[strBankName] = pBank;
    }
}

/**
 * @brief Load an FMOD Studio event
 *
 * @param strEventName Event path (e.g., "event:/Music/MainTheme")
 *
 * Creates an event instance from the event description.
 * The event's bank must be loaded first. If already loaded, returns early.
 */
void DKAudioEngine::LoadEvent(const std::string& strEventName) {
    auto tFoundit = sgpImplementation->mEvents.find(strEventName);
    if (tFoundit != sgpImplementation->mEvents.end())
        return;
    FMOD::Studio::EventDescription* pEventDescription = NULL;
    DKAudioEngine::ErrorCheck(sgpImplementation->mpStudioSystem->getEvent(strEventName.c_str(), &pEventDescription));
    if (pEventDescription) {
        FMOD::Studio::EventInstance* pEventInstance = NULL;
        DKAudioEngine::ErrorCheck(pEventDescription->createInstance(&pEventInstance));
        if (pEventInstance) {
            sgpImplementation->mEvents[strEventName] = pEventInstance;
        }
    }
}

/**
 * @brief Play an FMOD Studio event
 *
 * @param strEventName Event path to play
 *
 * If the event is not loaded, it will be loaded automatically.
 * Starts playback of the event instance.
 */
void DKAudioEngine::PlayEvent(const string& strEventName) {
    auto tFoundit = sgpImplementation->mEvents.find(strEventName);
    if (tFoundit == sgpImplementation->mEvents.end()) {
        LoadEvent(strEventName);
        tFoundit = sgpImplementation->mEvents.find(strEventName);
        if (tFoundit == sgpImplementation->mEvents.end())
            return;
    }
    tFoundit->second->start();
}

/**
 * @brief Stop an FMOD Studio event
 *
 * @param strEventName Event path to stop
 * @param bImmediate If true, stops immediately; if false, allows fadeout
 *
 * The event instance remains loaded and can be restarted.
 */
void DKAudioEngine::StopEvent(const string& strEventName, bool bImmediate) {
    auto tFoundIt = sgpImplementation->mEvents.find(strEventName);
    if (tFoundIt == sgpImplementation->mEvents.end())
        return;

    FMOD_STUDIO_STOP_MODE eMode;
    eMode = bImmediate ? FMOD_STUDIO_STOP_IMMEDIATE : FMOD_STUDIO_STOP_ALLOWFADEOUT;
    DKAudioEngine::ErrorCheck(tFoundIt->second->stop(eMode));
}

/**
 * @brief Check if an FMOD Studio event is currently playing
 *
 * @param strEventName Event path to check
 * @return true if the event is in the playing state
 * @return false if the event is not playing or not loaded
 */
bool DKAudioEngine::IsEventPlaying(const string& strEventName) {
    auto tFoundIt = sgpImplementation->mEvents.find(strEventName);
    if (tFoundIt == sgpImplementation->mEvents.end())
        return false;

    FMOD_STUDIO_PLAYBACK_STATE state;
    DKAudioEngine::ErrorCheck(tFoundIt->second->getPlaybackState(&state));
    return (state == FMOD_STUDIO_PLAYBACK_PLAYING);
}

/**
 * @brief Check if a channel is currently playing
 *
 * @param nChannelId Channel ID to check
 * @return true if the channel is playing
 * @return false if the channel is not playing or doesn't exist
 */
bool DKAudioEngine::IsPlaying(int nChannelId) {
    auto tFoundIt = sgpImplementation->mChannels.find(nChannelId);
    if (tFoundIt == sgpImplementation->mChannels.end())
        return false;

    bool bIsPlaying = false;
    tFoundIt->second->isPlaying(&bIsPlaying);
    return bIsPlaying;
}

/**
 * @brief Get the value of an event parameter
 *
 * @param strEventName Event path
 * @param strParameterName Name of the parameter to get
 * @param parameter Pointer to store the retrieved parameter value
 *
 * Uses FMOD 2.01+ parameter API for direct parameter access
 */
void DKAudioEngine::GetEventParameter(const string& strEventName, const string& strParameterName, float* parameter) {
    auto tFoundIt = sgpImplementation->mEvents.find(strEventName);
    if (tFoundIt == sgpImplementation->mEvents.end())
        return;

    // New FMOD 2.01+ parameter API - direct parameter access
    DKAudioEngine::ErrorCheck(tFoundIt->second->getParameterByName(strParameterName.c_str(), parameter));
}

/**
 * @brief Set the value of an event parameter
 *
 * @param strEventName Event path
 * @param strParameterName Name of the parameter to set
 * @param fValue New value for the parameter
 *
 * Uses FMOD 2.01+ parameter API for direct parameter access.
 * Parameters allow dynamic control of event behavior (e.g., intensity, distance)
 */
void DKAudioEngine::SetEventParameter(const string& strEventName, const string& strParameterName, float fValue) {
    auto tFoundIt = sgpImplementation->mEvents.find(strEventName);
    if (tFoundIt == sgpImplementation->mEvents.end())
        return;

    // New FMOD 2.01+ parameter API - direct parameter access
    DKAudioEngine::ErrorCheck(tFoundIt->second->setParameterByName(strParameterName.c_str(), fValue));
}

/**
 * @brief Stop a specific channel and remove it from the active channels
 *
 * @param nChannelId Channel ID to stop
 */
void DKAudioEngine::StopChannel(int nChannelId) {
    auto tFoundIt = sgpImplementation->mChannels.find(nChannelId);
    if (tFoundIt == sgpImplementation->mChannels.end())
        return;

    DKAudioEngine::ErrorCheck(tFoundIt->second->stop());
    sgpImplementation->mChannels.erase(tFoundIt);
}

/**
 * @brief Stop all currently playing channels
 *
 * Stops all active sound channels and clears the channel map
 */
void DKAudioEngine::StopAllChannels() {
    for (auto& channel : sgpImplementation->mChannels) {
        channel.second->stop();
    }
    sgpImplementation->mChannels.clear();
}

/**
 * @brief Set the 3D listener position and orientation
 *
 * @param vPos Position of the audio listener (typically the camera/player position)
 * @param fVolumedB Master volume in decibels (currently unused in implementation)
 *
 * The listener represents the "ears" in the 3D audio space.
 * Forward is set to (0,0,1) and up to (0,1,0) by default.
 */
void DKAudioEngine::Set3dListenerAndOrientation(const Vector3& vPos, float /*fVolumedB*/) {
    FMOD_VECTOR pos = VectorToFmod(vPos);
    FMOD_VECTOR vel = { 0.0f, 0.0f, 0.0f };
    FMOD_VECTOR forward = { 0.0f, 0.0f, 1.0f };
    FMOD_VECTOR up = { 0.0f, 1.0f, 0.0f };

    DKAudioEngine::ErrorCheck(sgpImplementation->mpSystem->set3DListenerAttributes(0, &pos, &vel, &forward, &up));
}

/**
 * @brief Convert Vector3 to FMOD_VECTOR format
 *
 * @param vPosition Vector3 to convert
 * @return FMOD_VECTOR Converted vector in FMOD format
 */
FMOD_VECTOR DKAudioEngine::VectorToFmod(const Vector3& vPosition) {
    FMOD_VECTOR fVec;
    fVec.x = vPosition.x;
    fVec.y = vPosition.y;
    fVec.z = vPosition.z;
    return fVec;
}

/**
 * @brief Convert decibels to linear volume
 *
 * @param dB Volume in decibels
 * @return float Linear volume value (0.0 to 1.0+ range)
 *
 * Uses the formula: volume = 10^(dB/20)
 */
float DKAudioEngine::dbToVolume(float dB) {
    return powf(10.0f, 0.05f * dB);
}

/**
 * @brief Convert linear volume to decibels
 *
 * @param volume Linear volume value
 * @return float Volume in decibels
 *
 * Uses the formula: dB = 20 * log10(volume)
 */
float DKAudioEngine::VolumeTodB(float volume) {
    return 20.0f * log10f(volume);
}

/**
 * @brief Check FMOD result and print detailed error message if failed
 *
 * @param result FMOD_RESULT to check
 * @return int 0 if success (FMOD_OK), 1 if error occurred
 *
 * Provides detailed error descriptions for common FMOD error codes including:
 * - File errors (not found, corrupted, format issues)
 * - System errors (initialization, memory)
 * - DSP and channel errors
 * - Network errors
 */
int DKAudioEngine::ErrorCheck(FMOD_RESULT result) {
    if (result != FMOD_OK) {
        std::cout << "FMOD ERROR " << static_cast<int>(result);

        // Add error description based on error codes
        switch (result) {
        case 1: // FMOD_ERR_BADCOMMAND
            std::cout << ": Bad command";
            break;
        case 2: // FMOD_ERR_CHANNEL_ALLOC
            std::cout << ": Channel allocation error";
            break;
        case 3: // FMOD_ERR_CHANNEL_STOLEN
            std::cout << ": Channel stolen";
            break;
        case 4: // FMOD_ERR_DMA
            std::cout << ": DMA error";
            break;
        case 5: // FMOD_ERR_DSP_CONNECTION
            std::cout << ": DSP connection error";
            break;
        case 6: // FMOD_ERR_DSP_DONTPROCESS
            std::cout << ": DSP don't process";
            break;
        case 7: // FMOD_ERR_DSP_FORMAT
            std::cout << ": DSP format error";
            break;
        case 8: // FMOD_ERR_DSP_INUSE
            std::cout << ": DSP in use";
            break;
        case 9: // FMOD_ERR_DSP_NOTFOUND
            std::cout << ": DSP not found";
            break;
        case 10: // FMOD_ERR_DSP_RESERVED
            std::cout << ": DSP reserved";
            break;
        case 11: // FMOD_ERR_DSP_SILENCE
            std::cout << ": DSP silence";
            break;
        case 12: // FMOD_ERR_DSP_TYPE
            std::cout << ": DSP type error";
            break;
        case 13: // FMOD_ERR_FILE_BAD
            std::cout << ": File is corrupted or invalid";
            break;
        case 14: // FMOD_ERR_FILE_COULDNOTSEEK
            std::cout << ": Could not seek in file";
            break;
        case 15: // FMOD_ERR_FILE_DISKEJECTED
            std::cout << ": Disk ejected";
            break;
        case 16: // FMOD_ERR_FILE_EOF
            std::cout << ": End of file";
            break;
        case 17: // FMOD_ERR_FILE_ENDOFDATA
            std::cout << ": End of data";
            break;
        case 18: // FMOD_ERR_FILE_NOTFOUND
            std::cout << ": File not found";
            break;
        case 19: // FMOD_ERR_FORMAT
            std::cout << ": Unsupported file format";
            break;
        case 20: // FMOD_ERR_HEADER_MISMATCH
            std::cout << ": Header mismatch";
            break;
        case 21: // FMOD_ERR_HTTP
            std::cout << ": HTTP error";
            break;
        case 22: // FMOD_ERR_HTTP_ACCESS
            std::cout << ": HTTP access error";
            break;
        case 23: // FMOD_ERR_HTTP_PROXY_AUTH
            std::cout << ": HTTP proxy auth error";
            break;
        case 24: // FMOD_ERR_HTTP_SERVER_ERROR
            std::cout << ": HTTP server error";
            break;
        case 25: // FMOD_ERR_HTTP_TIMEOUT
            std::cout << ": HTTP timeout";
            break;
        case 26: // FMOD_ERR_INITIALIZATION
            std::cout << ": FMOD not initialized";
            break;
        case 27: // FMOD_ERR_INITIALIZED
            std::cout << ": FMOD already initialized";
            break;
        case 28: // FMOD_ERR_INTERNAL
            std::cout << ": Internal error";
            break;
        case 29: // FMOD_ERR_INVALID_FLOAT
            std::cout << ": Invalid float";
            break;
        case 30: // FMOD_ERR_INVALID_HANDLE
            std::cout << ": Invalid handle";
            break;
        case 31: // FMOD_ERR_INVALID_PARAM
            std::cout << ": Invalid parameter";
            break;
        case 32: // FMOD_ERR_INVALID_POSITION
            std::cout << ": Invalid position";
            break;
        case 33: // FMOD_ERR_INVALID_SPEAKER
            std::cout << ": Invalid speaker";
            break;
        case 34: // FMOD_ERR_INVALID_SYNCPOINT
            std::cout << ": Invalid sync point";
            break;
        case 35: // FMOD_ERR_INVALID_THREAD
            std::cout << ": Invalid thread";
            break;
        case 36: // FMOD_ERR_INVALID_VECTOR
            std::cout << ": Invalid vector";
            break;
        case 37: // FMOD_ERR_MAXAUDIBLE
            std::cout << ": Max audible reached";
            break;
        case 38: // FMOD_ERR_MEMORY
            std::cout << ": Not enough memory";
            break;
        case 39: // FMOD_ERR_MEMORY_CANTPOINT
            std::cout << ": Memory can't point";
            break;
        case 40: // FMOD_ERR_NEEDS3D
            std::cout << ": Needs 3D";
            break;
        case 41: // FMOD_ERR_NEEDSHARDWARE
            std::cout << ": Needs hardware";
            break;
        case 42: // FMOD_ERR_NET_CONNECT
            std::cout << ": Network connect error";
            break;
        case 43: // FMOD_ERR_NET_SOCKET_ERROR
            std::cout << ": Network socket error";
            break;
        case 44: // FMOD_ERR_NET_URL
            std::cout << ": Network URL error";
            break;
        case 45: // FMOD_ERR_NET_WOULD_BLOCK
            std::cout << ": Network would block";
            break;
        default:
            std::cout << ": Unknown FMOD error code " << static_cast<int>(result);
            break;
        }
        std::cout << std::endl;
        return 1;
    }
    return 0;
}

/**
 * @brief Shutdown the audio engine and release all resources
 *
 * Deletes the AudioImplementation singleton and sets it to nullptr
 */
void DKAudioEngine::Shutdown() {
    delete sgpImplementation;
    sgpImplementation = nullptr;
}