/**
 * @file Registry.hpp
 * @author Adi (100%)
 * @brief Main ECS registry with built-in entity and component management
 *
 * Single-class ECS implementation that manages entities and components
 * without separate manager classes. Uses template static storage.
 *
 * Recycled Entity Handling:
 * When an entity is destroyed and its ID is reused, old components may still
 * exist in storage. The registry tracks recycled entities and prevents access
 * to stale components:
 * - m_dirtyEntities: Marks which entity IDs have been recycled
 * - m_cleanedComponents: Tracks which component types have been freshly added
 * - GetComponent returns nullptr for stale components on recycled entities
 * - AddComponent automatically cleans stale data before inserting new components
 */

#pragma once
#include <vector>
#include <unordered_set>
#include <unordered_map>
#include "Entity.hpp"
#include "Component.hpp"
#include "ComponentStorage.hpp"

namespace GP2Engine {

class Registry {
public:
    Registry() = default;

    // ==================== ENTITY MANAGEMENT ====================

    /**
     * @brief Create a new entity
     * Reuses destroyed entity IDs when available
     */
    EntityID CreateEntity() {
        EntityID id;

        // Reuse destroyed entity ID if available
        if (!m_reusableIDs.empty()) {
            id = m_reusableIDs.back();
            m_reusableIDs.pop_back();

            // Mark this ID as dirty - needs component cleanup
            m_dirtyEntities.insert(id);
        } else {
            id = m_nextEntityID++;
        }

        m_activeEntities.insert(id);
        return id;
    }

    /**
     * @brief Destroy entity and mark ID for reuse
     * Stale components are automatically cleaned when entity ID is reused
     */
    void DestroyEntity(EntityID entity) {
        if (m_activeEntities.erase(entity) > 0) {
            m_reusableIDs.push_back(entity);
            // Reset tracking for this entity
            m_cleanedComponents.erase(entity);
        }
    }

    /**
     * @brief Check if entity exists and is active
     */
    bool IsEntityAlive(EntityID entity) const {
        return m_activeEntities.count(entity) > 0;
    }

    /**
     * @brief Get all active entities
     */
    const std::unordered_set<EntityID>& GetActiveEntities() const {
        return m_activeEntities;
    }

    /**
     * @brief Get entity count
     */
    unsigned int GetEntityCount() const {
        return static_cast<unsigned int>(m_activeEntities.size());
    }

    /**
     * @brief Clone an entity with all its components
     * @param sourceEntity Entity to clone
     * @return New entity ID with copied components, or INVALID_ENTITY if source doesn't exist
     */
    EntityID CloneEntity(EntityID entity) {
        // Validate source entity exists
        if (!IsEntityAlive(entity)) {
            return INVALID_ENTITY;
        }

        // Create new entity for the clone
        EntityID newEntity = CreateEntity();

        // Copy Transform2D component if present
        if (Transform2D* srcTransform = GetComponent<Transform2D>(entity)) {
            AddComponent<Transform2D>(newEntity, *srcTransform);
        }

        // Copy SpriteComponent if present
        if (SpriteComponent* srcSprite = GetComponent<SpriteComponent>(entity)) {
            AddComponent<SpriteComponent>(newEntity, *srcSprite);
        }

        // Copy Tag component if present
        if (Tag* srcTag = GetComponent<Tag>(entity)) {
            AddComponent<Tag>(newEntity, *srcTag);
        }

        // Copy PhysicsComponent if present
        if (PhysicsComponent* srcPhysics = GetComponent<PhysicsComponent>(entity)) {
            AddComponent<PhysicsComponent>(newEntity, *srcPhysics);
        }

        // Copy TextComponent if present
        if (TextComponent* srcText = GetComponent<TextComponent>(entity)) {
            AddComponent<TextComponent>(newEntity, *srcText);
        }

        // Copy AudioComponent if present
        if (AudioComponent* srcAudio = GetComponent<AudioComponent>(entity)) {
            AddComponent<AudioComponent>(newEntity, *srcAudio);
        }

        // Copy TileMapComponent if present
        if (TileMapComponent* srcTileMap = GetComponent<TileMapComponent>(entity)) {
            AddComponent<TileMapComponent>(newEntity, *srcTileMap);
        }

        return newEntity;
    }

    /**
     * @brief Reset entity ID counter and reusable pool
     * Use this before loading scenes to ensure deterministic entity ID assignment
     */
    void ResetEntityIDs() {
        m_reusableIDs.clear();
        m_nextEntityID = 1;
        m_dirtyEntities.clear();
        m_cleanedComponents.clear();
    }

