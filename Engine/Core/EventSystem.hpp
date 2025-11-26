/**
 * @file EventSystem.hpp
 * @brief Event system with Observer pattern for decoupled messaging
 * @author Asri (100%)
 *
 * Implements a publish-subscribe event system allowing systems to communicate
 * without direct dependencies. Follows the Observer/Observable design pattern.
 *
 * Features:
 * - Type-safe event handling with templates
 * - Multiple listeners per event type
 * - Subscribe/unsubscribe at runtime
 * - Event broadcasting to all subscribers
 *
 * Example usage:
 * @code
 * struct CollisionEvent {
 *     EntityID entity1;
 *     EntityID entity2;
 *     Vector2D collisionPoint;
 * };
 *
 * // Subscribe to event
 * auto listenerId = EventSystem::Subscribe<CollisionEvent>([](const CollisionEvent& e) {
 *     std::cout << "Collision between " << e.entity1 << " and " << e.entity2 << std::endl;
 * });
 *
 * // Publish event
 * CollisionEvent event{player, enemy, Vector2D(100, 200)};
 * EventSystem::Publish(event);
 *
 * // Unsubscribe
 * EventSystem::Unsubscribe<CollisionEvent>(listenerId);
 * @endcode
 */

#pragma once

#include <functional>
#include <unordered_map>
#include <vector>
#include <typeindex>
#include <memory>
#include <algorithm>

namespace GP2Engine {

    /**
     * @brief Central event bus for publish-subscribe messaging
     *
     * Singleton event system managing event subscriptions and publishing.
     * Uses type erasure to support any event type.
     */
    class EventSystem {
    public:
        using ListenerID = size_t;

        /**
         * @brief Get singleton instance
         */
        static EventSystem& GetInstance() {
            static EventSystem instance;
            return instance;
        }

        /**
         * @brief Subscribe to an event type
         *
         * @tparam EventType The event struct/class to listen for
         * @param callback Function to call when event is published
         * @return Listener ID for unsubscribing
         */
        template<typename EventType>
        static ListenerID Subscribe(std::function<void(const EventType&)> callback) {
            return GetInstance().SubscribeInternal<EventType>(callback);
        }

        /**
         * @brief Unsubscribe from an event type
         *
         * @tparam EventType The event type
         * @param listenerId ID returned from Subscribe()
         */
        template<typename EventType>
        static void Unsubscribe(ListenerID listenerId) {
            GetInstance().UnsubscribeInternal<EventType>(listenerId);
        }

        /**
         * @brief Publish an event to all subscribers
         *
         * @tparam EventType The event type
         * @param event The event data to publish
         */
        template<typename EventType>
        static void Publish(const EventType& event) {
            GetInstance().PublishInternal(event);
        }

        /**
         * @brief Clear all event listeners (useful for cleanup)
         */
        static void ClearAll() {
            GetInstance().m_listeners.clear();
        }

        /**
         * @brief Get number of subscribers for an event type
         */
        template<typename EventType>
        static size_t GetSubscriberCount() {
            return GetInstance().GetSubscriberCountInternal<EventType>();
        }

    private:
        EventSystem() = default;
        ~EventSystem() = default;

        // Prevent copying
        EventSystem(const EventSystem&) = delete;
        EventSystem& operator=(const EventSystem&) = delete;

        // Type-erased listener storage
        struct IListenerWrapper {
            IListenerWrapper() : id(0) {}
            virtual ~IListenerWrapper() = default;
            ListenerID id;  // Initialized in constructor
        };

        template<typename EventType>
        struct ListenerWrapper : public IListenerWrapper {
            std::function<void(const EventType&)> callback;

            ListenerWrapper(ListenerID listenerId, std::function<void(const EventType&)> cb)
                : callback(cb) {
                id = listenerId;
            }
        };

        // Map: EventType -> List of listeners
        std::unordered_map<std::type_index, std::vector<std::shared_ptr<IListenerWrapper>>> m_listeners;

        // Counter for generating unique listener IDs
        ListenerID m_nextListenerId = 1;

        template<typename EventType>
        ListenerID SubscribeInternal(std::function<void(const EventType&)> callback) {
            std::type_index eventType = typeid(EventType);

            // Create listener wrapper
            ListenerID listenerId = m_nextListenerId++;
            auto wrapper = std::make_shared<ListenerWrapper<EventType>>(listenerId, callback);

            // Add to listeners map
            m_listeners[eventType].push_back(wrapper);

            return listenerId;
        }

        template<typename EventType>
        void UnsubscribeInternal(ListenerID listenerId) {
            std::type_index eventType = typeid(EventType);

            auto it = m_listeners.find(eventType);
            if (it == m_listeners.end()) return;

            // Remove listener with matching ID
            auto& listeners = it->second;
            listeners.erase(
                std::remove_if(listeners.begin(), listeners.end(),
                    [listenerId](const std::shared_ptr<IListenerWrapper>& wrapper) {
                        return wrapper->id == listenerId;
                    }),
                listeners.end()
            );
        }

        template<typename EventType>
        void PublishInternal(const EventType& event) {
            std::type_index eventType = typeid(EventType);

            auto it = m_listeners.find(eventType);
            if (it == m_listeners.end()) return;

            // Call all subscribed callbacks
            for (const auto& listenerWrapper : it->second) {
                auto* typedWrapper = dynamic_cast<ListenerWrapper<EventType>*>(listenerWrapper.get());
                if (typedWrapper && typedWrapper->callback) {
                    typedWrapper->callback(event);
                }
            }
        }

        template<typename EventType>
        size_t GetSubscriberCountInternal() {
            std::type_index eventType = typeid(EventType);

            auto it = m_listeners.find(eventType);
            if (it == m_listeners.end()) return 0;

            return it->second.size();
        }
    };

} // namespace GP2Engine
