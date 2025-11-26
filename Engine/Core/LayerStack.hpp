/**
 * @file LayerStack.hpp
 * @author Adi (100%)
 * @brief Manages an ordered collection of layers and overlays
 *
 * LayerStack maintains two categories of layers:
 * - Layers: Regular functionality (game logic, menus, etc.)
 * - Overlays: Always rendered on top (debug UI, editor panels, etc.)
 *
 * Layers are updated/rendered in the order they were pushed.
 * Overlays are always updated/rendered after all regular layers.
 *
 * Usage:
 *   LayerStack stack;
 *   stack.PushLayer(new GameLayer());     // Regular layer
 *   stack.PushOverlay(new DebugLayer());  // Always on top
 *
 *   for (Layer* layer : stack) {
 *       layer->Update(registry, deltaTime);
 *   }
 */

#pragma once

#include "Layer.hpp"
#include <vector>

namespace GP2Engine {

    /**
     * @brief Manages the application's layer stack
     *
     * Maintains two separate collections: regular layers and overlays.
     * Layers are processed first, overlays always render on top.
     */
    class LayerStack {
    public:
        LayerStack();
        ~LayerStack();

        void PushLayer(Layer* layer);
        void PushOverlay(Layer* overlay);
        void PopLayer(Layer* layer);
        void PopOverlay(Layer* overlay);

        // Get all layers in order (layers first, then overlays)
        std::vector<Layer*> GetAllLayers() const;

        // Get layers in reverse order (for cleanup)
        std::vector<Layer*> GetAllLayersReversed() const;

    private:
        std::vector<Layer*> m_layers;    // Regular layers
        std::vector<Layer*> m_overlays;  // Overlay layers (always on top)
    };

} // namespace GP2Engine
