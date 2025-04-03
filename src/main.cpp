#define SCREEN_WIDTH APP_SCREEN_WIDTH
#define SCREEN_HEIGHT APP_SCREEN_HEIGHT
#include "vulkan_setup.h"
#include "particle_system.h"
#include "gui.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>

// Función para inicializar GLFW
void initGLFW() {
    if (!glfwInit()) {
        throw std::runtime_error("Error al inicializar GLFW");
    }
}

// Función para crear ventana
GLFWwindow* createWindow() {
    GLFWwindow* window = glfwCreateWindow(SCREEN_WIDTH, SCREEN_HEIGHT, "Simulación de Partículas", nullptr, nullptr);
    if (!window) {
        glfwTerminate();
        throw std::runtime_error("Error al crear ventana GLFW");
    }
    return window;
}

// Función principal
int main() {
    try {
        // Inicializar GLFW
        initGLFW();

        // Crear ventana
        GLFWwindow* window = createWindow();

        // Inicializar subsistemas
        VulkanSetup vulkan;
        vulkan.initVulkan(window);

        ParticleSystem particles;
        particles.init();

        GUI gui;
        gui.init(window, &vulkan); // Pasa la referencia de VulkanSetup

        // Bucle principal
        while (!glfwWindowShouldClose(window)) {
            glfwPollEvents();

            vulkan.drawFrame();
            particles.update();
            gui.render();
        }

        // Limpieza de recursos
        vulkan.cleanup();
        particles.cleanup();
        gui.cleanup();
        glfwDestroyWindow(window);
        glfwTerminate();

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return -1;
    }
}
