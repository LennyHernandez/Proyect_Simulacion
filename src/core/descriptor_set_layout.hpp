#pragma once

#include <vulkan/vulkan.h>
#include <vector>

namespace particulas {

class DescriptorSetLayout {
public:
    DescriptorSetLayout(VkDevice device);
    ~DescriptorSetLayout();

    VkDescriptorSetLayout get() const { return descriptorSetLayout_; }

private:
    VkDevice device_;
    VkDescriptorSetLayout descriptorSetLayout_;

    void createDescriptorSetLayout();
};

} // namespace particulas