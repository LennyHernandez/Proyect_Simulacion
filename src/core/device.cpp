
#include "device.hpp"
#include "utils/vulkan_debug.hpp"
#include <vulkan/vulkan_core.h>

#include <stdexcept>
#include <iostream>
#include <vector>
#include <set>
#include <string>
#include <optional>

namespace particulas {

const std::vector<const char*> deviceExtensions = {
    VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

Device::Device(VkInstance instance, VkSurfaceKHR surface)
    : instance_(instance), surface_(surface), physicalDevice_(VK_NULL_HANDLE), logicalDevice_(VK_NULL_HANDLE),
      graphicsQueue_(VK_NULL_HANDLE), presentQueue_(VK_NULL_HANDLE),
      graphicsQueueFamilyIndex_(UINT32_MAX), presentQueueFamilyIndex_(UINT32_MAX) {
    pickPhysicalDevice();
    createLogicalDevice();
}

Device::~Device() {
    if (logicalDevice_ != VK_NULL_HANDLE) {
        vkDestroyDevice(logicalDevice_, nullptr);
    }
}

void Device::pickPhysicalDevice() {
    uint32_t deviceCount = 0;
    vkEnumeratePhysicalDevices(instance_, &deviceCount, nullptr);
    if (deviceCount == 0) throw std::runtime_error("Failed to find GPUs with Vulkan support!");
    std::vector<VkPhysicalDevice> devices(deviceCount);
    vkEnumeratePhysicalDevices(instance_, &deviceCount, devices.data());
    for (const auto& device : devices) {
        if (isDeviceSuitable(device)) {
            physicalDevice_ = device;
            VkPhysicalDeviceProperties props; vkGetPhysicalDeviceProperties(physicalDevice_, &props);
            std::cout << "Selected Physical Device: " << props.deviceName << std::endl;
            break;
        }
    }
    if (physicalDevice_ == VK_NULL_HANDLE) throw std::runtime_error("Failed to find a suitable GPU!");
}

bool Device::isDeviceSuitable(VkPhysicalDevice device) {
    // 1. Comprobar Extensiones
    uint32_t extensionCount; vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, nullptr);
    std::vector<VkExtensionProperties> availableExtensions(extensionCount); vkEnumerateDeviceExtensionProperties(device, nullptr, &extensionCount, availableExtensions.data());
    std::set<std::string> requiredExtensionsSet(deviceExtensions.begin(), deviceExtensions.end());
    for (const auto& extension : availableExtensions) { requiredExtensionsSet.erase(extension.extensionName); }
    if (!requiredExtensionsSet.empty()) { return false; }

    // 2. Encontrar Familias de Colas
    uint32_t queueFamilyCount = 0; vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    if (queueFamilyCount == 0) return false;
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount); vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
    std::optional<uint32_t> graphicsFamily, presentFamily;
    for (uint32_t i = 0; i < queueFamilyCount; ++i) {
        if (queueFamilies[i].queueCount > 0 && queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) { graphicsFamily = i; }
        VkBool32 presentSupport = false; if (surface_ != VK_NULL_HANDLE) vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface_, &presentSupport);
        if (queueFamilies[i].queueCount > 0 && presentSupport) { presentFamily = i; }
        if (graphicsFamily.has_value() && presentFamily.has_value()) break;
    }
    if (!graphicsFamily.has_value() || !presentFamily.has_value()) return false;

    // 3. Comprobar Soporte de Swapchain
    if (surface_ == VK_NULL_HANDLE) return false;
    SwapChainSupportDetails details; vkGetPhysicalDeviceSurfaceCapabilitiesKHR(device, surface_, &details.capabilities);
    uint32_t formatCount = 0, presentModeCount = 0; vkGetPhysicalDeviceSurfaceFormatsKHR(device, surface_, &formatCount, nullptr); vkGetPhysicalDeviceSurfacePresentModesKHR(device, surface_, &presentModeCount, nullptr);
    if (formatCount == 0 || presentModeCount == 0) return false;

    return true;
}


