#pragma once

#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include "vulkan_setup.h"

// Clase GUI: Controla la interfaz gráfica del usuario basada en ImGui
class GUI {
public:
    // Inicializa la GUI con la ventana GLFW y un puntero a VulkanSetup
    void init(GLFWwindow* window, VulkanSetup* vulkan);

    // Renderiza la interfaz gráfica
    void render();

    // Realiza la limpieza de recursos asociados a la GUI
    void cleanup();

private:
    VkDescriptorPool descriptorPool; // Necesario para manejar descriptores en ImGui con Vulkan
    VulkanSetup* vulkan;             // Puntero al objeto VulkanSetup
};