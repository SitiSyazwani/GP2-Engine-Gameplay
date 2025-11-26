/**
 * @file Layer.hpp
 * @author Adi (100%)
 * @brief Base class for application layers providing modular functionality
 *
 * Layers represent distinct pieces of application functionality (game logic,
 * UI, debug overlays, etc.) with their own lifecycle and update/render methods.
 * The Application class manages a LayerStack and calls each layer's methods
 * in order during the main loop.
 *
 * Usage:
 *   class GameLayer : public Layer {
 *       void OnStart(Registry& registry) override { ... }
 *       void Update(Registry& registry, float dt) override { ... }
 *       void Render(Registry& registry) override { ... }
 *   };
 *
 *   // In Application:
 *   PushLayer(new GameLayer());
 */

#pragma once

namespace GP2Engine {

    // Forward declaration
    class Registry;

    /**
     * @brief Base class for application layers
     *
     * Layers provide modular functionality with lifecycle hooks matching
     * the Application pattern. Override virtual methods to implement
     * layer-specific behavior.
     */
    class Layer {
    public:
        Layer() = default;
        virtual ~Layer() = default;

        /**
         * @brief Called when layer is pushed to the stack
         * @param registry ECS registry for entity management
         */
        virtual void OnStart(Registry& registry) { (void)registry; }

        /**
         * @brief Called every frame for layer logic
         * @param registry ECS registry for entity queries
         * @param deltaTime Time elapsed since last frame in seconds
         */
        virtual void Update(Registry& registry, float deltaTime) { (void)registry; (void)deltaTime; }

        /**
         * @brief Called every frame for layer rendering
         * @param registry ECS registry for entity rendering
         */
        virtual void Render(Registry& registry) { (void)registry; }

        /**
         * @brief Called when layer is popped from the stack
         * @param registry ECS registry for cleanup
         */
        virtual void OnShutdown(Registry& registry) { (void)registry; }
    };

} // namespace GP2Engine
