/**
 * @file Input.hpp
 * @author Asri (50%), Adi (50%)
 * @brief Static input system for GP2Engine
 *
 * Input captures keyboard and mouse events via GLFW callbacks and provides
 * three-state tracking (Pressed/Held/Released) for precise input handling.
 *
 * Features:
 * - Keyboard: Press/Hold/Release detection
 * - Mouse: Button Press/Hold/Release + Position + Delta tracking
 * - Frame-based state management (Pressed/Released only last 1 frame)
 */

#pragma once

#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace GP2Engine {
    
    /**
     * @brief Enumeration of supported keyboard keys
     * 
     * Maps common game keys to their GLFW key codes for easy access.
     * This enum provides a clean interface for checking key states without
     * needing to remember GLFW key constants.
     */
    enum class Key {
        // === MOVEMENT KEYS ===
        W = GLFW_KEY_W,          // Move forward/up
        A = GLFW_KEY_A,          // Move left
        S = GLFW_KEY_S,          // Move backward/down
        D = GLFW_KEY_D,          // Move right
        
        // === ARROW KEYS ===
        Up = GLFW_KEY_UP,        // Up arrow key
        Down = GLFW_KEY_DOWN,    // Down arrow key
        Left = GLFW_KEY_LEFT,    // Left arrow key
        Right = GLFW_KEY_RIGHT,  // Right arrow key
        
        // === ACTION KEYS ===
        Space = GLFW_KEY_SPACE,  // Spacebar (jump, confirm, etc.)
        E = GLFW_KEY_E,          // Interact key
        C = GLFW_KEY_C,          // Clone key
        LeftShift = GLFW_KEY_LEFT_SHIFT,  // Run/sprint modifier
        
        // === SCALING KEYS  ===
        Q = GLFW_KEY_Q,          // Scale up
        Z = GLFW_KEY_Z,          // Scale down
        
        // === ROTATION KEYS  ===
        R = GLFW_KEY_R,          // Rotate clockwise
        T = GLFW_KEY_T,          // Rotate counter-clockwise
        
        // === ANIMATION KEYS  ===
        Key_1 = GLFW_KEY_1,      // Trigger animation 1
        Key_2 = GLFW_KEY_2,      // Trigger animation 2
        Key_3 = GLFW_KEY_3,      // Trigger animation 3
        
        // === RENDERING KEYS  ===
        M = GLFW_KEY_M,          // Toggle mass rendering
        
        // === EDITOR KEYS ===
        F1 = GLFW_KEY_F1,        // Editor toggle

        // === SYSTEM KEYS ===
        Escape = GLFW_KEY_ESCAPE // Exit/Cancel
    };
    
    /**
     * @brief Enumeration of supported mouse buttons
     * 
     * Maps mouse buttons to their GLFW button codes for consistent
     * mouse input handling across the engine.
     */
    enum class MouseButton {
        Left = GLFW_MOUSE_BUTTON_LEFT,      // Left mouse button (primary click)
        Right = GLFW_MOUSE_BUTTON_RIGHT,    // Right mouse button (secondary click)
        Middle = GLFW_MOUSE_BUTTON_MIDDLE   // Middle mouse button (scroll wheel click)
    };
    
    /**
     * @brief Static input system with three-state tracking
     *
     * Input provides precise input handling via three states:
     * - Pressed: True only on first frame (e.g., jump, shoot)
     * - Held: True every frame while held (e.g., movement)
     * - Released: True only on release frame (e.g., stop action)
     *
     * Usage:
     * @code
     * // In game Update()
     * if (Input::IsKeyPressed(Key::Space)) {
     *     player.Jump();  // Only on first press
     * }
     * if (Input::IsKeyHeld(Key::W)) {
     *     player.MoveForward(deltaTime);  // While held
     * }
     * @endcode
     */
    class Input {
    public:
        // === INITIALIZATION ===
        
        /**
         * @brief Initialize the input system with a GLFW window
         * 
         * Sets up GLFW callbacks for keyboard and mouse input, and initializes
         * all input state arrays to zero. Must be called once before using
         * any input functions.
         * 
         * @param window The GLFW window to monitor for input events
         */
        static void Initialize(GLFWwindow* window);
        
        // === KEYBOARD INPUT ===
        
        /**
         * @brief Check if a key was just pressed this frame
         * 
         * Returns true only on the first frame a key is pressed down.
         * Useful for actions that should trigger once per key press
         * (e.g., jumping, shooting, menu navigation).
         * 
         * @param key The key to check
         * @return true if the key was pressed this frame, false otherwise
         */
        static bool IsKeyPressed(Key key);
        
        /**
         * @brief Check if a key is currently being held down
         * 
         * Returns true for every frame while a key is held down.
         * Useful for continuous actions (e.g., movement, camera rotation).
         * 
         * @param key The key to check
         * @return true if the key is currently held, false otherwise
         */
        static bool IsKeyHeld(Key key);
        
        /**
         * @brief Check if a key was just released this frame
         * 
         * Returns true only on the first frame a key is released.
         * Useful for actions that should trigger when stopping an input
         * (e.g., stopping movement, ending a charge attack).
         * 
         * @param key The key to check
         * @return true if the key was released this frame, false otherwise
         */
        static bool IsKeyReleased(Key key);
        
        // === MOUSE INPUT ===
        
        /**
         * @brief Check if a mouse button was just pressed this frame
         * 
         * Returns true only on the first frame a mouse button is pressed down.
         * Useful for actions that should trigger once per click
         * (e.g., shooting, selecting objects).
         * 
         * @param button The mouse button to check
         * @return true if the button was pressed this frame, false otherwise
         */
        static bool IsMouseButtonPressed(MouseButton button);
        
        /**
         * @brief Check if a mouse button is currently being held down
         * 
         * Returns true for every frame while a mouse button is held down.
         * Useful for continuous actions (e.g., dragging, holding objects).
         * 
         * @param button The mouse button to check
         * @return true if the button is currently held, false otherwise
         */
        static bool IsMouseButtonHeld(MouseButton button);
        
        /**
         * @brief Check if a mouse button was just released this frame
         * 
         * Returns true only on the first frame a mouse button is released.
         * Useful for actions that should trigger when stopping a mouse action
         * (e.g., dropping objects, ending a drag operation).
         * 
         * @param button The mouse button to check
         * @return true if the button was released this frame, false otherwise
         */
        static bool IsMouseButtonReleased(MouseButton button);
        
        // === MOUSE POSITION ===
        
        /**
         * @brief Get the current mouse cursor position
         * 
         * Returns the absolute position of the mouse cursor in screen coordinates.
         * Coordinates are relative to the top-left corner of the window.
         * 
         * @param x Reference to store the X coordinate
         * @param y Reference to store the Y coordinate
         */
        static void GetMousePosition(double& x, double& y);
        
        /**
         * @brief Get the mouse movement delta since last frame
         * 
         * Returns the relative movement of the mouse cursor since the last frame.
         * Useful for camera controls and smooth mouse-based interactions.
         * 
         * @param dx Reference to store the X delta (horizontal movement)
         * @param dy Reference to store the Y delta (vertical movement)
         */
        static void GetMouseDelta(double& dx, double& dy);
        
        // === FRAME MANAGEMENT ===
        
        /**
         * @brief Reset frame-specific input states
         * 
         * Clears all "pressed" and "released" states and resets mouse delta.
         * This should be called once per frame, typically at the end of the
         * main game loop. Called automatically by Application::Run().
         */
        static void ResetFrameStates();
        
        // === INTERNAL CALLBACK FUNCTIONS ===
        
        /**
         * @brief GLFW keyboard callback function
         * 
         * Called automatically by GLFW when keyboard events occur.
         * Updates the internal key state arrays based on the event.
         * 
         * @param window The GLFW window that received the event
         * @param key The key that was pressed/released
         * @param scancode Platform-specific scancode (unused)
         * @param action The action (GLFW_PRESS, GLFW_RELEASE, GLFW_REPEAT)
         * @param mods Modifier keys (Ctrl, Alt, Shift) - unused
         */
        static void KeyCallback(GLFWwindow* window, int key, int scancode, int action, int mods);
        
        /**
         * @brief GLFW mouse button callback function
         * 
         * Called automatically by GLFW when mouse button events occur.
         * Updates the internal mouse button state arrays based on the event.
         * 
         * @param window The GLFW window that received the event
         * @param button The mouse button that was pressed/released
         * @param action The action (GLFW_PRESS, GLFW_RELEASE)
         * @param mods Modifier keys (Ctrl, Alt, Shift) - unused
         */
        static void MouseButtonCallback(GLFWwindow* window, int button, int action, int mods);
        
        /**
         * @brief GLFW mouse position callback function
         * 
         * Called automatically by GLFW when the mouse cursor moves.
         * Updates mouse position and calculates movement delta.
         * 
         * @param window The GLFW window that received the event
         * @param xpos New X position of the mouse cursor
         * @param ypos New Y position of the mouse cursor
         */
        static void MousePositionCallback(GLFWwindow* window, double xpos, double ypos);
        
    private:
        // Keyboard state arrays
        static bool s_keys[GLFW_KEY_LAST];
        static bool s_keysPressed[GLFW_KEY_LAST];
        static bool s_keysReleased[GLFW_KEY_LAST];

        // Mouse button state arrays
        static bool s_mouseButtons[GLFW_MOUSE_BUTTON_LAST];
        static bool s_mouseButtonsPressed[GLFW_MOUSE_BUTTON_LAST];
        static bool s_mouseButtonsReleased[GLFW_MOUSE_BUTTON_LAST];

        // Mouse position tracking
        static double s_mouseX, s_mouseY;
        static double s_mouseDeltaX, s_mouseDeltaY;
    };

} // namespace GP2Engine
