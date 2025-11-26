/**
 * @file AudioComponent.cpp
 * @author Fauzan (100%)
 * @brief Implementation of audio component with serialization support
 *
 * UPDATED: Now uses Vector2D for 2D game compatibility
 */

#include "AudioComponent.hpp"
#include "AudioEngine.hpp" // Your existing audio engine
#include "./Math/Vector2D.hpp"    // Your Vector2D class
#include <sstream>
#include <iostream>

using namespace GP2Engine;

AudioComponent::AudioComponent()
    : mChannelID(-1)
    , mIsInitialized(false)
{
}

AudioComponent::~AudioComponent() {
    Shutdown();
}

/**
 * @brief Initialize the audio component with data
 * @param data Audio component data to initialize with
 */
void AudioComponent::Initialize(const AudioComponentData& data) {
    mData = data;

    // Load the sound if not already loaded
    if (!mData.soundPath.empty()) {
        DKAudioEngine::LoadSound(mData.soundPath, mData.is3D, mData.isLooping, mData.isStreaming);
        mIsInitialized = true;

        // Play immediately if configured to do so
        if (mData.playOnAwake) {
            Play();
        }
    }
}

/**
 * @brief Cleanup the audio component
 */
void AudioComponent::Shutdown() {
    if (mChannelID != -1) {
        DKAudioEngine::StopChannel(mChannelID);
        mChannelID = -1;
    }
    mIsInitialized = false;
}

/**
 * @brief Start playing the audio
 */
void AudioComponent::Play() {
    if (!mIsInitialized || mData.soundPath.empty()) {
        std::cout << "AudioComponent: Cannot play - not initialized or no sound path" << std::endl;
        return;
    }

    // Stop existing playback if any
    if (mChannelID != -1 && DKAudioEngine::IsPlaying(mChannelID)) {
        DKAudioEngine::StopChannel(mChannelID);
    }

    // Create a Vector3 from Vector2D (z = 0 for 2D games)
    // Your AudioEngine expects Vector3, so we pass x, y, and 0 for z
    Vector3 position3D;
    position3D.x = mData.positionX;
    position3D.y = mData.positionY;
    position3D.z = 0.0f;  // Always 0 for 2D

    // Start new playback
    mChannelID = DKAudioEngine::PlaySounds(mData.soundPath, position3D, mData.volumeDB);
}

/**
 * @brief Stop playing the audio
 */
void AudioComponent::Stop() {
    if (mChannelID != -1) {
        DKAudioEngine::StopChannel(mChannelID);
        mChannelID = -1;
    }
}

/**
 * @brief Pause the audio (Note: requires pause support in your engine)
 */
void AudioComponent::Pause() {
    // Note: Your current AudioEngine doesn't have pause functionality
    // You may need to add this to the engine, or just use Stop() for now
    Stop();
}

/**
 * @brief Resume the audio
 */
void AudioComponent::Resume() {
    Play();
}

/**
 * @brief Check if audio is currently playing
 * @return true if playing, false otherwise
 */
bool AudioComponent::IsPlaying() const {
    if (mChannelID == -1) return false;
    return DKAudioEngine::IsPlaying(mChannelID);
}

/**
 * @brief Set the volume of the audio
 * @param volumeDB Volume in decibels
 */
void AudioComponent::SetVolume(float volumeDB) {
    mData.volumeDB = volumeDB;
    if (mChannelID != -1) {
        DKAudioEngine::SetChannelVolume(mChannelID, volumeDB);
    }
}

/**
 * @brief Set the 2D position of the audio (x, y)
 * @param x X coordinate
 * @param y Y coordinate
 */
void AudioComponent::SetPosition(float x, float y) {
    mData.positionX = x;
    mData.positionY = y;

    if (mChannelID != -1 && mData.is3D) {
        // Convert 2D position to 3D (z = 0)
        Vector3 position3D;
        position3D.x = x;
        position3D.y = y;
        position3D.z = 0.0f;
        DKAudioEngine::SetChannel3dPosition(mChannelID, position3D);
    }
}

/**
 * @brief Set the 2D position using Vector2D
 * @param pos Vector2D position
 */
void AudioComponent::SetPosition(const Vector2D& pos) {
    SetPosition(pos.x, pos.y);
}

/**
 * @brief Get position as Vector2D
 * @return Vector2D containing x and y position
 */
Vector2D AudioComponent::GetPosition() const {
    return Vector2D(mData.positionX, mData.positionY);
}

/**
 * @brief Set looping state
 * @param loop Whether to loop
 */
void AudioComponent::SetLooping(bool loop) {
    // Note: Changing looping at runtime requires reloading the sound
    // This is a limitation of FMOD's sound creation flags
    if (mData.isLooping != loop) {
        mData.isLooping = loop;
        // Would need to reload the sound with new flags
        std::cout << "AudioComponent: Looping changed - sound needs to be reloaded" << std::endl;
    }
}

/**
 * @brief Serialize component to data structure
 * @return AudioComponentData containing all component properties
 */
AudioComponentData AudioComponent::Serialize() const {
    return mData;
}

/**
 * @brief Deserialize component from data structure
 * @param data Data to load into component
 */
void AudioComponent::Deserialize(const AudioComponentData& data) {
    // Shutdown existing audio
    Shutdown();

    // Initialize with new data
    Initialize(data);
}

// ============================================================================
// Serialization Helper Functions
// ============================================================================

namespace AudioSerialization {

