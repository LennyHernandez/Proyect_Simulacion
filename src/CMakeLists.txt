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
    # Fuentes de los BACKENDS de ImGui
    "${imgui_SOURCE_DIR}/backends/imgui_impl_glfw.cpp"
    "${imgui_SOURCE_DIR}/backends/imgui_impl_vulkan.cpp"

    # ¡¡AÑADIR FUENTES DEL CORE de ImGui!!
    "${imgui_SOURCE_DIR}/imgui.cpp"
    "${imgui_SOURCE_DIR}/imgui_draw.cpp"
    "${imgui_SOURCE_DIR}/imgui_tables.cpp"
    "${imgui_SOURCE_DIR}/imgui_widgets.cpp"
    # Añadir otros si los usas (ej: imgui_demo.cpp si quieres el demo)
)

# --- Definir el Ejecutable AQUÍ ---
add_executable(ParticleSimulation ${APP_SOURCES})

# --- Directorios de Inclusión DENTRO de src ---
target_include_directories(ParticleSimulation PRIVATE
    "." "core" "particles" "rendering" "window" "utils"
)

# --- Vincular Bibliotecas AQUÍ ---
target_link_libraries(ParticleSimulation PRIVATE
    glfw           # Target de GLFW (FetchContent lo crea)
    # ImGui          # <-- ELIMINADO (Compilamos las fuentes)
    Vulkan::Vulkan
)