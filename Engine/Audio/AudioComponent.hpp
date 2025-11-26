/**
 * @file AudioComponent.hpp
 * @author Fauzan (100%)
 * @brief Audio component for game objects with serialization support
 *
 * Provides a component-based audio system that can be attached to game objects
 * and supports serialization/deserialization for save/load functionality.
 *
 * UPDATED: Now uses Vector2D for 2D game compatibility
 */

#pragma once
#include <string>
#include <vector>

 // Use your existing Vector2D from the engine
namespace GP2Engine {
    struct Vector2D;
}

/**
 * @brief Audio component data structure for serialization
 *
 * Contains all the properties needed to recreate an audio source
 * on a game object. This struct is designed to be easily serializable
 * to JSON, XML, or binary formats.
 */
struct AudioComponentData {
    std::string soundPath;      // Path to the audio file
    bool is3D;                  // Whether sound is 3D or 2D
    bool isLooping;             // Whether sound loops
    bool isStreaming;           // Whether to stream from disk
    float volumeDB;             // Volume in decibels
    bool playOnAwake;           // Start playing when object is created
    bool spatialize;            // Use 3D spatialization
    float minDistance;          // Minimum distance for 3D sound attenuation
    float maxDistance;          // Maximum distance for 3D sound attenuation

    // 2D position (x, y) - z is always 0 for 2D games
    float positionX;
    float positionY;

    // Default constructor
    AudioComponentData()
        : soundPath("")
        , is3D(false)
        , isLooping(false)
        , isStreaming(false)
        , volumeDB(0.0f)
        , playOnAwake(false)
        , spatialize(true)
        , minDistance(1.0f)
        , maxDistance(100.0f)
        , positionX(0.0f)
        , positionY(0.0f)
    {
    }
};

/**
 * @brief Runtime audio component for game objects
 *
 * Manages the runtime state of an audio source attached to a game object.
 * Stores both the serializable data and runtime information like channel ID.
 */
class AudioComponent {
public:
    AudioComponent();
    ~AudioComponent();

    // Initialization and cleanup
    void Initialize(const AudioComponentData& data);
    void Shutdown();

    // Playback control
    void Play();
    void Stop();
    void Pause();
    void Resume();
    bool IsPlaying() const;

    // Property setters (2D version)
    void SetVolume(float volumeDB);
    void SetPosition(float x, float y);
    void SetPosition(const GP2Engine::Vector2D& pos);
    void SetLooping(bool loop);

    // Property getters
    const AudioComponentData& GetData() const { return mData; }
    AudioComponentData& GetData() { return mData; }
    int GetChannelID() const { return mChannelID; }

    // Get position as Vector2D
    GP2Engine::Vector2D GetPosition() const;

    // Serialization
    AudioComponentData Serialize() const;
    void Deserialize(const AudioComponentData& data);

private:
    AudioComponentData mData;   // Serializable component data
    int mChannelID;             // Runtime channel ID from audio engine
    bool mIsInitialized;        // Whether component has been initialized
};

/**
 * @brief Serialization helper functions
 *
 * These functions handle converting AudioComponentData to/from various formats.
 * Implement the format you need (JSON is most common).
 */
namespace AudioSerialization {
    // JSON serialization (you'll need to implement based on your JSON library)
    std::string ToJSON(const AudioComponentData& data);
    AudioComponentData FromJSON(const std::string& jsonStr);

    // Simple text format serialization (for debugging or simple saves)
    std::string ToText(const AudioComponentData& data);
    AudioComponentData FromText(const std::string& textStr);
}