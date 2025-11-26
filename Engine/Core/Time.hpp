/**
 * @file Time.hpp
 * @author Adi (100%)
 * @brief Static time management system for GP2Engine
 *
 * Time provides centralized timing for the entire engine using GLFW's
 * high-precision timer. All engine systems and games query Time for
 * deltaTime, FPS, and elapsed time.
 *
 * Features:
 * - Delta time for time-based movement
 * - FPS tracking (1-second averaging)
 * - Configurable frame rate limiting
 * - Total elapsed time since startup
 * - Busy-wait frame limiting for precision
 */

#pragma once

namespace GP2Engine {
    
    /**
     * @brief Static time management system
     *
     * Time is a static class that owns all timing calculations for the engine.
     * It uses GLFW's high-precision timer to calculate delta time, track FPS,
     * and provide frame rate limiting.
     *
     * Usage:
     * @code
     * // Called by Application once per frame
     * Time::Update();
     *
     * // Query in game logic (time-based)
     * float speed = 100.0f;  // units per second
     * position += velocity * Time::DeltaTime();  // Consistent speed regardless of FPS
     *
     * // Configure frame limiting
     * Time::SetTargetFPS(60);  // Limit to 60 FPS (0 = unlimited)
     * @endcode
     */
    class Time {
    public:
        /**
         * @brief Update the time system for the current frame
         *
         * Calculates delta time using GLFW's high-precision timer, updates
         * FPS counter, and accumulates total elapsed time. Must be called
         * once per frame by Application.
         */
        static void Update();
        
        /**
         * @brief Get time elapsed since last frame
         *
         * Use this for time-based movement.
         * Example: position += velocity * Time::DeltaTime();
         *
         * @return Delta time in seconds
         */
        static float DeltaTime() { return s_deltaTime; }

        /**
         * @brief Get total time since application started
         *
         * Useful for timers, animations, and time-based events.
         *
         * @return Total elapsed time in seconds
         */
        static float TotalTime() { return s_totalTime; }

        /**
         * @brief Get current frames per second
         *
         * FPS is averaged over 1-second intervals for stability.
         *
         * @return Current FPS
         */
        static int GetFPS() { return s_fps; }

        /**
         * @brief Set target frame rate limit
         *
         * @param targetFPS Target FPS (0 = unlimited, default: 60)
         */
        static void SetTargetFPS(int targetFPS);

        /**
         * @brief Limit frame rate to target FPS
         *
         * Uses busy-wait for maximum precision. Called automatically
         * by Application at end of each frame.
         */
        static void LimitFrameRate();
        
    private:
        static float s_deltaTime;
        static float s_totalTime;
        static int s_fps;
        static float s_fpsTimer;
        static int s_frameCount;
        static double s_lastFrameTime;
        static int s_targetFPS;
        static float s_targetFrameTime;
    };

} // namespace GP2Engine
