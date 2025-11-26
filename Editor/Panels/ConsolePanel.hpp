/**
 * @file ConsolePanel.hpp
 * @author Adi (100%)
 * @brief Console/log panel for displaying editor and engine console output
 *
 * Captures std::cout output using a custom streambuf and displays it in an
 * ImGui panel. Useful for debugging and monitoring engine logs during play mode.
 */

#pragma once

#include <Engine.hpp>
#include <imgui.h>
#include <sstream>
#include <iostream>

class ConsolePanel {
public:
    ConsolePanel() = default;
    ~ConsolePanel();

    /**
     * @brief Initialize the console panel and redirect cout
     */
    void Initialize();

    /**
     * @brief Render the console panel
     */
    void Render();

    /**
     * @brief Clear all log messages
     */
    void Clear();

    /**
     * @brief Shutdown and restore cout
     */
    void Shutdown();

    // === PANEL VISIBILITY ===
    bool IsVisible() const { return m_isVisible; }
    void SetVisible(bool visible) { m_isVisible = visible; }

private:
    bool m_isVisible = true;
    // Custom streambuf to capture cout
    class ConsoleBuf : public std::stringbuf {
    public:
        ConsoleBuf(ConsolePanel* console, std::ostream& stream);
        ~ConsoleBuf();

    protected:
        int sync() override;

    private:
        ConsolePanel* m_console;
        std::ostream& m_stream;
        std::streambuf* m_oldBuf;
    };

    std::vector<std::string> m_logs;
    std::unique_ptr<ConsoleBuf> m_coutBuf;

    void AddLog(const std::string& message);
};
