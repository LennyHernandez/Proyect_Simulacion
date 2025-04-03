cmake_minimum_required(VERSION 3.16)
project(SimulacionParticulas)

# Especifica el estándar de C++
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# Detectar el compilador
if (CMAKE_CXX_COMPILER_ID MATCHES "Clang")
    message(STATUS "Usando Clang")
elseif (CMAKE_CXX_COMPILER_ID MATCHES "GNU")
    message(STATUS "Usando GCC/G++")
else()
    message(FATAL_ERROR "Compilador no soportado. Usa Clang o G++")
endif()

# Habilitar optimizaciones si es Release
if (CMAKE_BUILD_TYPE STREQUAL "Release")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -O3 -march=native")
endif()

# Configurar salida en 1080p
add_definitions(-DAPP_SCREEN_WIDTH=1920 -DAPP_SCREEN_HEIGHT=1080)

# Incluir directorios de librerías externas
include_directories(
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/external/glfw/include
    ${CMAKE_SOURCE_DIR}/external/glm
    ${CMAKE_SOURCE_DIR}/external/imgui
    ${CMAKE_SOURCE_DIR}/external/imgui/backends
)

# Archivos fuente
set(SOURCES
    src/main.cpp
    src/vulkan_setup.cpp
    src/particle_system.cpp
    src/gui.cpp
    external/imgui/imgui.cpp
    external/imgui/imgui_draw.cpp
    external/imgui/imgui_tables.cpp
    external/imgui/imgui_widgets.cpp
    external/imgui/backends/imgui_impl_glfw.cpp
    external/imgui/backends/imgui_impl_vulkan.cpp
)

# Buscar Vulkan
find_package(Vulkan REQUIRED)

# Crear el ejecutable
add_executable(SimulacionParticulas ${SOURCES})

# Enlazar librerías
target_link_libraries(SimulacionParticulas Vulkan::Vulkan glfw)