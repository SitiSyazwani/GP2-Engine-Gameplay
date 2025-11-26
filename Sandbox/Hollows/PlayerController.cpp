/**
 * @file PlayerController.cpp
 * @author Syazwani (100%)
 * @brief Implementation of the player movement and collision control system
 */
#include "PlayerController.hpp"
#include <fstream>

namespace Hollows {

    void PlayerController::Initialize(GP2Engine::EntityID playerEntity) {
        m_playerEntity = playerEntity;
        InitializeAudioComponents();

        SavePlayerAudioToJSON();
    }

    void PlayerController::InitializeAudioComponents() {
        if (m_audioComponentsInitialized) return;

        std::cout << "PlayerController: Initializing audio components..." << std::endl;

        // Setup collision sound component
        AudioComponentData collisionData;
        collisionData.soundPath = "assets/audio_files/collision.wav";
        collisionData.is3D = false;
        collisionData.isLooping = false;
        collisionData.volumeDB = -3.0f;
        m_collisionAudioComp.Initialize(collisionData);

        // Setup keyboard sound component
        AudioComponentData keyboardData;
        keyboardData.soundPath = "assets/audio_files/keyboard_input.wav";
        keyboardData.is3D = false;
        keyboardData.isLooping = false;
        keyboardData.volumeDB = 0.0f;
        m_keyboardAudioComp.Initialize(keyboardData);

        // Setup mouse click sound component
        AudioComponentData mouseData;
        mouseData.soundPath = "assets/audio_files/mouse_click.wav";
        mouseData.is3D = false;
        mouseData.isLooping = false;
        mouseData.volumeDB = -6.0f;
        m_mouseClickAudioComp.Initialize(mouseData);

        m_audioComponentsInitialized = true;
        std::cout << "PlayerController: Audio components initialized!" << std::endl;
    }

    void PlayerController::SavePlayerAudioToJSON() {
        std::cout << "PlayerController: Saving audio to JSON files..." << std::endl;

        // Save collision audio
        std::ofstream file1("Player_Collision_audio.json");
        if (file1.is_open()) {
            file1 << AudioSerialization::ToJSON(m_collisionAudioComp.Serialize());
            file1.close();
            std::cout << "Created Player_Collision_audio.json" << std::endl;
        }

        // Save keyboard audio
        std::ofstream file2("Player_Keyboard_audio.json");
        if (file2.is_open()) {
            file2 << AudioSerialization::ToJSON(m_keyboardAudioComp.Serialize());
            file2.close();
            std::cout << "Created Player_Keyboard_audio.json" << std::endl;
        }

        // Save mouse click audio
        std::ofstream file3("Player_MouseClick_audio.json");
        if (file3.is_open()) {
            file3 << AudioSerialization::ToJSON(m_mouseClickAudioComp.Serialize());
            file3.close();
            std::cout << "Created Player_MouseClick_audio.json" << std::endl;
        }

        std::cout << "PlayerController: 3 audio JSON files created!" << std::endl;
    }

    void PlayerController::Update(GP2Engine::Registry& registry, float deltaTime) {
        if (m_playerEntity == GP2Engine::INVALID_ENTITY) return;

        GP2Engine::Transform2D* transform = registry.GetComponent<GP2Engine::Transform2D>(m_playerEntity);
        if (!transform) return;

        // Update collision sound cooldown
        if (m_collisionSoundCooldown > 0.0f) {
            m_collisionSoundCooldown -= deltaTime;
        }

        // Handle WASD movement
        GP2Engine::Vector2D movement(0.0f, 0.0f);
        if (GP2Engine::Input::IsKeyHeld(GP2Engine::Key::W)) movement.y -= 1.0f;
        if (GP2Engine::Input::IsKeyHeld(GP2Engine::Key::S)) movement.y += 1.0f;
        if (GP2Engine::Input::IsKeyHeld(GP2Engine::Key::A)) movement.x -= 1.0f;
        if (GP2Engine::Input::IsKeyHeld(GP2Engine::Key::D)) movement.x += 1.0f;

        bool shouldAnimate = false;
        bool didCollide = false;  // NEW: Track if collision happened

        // Normalize and apply movement
        if (movement.x != 0.0f || movement.y != 0.0f) {
            movement = movement.normalized();
            float speed = GP2Engine::Input::IsKeyHeld(GP2Engine::Key::LeftShift) ? m_playerSpeed * 2.0f : m_playerSpeed;

            m_playerVelocity = movement * speed;

            GP2Engine::Vector2D oldPosition = transform->position;
            GP2Engine::Vector2D newPosition = transform->position;
            newPosition.x += movement.x * speed * deltaTime;
            newPosition.y += movement.y * speed * deltaTime;

            if (!WouldCollide(registry, newPosition)) {
                transform->position = newPosition;
            }
            else {
                didCollide = true;  // NEW: Mark collision

                GP2Engine::Vector2D horizontalMove = transform->position;
                horizontalMove.x += movement.x * speed * deltaTime;
                if (!WouldCollide(registry, horizontalMove)) {
                    transform->position = horizontalMove;
                }
                else {
                    GP2Engine::Vector2D verticalMove = transform->position;
                    verticalMove.y += movement.y * speed * deltaTime;
                    if (!WouldCollide(registry, verticalMove)) {
                        transform->position = verticalMove;
                    }
                }
            }

            // NEW: Play collision sound if we hit something (with cooldown)
            if (didCollide && m_collisionSoundCooldown <= 0.0f) {
                DKAudioEngine::PlaySounds("assets/audio_files/collision.wav");
                m_collisionSoundCooldown = 0.3f;  // 300ms cooldown to avoid spam
            }

            // Check if player moved in the intended direction
            GP2Engine::Vector2D actualMovement = transform->position - oldPosition;
            float dotProduct = actualMovement.x * movement.x + actualMovement.y * movement.y;

            if (dotProduct > 0.0001f) {
                UpdateAnimation(registry, movement);
                shouldAnimate = true;
            }
            else {
                PauseAnimation(registry);
            }
        }
        else {
            m_playerVelocity = GP2Engine::Vector2D(0.0f, 0.0f);
            PauseAnimation(registry);
        }

        // Update sprite animation in all cases
        GP2Engine::SpriteComponent* spriteComp = registry.GetComponent<GP2Engine::SpriteComponent>(m_playerEntity);
        if (spriteComp && spriteComp->sprite) {
            spriteComp->sprite->UpdateAnimation(deltaTime);
        }

        // NEW: Keyboard input sound (keeping your spacebar example)
        bool spacebarPressed = GP2Engine::Input::IsKeyHeld(GP2Engine::Key::Space);
        if (spacebarPressed && !m_spacebarWasPressed) {
            DKAudioEngine::PlaySounds("assets/audio_files/keyboard_input.wav");
        }
        m_spacebarWasPressed = spacebarPressed;

        // NEW: Mouse input sound (left click)
        bool leftMousePressed = GP2Engine::Input::IsMouseButtonPressed(GP2Engine::MouseButton::Left);
        if (leftMousePressed && !m_leftMouseWasPressed) {
            DKAudioEngine::PlaySounds("assets/audio_files/mouse_click.wav");
        }
        m_leftMouseWasPressed = leftMousePressed;
    }

