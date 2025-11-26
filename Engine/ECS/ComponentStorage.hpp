/**
 * @file ComponentStorage.hpp
 * @author Adi (100%)
 * @brief Packed component storage with parallel entity tracking
 *
 * Uses packed arrays for cache-friendly iteration with a simple
 * parallel array to track entity ownership.
 */

#pragma once
#include <vector>
#include <unordered_map>
#include "Entity.hpp"

namespace GP2Engine {

/**
 * @brief Stores components of type T in a packed array
 *
 * Design:
 * - m_data: packed array of components
 * - m_owners: parallel array tracking which entity owns each component
 * - m_lookup: fast entity->index mapping
 */
template<typename T>
class ComponentStorage {
public:
    /**
     * @brief Insert or replace component for entity
     */
    T& Insert(EntityID entity, const T& component) {
        // Check if entity already has component
        auto it = m_lookup.find(entity);
        if (it != m_lookup.end()) {
            // Update existing
            m_data[it->second] = component;
            return m_data[it->second];
        }

        // Add new component
        size_t index = m_data.size();
        m_data.push_back(component);
        m_owners.push_back(entity);
        m_lookup[entity] = index;

        return m_data.back();
    }

    /**
     * @brief Retrieve component for entity
     */
    T* Retrieve(EntityID entity) {
        auto it = m_lookup.find(entity);
        if (it != m_lookup.end()) {
            return &m_data[it->second];
        }
        return nullptr;
    }

    /**
     * @brief Check if entity has this component
     */
    bool Exists(EntityID entity) const {
        return m_lookup.count(entity) > 0;
    }

    /**
     * @brief Remove component from entity
     * Uses swap-with-last technique to keep array packed
     */
    void Erase(EntityID entity) {
        auto it = m_lookup.find(entity);
        if (it == m_lookup.end()) return;

        size_t index = it->second;
        size_t lastIndex = m_data.size() - 1;

        // If not the last element, swap with last
        if (index != lastIndex) {
            m_data[index] = std::move(m_data[lastIndex]);
            m_owners[index] = m_owners[lastIndex];

            // Update lookup for the swapped entity
            m_lookup[m_owners[index]] = index;
        }

        // Remove last element
        m_data.pop_back();
        m_owners.pop_back();
        m_lookup.erase(entity);
    }

    /**
     * @brief Get packed component array for iteration
     */
    std::vector<T>& GetData() { return m_data; }
    const std::vector<T>& GetData() const { return m_data; }

    /**
     * @brief Get entity owner array
     */
    std::vector<EntityID>& GetOwners() { return m_owners; }
    const std::vector<EntityID>& GetOwners() const { return m_owners; }

    /**
     * @brief Get number of components
     */
    size_t Count() const { return m_data.size(); }

    /**
     * @brief Clear all components
     */
    void Clear() {
        m_data.clear();
        m_owners.clear();
        m_lookup.clear();
    }

private:
    std::vector<T> m_data;                          // Packed components
    std::vector<EntityID> m_owners;                 // Parallel array of owners
    std::unordered_map<EntityID, size_t> m_lookup;  // Entity to index map
};

} // namespace GP2Engine
