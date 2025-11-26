/**
 * @file LayerStack.cpp
 * @author Adi (100%)
 * @brief Implementation of the LayerStack class
 */

#include "LayerStack.hpp"
#include <algorithm>

namespace GP2Engine {

    LayerStack::LayerStack() {
    }

    LayerStack::~LayerStack() {
        // Clean up all layers
        for (Layer* layer : m_layers) {
            delete layer;
        }
        for (Layer* overlay : m_overlays) {
            delete overlay;
        }
        m_layers.clear();
        m_overlays.clear();
    }

    void LayerStack::PushLayer(Layer* layer) {
        m_layers.push_back(layer);
    }

    void LayerStack::PushOverlay(Layer* overlay) {
        m_overlays.push_back(overlay);
    }

    void LayerStack::PopLayer(Layer* layer) {
        auto it = std::find(m_layers.begin(), m_layers.end(), layer);
        if (it != m_layers.end()) {
            m_layers.erase(it);
        }
    }

    void LayerStack::PopOverlay(Layer* overlay) {
        auto it = std::find(m_overlays.begin(), m_overlays.end(), overlay);
        if (it != m_overlays.end()) {
            m_overlays.erase(it);
        }
    }

    std::vector<Layer*> LayerStack::GetAllLayers() const {
        std::vector<Layer*> result;
        result.reserve(m_layers.size() + m_overlays.size());

        // Add layers first
        result.insert(result.end(), m_layers.begin(), m_layers.end());

        // Then add overlays
        result.insert(result.end(), m_overlays.begin(), m_overlays.end());

        return result;
    }

    std::vector<Layer*> LayerStack::GetAllLayersReversed() const {
        std::vector<Layer*> result = GetAllLayers();
        std::reverse(result.begin(), result.end());
        return result;
    }

} // namespace GP2Engine
