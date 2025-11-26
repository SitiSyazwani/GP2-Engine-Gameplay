/**
 * @file JsonSerializer.hpp
 * @author Rifqah (100%)
 * @brief Saves and loads game data to/from JSON files
 *
 * This class handles reading and writing game information like player stats,
 * level data, and object positions using JSON format files.
 * It can save game progress or load previously saved games.
 */

#pragma once
#include "ISerializer.hpp"
#include <nlohmann/json.hpp>
#include <fstream>
#include <iostream>
#include <stack>
#include <string>

namespace GP2Engine {

    class JsonSerializer : public ISerializer {
    public:
        /**
         * @brief Create a new JSON serializer
         */
        JsonSerializer();
        /**
         * @brief Clean up when serializer is destroyed
         */
        virtual ~JsonSerializer();

        /**
        * @brief Load game data from a JSON file
        * @param filename The path to the JSON file to load
        * @return True if the file was loaded successfully, false if there was an error
        */
        bool LoadFromFile(const std::string& filename);

        /**
         * @brief Check if the serializer is ready to use
         * @return True if data is loaded and valid, false if there was a problem
         */
        bool IsValid() const override { return m_isValid; }


        // ISerializer interface methods

        /**
         * @brief Save or load a number value
         * @param value The number to save or load
         * @param name The name to use for this number in the file
         */
        void Serialize(int& value, const std::string& name = "") override;

        /**
         * @brief Save or load a decimal number value
         * @param value The decimal number to save or load
         * @param name The name to use for this number in the file
         */
        void Serialize(float& value, const std::string& name = "") override;

        /**
         * @brief Save or load a true/false value
         * @param value The true/false value to save or load
         * @param name The name to use for this value in the file
         */
        void Serialize(bool& value, const std::string& name = "") override;

        /**
         * @brief Save or load text
         * @param value The text to save or load
         * @param name The name to use for this text in the file
         */
        void Serialize(std::string& value, const std::string& name = "") override;

        /**
         * @brief Start reading or writing a group of related data
         * @param name The name of the group (like "player" or "level")
         * @return True if the group exists and can be accessed
         */
        bool BeginObject(const std::string& name = "") override;

        /**
         * @brief Stop working with the current group of data
         *
         * Call this after you finish reading or writing a group that you started with BeginObject.
         */
        void EndObject() override;

        /**
         * @brief Check if a group of data exists in the file
         * @param name The name of the group to check for
         * @return True if the group exists, false if it doesn't
         */
        bool HasObject(const std::string& name) const override;

        /**
         * @brief Save all entities and their components to a JSON file
         * @param registry The ECS registry containing entities to save
         * @param filename Path to save the scene file
         * @return true if save was successful, false otherwise
         */
        static bool SaveScene(class Registry& registry, const std::string& filename);

        /**
         * @brief Load entities and components from a JSON file
         * @param registry The ECS registry to load entities into
         * @param filename Path to the scene file to load
         * @return true if load was successful, false otherwise
         */
        static bool LoadScene(class Registry& registry, const std::string& filename);

    private:
        bool m_isValid = false; /**< Whether the serializer is ready to use */
        nlohmann::json m_jsonData;  // Use nlohmann::json instead of stringstream
        std::stack<nlohmann::json*> m_contextStack; /**< Keeps track of which group we're currently working with */
        nlohmann::json* m_currentContext = nullptr; /**< The current group we're reading or writing */
    };
}