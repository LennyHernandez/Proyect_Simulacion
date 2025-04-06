#include "swapchain.hpp"
// device.hpp y window.hpp incluidos vía swapchain.hpp
#include "utils/vulkan_debug.hpp"

#include <stdexcept>
#include <algorithm>
#include <iostream>
#include <limits>

namespace particulas {

// --- Implementación de funciones auxiliares ---
SwapChainSupportDetails Swapchain::querySwapChainSupport() { /* ... (como antes) ... */
    if (physicalDevice_ == VK_NULL_HANDLE || surface_ == VK_NULL_HANDLE) throw std::runtime_error("Cannot query swapchain support: Physical device or surface not set.");
    SwapChainSupportDetails details;
    vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice_, surface_, &details.capabilities);
    uint32_t formatCount;
    vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice_, surface_, &formatCount, nullptr);
    if (formatCount != 0) { details.formats.resize(formatCount); vkGetPhysicalDeviceSurfaceFormatsKHR(physicalDevice_, surface_, &formatCount, details.formats.data()); }
    uint32_t presentModeCount;
    vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice_, surface_, &presentModeCount, nullptr);
    if (presentModeCount != 0) { details.presentModes.resize(presentModeCount); vkGetPhysicalDeviceSurfacePresentModesKHR(physicalDevice_, surface_, &presentModeCount, details.presentModes.data()); }
    return details;
}
VkSurfaceFormatKHR Swapchain::chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats) { /* ... (como antes) ... */
    if (availableFormats.empty()) throw std::runtime_error("No surface formats available!");
    for (const auto& availableFormat : availableFormats) { if (availableFormat.format == VK_FORMAT_B8G8R8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR) return availableFormat; }
    return availableFormats[0];
}
VkPresentModeKHR Swapchain::chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes) { /* ... (como antes) ... */
    if (availablePresentModes.empty()) throw std::runtime_error("No present modes available!");
    for (const auto& availablePresentMode : availablePresentModes) { if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR) return availablePresentMode; }
    return VK_PRESENT_MODE_FIFO_KHR;
}
VkExtent2D Swapchain::chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, const Window& window) { /* ... (como antes) ... */
    if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max()) return capabilities.currentExtent;
    else {
        VkExtent2D actualExtent = window.getFramebufferExtent();
        actualExtent.width = std::clamp(actualExtent.width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width);
        actualExtent.height = std::clamp(actualExtent.height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height);
        return actualExtent;
    }
}

// --- Constructor (Corregido) ---
Swapchain::Swapchain(const Device& device, VkSurfaceKHR surface, const Window& window)
    : device_(device.getLogicalDevice()),
      physicalDevice_(device.getPhysicalDevice()),
      surface_(surface),
      graphicsQueueFamilyIndex_(device.getGraphicsQueueFamilyIndex()),
      // Obtener el índice de presentación que encontró Device
      // Asume que Device tiene un getter o lo almacena como hicimos.
      // Necesitas asegurarte que presentQueueFamilyIndex_ se inicialice correctamente en Device
      presentQueueFamilyIndex_(UINT32_MAX) // Inicializar a valor inválido
{
    // Obtener el índice de presentación después de que Device lo haya encontrado
    // Necesitaríamos un getter en Device o pasarlo explícitamente.
    // Solución simple temporal (si asumimos que Device::createLogicalDevice lo hizo bien):
     uint32_t tempPresentIndex = UINT32_MAX;
     uint32_t queueFamilyCount = 0;
     vkGetPhysicalDeviceQueueFamilyProperties(physicalDevice_, &queueFamilyCount, nullptr);
     for (uint32_t i = 0; i < queueFamilyCount; ++i) {
         VkBool32 presentSupport = false;
         vkGetPhysicalDeviceSurfaceSupportKHR(physicalDevice_, i, surface_, &presentSupport);
         if (presentSupport) {
             tempPresentIndex = i;
             break;
         }
     }
     if(tempPresentIndex == UINT32_MAX) throw std::runtime_error("Could not find present queue family index in Swapchain constructor");
     presentQueueFamilyIndex_ = tempPresentIndex;


    createSwapchain(window);
    createImageViews();
    std::cout << "Swapchain created successfully." << std::endl;
}


