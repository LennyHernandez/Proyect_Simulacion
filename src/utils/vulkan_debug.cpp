#include "vulkan_debug.hpp"
#include <iostream> // Para std::cerr y std::cout

namespace particulas::debug {

// --- Implementación de checkVkResult ---
void checkVkResult(VkResult result, const std::string& originMessage) {
    if (result != VK_SUCCESS) {
        // Podríamos tener una función más elaborada para convertir VkResult a string
        throw std::runtime_error(originMessage + " failed! Vulkan Error code: " + std::to_string(result));
    }
}

// --- Implementación de las Funciones del Debug Messenger ---

// Función interna para cargar y llamar a vkCreateDebugUtilsMessengerEXT
VkResult CreateDebugUtilsMessengerEXT_internal(VkInstance instance,
                                               const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
                                               const VkAllocationCallbacks* pAllocator,
                                               VkDebugUtilsMessengerEXT* pDebugMessenger) {
    // Cargar el puntero a la función de extensión
    auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkCreateDebugUtilsMessengerEXT");
    if (func != nullptr) {
        // Llamar a la función cargada
        return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
    } else {
        // La extensión no está presente o no se pudo cargar
        return VK_ERROR_EXTENSION_NOT_PRESENT;
    }
}

// Función interna para cargar y llamar a vkDestroyDebugUtilsMessengerEXT
void DestroyDebugUtilsMessengerEXT_internal(VkInstance instance,
                                            VkDebugUtilsMessengerEXT debugMessenger,
                                            const VkAllocationCallbacks* pAllocator) {
    // Cargar el puntero a la función de extensión
    auto func = (PFN_vkDestroyDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance, "vkDestroyDebugUtilsMessengerEXT");
    if (func != nullptr) {
        // Llamar a la función cargada
        func(instance, debugMessenger, pAllocator);
    }
    // Si no se encuentra la función, no podemos hacer mucho más.
}

// Función pública para configurar el messenger
VkResult setupDebugMessenger(VkInstance instance,
                             VkDebugUtilsMessengerEXT* pDebugMessenger,
                             const VkAllocationCallbacks* pAllocator) {
    VkDebugUtilsMessengerCreateInfoEXT createInfo;
    populateDebugMessengerCreateInfo(createInfo); // Usar la función auxiliar
    return CreateDebugUtilsMessengerEXT_internal(instance, &createInfo, pAllocator, pDebugMessenger);
    // El código que llama a setupDebugMessenger debe verificar el VkResult devuelto.
}

// Función pública para destruir el messenger
void destroyDebugMessenger(VkInstance instance,
                           VkDebugUtilsMessengerEXT debugMessenger,
                           const VkAllocationCallbacks* pAllocator) {
    DestroyDebugUtilsMessengerEXT_internal(instance, debugMessenger, pAllocator);
}


// --- Implementación del Callback de Depuración ---
VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData) { // pUserData no se usa aquí, pero está disponible

    // Imprimir mensajes de error en std::cerr, otros en std::cout
    std::ostream& output_stream = (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) ? std::cerr : std::cout;

    output_stream << "Validation Layer ";

    // Añadir etiquetas de severidad para claridad
    if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT) {
        output_stream << "[ERROR]";
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT) {
        output_stream << "[WARNING]";
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT) {
        output_stream << "[INFO]";
    } else if (messageSeverity & VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT) {
        output_stream << "[VERBOSE]";
    }

    // Añadir etiquetas de tipo
    if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT) {
        output_stream << "[GENERAL]";
    } else if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT) {
        output_stream << "[VALIDATION]";
    } else if (messageType & VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT) {
        output_stream << "[PERFORMANCE]";
    }

    output_stream << ": " << pCallbackData->pMessage << std::endl;

    // La especificación indica que la aplicación debe devolver VK_FALSE siempre.
    return VK_FALSE;
}

// --- Implementación de populateDebugMessengerCreateInfo ---
void populateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& createInfo) {
    createInfo = {}; // Inicializar a ceros
    createInfo.sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
    // Seleccionar los niveles de severidad que activarán el callback
    createInfo.messageSeverity = // VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | // Descomentar para mensajes muy detallados
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_INFO_BIT_EXT    |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
    // Seleccionar los tipos de mensaje que activarán el callback
    createInfo.messageType =   VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                               VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT; // Incluir advertencias de rendimiento
    // Establecer el puntero a nuestra función de callback
    createInfo.pfnUserCallback = debugCallback;
    createInfo.pUserData = nullptr; // Datos de usuario opcionales
    createInfo.pNext = nullptr; // Para extensiones futuras
    createInfo.flags = 0; // Reservado para uso futuro
}

} // namespace particulas::debug