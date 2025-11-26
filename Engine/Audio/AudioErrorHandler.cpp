/**
 * @file AudioErrorHandler.cpp
 * @author Fauzan (100%)
 * @brief Implementation of audio error detection and popup system
 */

#include "AudioErrorHandler.hpp"
#include <algorithm>
#include <iostream>
#include <fstream>

 // Platform-specific includes for message boxes
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#endif

// Initialize static member
std::set<std::string> AudioErrorHandler::sSupportedFormats;

/**
 * @brief Initialize the error handler with supported formats
 */
void AudioErrorHandler::Initialize(const std::vector<std::string>& supportedFormats) {
    sSupportedFormats.clear();
    for (const auto& format : supportedFormats) {
        sSupportedFormats.insert(ToLowerCase(format));
    }

    std::cout << "AudioErrorHandler initialized with formats: " << GetSupportedFormatsString() << std::endl;
}

/**
 * @brief Check if a file format is supported
 */
bool AudioErrorHandler::IsFormatSupported(const std::string& filePath) {
    std::string ext = GetFileExtension(filePath);
    return sSupportedFormats.find(ext) != sSupportedFormats.end();
}

/**
 * @brief Get the audio format from file extension
 */
AudioErrorHandler::AudioFormat AudioErrorHandler::GetFileFormat(const std::string& filePath) {
    std::string ext = GetFileExtension(filePath);

    if (ext == ".ogg") return AudioFormat::OGG;
    if (ext == ".mp3") return AudioFormat::MP3;
    if (ext == ".wav") return AudioFormat::WAV;
    if (ext == ".flac") return AudioFormat::FLAC;

    return AudioFormat::UNKNOWN;
}

/**
 * @brief Validate an audio file before loading
 */
AudioErrorHandler::ErrorType AudioErrorHandler::ValidateAudioFile(const std::string& filePath) {
    // Check if format is supported
    if (!IsFormatSupported(filePath)) {
        return ErrorType::UNSUPPORTED_FORMAT;
    }

    // Check if file exists
    std::ifstream file(filePath);
    if (!file.good()) {
        return ErrorType::FILE_NOT_FOUND;
    }
    file.close();

    // Basic file size check (files should be > 0 bytes)
    file.open(filePath, std::ios::binary | std::ios::ate);
    std::streamsize size = file.tellg();
    file.close();

    if (size <= 0) {
        return ErrorType::FILE_CORRUPTED;
    }

    // All checks passed
    return ErrorType::UNKNOWN_ERROR; // Using UNKNOWN_ERROR to mean "no error"
}

/**
 * @brief Show an error popup to the user
 */
void AudioErrorHandler::ShowErrorPopup(ErrorType errorType, const std::string& filePath, const std::string& additionalInfo) {
    std::string message = GetErrorMessage(errorType, filePath);

    if (!additionalInfo.empty()) {
        message += "\n\nAdditional Information:\n" + additionalInfo;
    }

    std::string title = "Audio Loading Error";

#ifdef _WIN32
    // Show Windows message box on Windows
    ShowWindowsMessageBox(title, message);
#else
    // Fallback to console error on other platforms
    ShowConsoleError(title, message);
#endif
}

/**
 * @brief Get a user-friendly error message
 */
std::string AudioErrorHandler::GetErrorMessage(ErrorType errorType, const std::string& filePath) {
    std::string message;

    switch (errorType) {
    case ErrorType::UNSUPPORTED_FORMAT:
        message = "Unsupported Audio Format\n\n";
        message += "File: " + filePath + "\n\n";
        message += "The audio file format is not supported by this engine.\n";
        message += "Supported formats: " + GetSupportedFormatsString() + "\n\n";
        message += "Please convert your audio file to one of the supported formats.";
        break;

    case ErrorType::FILE_NOT_FOUND:
        message = "Audio File Not Found\n\n";
        message += "File: " + filePath + "\n\n";
        message += "The specified audio file could not be found.\n";
        message += "Please check that:\n";
        message += "• The file path is correct\n";
        message += "• The file exists in the specified location\n";
        message += "• You have permission to access the file";
        break;

    case ErrorType::FILE_CORRUPTED:
        message = "Audio File Corrupted\n\n";
        message += "File: " + filePath + "\n\n";
        message += "The audio file appears to be corrupted or empty.\n";
        message += "Please try:\n";
        message += "• Re-exporting the audio file\n";
        message += "• Checking the file in an audio editor\n";
        message += "• Using a different version of the file";
        break;

    case ErrorType::MEMORY_ERROR:
        message = "Memory Error\n\n";
        message += "File: " + filePath + "\n\n";
        message += "Not enough memory to load the audio file.\n";
        message += "Please try:\n";
        message += "• Closing other applications\n";
        message += "• Using streaming mode for large files\n";
        message += "• Reducing the file size or quality";
        break;

    case ErrorType::ENGINE_NOT_INITIALIZED:
        message = "Audio Engine Not Initialized\n\n";
        message += "File: " + filePath + "\n\n";
        message += "The audio engine has not been initialized.\n";
        message += "Please ensure DKAudioEngine::Init() has been called before loading sounds.";
        break;

    default:
        message = "Unknown Audio Error\n\n";
        message += "File: " + filePath + "\n\n";
        message += "An unknown error occurred while loading the audio file.";
        break;
    }

    return message;
}

/**
 * @brief Extract file extension from path
 */
std::string AudioErrorHandler::GetFileExtension(const std::string& filePath) {
    size_t dotPos = filePath.find_last_of('.');
    if (dotPos == std::string::npos) {
        return "";
    }

    std::string ext = filePath.substr(dotPos);
    return ToLowerCase(ext);
}

/**
 * @brief Get list of supported formats as a string
 */
std::string AudioErrorHandler::GetSupportedFormatsString() {
    if (sSupportedFormats.empty()) {
        return "None configured";
    }

    std::string result;
    for (const auto& format : sSupportedFormats) {
        if (!result.empty()) {
            result += ", ";
        }
        result += format;
    }
    return result;
}

/**
 * @brief Show a Windows message box
 */
void AudioErrorHandler::ShowWindowsMessageBox(const std::string& title, const std::string& message) {
#ifdef _WIN32
    // Use MessageBoxA for ANSI strings
    MessageBoxA(
        NULL,
        message.c_str(),
        title.c_str(),
        MB_OK | MB_ICONERROR | MB_TOPMOST | MB_SETFOREGROUND
    );
#else
    // Fallback if not on Windows
    ShowConsoleError(title, message);
#endif
}

/**
 * @brief Show a console error message
 */
void AudioErrorHandler::ShowConsoleError(const std::string& title, const std::string& message) {
    std::cerr << "\n";
    std::cerr << "========================================\n";
    std::cerr << title << "\n";
    std::cerr << "========================================\n";
    std::cerr << message << "\n";
    std::cerr << "========================================\n";
    std::cerr << "\n";
}

/**
 * @brief Convert string to lowercase
 */
std::string AudioErrorHandler::ToLowerCase(const std::string& str) {
    std::string result = str;
    std::transform(result.begin(), result.end(), result.begin(),
        [](unsigned char c) { return std::tolower(c); });
    return result;
}