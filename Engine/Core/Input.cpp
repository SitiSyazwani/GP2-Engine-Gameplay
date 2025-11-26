/**
 * @file Input.cpp
 * @author Asri (50%), Adi (50%)
 * @brief Implementation of the Input system for GP2Engine
 *
 * Input uses GLFW callbacks to track keyboard and mouse state across frames.
 * Three-state tracking (Pressed/Held/Released) enables precise input handling.
 *
 * Architecture:
 * - Static arrays store input states for all GLFW keys/buttons
 * - GLFW callbacks automatically update states when events occur
 * - ResetFrameStates() clears Pressed/Released states each frame
 */

#include "Input.hpp"
#include <algorithm>

namespace GP2Engine {

    // === STATIC MEMBER VARIABLE DEFINITIONS ===

    bool Input::s_keys[GLFW_KEY_LAST] = {};
    bool Input::s_keysPressed[GLFW_KEY_LAST] = {};
    bool Input::s_keysReleased[GLFW_KEY_LAST] = {};

    bool Input::s_mouseButtons[GLFW_MOUSE_BUTTON_LAST] = {};
    bool Input::s_mouseButtonsPressed[GLFW_MOUSE_BUTTON_LAST] = {};
    bool Input::s_mouseButtonsReleased[GLFW_MOUSE_BUTTON_LAST] = {};

    double Input::s_mouseX = 0.0;
    double Input::s_mouseY = 0.0;
    double Input::s_mouseDeltaX = 0.0;
    double Input::s_mouseDeltaY = 0.0;

    void Input::Initialize(GLFWwindow* window) {
        // Register GLFW callbacks
        glfwSetKeyCallback(window, KeyCallback);
        glfwSetMouseButtonCallback(window, MouseButtonCallback);
        glfwSetCursorPosCallback(window, MousePositionCallback);

        // Initialize all state arrays to false
        std::fill(std::begin(s_keys), std::end(s_keys), false);
        std::fill(std::begin(s_keysPressed), std::end(s_keysPressed), false);
        std::fill(std::begin(s_keysReleased), std::end(s_keysReleased), false);
        std::fill(std::begin(s_mouseButtons), std::end(s_mouseButtons), false);
        std::fill(std::begin(s_mouseButtonsPressed), std::end(s_mouseButtonsPressed), false);
        std::fill(std::begin(s_mouseButtonsReleased), std::end(s_mouseButtonsReleased), false);
    }
    
    bool Input::IsKeyPressed(Key key) {
        return s_keysPressed[static_cast<int>(key)];
    }

    bool Input::IsKeyHeld(Key key) {
        return s_keys[static_cast<int>(key)];
    }

    bool Input::IsKeyReleased(Key key) {
        return s_keysReleased[static_cast<int>(key)];
    }

    bool Input::IsMouseButtonPressed(MouseButton button) {
        return s_mouseButtonsPressed[static_cast<int>(button)];
    }

    bool Input::IsMouseButtonHeld(MouseButton button) {
        return s_mouseButtons[static_cast<int>(button)];
    }

    bool Input::IsMouseButtonReleased(MouseButton button) {
        return s_mouseButtonsReleased[static_cast<int>(button)];
    }

    void Input::GetMousePosition(double& x, double& y) {
        x = s_mouseX;
        y = s_mouseY;
    }

    void Input::GetMouseDelta(double& dx, double& dy) {
        dx = s_mouseDeltaX;
        dy = s_mouseDeltaY;
    }

    void Input::ResetFrameStates() {
        // Clear Pressed/Released states (only last 1 frame)
        std::fill(std::begin(s_keysPressed), std::end(s_keysPressed), false);
        std::fill(std::begin(s_keysReleased), std::end(s_keysReleased), false);
        std::fill(std::begin(s_mouseButtonsPressed), std::end(s_mouseButtonsPressed), false);
        std::fill(std::begin(s_mouseButtonsReleased), std::end(s_mouseButtonsReleased), false);

        // Reset mouse delta
        s_mouseDeltaX = 0.0;
        s_mouseDeltaY = 0.0;
    }

    void Input::KeyCallback(GLFWwindow*, int key, int, int action, int) {
        if (key >= 0 && key < GLFW_KEY_LAST) {
            if (action == GLFW_PRESS) {
                s_keys[key] = true;
                s_keysPressed[key] = true;
            } else if (action == GLFW_RELEASE) {
                s_keys[key] = false;
                s_keysReleased[key] = true;
            }
            // use IsKeyHeld() for continuous input
        }
    }

    void Input::MouseButtonCallback(GLFWwindow*, int button, int action, int) {
        if (button >= 0 && button < GLFW_MOUSE_BUTTON_LAST) {
            if (action == GLFW_PRESS) {
                s_mouseButtons[button] = true;
                s_mouseButtonsPressed[button] = true;
            } else if (action == GLFW_RELEASE) {
                s_mouseButtons[button] = false;
                s_mouseButtonsReleased[button] = true;
            }
        }
    }

    void Input::MousePositionCallback(GLFWwindow*, double xpos, double ypos) {
        // Calculate movement delta
        s_mouseDeltaX = xpos - s_mouseX;
        s_mouseDeltaY = ypos - s_mouseY;

        // Update position
        s_mouseX = xpos;
        s_mouseY = ypos;
    }

} // namespace GP2Engine
