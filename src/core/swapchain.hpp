#ifndef PARTICULAS_CORE_SWAPCHAIN_HPP
#define PARTICULAS_CORE_SWAPCHAIN_HPP

#include "core/device.hpp"      // Incluye Device y SwapChainSupportDetails
#include "window/window.hpp"    // <-- Incluir para particulas::Window

#include <vulkan/vulkan.h>
#include <vector>

namespace particulas {

// SwapChainSupportDetails está definido en device.hpp

class Swapchain {
public:
    Swapchain(const Device& device, VkSurfaceKHR surface, const Window& window);
    ~Swapchain();

    VkSwapchainKHR get() const { return swapchain_; }
    const std::vector<VkImage>& getImages() const { return swapchainImages_; }
    const std::vector<VkImageView>& getImageViews() const { return swapchainImageViews_; }
    VkFormat getImageFormat() const { return swapchainImageFormat_; }
    VkExtent2D getExtent() const { return swapchainExtent_; }

private:
    void createSwapchain(const Window& window); // Pasar Window
    void createImageViews();
    SwapChainSupportDetails querySwapChainSupport();
    VkSurfaceFormatKHR chooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
    VkPresentModeKHR chooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
    VkExtent2D chooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities, const Window& window);

    VkDevice device_;
    VkPhysicalDevice physicalDevice_;
    VkSurfaceKHR surface_;
    VkSwapchainKHR swapchain_ = VK_NULL_HANDLE;
    std::vector<VkImage> swapchainImages_;
    std::vector<VkImageView> swapchainImageViews_;
    VkFormat swapchainImageFormat_ = VK_FORMAT_UNDEFINED;
    VkExtent2D swapchainExtent_ = {0, 0};
    uint32_t graphicsQueueFamilyIndex_;
    uint32_t presentQueueFamilyIndex_; // Guardar el índice de presentación
};

} // namespace particulas

#endif // PARTICULAS_CORE_SWAPCHAIN_HPP