    bool PlayerController::WouldCollide(GP2Engine::Registry& registry, const GP2Engine::Vector2D& position) {
        return m_collisionSystem.WouldCollide(registry, m_playerEntity, position);
    }

    void PlayerController::UpdateAnimation(GP2Engine::Registry& registry, const GP2Engine::Vector2D& movement) {
        GP2Engine::SpriteComponent* spriteComp = registry.GetComponent<GP2Engine::SpriteComponent>(m_playerEntity);
        if (!spriteComp || !spriteComp->sprite) {
            return;
        }

        std::string targetAnimation = "walk_right";
        bool shouldFlipX = false;
        GP2Engine::TexturePtr targetTexture = m_horizontalTexture;

        // Determine animation and texture based on movement
        if (movement.x != 0.0f || movement.y != 0.0f) {
            // Check if movement is more horizontal or vertical using MathUtils
            bool isHorizontalDominant = GP2Engine::MathUtils::Abs(movement.x) >= GP2Engine::MathUtils::Abs(movement.y);

            if (isHorizontalDominant) {
                // Horizontal movement - use horizontal texture
                targetTexture = m_horizontalTexture;
                targetAnimation = "walk_right";

                // Flip horizontally when moving RIGHT, don't flip when moving LEFT
                if (movement.x < 0.0f) {
                    shouldFlipX = false;
                    m_facingRight = false;
                } else {
                    shouldFlipX = true;
                    m_facingRight = true;
                }
            } else {
                // Vertical movement - use vertical texture
                targetTexture = m_verticalTexture;
                shouldFlipX = false; // No flip for vertical movement

                if (movement.y < 0.0f) {
                    // Moving up - use dedicated walk up texture
                    targetTexture = m_walkUpTexture;
                    targetAnimation = "walk_up";
                } else {
                    // Moving down - use vertical texture
                    targetAnimation = "walk_down";
                }
            }
        }

        // Switch texture if needed
        if (targetTexture && spriteComp->sprite->GetTexture() != targetTexture) {
            spriteComp->sprite->SetTexture(targetTexture);
        }

        // Apply horizontal flip only
        spriteComp->sprite->SetFlipX(shouldFlipX);

        // Only play animation if it's different from current
        if (targetAnimation != m_currentAnimation) {
            spriteComp->sprite->PlayAnimation(targetAnimation);
            m_currentAnimation = targetAnimation;
        } else {
            // Same animation, make sure it's playing (resume if paused)
            spriteComp->sprite->ResumeAnimation();
        }
    }

    void PlayerController::PauseAnimation(GP2Engine::Registry& registry) {
        GP2Engine::SpriteComponent* spriteComp = registry.GetComponent<GP2Engine::SpriteComponent>(m_playerEntity);
        if (!spriteComp || !spriteComp->sprite) {
            return;
        }

        // Switch to idle animation based on the last movement direction
        std::string idleAnimation = "idle"; // Default idle animation
        GP2Engine::TexturePtr targetTexture = m_idleTexture;
        
        // Determine which idle animation to use based on the last movement
        if (m_currentAnimation == "walk_up") {
            idleAnimation = "idle";
            targetTexture = m_idleTexture;
        } 
        else if (m_currentAnimation == "walk_down") {
            idleAnimation = "idle";
            targetTexture = m_idleTexture;
        }
        else if (m_currentAnimation == "walk_right" || m_currentAnimation == "walk_left") {
            idleAnimation = "idle";
            targetTexture = m_idleTexture;
        }
        
        // Only change animation if we're not already in an idle state
        if (m_currentAnimation != idleAnimation) {
            // Set the idle texture first
            if (targetTexture && spriteComp->sprite->GetTexture() != targetTexture) {
                spriteComp->sprite->SetTexture(targetTexture);
            }
            
            // Play the idle animation
            spriteComp->sprite->PlayAnimation(idleAnimation);
            m_currentAnimation = idleAnimation;
            
            // Debug output
            std::cout << "Switched to idle animation: " << idleAnimation << std::endl;
        }
    }

} // namespace Hollows