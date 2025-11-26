/**
 * @file Time.cpp
 * @author Adi (100%)
 * @brief Implementation of the Time system for GP2Engine
 *
 * Time owns all timing calculations using GLFW's high-precision timer.
 * Application and games query Time for deltaTime, FPS, and total elapsed time.
 *
 * Implementation details:
 * - Uses glfwGetTime() for high-precision timing
 * - Calculates delta time internally
 * - FPS averaged over 1-second intervals
 * - Busy-wait frame limiting for maximum precision
 */

#include "Time.hpp"
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace GP2Engine {
    
    // === STATIC MEMBER VARIABLE DEFINITIONS ===

    float Time::s_deltaTime = 0.0f;
    float Time::s_totalTime = 0.0f;
    int Time::s_fps = 0;
    float Time::s_fpsTimer = 0.0f;
    int Time::s_frameCount = 0;
    double Time::s_lastFrameTime = 0.0;
    int Time::s_targetFPS = 60;
    float Time::s_targetFrameTime = 1.0f / 60.0f;

    void Time::Update() {
        // Get current time from GLFW
        double currentTime = glfwGetTime();

        // Calculate delta time (first frame will have deltaTime = 0)
        if (s_lastFrameTime > 0.0) {
            s_deltaTime = static_cast<float>(currentTime - s_lastFrameTime);
        } else {
            s_deltaTime = 0.0f;  // First frame
        }

        // Accumulate total application time
        s_totalTime += s_deltaTime;

        // FPS calculation (1-second averaging)
        s_fpsTimer += s_deltaTime;
        s_frameCount++;

        if (s_fpsTimer >= 1.0f) {
            s_fps = s_frameCount;
            s_frameCount = 0;
            s_fpsTimer = 0.0f;
        }

        // Store for next frame
        s_lastFrameTime = currentTime;
    }
    
    void Time::SetTargetFPS(int targetFPS) {
        s_targetFPS = targetFPS;
        s_targetFrameTime = (targetFPS > 0) ? 1.0f / static_cast<float>(targetFPS) : 0.0f;
    }

    void Time::LimitFrameRate() {
        if (s_targetFPS <= 0) return;

        double targetNextFrameTime = s_lastFrameTime + static_cast<double>(s_targetFrameTime);

        // Busy-wait for maximum precision
        while (glfwGetTime() < targetNextFrameTime) {
            // loop until target time reached
        }
    }

} // namespace GP2Engine
