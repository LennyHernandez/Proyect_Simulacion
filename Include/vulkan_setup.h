#pragma once
#include <vector>
#include <stdexcept>  // Add this line
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <string>
#include <GLFW/glfw3.h>


// Clase para configurar y manejar Vulkan
class VulkanSetup {
private:
    VkDevice device = VK_NULL_HANDLE;            // Dispositivo lógico
    VkInstance instance = VK_NULL_HANDLE;        // Instancia Vulkan
    VkPhysicalDevice physicalDevice = VK_NULL_HANDLE; // Dispositivo físico
    uint32_t queueFamilyIndex = UINT32_MAX;      // Índice de la familia de colas
    VkQueue graphicsQueue = VK_NULL_HANDLE;      // Cola gráfica
    VkRenderPass renderPass = VK_NULL_HANDLE;    // Render pass

public:
    // Métodos Getter: Acceso a componentes de Vulkan
    VkDevice getDevice() const { 
        if (!device) throw std::runtime_error("VkDevice no inicializado");
        return device; 
    }
    VkInstance getInstance() const { 
        if (!instance) throw std::runtime_error("VkInstance no inicializado");
        return instance; 
    }
    VkPhysicalDevice getPhysicalDevice() const { return physicalDevice; }
    uint32_t getQueueFamilyIndex() const { return queueFamilyIndex; }
    VkQueue getGraphicsQueue() const { return graphicsQueue; }
    VkRenderPass getRenderPass() const { return renderPass; }

    // Funciones de inicialización de Vulkan
    void initVulkan(GLFWwindow* window);          // Inicializa Vulkan
    void createInstance();                       // Crea la instancia Vulkan
    void createDevice();                         // Crea el dispositivo lógico
    void createGraphicsPipeline();               // Configura el pipeline gráfico
    void createBuffers();                        // Maneja los buffers de vértices
    void drawFrame();                            // Renderiza un frame
    void cleanup();                              // Limpia los recursos

    // Funciones auxiliares para manejar shaders
    std::vector<char> readFile(const std::string& filename);  // Lee un archivo SPIR-V
    VkShaderModule createShaderModule(const std::vector<char>& code); // Crea un módulo de shader
};