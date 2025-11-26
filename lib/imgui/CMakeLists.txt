cmake_minimum_required(VERSION 3.20)
project(imgui LANGUAGES C CXX)

add_library(imgui STATIC
  imgui.cpp
  imgui_draw.cpp
  imgui_tables.cpp
  imgui_widgets.cpp
  imgui_demo.cpp
  backends/imgui_impl_glfw.cpp
  backends/imgui_impl_opengl3.cpp
)

target_include_directories(imgui PUBLIC
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/backends
)

if (TARGET libglew_static)
  target_link_libraries(imgui PUBLIC libglew_static)
  target_compile_definitions(imgui PUBLIC IMGUI_IMPL_OPENGL_LOADER_GLEW)
endif()

if (TARGET glfw)
  target_link_libraries(imgui PUBLIC glfw)
endif()
