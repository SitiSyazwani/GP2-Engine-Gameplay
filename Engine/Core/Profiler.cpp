/**
 * @file Profiler.cpp
 * @author Adi (100%)
 * @brief Implementation of the Profiler system for GP2Engine
 *
 * Profiler uses GLFW's high-precision timer for accurate timing.
 */

#include "Profiler.hpp"

namespace GP2Engine {

    Profiler& Profiler::GetInstance() {
        static Profiler instance;
        return instance;
    }

    void Profiler::StartTiming(const std::string& systemName) {
        // Record start time in seconds (GLFW high-precision timer)
        m_TimingData[systemName].startTime = glfwGetTime();
    }

    void Profiler::EndTiming(const std::string& systemName) {
        // Calculate elapsed time in seconds
        double endTime = glfwGetTime();
        double duration = endTime - m_TimingData[systemName].startTime;

        // Convert to milliseconds and accumulate for this frame
        m_TimingData[systemName].accumulatedTime += static_cast<float>(duration * 1000.0);
    }

    void Profiler::EndFrame() {
        // Sum up all system times to get total frame time
        m_TotalFrameTime = 0.0f;
        for (const auto& pair : m_TimingData) {
            m_TotalFrameTime += pair.second.accumulatedTime;
        }

        // Avoid division by zero if no profiling occurred this frame
        float divisor = (m_TotalFrameTime > 0.0f) ? m_TotalFrameTime : 1.0f;

        // Calculate final percentages for each system
        for (const auto& pair : m_TimingData) {
            const std::string& systemName = pair.first;
            float systemTime = pair.second.accumulatedTime;

            m_SystemTimes[systemName] = systemTime;
            m_SystemPercentages[systemName] = (systemTime / divisor) * 100.0f;
        }

        // Clear accumulated times - ready for next frame
        for (auto& pair : m_TimingData) {
            pair.second.accumulatedTime = 0.0f;
        }
    }

    float Profiler::GetSystemPercentage(const std::string& systemName) const {
        // Return percentage of frame time, or 0 if system not found
        auto it = m_SystemPercentages.find(systemName);
        return (it != m_SystemPercentages.end()) ? it->second : 0.0f;
    }

    float Profiler::GetSystemTimeMs(const std::string& systemName) const {
        // Return time in milliseconds, or 0 if system not found
        auto it = m_SystemTimes.find(systemName);
        return (it != m_SystemTimes.end()) ? it->second : 0.0f;
    }

    void Profiler::Reset() {
        // Clear all profiling data
        m_TimingData.clear();
        m_SystemTimes.clear();
        m_SystemPercentages.clear();
        m_TotalFrameTime = 0.0f;
    }

} // namespace GP2Engine