// --- Destructor ---
Swapchain::~Swapchain() { /* ... (como antes, sin cambios lógicos) ... */
    if (!swapchainImageViews_.empty()) {
        for (VkImageView imageView : swapchainImageViews_) {
            if (imageView != VK_NULL_HANDLE) vkDestroyImageView(device_, imageView, nullptr);
        }
        swapchainImageViews_.clear();
    }
    if (swapchain_ != VK_NULL_HANDLE) {
        vkDestroySwapchainKHR(device_, swapchain_, nullptr);
    }
}

// --- createSwapchain (La firma DEBE coincidir) ---
void Swapchain::createSwapchain(const Window& window) { /* ... (como antes, sin cambios lógicos) ... */
    SwapChainSupportDetails swapChainSupport = querySwapChainSupport();
    VkSurfaceFormatKHR surfaceFormat = chooseSwapSurfaceFormat(swapChainSupport.formats);
    VkPresentModeKHR presentMode = chooseSwapPresentMode(swapChainSupport.presentModes);
    VkExtent2D extent = chooseSwapExtent(swapChainSupport.capabilities, window);
    uint32_t imageCount = swapChainSupport.capabilities.minImageCount + 1;
    if (swapChainSupport.capabilities.maxImageCount > 0 && imageCount > swapChainSupport.capabilities.maxImageCount) imageCount = swapChainSupport.capabilities.maxImageCount;

    VkSwapchainCreateInfoKHR createInfo{};
    createInfo.sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
    createInfo.surface = surface_;
    createInfo.minImageCount = imageCount;
    createInfo.imageFormat = surfaceFormat.format;
    createInfo.imageColorSpace = surfaceFormat.colorSpace;
    createInfo.imageExtent = extent;
    createInfo.imageArrayLayers = 1;
    createInfo.imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

    uint32_t queueFamilyIndicesArray[] = {graphicsQueueFamilyIndex_, presentQueueFamilyIndex_};
    if (graphicsQueueFamilyIndex_ != presentQueueFamilyIndex_) {
        createInfo.imageSharingMode = VK_SHARING_MODE_CONCURRENT;
        createInfo.queueFamilyIndexCount = 2;
        createInfo.pQueueFamilyIndices = queueFamilyIndicesArray;
    } else {
        createInfo.imageSharingMode = VK_SHARING_MODE_EXCLUSIVE;
        createInfo.queueFamilyIndexCount = 0;
        createInfo.pQueueFamilyIndices = nullptr;
    }
    createInfo.preTransform = swapChainSupport.capabilities.currentTransform;
    createInfo.compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
    createInfo.presentMode = presentMode;
    createInfo.clipped = VK_TRUE;
    createInfo.oldSwapchain = VK_NULL_HANDLE;

    VkResult result = vkCreateSwapchainKHR(device_, &createInfo, nullptr, &swapchain_);
    particulas::debug::checkVkResult(result, "Swapchain creation");

    vkGetSwapchainImagesKHR(device_, swapchain_, &imageCount, nullptr);
    swapchainImages_.resize(imageCount);
    vkGetSwapchainImagesKHR(device_, swapchain_, &imageCount, swapchainImages_.data());
    swapchainImageFormat_ = surfaceFormat.format;
    swapchainExtent_ = extent;
    std::cout << "  - Swapchain Image Count: " << imageCount << std::endl;
    std::cout << "  - Swapchain Format: " << swapchainImageFormat_ << std::endl;
    std::cout << "  - Swapchain Extent: " << swapchainExtent_.width << "x" << swapchainExtent_.height << std::endl;
}

// --- createImageViews ---
void Swapchain::createImageViews() { /* ... (como antes, sin cambios lógicos) ... */
    swapchainImageViews_.resize(swapchainImages_.size());
    for (size_t i = 0; i < swapchainImages_.size(); i++) {
        VkImageViewCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
        createInfo.image = swapchainImages_[i];
        createInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
        createInfo.format = swapchainImageFormat_;
        createInfo.components = { VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY, VK_COMPONENT_SWIZZLE_IDENTITY };
        createInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
        createInfo.subresourceRange.baseMipLevel = 0;
        createInfo.subresourceRange.levelCount = 1;
        createInfo.subresourceRange.baseArrayLayer = 0;
        createInfo.subresourceRange.layerCount = 1;
        VkResult result = vkCreateImageView(device_, &createInfo, nullptr, &swapchainImageViews_[i]);
        particulas::debug::checkVkResult(result, "Swapchain image view creation for image " + std::to_string(i));
    }
}

} // namespace particulas