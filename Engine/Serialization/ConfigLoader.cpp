/**
 * @file ConfigLoader.cpp
 * @author Rifqah (100%)
 * @brief Configuration file loader and manager for game settings
 *
 * Implements loading and managing game configuration settings from files with support for
 * different data types and default values with singleton pattern
 */

#include "ConfigLoader.hpp"
#include <fstream>
#include <nlohmann/json.hpp>
#include <iostream>
#include <algorithm>

namespace GP2Engine {

    ConfigLoader& ConfigLoader::GetInstance() {
        static ConfigLoader instance;
        return instance;
    }


    bool ConfigLoader::LoadConfig(const std::string& filename) {
        std::ifstream file(filename);
        if (!file.is_open()) {
            std::cout << "ERROR: Could not open config file: " << filename << std::endl;
            return false;
        }

        try {
            // Parse the JSON
            nlohmann::json jsonData;
            file >> jsonData;

            // Clear existing config
            m_configData.clear();

            // Extract ALL values from JSON (including nested)
            ExtractJSONValues(jsonData, "");

            std::cout << "Config loaded successfully! Found " << m_configData.size() << " keys" << std::endl;
            return true;
        }
        catch (const std::exception& e) {
            std::cout << "ERROR parsing JSON: " << e.what() << std::endl;
            return false;
        }
    }

    void ConfigLoader::ExtractJSONValues(const nlohmann::json& json, const std::string& prefix) {
        for (auto it = json.begin(); it != json.end(); ++it) {
            std::string key = prefix.empty() ? it.key() : prefix + "." + it.key();

            if (it->is_object()) {
                // Recursively extract nested objects
                ExtractJSONValues(*it, key);
            }
            else {
                // Store the value as string
                m_configData[key] = it->dump();
                // Remove quotes from strings
                if (it->is_string()) {
                    m_configData[key] = m_configData[key].substr(1, m_configData[key].size() - 2);
                }
                std::cout << "Loaded: " << key << " = " << m_configData[key] << std::endl;
            }
        }
    }

    std::string ConfigLoader::GetString(const std::string& key, const std::string& defaultValue) {
        auto it = m_configData.find(key);
        return it != m_configData.end() ? it->second : defaultValue;
    }

    int ConfigLoader::GetInt(const std::string& key, int defaultValue) {
        auto it = m_configData.find(key);
        // Convert string to integer, return default if key not found
        return it != m_configData.end() ? std::stoi(it->second) : defaultValue;
    }

    float ConfigLoader::GetFloat(const std::string& key, float defaultValue) {
        auto it = m_configData.find(key);
        // Convert string to float, return default if key not found
        return it != m_configData.end() ? std::stof(it->second) : defaultValue;
    }

    void ConfigLoader::PrintConfig() const {
        std::cout << "=== Current Config ===" << std::endl;
        for (const auto& pair : m_configData) {
            std::cout << pair.first << " = " << pair.second << std::endl;
        }
        std::cout << "======================" << std::endl;
    }

    bool ConfigLoader::GetBool(const std::string& key, bool defaultValue) {
        // Search for the key in the configuration data map
        auto it = m_configData.find(key);
        if (it == m_configData.end()) {
            // Key not found - return default value and log for debugging
            std::cout << "KEY NOT FOUND: " << key << std::endl;
            return defaultValue;
        }

        // Extract the string value associated with the key
        std::string value = it->second;
        std::cout << "GETBOOL: " << key << " = '" << value << "'" << std::endl;

        // Convert string to boolean - supports "true" and "1" as true values
        return (value == "true" || value == "1");
    }
}

