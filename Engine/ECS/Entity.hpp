/**
 * @file Entity.hpp
 * @author Adi  (100%)
 * @brief Entity ID definition for GP2Engine ECS
 *
 * In the ECS architecture, entities are simple unique identifiers.
 * They have no data or behavior, they are just IDs that link components together.
 * Components hold the actual data, and systems operate on that data.
 */

#pragma once

namespace GP2Engine {
    /**
     * @brief Entity identifier type
     *
     * Entities are represented as unsigned integers.
     *
     * Valid entity IDs start from 1. ID 0 is reserved as INVALID_ENTITY.
     */
    using EntityID = unsigned int;

    /**
     * @brief Invalid/null entity constant
     *
     * Used to represent:
     * - Uninitialized entity references
     * - Failed entity creation
     * - Null entity pointers
     *
     * Entity IDs start from 1, so 0 is always invalid.
     */
    constexpr EntityID INVALID_ENTITY = 0;
}