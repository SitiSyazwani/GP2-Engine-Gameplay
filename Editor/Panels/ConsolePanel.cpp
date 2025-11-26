/**
 * @file ConsolePanel.cpp
 * @author Adi (100%)
 * @brief Implementation of the console panel
 *
 * Uses a custom std::streambuf to intercept std::cout output and capture it
 * for display in ImGui. Output is still forwarded to the original console.
 */

#include "ConsolePanel.hpp"

// ConsoleBuf implementation
ConsolePanel::ConsoleBuf::ConsoleBuf(ConsolePanel* console, std::ostream& stream)
    : m_console(console), m_stream(stream), m_oldBuf(stream.rdbuf()) {
    // Redirect stream to this buffer
    m_stream.rdbuf(this);
}

ConsolePanel::ConsoleBuf::~ConsoleBuf() {
    // Restore original buffer
    m_stream.rdbuf(m_oldBuf);
}

int ConsolePanel::ConsoleBuf::sync() {
    // Get the buffered string
    std::string text = str();
    if (!text.empty()) {
        // Add to console
        m_console->AddLog(text);

        // Output to original stream
        m_oldBuf->sputn(text.c_str(), text.size());
        m_oldBuf->pubsync();

        // Clear the buffer
        str("");
    }
    return 0;
}

// ConsolePanel implementation
ConsolePanel::~ConsolePanel() {
    Shutdown();
}

void ConsolePanel::Initialize() {
    // Redirect cout to capture console output
    m_coutBuf = std::make_unique<ConsoleBuf>(this, std::cout);
    std::cout << "Console panel initialized - capturing cout" << std::endl;
}

void ConsolePanel::Render() {
    if (!m_isVisible) return;

    ImGui::Begin("Console", &m_isVisible);

    // Toolbar
    if (ImGui::Button("Clear")) {
        Clear();
    }

    ImGui::Separator();

    // Log messages area and use remaining space for scrollable region
    ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), true, ImGuiWindowFlags_HorizontalScrollbar);

    // Use a monospace font style for logs
    ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(0, 0));

    for (const auto& log : m_logs) {
        ImGui::TextUnformatted(log.c_str());
    }

    ImGui::PopStyleVar();

    ImGui::EndChild();

    ImGui::End();
}

void ConsolePanel::AddLog(const std::string& message) {
    // Split by newlines and add each line
    std::string line;
    std::stringstream ss(message);
    while (std::getline(ss, line)) {
        if (!line.empty()) {
            m_logs.push_back(line);
        }
    }
}

void ConsolePanel::Clear() {
    m_logs.clear();
    std::cout << "Console cleared" << std::endl;
}

void ConsolePanel::Shutdown() {
    if (m_coutBuf) {
        m_coutBuf.reset();  // Restores original cout buffer
    }
}
