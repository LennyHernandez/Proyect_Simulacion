# --- Listar Archivos Fuente (.cpp solamente) ---
set(APP_SOURCES
    main.cpp
    core/instance.cpp
    core/device.cpp
    core/swapchain.cpp
    core/command_pool.cpp
    core/sync.cpp
    core/pipeline.cpp
    core/render_pass.cpp
    particles/particle_system.cpp
    rendering/particle_renderer.cpp
    window/window.cpp
    utils/vulkan_debug.cpp
    # Backends de ImGui
    "${CMAKE_SOURCE_DIR}/external/imgui/backends/imgui_impl_glfw.cpp"
    "${CMAKE_SOURCE_DIR}/external/imgui/backends/imgui_impl_vulkan.cpp"
)

# --- Definir el Ejecutable AQUÍ ---
add_executable(ParticleSimulation ${APP_SOURCES})

# --- Directorios de Inclusión DENTRO de src ---
target_include_directories(ParticleSimulation PRIVATE
    "."
    "core"
    "particles"
    "rendering"
    "window"
    "utils"
)

# --- Vincular Bibliotecas AQUÍ ---
target_link_libraries(ParticleSimulation PRIVATE
    glfw           # Target de GLFW
    ImGui          # Target de ImGui
    Vulkan::Vulkan
    # Threads::Threads # <-- ELIMINADO
)