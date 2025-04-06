#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace particulas {

class Framebuffer {
public:
    Framebuffer(VkDevice device, VkRenderPass renderPass, VkExtent2D swapChainExtent, VkImageView colorImageView, VkImageView depthImageView);
    ~Framebuffer();

    VkFramebuffer get() const { return framebuffer_; }

private:
    VkDevice device_;
    VkFramebuffer framebuffer_;

    void createFramebuffer(VkRenderPass renderPass, VkExtent2D swapChainExtent, VkImageView colorImageView, VkImageView depthImageView);
};

} // namespace particulas