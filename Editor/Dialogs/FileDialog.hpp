/**
 * @file FileDialog.hpp
 * @author Adi (100%)
 * @brief File dialog wrapper
 *
 * Provides file open/save dialogs for the editor.
 * Currently implements Windows native dialogs.
 */

#pragma once

#include <string>

namespace FileDialog {
    /**
     * @brief Open file dialog to select a file
     * @param title Dialog window title
     * @param filter File type filter (Windows format: "Text Files\0*.txt\0All Files\0*.*\0")
     * @return Full path to selected file, or empty string if cancelled
     */
    std::string OpenFile(const char* title, const char* filter);

    /**
     * @brief Save file dialog to specify save location
     * @param title Dialog window title
     * @param filter File type filter
     * @param defaultExtension Default file extension (e.g., "txt")
     * @return Full path to save location, or empty string if cancelled
     */
    std::string SaveFile(const char* title, const char* filter, const char* defaultExtension);
}
