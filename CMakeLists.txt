# Projects Settings
cmake_minimum_required(VERSION 3.10...3.28 FATAL_ERROR)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

project(GP2Engine LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Function to set common warning flags
function(gp2_set_common_warnings)
  if (MSVC)
    add_compile_options(/W4 /permissive- /MP /WX)
  else()
    add_compile_options(-Wall -Wextra -Wpedantic -Wdeprecated-copy -Wdeprecated-copy-dtor -Werror) 
  endif()
endfunction()

# Apply common warnings
gp2_set_common_warnings()

# Set the directory where your custom modules are located
set(CMAKE_MODULE_PATH "${CMAKE_CURRENT_LIST_DIR}/cmake")

# Import dependencies
include(ImportDependencies)
import_dependencies()

# Suppress warnings from third-party libraries
if(MSVC)
  # imgui - has uninitialized variable warnings
  if(TARGET imgui)
    target_compile_options(imgui PRIVATE /W0)
  endif()
endif()

ADD_SUBDIRECTORY(Engine)
ADD_SUBDIRECTORY(Editor)
ADD_SUBDIRECTORY(Sandbox)

# Set the Editor as the startup project for development
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT Editor)