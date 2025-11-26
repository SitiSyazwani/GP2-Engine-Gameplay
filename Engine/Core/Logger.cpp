/**
 * @file Logger.cpp
 * @author Adi (100%)
 * @brief Implementation of the Logger system for GP2Engine
 *
 * Logger writes messages to both file and console.
 */

#include "Logger.hpp"
#include <iostream>

namespace GP2Engine {

    Logger& Logger::GetInstance() {
        static Logger instance;
        return instance;
    }

    bool Logger::Initialize(const std::string& logFilePath) {
        if (m_Initialized) {
            return true;
        }

        // Open log file in append mode
        m_LogFile = std::make_unique<std::ofstream>(logFilePath, std::ios::app);

        if (!m_LogFile->is_open()) {
            std::cerr << "Failed to open log file: " << logFilePath << std::endl;
            return false;
        }

        m_Initialized = true;

        // Write initialization message to file
        *m_LogFile << "\n[INIT] Logger initialized" << std::endl;
        m_LogFile->flush();

        std::cout << "Logger initialized - writing to: " << logFilePath << std::endl;
        return true;
    }

    void Logger::Log(LogLevel level, const std::string& message) {
        if (!m_Initialized) {
            // Fallback to console if logger not initialized
            std::cout << LogLevelToString(level) << ": " << message << std::endl;
            return;
        }

        // Format: "[INFO] message"
        std::string logEntry = "[" + LogLevelToString(level) + "] " + message;

        // Write to file and flush 
        *m_LogFile << logEntry << std::endl;
        m_LogFile->flush();

        // Output to console for immediate feedback
        std::cout << logEntry << std::endl;
    }

    void Logger::LogInfo(const std::string& message) {
        Log(LogLevel::INFO, message);
    }

    void Logger::LogWarning(const std::string& message) {
        Log(LogLevel::WARNING, message);
    }

    void Logger::LogError(const std::string& message) {
        Log(LogLevel::ERROR, message);
    }

    void Logger::LogCrash(const std::string& message) {
        Log(LogLevel::CRASH, message);

        if (m_Initialized) {
            *m_LogFile << "==================== CRASH LOG ====================" << std::endl;
            m_LogFile->flush();
        }
    }

    void Logger::Shutdown() {
        if (m_Initialized && m_LogFile) {
            // Write shutdown message and close file
            *m_LogFile << "[SHUTDOWN] Logger shutting down" << std::endl;
            m_LogFile->close();
            m_LogFile.reset();
            m_Initialized = false;
        }
    }

    Logger::~Logger() {
        Shutdown();
    }

    std::string Logger::LogLevelToString(LogLevel level) {
        // Convert log level enum to string
        switch (level) {
            case LogLevel::INFO:    return "INFO";
            case LogLevel::WARNING: return "WARN";
            case LogLevel::ERROR:   return "ERROR";
            case LogLevel::CRASH:   return "CRASH";
            default:                return "UNKNOWN";
        }
    }

} // namespace GP2Engine