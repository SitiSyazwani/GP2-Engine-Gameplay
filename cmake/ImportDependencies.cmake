# ImportDependencies.cmake
# Manages external dependencies for GP2Engine
include(FetchContent)

# Macro to import GLFW
macro(import_glfw)
    if(NOT TARGET glfw)
        message(STATUS "Importing GLFW...")
        FetchContent_Declare(
            glfw
            GIT_REPOSITORY https://github.com/glfw/glfw.git
            GIT_TAG 3.3.8
        )
        
        # Configure GLFW
        set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
        set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
        set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
        
        FetchContent_MakeAvailable(glfw)
        message(STATUS "GLFW imported successfully.")
    endif()
endmacro()

# Macro to import GLM
macro(import_glm)
    if(NOT TARGET glm)
        message(STATUS "Importing GLM...")
        FetchContent_Declare(
            glm
            GIT_REPOSITORY https://github.com/g-truc/glm.git
            GIT_TAG 1.0.1
        )
        FetchContent_MakeAvailable(glm)
        message(STATUS "GLM imported successfully.")
    endif()
endmacro()

# Macro to import ImGUI
macro(import_imgui)
    if(NOT TARGET imgui)
        message(STATUS "Importing ImGUI...")
        FetchContent_Declare(
            imgui
            GIT_REPOSITORY https://github.com/ocornut/imgui.git
            GIT_TAG docking  # Use docking branch for docking support
        )
        FetchContent_MakeAvailable(imgui)
        
        # Create ImGUI target
        set(IMGUI_SOURCES
            ${imgui_SOURCE_DIR}/imgui.cpp
            ${imgui_SOURCE_DIR}/imgui_demo.cpp
            ${imgui_SOURCE_DIR}/imgui_draw.cpp
            ${imgui_SOURCE_DIR}/imgui_tables.cpp
            ${imgui_SOURCE_DIR}/imgui_widgets.cpp
            ${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp
            ${imgui_SOURCE_DIR}/backends/imgui_impl_opengl3.cpp
        )
        
        add_library(imgui STATIC ${IMGUI_SOURCES})
        
        target_include_directories(imgui PUBLIC 
            ${imgui_SOURCE_DIR}
            ${imgui_SOURCE_DIR}/backends
        )
        
        target_link_libraries(imgui PUBLIC glfw)
        message(STATUS "ImGUI imported successfully.")
    endif()
endmacro()

# Macro to import STB
macro(import_stb)
    if(NOT TARGET stb)
        message(STATUS "Importing STB...")
        
        # Set STB paths
        set(STB_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/stb")
        
        # Check if STB directory exists
        if(NOT EXISTS "${STB_INCLUDE_DIR}")
            message(FATAL_ERROR "STB include directory not found at ${STB_INCLUDE_DIR}")
        endif()
        
        # Create STB interface library (header-only)
        add_library(stb INTERFACE)
        target_include_directories(stb INTERFACE "${STB_INCLUDE_DIR}")
        
        message(STATUS "STB imported successfully from ${STB_INCLUDE_DIR}")
    endif()
endmacro()

# Macro to import FreeType
macro(import_freetype)
    if(NOT TARGET freetype)
        message(STATUS "Importing FreeType...")

        # Set FreeType paths - everything in lib/Freetype
        set(FREETYPE_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/Freetype")
        set(FREETYPE_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/Freetype/freetype")

        # Check if FreeType directories exist
        if(NOT EXISTS "${FREETYPE_LIB_DIR}")
            message(WARNING "FreeType library directory not found at ${FREETYPE_LIB_DIR} - FreeType support disabled")
            set(FREETYPE_FOUND FALSE)
            return()
        endif()

        if(NOT EXISTS "${FREETYPE_INCLUDE_DIR}")
            message(WARNING "FreeType include directory not found at ${FREETYPE_INCLUDE_DIR} - FreeType support disabled")
            set(FREETYPE_FOUND FALSE)
            return()
        endif()
        
        set(FREETYPE_FOUND TRUE)
        
        # Set platform-specific file extensions and prefixes
        if(WIN32)
            set(FREETYPE_LIB_EXT ".dll")
            set(FREETYPE_IMPORT_EXT ".lib")
        else()
            set(FREETYPE_LIB_EXT ".so")
            set(FREETYPE_IMPORT_EXT ".a")
        endif()
        
        # Create FreeType target
        add_library(freetype SHARED IMPORTED)
        
        if(WIN32)
            set_target_properties(freetype PROPERTIES
                IMPORTED_LOCATION "${FREETYPE_LIB_DIR}/freetype${FREETYPE_LIB_EXT}"
                IMPORTED_IMPLIB "${FREETYPE_LIB_DIR}/freetype${FREETYPE_IMPORT_EXT}"
                INTERFACE_INCLUDE_DIRECTORIES "${FREETYPE_INCLUDE_DIR}"
            )
        else()
            set_target_properties(freetype PROPERTIES
                IMPORTED_LOCATION "${FREETYPE_LIB_DIR}/freetype${FREETYPE_LIB_EXT}"
                INTERFACE_INCLUDE_DIRECTORIES "${FREETYPE_INCLUDE_DIR}"
            )
        endif()
        
        # Set target properties for organization
        set_target_properties(freetype PROPERTIES FOLDER "ThirdParty")
        
        # We'll handle DLL copying in the main CMakeLists.txt after the target is created
        set(FREETYPE_DLL_PATH "${FREETYPE_LIB_DIR}/freetype${FREETYPE_LIB_EXT}" CACHE INTERNAL "Path to FreeType DLL")
        
        message(STATUS "FreeType imported successfully from ${FREETYPE_LIB_DIR}")
    endif()
endmacro()

# Macro to import GLAD
macro(import_glad)
    if(NOT TARGET glad)
        message(STATUS "Importing GLAD...")
        
        # Set GLAD paths
        set(GLAD_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/glad/include")
        set(GLAD_SRC_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/glad/src")
        
        # Check if GLAD directories exist
        if(NOT EXISTS "${GLAD_INCLUDE_DIR}")
            message(FATAL_ERROR "GLAD include directory not found at ${GLAD_INCLUDE_DIR}")
        endif()
        
        if(NOT EXISTS "${GLAD_SRC_DIR}/glad.c")
            message(FATAL_ERROR "GLAD source file not found at ${GLAD_SRC_DIR}/glad.c")
        endif()
        
        # Create GLAD library with source file
        add_library(glad STATIC "${GLAD_SRC_DIR}/glad.c")
        target_include_directories(glad PUBLIC "${GLAD_INCLUDE_DIR}")
        
        message(STATUS "GLAD imported successfully from ${GLAD_INCLUDE_DIR} with source file")
    endif()
endmacro()

# Macro to import FMOD
macro(import_fmod)
    if(NOT TARGET fmod)
        message(STATUS "Importing FMOD...")

        # Set FMOD paths - everything in lib/FMOD
        set(FMOD_LIB_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/FMOD")
        set(FMOD_INCLUDE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/lib/FMOD/include")

        # Check if FMOD directories exist
        if(NOT EXISTS "${FMOD_LIB_DIR}")
            message(WARNING "FMOD library directory not found at ${FMOD_LIB_DIR} - FMOD support disabled")
            set(FMOD_FOUND FALSE)
            return()
        endif()

        if(NOT EXISTS "${FMOD_INCLUDE_DIR}")
            message(WARNING "FMOD include directory not found at ${FMOD_INCLUDE_DIR} - FMOD support disabled")
            set(FMOD_FOUND FALSE)
            return()
        endif()
        
        set(FMOD_FOUND TRUE)
        
        # Set platform-specific file extensions and prefixes
        if(WIN32)
            set(FMOD_LIB_EXT ".dll")
            set(FMOD_IMPORT_EXT "_vc.lib")
        elseif(APPLE)
            set(FMOD_LIB_EXT ".dylib")
            set(FMOD_LIB_PREFIX "lib")
        elseif(UNIX)
            set(FMOD_LIB_EXT ".so")
            set(FMOD_LIB_PREFIX "lib")
        endif()
        
        # Create FMOD Core target
        add_library(fmod SHARED IMPORTED)
        
        if(WIN32)
            set_target_properties(fmod PROPERTIES
                IMPORTED_LOCATION "${FMOD_LIB_DIR}/fmod${FMOD_LIB_EXT}"
                IMPORTED_IMPLIB "${FMOD_LIB_DIR}/fmod${FMOD_IMPORT_EXT}"
                INTERFACE_INCLUDE_DIRECTORIES "${FMOD_INCLUDE_DIR}"
            )
        else()
            set_target_properties(fmod PROPERTIES
                IMPORTED_LOCATION "${FMOD_LIB_DIR}/${FMOD_LIB_PREFIX}fmod${FMOD_LIB_EXT}"
                INTERFACE_INCLUDE_DIRECTORIES "${FMOD_INCLUDE_DIR}"
            )
        endif()
        
        # Create FMOD Studio target (optional)
        add_library(fmodstudio SHARED IMPORTED)
        
        if(WIN32)
            set_target_properties(fmodstudio PROPERTIES
                IMPORTED_LOCATION "${FMOD_LIB_DIR}/fmodstudio${FMOD_LIB_EXT}"
                IMPORTED_IMPLIB "${FMOD_LIB_DIR}/fmodstudio${FMOD_IMPORT_EXT}"
                INTERFACE_INCLUDE_DIRECTORIES "${FMOD_INCLUDE_DIR}"
            )
        else()
            set_target_properties(fmodstudio PROPERTIES
                IMPORTED_LOCATION "${FMOD_LIB_DIR}/${FMOD_LIB_PREFIX}fmodstudio${FMOD_LIB_EXT}"
                INTERFACE_INCLUDE_DIRECTORIES "${FMOD_INCLUDE_DIR}"
            )
        endif()
        
        message(STATUS "FMOD imported successfully from ${FMOD_LIB_DIR}")
    endif()
endmacro()


# Macro to import nlohmann JSON
macro(import_nlohmann_json)
    if(NOT TARGET nlohmann_json)
        message(STATUS "Importing nlohmann JSON...")
        FetchContent_Declare(
            nlohmann_json
            GIT_REPOSITORY https://github.com/nlohmann/json.git
            GIT_TAG v3.11.3
        )
        FetchContent_MakeAvailable(nlohmann_json)
        message(STATUS "nlohmann JSON imported successfully.")
    endif()
endmacro()

# Macro to import all dependencies
macro(import_dependencies)
    message(STATUS "Starting to import dependencies...")
    import_glfw()
    import_glm()
    import_imgui()
    import_stb()
    import_glad()
    import_fmod()
    import_freetype()
    import_nlohmann_json()
    message(STATUS "All dependencies imported successfully.")
endmacro()