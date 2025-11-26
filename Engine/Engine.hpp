#pragma once

// Core modules
#include "Core/Application.hpp"
#include "Core/SystemManager.hpp"
#include "Core/EventSystem.hpp"
#include "Core/Input.hpp"
#include "Core/Time.hpp"
#include "Core/Logger.hpp"
#include "Core/Profiler.hpp"
#include "Core/Layer.hpp"
#include "Core/LayerStack.hpp"

// ECS modules
#include "ECS/Entity.hpp"
#include "ECS/Component.hpp"
#include "ECS/Registry.hpp"
#include "ECS/Systems.hpp"

// Graphics modules
#include "Graphics/Camera.hpp"
#include "Graphics/Renderer.hpp"
#include "Graphics/DebugRenderer.hpp"
#include "Graphics/Shader.hpp"
#include "Graphics/Sprite.hpp"
#include "Graphics/Texture.hpp"
#include "Graphics/Font.hpp"
#include "Graphics/Framebuffer.hpp"

// Audio modules
#include "Audio/AudioEngine.hpp"

// Math modules
#include "Math/MathUtils.hpp"
#include "Math/Matrix3x3.hpp"
#include "Math/Vector2D.hpp"

// Physics modules
#include "Physics/PhysicsSystem.hpp"

// Serialization modules
#include "Serialization/ConfigLoader.hpp"
#include "Serialization/JsonSerializer.hpp"

// Resources modules
#include "Resources/ResourceManager.hpp"

// TileMap modules
#include "TileMap/TileMap.hpp"
#include "TileMap/TileRenderer.hpp"
#include "TileMap/LevelEditor.hpp"

// AI modules (integrated with ECS)
#include "AI/AISystem.hpp"

// UI modules
#include "UI/MainMenu.h"

// Editor modules have been moved to standalone Editor application