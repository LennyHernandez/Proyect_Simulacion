#ifndef PARTICULAS_CORE_DEVICE_HPP
#define PARTICULAS_CORE_DEVICE_HPP

#include <vulkan/vulkan.h>
#include <vector>
#include <optional>
#include <string> // Para std::string en checkVkResult

// Definición movida aquí desde swapchain.hpp
struct SwapChainSupportDetails {
    VkSurfaceCapabilitiesKHR capabilities;
    std::vector<VkSurfaceFormatKHR> formats;
    std::vector<VkPresentModeKHR> presentModes;
};

namespace particulas {

class Device {
public:
    Device(VkInstance instance, VkSurfaceKHR surface);
    ~Device();

    VkDevice getLogicalDevice() const { return logicalDevice_; }
    VkPhysicalDevice getPhysicalDevice() const { return physicalDevice_; }
    VkQueue getGraphicsQueue() const { return graphicsQueue_; }
    VkQueue getPresentQueue() const { return presentQueue_; }
    uint32_t getGraphicsQueueFamilyIndex() const { return graphicsQueueFamilyIndex_; }
    // uint32_t getPresentQueueFamilyIndex() const { return presentQueueFamilyIndex_; } // Si se almacenara

    // --- Función de Utilidad ---
    uint32_t findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const; // <-- Añadida declaración

private:
    void pickPhysicalDevice();
    void createLogicalDevice();
    bool isDeviceSuitable(VkPhysicalDevice device);
    uint32_t findQueueFamilies(VkPhysicalDevice device); // Encuentra solo gráfica por ahora
    // uint32_t findPresentQueueFamily(VkPhysicalDevice device); // Necesitaría implementarse si las colas son diferentes

    VkInstance instance_;
    VkSurfaceKHR surface_;
    VkPhysicalDevice physicalDevice_ = VK_NULL_HANDLE;
    VkDevice logicalDevice_ = VK_NULL_HANDLE;
    VkQueue graphicsQueue_ = VK_NULL_HANDLE;
    VkQueue presentQueue_ = VK_NULL_HANDLE;
    uint32_t graphicsQueueFamilyIndex_ = UINT32_MAX; // Inicializar a valor inválido
    uint32_t presentQueueFamilyIndex_ = UINT32_MAX; // Almacenar también el índice de presentación
};

} // namespace particulas

#endif // PARTICULAS_CORE_DEVICE_HPP