/**
 * @file ConfigLoader.hpp
 * @author Rifqah (100%)
 * @brief Configuration file loader and manager for game settings
 *
 * Loads and manages game configuration settings from files with support for
 * different data types and default values with singleton pattern
 */

#pragma once
/**
 * @file ConfigLoader.hpp
 * @brief Loads and reads configuration settings from files
 */
#include <string>
#include <unordered_map>
#include <nlohmann/json.hpp>
#include <string>
#include <unordered_map>

namespace GP2Engine {

    /**
     * @brief Configuration file loader and settings manager
     *
     * Singleton class that loads game settings from configuration files
     * including screen resolution, audio levels, and gameplay parameters
     * Provides type-safe access to settings with fallback default values
     */
    class ConfigLoader {
    public:
        /**
         * @brief Get the singleton instance of ConfigLoader
         * @return Reference to the singleton ConfigLoader instance
         */
        static ConfigLoader& GetInstance();

        /**
         * @brief Load configuration settings from specified file
         * @param filename Path to the configuration file to load
         * @return True if file loaded successfully, false on error
         */
        bool LoadConfig(const std::string& filename);

        /**
         * @brief Retrieve string configuration value
         * @param key Configuration key to lookup
         * @param defaultValue Fallback value if key not found
         * @return String value or default if key missing
         */
        std::string GetString(const std::string& key, const std::string& defaultValue = "");

        /**
         * @brief Retrieve integer configuration value
         * @param key Configuration key to lookup
         * @param defaultValue Fallback value if key not found
         * @return Integer value or default if key missing
         */
        int GetInt(const std::string& key, int defaultValue = 0);

        /**
         * @brief Retrieve floating-point configuration value
         * @param key Configuration key to lookup
         * @param defaultValue Fallback value if key not found
         * @return Float value or default if key missing
         */
        float GetFloat(const std::string& key, float defaultValue = 0.0f);

        /**
         * @brief Display all loaded configuration settings for debugging
         *
         * Prints all currently loaded key-value pairs to standard output
         * Useful for verifying configuration file parsing and values
         */
        void PrintConfig() const;

        /**
         * @brief Retrieves a boolean value from configuration data
         * @param key Configuration key to lookup, defaultValue Fallback value if key not found
         */
        bool GetBool(const std::string& key, bool defaultValue = false);

        /**
         * @brief Recursively extracts all values from JSON object into flat key-value map
         * @param json JSON object to extract from, prefix Current key prefix for nested objects
         */
        void ExtractJSONValues(const nlohmann::json& json, const std::string& prefix);

    private:
        /**
         * @brief Private constructor for singleton pattern enforcement
         */
        ConfigLoader() = default;

        std::unordered_map<std::string, std::string> m_configData; /**< Internal storage for configuration key-value pairs */
    };
}