    /**
     * @brief Clear all component storage
     * Call this when loading/creating scenes to prevent component data leakage
     */
    void ClearAllComponents() {
        // Clear all known component types
        GetStorage<Transform2D>().Clear();
        GetStorage<SpriteComponent>().Clear();
        GetStorage<PhysicsComponent>().Clear();
        GetStorage<AudioComponent>().Clear();
        GetStorage<Tag>().Clear();
        GetStorage<TileMapComponent>().Clear();
        GetStorage<TextComponent>().Clear();
    }

    // ==================== COMPONENT MANAGEMENT ====================

    /**
     * @brief Add or update component for entity
     * Automatically clears old component data if entity ID was recycled
     */
    template<typename T>
    T& AddComponent(EntityID entity, const T& component) {
        // If this is a recycled entity ID, remove any old component first
        if (m_dirtyEntities.count(entity) > 0) {
            GetStorage<T>().Erase(entity);
        }

        T& result = GetStorage<T>().Insert(entity, component);

        // Mark this component type as freshly added (for recycled entities)
        if (m_dirtyEntities.count(entity) > 0) {
            unsigned int typeID = GetTypeID<T>();
            m_cleanedComponents[entity].insert(typeID);
        }

        return result;
    }

    /**
     * @brief Get component from entity
     * Returns nullptr if entity is not alive or doesn't have component
     */
    template<typename T>
    T* GetComponent(EntityID entity) {
        // Check if entity is alive first
        if (!IsEntityAlive(entity)) {
            // Lazy cleanup: remove component from dead entity
            GetStorage<T>().Erase(entity);
            return nullptr;
        }

        // If entity ID was recycled, only return components that were freshly added
        if (m_dirtyEntities.count(entity) > 0) {
            unsigned int typeID = GetTypeID<T>();
            auto it = m_cleanedComponents.find(entity);

            // If this component type hasn't been freshly added, it's stale
            if (it == m_cleanedComponents.end() || it->second.count(typeID) == 0) {
                // Clean up stale component
                GetStorage<T>().Erase(entity);
                return nullptr;
            }
        }

        return GetStorage<T>().Retrieve(entity);
    }

    /**
     * @brief Check if entity has component
     * Returns false if entity is not alive or was recycled
     */
    template<typename T>
    bool HasComponent(EntityID entity) {
        if (!IsEntityAlive(entity)) {
            return false;
        }

        // Recycled entities don't have components until explicitly added
        if (m_dirtyEntities.count(entity) > 0) {
            unsigned int typeID = GetTypeID<T>();
            auto it = m_cleanedComponents.find(entity);

            // Only return true if this component type was freshly added
            if (it == m_cleanedComponents.end() || it->second.count(typeID) == 0) {
                return false;
            }
        }

        return GetStorage<T>().Exists(entity);
    }

    /**
     * @brief Remove component from entity
     */
    template<typename T>
    void RemoveComponent(EntityID entity) {
        GetStorage<T>().Erase(entity);
    }

    /**
     * @brief Get all components of type T for iteration
     */
    template<typename T>
    std::vector<T>& GetAllComponents() {
        return GetStorage<T>().GetData();
    }

    /**
     * @brief Register component type (for API compatibility)
     * Storage is created automatically, so this does nothing
     */
    template<typename T>
    void RegisterComponent() {
        // Automatic via template static storage
    }

    /**
     * @brief Get component type ID (for API compatibility)
     */
    template<typename T>
    unsigned int GetComponentType() {
        return GetTypeID<T>();
    }

private:
    // Entity management
    EntityID m_nextEntityID = 1;
    std::unordered_set<EntityID> m_activeEntities;
    std::vector<EntityID> m_reusableIDs;

    // Recycled entity tracking (prevents stale component access)
    std::unordered_set<EntityID> m_dirtyEntities;  // Entity IDs that have been recycled
    std::unordered_map<EntityID, std::unordered_set<unsigned int>> m_cleanedComponents;  // ComponentTypeID -> tracks fresh components on recycled entities

    /**
     * @brief Get component storage for type T
     * Uses function static to create one storage per component type
     */
    template<typename T>
    static ComponentStorage<T>& GetStorage() {
        static ComponentStorage<T> storage;
        return storage;
    }

    /**
     * @brief Get unique type ID for component T
     */
    template<typename T>
    static unsigned int GetTypeID() {
        static unsigned int id = s_typeCounter++;
        return id;
    }

    static inline unsigned int s_typeCounter = 0;
};

} // namespace GP2Engine
