project(Hollows LANGUAGES C CXX)

# Collect all Hollows game files
file(GLOB_RECURSE SANDBOX_SRC CONFIGURE_DEPENDS
  "${CMAKE_CURRENT_SOURCE_DIR}/Hollows/*.cpp"
)
file(GLOB_RECURSE SANDBOX_HEADERS CONFIGURE_DEPENDS
    "${CMAKE_CURRENT_SOURCE_DIR}/Hollows/*.hpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/Hollows/*.h"
)

set(ALL_SANDBOX_FILES ${SANDBOX_SRC} ${SANDBOX_HEADERS})

add_executable(Hollows ${ALL_SANDBOX_FILES})

# Link against the Engine library
target_link_libraries(Hollows PRIVATE Engine)

set_property(TARGET Hollows PROPERTY CXX_STANDARD 20)

# Organize files by component in Visual Studio Solution Explorer
source_group("Components" FILES ${CMAKE_CURRENT_SOURCE_DIR}/Hollows/Components/*)
source_group("Systems" FILES ${CMAKE_CURRENT_SOURCE_DIR}/Hollows/Systems/*)
source_group("GameStates" FILES ${CMAKE_CURRENT_SOURCE_DIR}/Hollows/GameStates/*)
source_group("Main" FILES ${CMAKE_CURRENT_SOURCE_DIR}/Hollows/main.cpp)

# Copy DLLs (Windows only)
if(WIN32)
    set(FMOD_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/../lib/FMOD")
    add_custom_command(TARGET Hollows POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${CMAKE_CURRENT_SOURCE_DIR}/../lib/Freetype/freetype.dll"
            "$<TARGET_FILE_DIR:Hollows>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${FMOD_LIB_DIR}/fmod.dll"
            "$<TARGET_FILE_DIR:Hollows>"
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${FMOD_LIB_DIR}/fmodstudio.dll"
            "$<TARGET_FILE_DIR:Hollows>"
        COMMENT "Copying DLLs"
    )
endif()

# Copy assets to build directory
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/assets")
    add_custom_command(TARGET Hollows POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_directory
            "${CMAKE_CURRENT_SOURCE_DIR}/assets"
            "${CMAKE_CURRENT_BINARY_DIR}/assets"
        COMMENT "Copying assets"
    )
endif()