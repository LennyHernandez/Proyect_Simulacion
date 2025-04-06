#include "framebuffer.hpp"

#include <stdexcept>
#include <iostream>

namespace particulas {

Framebuffer::Framebuffer(VkDevice device, VkRenderPass renderPass, VkExtent2D swapChainExtent, VkImageView colorImageView, VkImageView depthImageView)
    : device_(device), framebuffer_(VK_NULL_HANDLE) {
    createFramebuffer(renderPass, swapChainExtent, colorImageView, depthImageView);
}

Framebuffer::~Framebuffer() {
    vkDestroyFramebuffer(device_, framebuffer_, nullptr);
}

void Framebuffer::createFramebuffer(VkRenderPass renderPass, VkExtent2D swapChainExtent, VkImageView colorImageView, VkImageView depthImageView) {
    // Definir las referencias a las attachments del framebuffer
    std::vector<VkImageView> attachments = { colorImageView, depthImageView };

    VkFramebufferCreateInfo framebufferInfo{};
    framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
    framebufferInfo.renderPass = renderPass;
    framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    framebufferInfo.pAttachments = attachments.data();
    framebufferInfo.width = swapChainExtent.width;
    framebufferInfo.height = swapChainExtent.height;
    framebufferInfo.layers = 1;

    if (vkCreateFramebuffer(device_, &framebufferInfo, nullptr, &framebuffer_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create framebuffer!");
    }
}

} // namespace particulas