    /**
     * @brief Convert AudioComponentData to JSON string
     * @param data Audio component data to serialize
     * @return JSON string representation
     */
    std::string ToJSON(const AudioComponentData& data) {
        std::ostringstream oss;
        oss << "{\n";
        oss << "  \"soundPath\": \"" << data.soundPath << "\",\n";
        oss << "  \"is3D\": " << (data.is3D ? "true" : "false") << ",\n";
        oss << "  \"isLooping\": " << (data.isLooping ? "true" : "false") << ",\n";
        oss << "  \"isStreaming\": " << (data.isStreaming ? "true" : "false") << ",\n";
        oss << "  \"volumeDB\": " << data.volumeDB << ",\n";
        oss << "  \"playOnAwake\": " << (data.playOnAwake ? "true" : "false") << ",\n";
        oss << "  \"spatialize\": " << (data.spatialize ? "true" : "false") << ",\n";
        oss << "  \"minDistance\": " << data.minDistance << ",\n";
        oss << "  \"maxDistance\": " << data.maxDistance << ",\n";
        oss << "  \"position\": {\n";
        oss << "    \"x\": " << data.positionX << ",\n";
        oss << "    \"y\": " << data.positionY << "\n";
        oss << "  }\n";
        oss << "}";
        return oss.str();
    }

    /**
     * @brief Parse AudioComponentData from JSON string
     * @param jsonStr JSON string to parse
     * @return Parsed audio component data
     *
     * Note: This is a simple implementation. For production, use a proper JSON library
     * like rapidjson, nlohmann/json, or jsoncpp
     */
    AudioComponentData FromJSON(const std::string& jsonStr) {
        AudioComponentData data;

        // Simple parsing - in production, use a proper JSON library
        // This is just a placeholder implementation
        std::istringstream iss(jsonStr);
        std::string line;

        while (std::getline(iss, line)) {
            // Remove whitespace and parse key-value pairs
            // This is a simplified parser - use a real JSON library in production

            if (line.find("\"soundPath\"") != std::string::npos) {
                size_t start = line.find("\"", line.find(":")) + 1;
                size_t end = line.find("\"", start);
                if (start != std::string::npos && end != std::string::npos) {
                    data.soundPath = line.substr(start, end - start);
                }
            }
            else if (line.find("\"is3D\"") != std::string::npos) {
                data.is3D = (line.find("true") != std::string::npos);
            }
            else if (line.find("\"isLooping\"") != std::string::npos) {
                data.isLooping = (line.find("true") != std::string::npos);
            }
            else if (line.find("\"isStreaming\"") != std::string::npos) {
                data.isStreaming = (line.find("true") != std::string::npos);
            }
            else if (line.find("\"volumeDB\"") != std::string::npos) {
                size_t pos = line.find(":") + 1;
                data.volumeDB = std::stof(line.substr(pos));
            }
            else if (line.find("\"playOnAwake\"") != std::string::npos) {
                data.playOnAwake = (line.find("true") != std::string::npos);
            }
            // Parse position x and y
            else if (line.find("\"x\":") != std::string::npos) {
                size_t pos = line.find(":") + 1;
                data.positionX = std::stof(line.substr(pos));
            }
            else if (line.find("\"y\":") != std::string::npos) {
                size_t pos = line.find(":") + 1;
                data.positionY = std::stof(line.substr(pos));
            }
        }

        return data;
    }

    /**
     * @brief Convert AudioComponentData to simple text format
     * @param data Audio component data to serialize
     * @return Text string representation
     */
    std::string ToText(const AudioComponentData& data) {
        std::ostringstream oss;
        oss << "AudioComponent\n";
        oss << "soundPath=" << data.soundPath << "\n";
        oss << "is3D=" << data.is3D << "\n";
        oss << "isLooping=" << data.isLooping << "\n";
        oss << "isStreaming=" << data.isStreaming << "\n";
        oss << "volumeDB=" << data.volumeDB << "\n";
        oss << "playOnAwake=" << data.playOnAwake << "\n";
        oss << "spatialize=" << data.spatialize << "\n";
        oss << "minDistance=" << data.minDistance << "\n";
        oss << "maxDistance=" << data.maxDistance << "\n";
        oss << "positionX=" << data.positionX << "\n";
        oss << "positionY=" << data.positionY << "\n";
        return oss.str();
    }

    /**
     * @brief Parse AudioComponentData from text format
     * @param textStr Text string to parse
     * @return Parsed audio component data
     */
    AudioComponentData FromText(const std::string& textStr) {
        AudioComponentData data;
        std::istringstream iss(textStr);
        std::string line;

        while (std::getline(iss, line)) {
            size_t pos = line.find('=');
            if (pos == std::string::npos) continue;

            std::string key = line.substr(0, pos);
            std::string value = line.substr(pos + 1);

            if (key == "soundPath") data.soundPath = value;
            else if (key == "is3D") data.is3D = (value == "1");
            else if (key == "isLooping") data.isLooping = (value == "1");
            else if (key == "isStreaming") data.isStreaming = (value == "1");
            else if (key == "volumeDB") data.volumeDB = std::stof(value);
            else if (key == "playOnAwake") data.playOnAwake = (value == "1");
            else if (key == "spatialize") data.spatialize = (value == "1");
            else if (key == "minDistance") data.minDistance = std::stof(value);
            else if (key == "maxDistance") data.maxDistance = std::stof(value);
            else if (key == "positionX") data.positionX = std::stof(value);
            else if (key == "positionY") data.positionY = std::stof(value);
        }

        return data;
    }

} // namespace AudioSerialization