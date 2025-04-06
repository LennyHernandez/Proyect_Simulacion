#include "instance.hpp"
#include "utils/vulkan_debug.hpp"

#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>
#include <vector>
#include <cstring>

namespace particulas {

// --- Constructor ---
Instance::Instance(const std::vector<const char*>& enabledLayers,
                 const std::vector<const char*>& additionalExtensions) {
#ifndef NDEBUG
    validationLayersEnabled_ = true;
    if (!checkValidationLayerSupport(enabledLayers)) {
        std::cerr << "Warning: Requested validation layers are not available. Disabling validation layers.\n";
        validationLayersEnabled_ = false;
    }
#else
    validationLayersEnabled_ = false;
#endif

    const std::vector<const char*>& finalEnabledLayers = validationLayersEnabled_ ? enabledLayers : std::vector<const char*>{};
    createInstance(finalEnabledLayers, additionalExtensions);

    if (validationLayersEnabled_) {
        setupDebugMessenger();
    }
}

// --- Destructor ---
Instance::~Instance() {
    if (debugMessenger_ != VK_NULL_HANDLE) {
        particulas::debug::destroyDebugMessenger(instance_, debugMessenger_, nullptr);
    }
    if (instance_ != VK_NULL_HANDLE) {
        vkDestroyInstance(instance_, nullptr);
    }
}

// --- createInstance ---
void Instance::createInstance(const std::vector<const char*>& enabledLayers,
                            const std::vector<const char*>& additionalExtensions) {

    VkApplicationInfo appInfo{};
    appInfo.sType = VK_STRUCTURE_TYPE_APPLICATION_INFO;
    appInfo.pApplicationName = "Particulas Simulación";
    appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.pEngineName = "No Engine";
    appInfo.engineVersion = VK_MAKE_VERSION(1, 0, 0);
    appInfo.apiVersion = VK_API_VERSION_1_1;

    VkInstanceCreateInfo createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
    createInfo.pApplicationInfo = &appInfo;

    auto requiredExtensions = getRequiredExtensions();
    requiredExtensions.insert(requiredExtensions.end(), additionalExtensions.begin(), additionalExtensions.end());

    if (validationLayersEnabled_) {
        requiredExtensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
    }

    createInfo.enabledExtensionCount = static_cast<uint32_t>(requiredExtensions.size());
    createInfo.ppEnabledExtensionNames = requiredExtensions.data();

    std::cout << "Enabled Instance Extensions:" << std::endl;
    for(const char* extName : requiredExtensions) { std::cout << "  - " << extName << std::endl; }

    VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
    if (validationLayersEnabled_) {
        createInfo.enabledLayerCount = static_cast<uint32_t>(enabledLayers.size());
        createInfo.ppEnabledLayerNames = enabledLayers.data();
        std::cout << "Enabled Validation Layers:" << std::endl;
        for(const char* layerName : enabledLayers) { std::cout << "  - " << layerName << std::endl; }

        particulas::debug::populateDebugMessengerCreateInfo(debugCreateInfo);
        createInfo.pNext = &debugCreateInfo;
    } else {
        createInfo.enabledLayerCount = 0;
        createInfo.ppEnabledLayerNames = nullptr;
        createInfo.pNext = nullptr;
    }

    VkResult result = vkCreateInstance(&createInfo, nullptr, &instance_);
    if (result != VK_SUCCESS) {
        particulas::debug::checkVkResult(result, "Instance creation");
    } else {
         std::cout << "Vulkan Instance created successfully." << std::endl;
    }
}

// --- checkValidationLayerSupport ---
bool Instance::checkValidationLayerSupport(const std::vector<const char*>& layersToCheck) const {
    if (layersToCheck.empty()) return true;
    uint32_t layerCount;
    vkEnumerateInstanceLayerProperties(&layerCount, nullptr);
    std::vector<VkLayerProperties> availableLayers(layerCount);
    vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

    std::cout << "Available Validation Layers:" << std::endl;
    for (const auto& layerProperties : availableLayers) { std::cout << "  - " << layerProperties.layerName << std::endl; }

    for (const char* requestedLayerName : layersToCheck) {
        bool layerFound = false;
        for (const auto& layerProperties : availableLayers) {
            if (strcmp(requestedLayerName, layerProperties.layerName) == 0) {
                layerFound = true; break;
            }
        }
        if (!layerFound) {
            std::cerr << "Error: Requested validation layer not found: " << requestedLayerName << std::endl;
            return false;
        }
    }
    return true;
}

// --- getRequiredExtensions ---
std::vector<const char*> Instance::getRequiredExtensions() const {
    uint32_t glfwExtensionCount = 0;
    const char** glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
    std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
    return extensions;
}

// --- setupDebugMessenger ---
void Instance::setupDebugMessenger() {
     if (!validationLayersEnabled_) return;
    VkResult result = particulas::debug::setupDebugMessenger(instance_, &debugMessenger_, nullptr);
    if (result != VK_SUCCESS) {
        throw std::runtime_error("Failed to set up debug messenger after instance creation! Error code: " + std::to_string(result));
    } else {
        std::cout << "Debug messenger set up successfully (persistent)." << std::endl;
    }
}

} // namespace particulas