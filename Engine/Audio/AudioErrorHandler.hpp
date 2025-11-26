#pragma once
/**
 * @file AudioErrorHandler.hpp
 * @author Fauzan (100%)
 * @brief Audio error detection and popup display system
 *
 * Provides format validation and user-friendly error popups for audio loading failures.
 * Supports both Windows message boxes and cross-platform console fallback.
 */

#pragma once
#include <string>
#include <vector>
#include <set>

 /**
  * @brief Audio format validation and error handling
  */
class AudioErrorHandler {
public:
    /**
     * @brief Supported audio file formats
     */
    enum class AudioFormat {
        OGG,
        MP3,
        WAV,
        FLAC,
        UNKNOWN
    };

    /**
     * @brief Error types for audio operations
     */
    enum class ErrorType {
        UNSUPPORTED_FORMAT,
        FILE_NOT_FOUND,
        FILE_CORRUPTED,
        MEMORY_ERROR,
        ENGINE_NOT_INITIALIZED,
        UNKNOWN_ERROR
    };

    /**
     * @brief Initialize the error handler
     * @param supportedFormats List of supported audio formats (e.g., {".ogg", ".mp3", ".wav"})
     */
    static void Initialize(const std::vector<std::string>& supportedFormats = { ".ogg", ".mp3", ".wav" });

    /**
     * @brief Check if a file format is supported
     * @param filePath Path to the audio file
     * @return true if format is supported, false otherwise
     */
    static bool IsFormatSupported(const std::string& filePath);

    /**
     * @brief Get the audio format from a file path
     * @param filePath Path to the audio file
     * @return AudioFormat enum value
     */
    static AudioFormat GetFileFormat(const std::string& filePath);

    /**
     * @brief Validate an audio file before loading
     * @param filePath Path to the audio file
     * @return ErrorType indicating the validation result (UNKNOWN_ERROR = success)
     */
    static ErrorType ValidateAudioFile(const std::string& filePath);

    /**
     * @brief Show an error popup to the user
     * @param errorType Type of error that occurred
     * @param filePath Path to the file that caused the error
     * @param additionalInfo Additional error information
     */
    static void ShowErrorPopup(ErrorType errorType, const std::string& filePath, const std::string& additionalInfo = "");

    /**
     * @brief Get a user-friendly error message
     * @param errorType Type of error
     * @param filePath Path to the file
     * @return Formatted error message string
     */
    static std::string GetErrorMessage(ErrorType errorType, const std::string& filePath);

    /**
     * @brief Extract file extension from path
     * @param filePath Path to the file
     * @return File extension including the dot (e.g., ".ogg")
     */
    static std::string GetFileExtension(const std::string& filePath);

    /**
     * @brief Get list of supported formats as a string
     * @return Comma-separated list of supported formats
     */
    static std::string GetSupportedFormatsString();

private:
    static std::set<std::string> sSupportedFormats;

    /**
     * @brief Show a Windows message box (Windows only)
     */
    static void ShowWindowsMessageBox(const std::string& title, const std::string& message);

    /**
     * @brief Show a console error message (cross-platform fallback)
     */
    static void ShowConsoleError(const std::string& title, const std::string& message);

    /**
     * @brief Convert string to lowercase
     */
    static std::string ToLowerCase(const std::string& str);
};