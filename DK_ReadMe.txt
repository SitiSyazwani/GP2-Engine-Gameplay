GP2Engine

Game Concept

HOLLOWS — A Stealth Survival Horror Experience

You are a paranormal investigator trapped inside the Holloway Correctional Facility, a prison shut down after sound therapy experiments twisted the inmates into creatures that hunt by sound.

To escape, you must scavenge materials to craft tools, using stealth to avoid making noise that attracts the 'Listeners'. Your curiosity opened this Box, and your craftiness is your only way out. Sprint to escape, but it drains stamina and makes noise. Use your flashlight to avoid broken tiles that trigger sound, but it consumes precious battery.

Move silently, manage your resources, and survive. One sound at the wrong time, and it's over.

====================================================================================================

Engine Overview

GP2Engine is a custom 2D game engine built from scratch in C++. It provides a comprehensive foundation for creating 2D games. The engine features a modular architecture with an Entity-Component-System (ECS) design, making it highly extensible and performant.

====================================================================================================

Core Engine Features

ECS Architecture: Entity-Component-System for flexible game object management
Graphics System: OpenGL-based 2D rendering with sprite support, texture management, and camera systems
Physics System: Collision detection and resolution with support for dynamic objects
Audio System: FMOD integration for high-quality audio playback
Input System: Comprehensive keyboard and mouse input handling with three-state tracking (Pressed/Held/Released)
Serialization: JSON-based configuration and save system
TileMap System: Efficient tile-based rendering for level design
Debug Tools: Built-in profiler, logger, and debug renderer
ImGui Integration: Real-time debug interface for development

====================================================================================================

Hollows Demo

The Hollows demo showcases the engine's capabilities through an interactive 2D environment with:

Demo Features

Player Controller: WASD movement with collision detection and sprint functionality
Debug Interface: Real-time debugging tools accessible via F1
Performance Monitoring: Frame time tracking and system performance metrics
Entity Inspector: Live entity and component editing capabilities
Stress Testing: Performance testing with 2,500+ animated objects
Audio System: Sound effects triggered by right mouse button
Collision System: Real-time collision detection and resolution

====================================================================================================

Controls

Key: WASD - Action: Player movement
Key: Left Shift - Action: Sprint
Key: Right Mouse Button - Action: Play sound effect
Key: F1 - Action: Toggle debug interface
Key: ESC - Action: Quit application

====================================================================================================

Using the ImGui Debug Interface

Press F1 to open the debug interface.

Main Debug Controls Panel

Checkboxes: Enable/disable individual debug panels
Simulate Crash: Test the engine's crash-handling system
Stress Test Controls: Access performance testing tools

Performance Panel

Frame Time Graph: Visual representation of frame performance over time
FPS Counter: Current frames per second
System Performance: Breakdown of time spent in different systems (Input, Physics, Graphics, UI)
Memory Usage: Monitor engine memory consumption

Entity Inspector Panel

Entity List: View all entities in the scene
Create New Entity: Add new game objects to the scene
Copy Entity: Duplicate existing entities
Delete Entity: Remove entities from the scene
Entity Details: Inspect and modify entity components including transforms, sprites, physics properties, and tags

Player Controls Panel

Speed Slider: Adjust player movement speed in real-time
Reset Position: Teleport player back to the center
Movement Controls: Reference for WASD and Shift controls

Debug Visualization Panel

Collision Boxes: Toggle visibility of entity collision boundaries
Velocity Vectors: Show movement direction and speed as arrows
Debug Rendering: Enable/disable debug graphics overlays

Stress Test Panel

Object Count: Set the number of test objects (default: 2,500)
Animation Speed: Control how fast test objects move
Start/Stop Test: Spawn or remove stress test objects
Performance Impact: Monitor how stress testing affects frame rate

Controls Help Panel

Key Reference: Complete list of all available controls
Mouse Controls: Information about mouse interactions
Debug Shortcuts: Quick reference for debug interface keys

====================================================================================================

Usage & Setup

Building the Project

1. Prerequisites:

   CMake 3.16+
   Visual Studio 2019+ (Windows) or compatible C++ compiler
   OpenGL 3.3+ support

2. Build Instructions:

   Clone the repository
   git clone [repository-url]
   cd GP2Engine

   Build using the provided script
   build_engine.bat

3. Running the Demo:

   Navigate to build/Sandbox/Debug/
   Execute Hollows.exe
   Press F1 to access debug tools

====================================================================================================

Development Team

Core Team Members

Name: Adi - Role: Tech Lead - Contributions: Core application framework, ECS system, System Manager, Time system, Logger, Profiler, Input system
Name: Asri - Role: Programmer - Contributions: Renderer, Shader system, Sprite management, Camera system, Texture handling, Debug rendering, Graphics pipeline
Name: Fauzan - Role: Programmer - Contributions: Physics system, Collision detection, Audio engine (FMOD integration), Audio components
Name: Rifqah - Role: Product Manager & Programmer - Contributions: Product vision and direction, team coordination, Vector2D, Matrix3x3, Math utilities, JSON serialization, configuration system, data persistence
Name: Syazwani - Role: Programmer - Contributions: Level editor, ImGui integration, Debug interface development

Leadership & Championing

ECS Champion: Adi — technical lead, overseeing architecture and core systems
Graphics Champion: Asri — leads graphics and rendering pipeline development
Audio/Physics/Collision Champion: Fauzan — physics and audio system integration
Story & Level Design Champion: Rifqah — mathematical utilities and data management systems
Level Editor Champion: Syazwani — tools and interfaces for usability and debugging

====================================================================================================

Development Philosophy

The team follows modern C++ practices with:

Modular Design: Each system is independently developed and tested
Performance Focus: Optimized for real-time 2D game development
Developer Experience: Comprehensive debugging tools and clear APIs
Documentation: Extensive code documentation and examples

====================================================================================================

Architecture Overview

GP2Engine/
├── Engine/             Core engine systems
│   ├── Core/           Application, Input, Time, Logger
│   ├── ECS/            Entity-Component-System
│   ├── Graphics/       Rendering pipeline
│   ├── Physics/        Collision and physics
│   ├── Audio/          Sound system
│   ├── Math/           Mathematical utilities
│   ├── Serialization/  Data persistence
│   └── TileMap/        Tile-based rendering
├── Sandbox/            Demo applications
│   └── Hollows/        Main demo game
├── Editor/             Game editor tools
└── lib/                Third-party libraries

====================================================================================================

Performance Features

Target FPS: 60 FPS with adaptive frame limiting
Stress Testing: Handles 2,500+ entities with smooth performance
Memory Management: RAII-based resource management
Profiling: Built-in performance profiling and monitoring
Optimization: Efficient ECS architecture for minimal overhead

====================================================================================================

Summary

GP2Engine represents a comprehensive 2D game engine solution, demonstrating modern C++ game development practices with a focus on performance, modularity, and developer experience.