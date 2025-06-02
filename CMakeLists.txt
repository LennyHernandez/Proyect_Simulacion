cmake_minimum_required(VERSION 3.14) # Requerir versión con buen soporte FetchContent
project(ParticleSimulation LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# --- Gestión de Dependencias con FetchContent ---
include(FetchContent)
# set(FETCHCONTENT_QUIET OFF) # Descomentar para ver detalles de descarga/configuración

# -- GLFW --
message(STATUS "Setting up GLFW dependency...")
FetchContent_Declare(
    glfw            # Nombre para CMake
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4 # Usar una etiqueta de versión estable (o commit)
    CMAKE_ARGS     -DGLFW_BUILD_EXAMPLES=OFF -DGLFW_BUILD_TESTS=OFF -DGLFW_BUILD_DOCS=OFF # Opcional
)
FetchContent_MakeAvailable(glfw)
message(STATUS "GLFW available. Source: ${glfw_SOURCE_DIR}")

# -- ImGui --
message(STATUS "Setting up ImGui dependency...")
FetchContent_Declare(
    imgui           # Nombre para CMake
    GIT_REPOSITORY https://github.com/ocornut/imgui.git
    GIT_TAG        v1.90.8 # Usar una etiqueta de versión estable (o docking)
    CMAKE_ARGS     -DIMGUI_BUILD_EXAMPLES=OFF # Opcional
)
FetchContent_MakeAvailable(imgui)
message(STATUS "ImGui available. Source: ${imgui_SOURCE_DIR}")

# -- GLM --
message(STATUS "Setting up GLM dependency...")
FetchContent_Declare(
    glm             # Nombre para CMake
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1 # Usar una etiqueta de versión estable
)
FetchContent_MakeAvailable(glm)
message(STATUS "GLM available. Source: ${glm_SOURCE_DIR}")

# --- Encuentra GLSLc ---
find_program(GLSLC_EXECUTABLE NAMES glslc HINTS "$ENV{VULKAN_SDK}/Bin" "$ENV{VULKAN_SDK}/bin" 
PATHS
        /usr/bin # Ruta común del sistema en Linux
    REQUIRED # Sigue siendo requerido
    DOC "Path to the GLSL to SPIR-V compiler (glslc)"
)
message(STATUS "Found glslc compiler: ${GLSLC_EXECUTABLE}")

# --- Compilar Shaders ---
set(SHADER_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/shaders")
set(SHADER_OUTPUT_DIR "${CMAKE_CURRENT_BINARY_DIR}/shaders")
file(MAKE_DIRECTORY ${SHADER_OUTPUT_DIR})
file(GLOB SHADER_SOURCE_FILES "${SHADER_SOURCE_DIR}/*.vert" "${SHADER_SOURCE_DIR}/*.frag")
set(SPIRV_GENERATED_FILES "")
foreach(SHADER_SOURCE_FILE ${SHADER_SOURCE_FILES})
    get_filename_component(SHADER_BASENAME ${SHADER_SOURCE_FILE} NAME)
    set(SPIRV_OUTPUT_FILE "${SHADER_OUTPUT_DIR}/${SHADER_BASENAME}.spv")
    list(APPEND SPIRV_GENERATED_FILES ${SPIRV_OUTPUT_FILE})
    add_custom_command( OUTPUT ${SPIRV_OUTPUT_FILE} COMMAND ${GLSLC_EXECUTABLE} ${SHADER_SOURCE_FILE} -o ${SPIRV_OUTPUT_FILE}
        DEPENDS ${SHADER_SOURCE_FILE} COMMENT "Compiling ${SHADER_BASENAME} to SPIR-V" VERBATIM )
endforeach()
add_custom_target(Shaders ALL DEPENDS ${SPIRV_GENERATED_FILES})
message(STATUS "SPIR-V shader output directory: ${SHADER_OUTPUT_DIR}")

# --- Buscar Vulkan ---
find_package(Vulkan REQUIRED)
message(STATUS "Found Vulkan: ${Vulkan_LIBRARIES}")

# --- Código Fuente Principal ---
# add_subdirectory(external/glfw) # <-- ¡¡ELIMINADA!!
# add_subdirectory(external/imgui) # <-- ¡¡ELIMINADA!!
add_subdirectory(src) # <-- Esta sí se queda

# --- Configuración del Target 'ParticleSimulation' ---
add_dependencies(ParticleSimulation Shaders)

target_include_directories(ParticleSimulation PRIVATE
    # Directorios de dependencias descargadas
    ${glfw_SOURCE_DIR}/include
    ${imgui_SOURCE_DIR}
    ${imgui_SOURCE_DIR}/backends
    ${glm_SOURCE_DIR}
    # Directorio 'src'
    "${CMAKE_CURRENT_SOURCE_DIR}/src"
)

# --- Enlace (se hace en src/CMakeLists.txt) ---