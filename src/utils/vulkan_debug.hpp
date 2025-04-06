#ifndef PARTICULAS_UTILS_VULKAN_DEBUG_HPP
#define PARTICULAS_UTILS_VULKAN_DEBUG_HPP

#include <vulkan/vulkan.h>
#include <string>
#include <stdexcept> // Para std::runtime_error

namespace particulas::debug { // Usamos un sub-namespace para claridad

// Declaración para verificar los resultados de Vulkan. Lanza excepción en error.
void checkVkResult(VkResult result, const std::string& originMessage);

// Configura el debug messenger de Vulkan.
// Devuelve el resultado de vkCreateDebugUtilsMessengerEXT.
VkResult setupDebugMessenger(VkInstance instance,
                             VkDebugUtilsMessengerEXT* pDebugMessenger,
                             const VkAllocationCallbacks* pAllocator = nullptr);

// Destruye el debug messenger de Vulkan.
void destroyDebugMessenger(VkInstance instance,
                           VkDebugUtilsMessengerEXT debugMessenger,
                           const VkAllocationCallbacks* pAllocator = nullptr);

// Declaración de la función de callback de depuración.
VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData);

// Función auxiliar para rellenar la estructura de creación del messenger.
void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo);

} // namespace particulas::debug

#endif // PARTICULAS_UTILS_VULKAN_DEBUG_HPP