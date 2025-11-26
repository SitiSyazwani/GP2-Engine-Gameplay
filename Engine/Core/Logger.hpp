/**
 * @file Logger.hpp
 * @author Adi (100%)
 * @brief Simple logging system for GP2Engine
 *
 * Logger provides file and console output with severity levels.
 *
 * Features:
 * - File and console output
 * - Four severity levels (INFO, WARNING, ERROR, CRASH)
 * - Singleton pattern for global access
 *
 * Usage:
 * @code
 * // Initialize once at startup
 * Logger::GetInstance().Initialize("game_log.txt");
 *
 * // Use convenience macros anywhere
 * LOG_INFO("Game started");
 * LOG_WARNING("Low memory detected");
 * LOG_ERROR("Failed to load texture");
 * LOG_CRASH("Fatal error occurred");
 * @endcode
 */

#pragma once

#include <string>
#include <fstream>
#include <memory>

namespace GP2Engine {

    /**
     * @brief Log severity levels
     *
     * Defines the severity of log messages for filtering and display.
     */
    enum class LogLevel {
        INFO,     // General information
        WARNING,  // Warning messages
        ERROR,    // Error messages
        CRASH     // Critical crash messages
    };

    /**
     * @brief Simple logging system with file and console output
     *
     * Logger writes timestamped messages to both file and console.
     */
    class Logger {
    public:
        /**
         * @brief Get the singleton Logger instance
         *
         * @return Reference to the global Logger
         */
        static Logger& GetInstance();

        /**
         * @brief Initialize the logger with a log file
         *
         * Creates or appends to the specified log file. Must be called
         * before logging. If not initialized, logs fall back to console only.
         *
         * @param logFilePath Path to the log file (default: "debug_log.txt")
         * @return true if initialization succeeded, false otherwise
         */
        bool Initialize(const std::string& logFilePath = "debug_log.txt");

        /**
         * @brief Log a message with specified severity level
         *
         * @param level Severity level of the message
         * @param message The message to log
         */
        void Log(LogLevel level, const std::string& message);

        /**
         * @brief Log an informational message
         *
         * @param message The message to log
         */
        void LogInfo(const std::string& message);

        /**
         * @brief Log a warning message
         *
         * @param message The message to log
         */
        void LogWarning(const std::string& message);

        /**
         * @brief Log an error message
         *
         * @param message The message to log
         */
        void LogError(const std::string& message);

        /**
         * @brief Log a critical crash message
         *
         * Adds extra separator for visibility in log files.
         *
         * @param message The message to log
         */
        void LogCrash(const std::string& message);

        /**
         * @brief Shutdown the logger and close the log file
         *
         * Called automatically by destructor.
         */
        void Shutdown();

    private:
        Logger() = default;
        ~Logger();

        // Non-copyable
        Logger(const Logger&) = delete;
        Logger& operator=(const Logger&) = delete;

        // Helper functions
        std::string LogLevelToString(LogLevel level);

        std::unique_ptr<std::ofstream> m_LogFile;
        bool m_Initialized = false;
    };

    /**
     * @brief Convenience macro for logging info messages
     *
     * Usage: LOG_INFO("Player connected");
     */
    #define LOG_INFO(msg) GP2Engine::Logger::GetInstance().LogInfo(msg)

    /**
     * @brief Convenience macro for logging warnings
     *
     * Usage: LOG_WARNING("Low FPS detected");
     */
    #define LOG_WARNING(msg) GP2Engine::Logger::GetInstance().LogWarning(msg)

    /**
     * @brief Convenience macro for logging errors
     *
     * Usage: LOG_ERROR("Failed to load file");
     */
    #define LOG_ERROR(msg) GP2Engine::Logger::GetInstance().LogError(msg)

    /**
     * @brief Convenience macro for logging crashes
     *
     * Usage: LOG_CRASH("Null pointer dereference");
     */
    #define LOG_CRASH(msg) GP2Engine::Logger::GetInstance().LogCrash(msg)

} // namespace GP2Engine