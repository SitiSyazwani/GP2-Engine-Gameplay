# Engine CMakeLists.txt
set(ENGINE_NAME Engine)

# Collect all .cpp and .hpp files from component directories
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Core/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Math/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ECS/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Graphics/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Physics/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Serialization/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/TileMap/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Resources/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/AI/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/UI/*.cpp"
)

file(GLOB_RECURSE ENGINE_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Core/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Math/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/ECS/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Graphics/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Physics/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Serialization/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/TileMap/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Resources/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/AI/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/UI/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/UI/*.hpp"
)

# Conditionally include Audio files only if FMOD is available
if(FMOD_FOUND)
    file(GLOB_RECURSE AUDIO_SOURCES CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/Audio/*.cpp"
    )
    file(GLOB_RECURSE AUDIO_HEADERS CONFIGURE_DEPENDS
        "${CMAKE_CURRENT_SOURCE_DIR}/Audio/*.hpp"
    )
    list(APPEND ENGINE_SOURCES ${AUDIO_SOURCES})
    list(APPEND ENGINE_HEADERS ${AUDIO_HEADERS})
endif()

set(ALL_FILES ${ENGINE_SOURCES} ${ENGINE_HEADERS})

# Create the Engine library
add_library(${ENGINE_NAME} STATIC ${ALL_FILES})
set_property(TARGET ${ENGINE_NAME} PROPERTY CXX_STANDARD 20)

# Organize files by component in Visual Studio Solution Explorer
# Use TREE structure to organize by directory
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES ${ALL_FILES})

# Include the Engine directory so components can include each other
target_include_directories(${ENGINE_NAME} PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# Include directories for FreeType
target_include_directories(${ENGINE_NAME} PUBLIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/Freetype
    ${CMAKE_CURRENT_SOURCE_DIR}/../lib/Freetype/freetype
)

# Find OpenGL
find_package(OpenGL REQUIRED)

# Link dependencies
target_link_libraries(${ENGINE_NAME} PUBLIC
    glfw
    glm
    imgui      # Link ImGui for debug UI
    glad       # Link GLAD for OpenGL loading
    stb        # Link STB for image loading
    nlohmann_json::nlohmann_json  # Link nlohmann JSON for serialization
    OpenGL::GL
    freetype   # Link FreeType for font rendering
)

# Conditionally link FMOD libraries
if(FMOD_FOUND)
    target_link_libraries(${ENGINE_NAME} PUBLIC
        fmod
        fmodstudio
    )
    target_compile_definitions(${ENGINE_NAME} PRIVATE FMOD_FOUND)
endif()

