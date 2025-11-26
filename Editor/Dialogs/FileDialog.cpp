/**
 * @file FileDialog.cpp
 * @author Adi (100%)
 * @brief Implementation of file dialog wrappers
 *
 * Windows implementation using native Win32 Common Dialogs API.
 * Uses GetOpenFileNameA and GetSaveFileNameA for file selection.
 */

#include "FileDialog.hpp"

#ifdef _WIN32
#include <windows.h>
#include <commdlg.h>

namespace FileDialog {

std::string OpenFile(const char* title, const char* filter) {
    // Buffer to store selected file path 
    char filename[MAX_PATH] = {};

    // Initialize OPENFILENAME structure with dialog configuration
    OPENFILENAME ofn = {};
    ofn.lStructSize = sizeof(OPENFILENAME);     // Structure size for version compatibility
    ofn.hwndOwner = nullptr;                     // No parent window (standalone dialog)
    ofn.lpstrFile = filename;                    // Buffer to receive selected file path
    ofn.nMaxFile = MAX_PATH;                     // Buffer size
    ofn.lpstrFilter = filter;                    // File type filter (e.g., "JSON Files\0*.json\0")
    ofn.nFilterIndex = 1;                        // Default to first filter
    ofn.lpstrFileTitle = nullptr;                // Don't need filename without path
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = nullptr;               // Use current directory
    ofn.lpstrTitle = title;                      // Dialog window title
    ofn.Flags = OFN_PATHMUSTEXIST |              // Selected path must exist
                OFN_FILEMUSTEXIST |              // Selected file must exist
                OFN_NOCHANGEDIR;                 // Don't change working directory

    // Show Windows file open dialog
    if (GetOpenFileNameA(&ofn)) {
        // User selected a file - return full path
        return std::string(filename);
    }

    // User cancelled or error occurred - return empty string
    return "";
}

std::string SaveFile(const char* title, const char* filter, const char* defaultExtension) {
    // Buffer to store selected file path
    char filename[MAX_PATH] = {};

    // Initialize OPENFILENAME structure with dialog configuration
    OPENFILENAME ofn = {};
    ofn.lStructSize = sizeof(OPENFILENAME);     // Structure size for version compatibility
    ofn.hwndOwner = nullptr;                     // No parent window (standalone dialog)
    ofn.lpstrFile = filename;                    // Buffer to receive selected file path
    ofn.nMaxFile = MAX_PATH;                     // Buffer size
    ofn.lpstrFilter = filter;                    // File type filter (e.g., "JSON Files\0*.json\0")
    ofn.nFilterIndex = 1;                        // Default to first filter
    ofn.lpstrFileTitle = nullptr;                // Don't need filename without path
    ofn.nMaxFileTitle = 0;
    ofn.lpstrInitialDir = nullptr;               // Use current directory
    ofn.lpstrTitle = title;                      // Dialog window title
    ofn.lpstrDefExt = defaultExtension;          // Auto-append extension if user doesn't type it
    ofn.Flags = OFN_PATHMUSTEXIST |              // Selected path must exist
                OFN_OVERWRITEPROMPT |            // Warn if overwriting existing file
                OFN_NOCHANGEDIR;                 // Don't change working directory

    // Show Windows file save dialog
    if (GetSaveFileNameA(&ofn)) {
        // User confirmed save location - return full path
        return std::string(filename);
    }

    // User cancelled or error occurred - return empty string
    return "";
}

} // namespace FileDialog
#endif
