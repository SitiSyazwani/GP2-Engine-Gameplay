/**
 * @file ISerializer.hpp
 * @author Rifqah (100%)
 * @brief Interface for serialization and deserialization of game data
 *
 * Defines the contract for all serialization implementations allowing
 * the game to work with multiple file formats through a unified interface
 */

#pragma once

#include <string>

 // Forward declare the namespace
namespace GP2Engine {
    class ISerializer;
}

/**
 * @brief Interface for serialization systems supporting multiple formats
 *
 * Abstract base class defining the contract for all serialization implementations
 * Enables saving and loading game data through JSON, XML, binary, or other formats
 * without requiring changes to game code through polymorphic interface
 */
class GP2Engine::ISerializer {
public:
    /**
     * @brief Virtual destructor for proper polymorphic cleanup
     */
    virtual ~ISerializer() = default;

    /**
     * @brief Check if serializer is in valid state for operations
     * @return True if serializer is ready for use, false if error state
     */
    virtual bool IsValid() const = 0;

    /**
     * @brief Serialize or deserialize integer value
     * @param value Integer value to save or load
     * @param name Optional identifier for the value in serialized format
     */
    virtual void Serialize(int& value, const std::string& name = "") = 0;

    /**
     * @brief Serialize or deserialize floating-point value
     * @param value Float value to save or load
     * @param name Optional identifier for the value in serialized format
     */
    virtual void Serialize(float& value, const std::string& name = "") = 0;

    /**
     * @brief Serialize or deserialize boolean value
     * @param value Boolean value to save or load
     * @param name Optional identifier for the value in serialized format
     */
    virtual void Serialize(bool& value, const std::string& name = "") = 0;

    /**
     * @brief Serialize or deserialize string value
     * @param value String value to save or load
     * @param name Optional identifier for the value in serialized format
     */
    virtual void Serialize(std::string& value, const std::string& name = "") = 0;

    /**
     * @brief Begin serialization/deserialization of object group
     * @param name Optional name identifier for the object group
     * @return True if object group successfully opened, false on error
     *
     * Starts processing a structured group of related data values
     * Used for organizing hierarchical data like player profiles or inventories
     */
    virtual bool BeginObject(const std::string& name = "") = 0;

    /**
     * @brief End current object group processing
     *
     * Completes processing of the current object group started with BeginObject
     * Must be called for each BeginObject to maintain proper structure
     */
    virtual void EndObject() = 0;

    /**
     * @brief Check for existence of named object group
     * @param name Name of object group to check for existence
     * @return True if specified object group exists, false otherwise
     *
     * Useful for checking availability of saved data before attempting to load
     * Prevents errors when loading incomplete or missing data structures
     */
    virtual bool HasObject(const std::string& name) const = 0;
};