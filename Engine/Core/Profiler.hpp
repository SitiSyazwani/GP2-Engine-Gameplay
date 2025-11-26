/**
 * @file Profiler.hpp
 * @author Adi (100%)
 * @brief CPU profiling system for GP2Engine
 *
 * Profiler tracks execution time of code sections for performance analysis.
 * Use PROFILE_SCOPE(name) macro for automatic RAII-based profiling.
 *
 * Features:
 * - High-precision timing using GLFW timer
 * - Per-frame percentage calculations
 * - Singleton pattern for global access
 *
 * Usage:
 * @code
 * void Update(float dt) {
 *     PROFILE_SCOPE("Update");
 *     // ... code to profile ...
 * }
 *
 * // At end of frame:
 * Profiler::GetInstance().EndFrame();
 * @endcode
 */

#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>
#include <unordered_map>
#include <string>

namespace GP2Engine {

    /**
     * @brief CPU profiling system for performance analysis
     *
     * Profiler measures execution time of code sections and calculates
     * their percentage of total frame time.
     */
    class Profiler {
    public:
        /**
         * @brief Get the singleton Profiler instance
         *
         * @return Reference to the global Profiler
         */
        static Profiler& GetInstance();

        /**
         * @brief Start timing a code section
         *
         * @param systemName Unique name for this profiling scope
         */
        void StartTiming(const std::string& systemName);

        /**
         * @brief End timing a code section
         *
         * @param systemName Name of the scope to end (must match StartTiming)
         */
        void EndTiming(const std::string& systemName);

        /**
         * @brief Get percentage of total frame time for a system
         *
         * @param systemName Name of the profiled system
         * @return Percentage of frame time (0-100)
         */
        float GetSystemPercentage(const std::string& systemName) const;

        /**
         * @brief Get execution time in milliseconds for a system
         *
         * @param systemName Name of the profiled system
         * @return Time in milliseconds
         */
        float GetSystemTimeMs(const std::string& systemName) const;

        /**
         * @brief End frame and calculate percentages
         *
         * Call this at the end of each frame to finalize timing data
         * and calculate percentages. Resets accumulated times for next frame.
         */
        void EndFrame();

        /**
         * @brief Get all system timing data
         *
         * @return Map of system names to their times in milliseconds
         */
        const std::unordered_map<std::string, float>& GetSystemTimes() const { return m_SystemTimes; }

        /**
         * @brief Reset all profiling data
         */
        void Reset();

    private:
        Profiler() = default;

        // Non-copyable
        Profiler(const Profiler&) = delete;
        Profiler& operator=(const Profiler&) = delete;

        struct TimingData {
            double startTime = 0.0;
            float accumulatedTime = 0.0f;
        };

        std::unordered_map<std::string, TimingData> m_TimingData;
        std::unordered_map<std::string, float> m_SystemTimes;
        std::unordered_map<std::string, float> m_SystemPercentages;
        float m_TotalFrameTime = 0.0f;
    };

    /**
     * @brief RAII helper for automatic profiling
     *
     * Creates a profiling scope that automatically starts timing on
     * construction and ends timing on destruction.
     */
    class ScopedProfiler {
    public:
        ScopedProfiler(const std::string& systemName) : m_SystemName(systemName) {
            Profiler::GetInstance().StartTiming(m_SystemName);
        }

        ~ScopedProfiler() {
            Profiler::GetInstance().EndTiming(m_SystemName);
        }

    private:
        std::string m_SystemName;
    };

    /**
     * @brief Convenience macro for automatic profiling
     *
     * Usage: PROFILE_SCOPE("ScopeName");
     */
    #define PROFILE_SCOPE(name) GP2Engine::ScopedProfiler _prof(name)

} // namespace GP2Engine