uint32_t Device::findQueueFamilies(VkPhysicalDevice device) {
    uint32_t queueFamilyCount = 0; vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount); vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
    for (uint32_t i = 0; i < queueFamilyCount; ++i) { if (queueFamilies[i].queueCount > 0 && queueFamilies[i].queueFlags & VK_QUEUE_GRAPHICS_BIT) { return i; } }
    throw std::runtime_error("Internal Error: No graphics queue family found!");
}

static uint32_t findPresentQueueFamilyInternal(VkPhysicalDevice device, VkSurfaceKHR surface) {
    uint32_t queueFamilyCount = 0; vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, nullptr);
    if (queueFamilyCount == 0) throw std::runtime_error("Device has no queue families!");
    std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount); vkGetPhysicalDeviceQueueFamilyProperties(device, &queueFamilyCount, queueFamilies.data());
    for (uint32_t i = 0; i < queueFamilyCount; ++i) { VkBool32 presentSupport = false; if (surface != VK_NULL_HANDLE) vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &presentSupport); if (queueFamilies[i].queueCount > 0 && presentSupport) { return i; } }
    throw std::runtime_error("Internal Error: No present queue family found!");
}

void Device::createLogicalDevice() {
    graphicsQueueFamilyIndex_ = findQueueFamilies(physicalDevice_);
    presentQueueFamilyIndex_ = findPresentQueueFamilyInternal(physicalDevice_, surface_);

    std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
    std::set<uint32_t> uniqueQueueFamilies = {graphicsQueueFamilyIndex_, presentQueueFamilyIndex_};
    float queuePriority = 1.0f;
    for (uint32_t queueFamily : uniqueQueueFamilies) {
        VkDeviceQueueCreateInfo queueCreateInfo{}; queueCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO; queueCreateInfo.queueFamilyIndex = queueFamily; queueCreateInfo.queueCount = 1; queueCreateInfo.pQueuePriorities = &queuePriority;
        queueCreateInfos.push_back(queueCreateInfo);
    }

    // --- Habilitar Características - USAR VkPhysicalDeviceFeatures BÁSICA Y VACÍA ---
    VkPhysicalDeviceFeatures deviceFeaturesToEnable{}; // <-- Estructura básica vacía

    // --- Información de Creación ---
    VkDeviceCreateInfo createInfo = {};
    createInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;
    createInfo.pNext = nullptr;                         // <-- SIN pNext
    createInfo.pEnabledFeatures = &deviceFeaturesToEnable; // <-- Apuntar a features básicas vacías

    createInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
    createInfo.pQueueCreateInfos = queueCreateInfos.data();
    createInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
    createInfo.ppEnabledExtensionNames = deviceExtensions.data();
    createInfo.enabledLayerCount = 0;
    createInfo.ppEnabledLayerNames = nullptr;

    VkResult result = vkCreateDevice(physicalDevice_, &createInfo, nullptr, &logicalDevice_);
    particulas::debug::checkVkResult(result, "Logical device creation");

    vkGetDeviceQueue(logicalDevice_, graphicsQueueFamilyIndex_, 0, &graphicsQueue_);
    vkGetDeviceQueue(logicalDevice_, presentQueueFamilyIndex_, 0, &presentQueue_);
    std::cout << "Logical device created successfully (without explicit shaderPointSize)." << std::endl;
}

uint32_t Device::findMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags properties) const {
    if (physicalDevice_ == VK_NULL_HANDLE) throw std::runtime_error("Physical device handle is null in findMemoryType.");
    VkPhysicalDeviceMemoryProperties memProperties; vkGetPhysicalDeviceMemoryProperties(physicalDevice_, &memProperties);
    for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++) { if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties) { return i; } }
    throw std::runtime_error("Failed to find suitable memory type!");
}

} // namespace particulas