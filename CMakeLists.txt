cmake_minimum_required(VERSION 3.10)
project(ParticleSimulation)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# --- Encuentra GLSLc ---
find_program(GLSLC_EXECUTABLE ...)
message(...)

# --- Compilar Shaders ---
# (Sin cambios - add_custom_command / add_custom_target)
set(SHADER_SOURCE_DIR ...)
set(SHADER_OUTPUT_DIR ...)
file(MAKE_DIRECTORY ${SHADER_OUTPUT_DIR})
file(GLOB SHADER_SOURCE_FILES ...)
set(SPIRV_GENERATED_FILES "")
foreach(SHADER_SOURCE_FILE ${SHADER_SOURCE_FILES})
    # ... add_custom_command ...
endforeach()
add_custom_target(Shaders ALL DEPENDS ${SPIRV_GENERATED_FILES})
message(...)
message(...)

# --- Bibliotecas Externas ---
add_subdirectory(external/glfw)
add_subdirectory(external/imgui) # <-- RE-AÑADIDO

# --- Buscar Vulkan ---
find_package(Vulkan REQUIRED)

# --- Código Fuente Principal ---
add_subdirectory(src)

# --- Configuración del Target (Definido en src/) ---
add_dependencies(ParticleSimulation Shaders)

# Incluir directorios externos (ImGui también necesita ser incluido aquí)
target_include_directories(ParticleSimulation PRIVATE
    "${CMAKE_CURRENT_SOURCE_DIR}/external/glfw/include"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/glm"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/imgui" # Necesario para #include "imgui.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/external/imgui/backends" # Necesario para backends si los usas
)