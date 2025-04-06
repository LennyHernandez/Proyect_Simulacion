#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace particulas {

class RenderPass {
public:
    RenderPass(VkDevice device, VkFormat swapChainImageFormat, VkFormat depthFormat);
    ~RenderPass();

    VkRenderPass get() const { return renderPass_; }

private:
    VkDevice device_;
    VkRenderPass renderPass_;

    void createRenderPass(VkFormat swapChainImageFormat, VkFormat depthFormat);
};

} // namespace particulas