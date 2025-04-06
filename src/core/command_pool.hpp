#ifndef PARTICULAS_CORE_COMMAND_POOL_HPP
#define PARTICULAS_CORE_COMMAND_POOL_HPP

#include <vulkan/vulkan.h>
#include <vector>

namespace particulas {

class CommandPool {
public:
    CommandPool(VkDevice device, uint32_t queueFamilyIndex);
    ~CommandPool();
    VkCommandPool get() const { return commandPool_; }
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer, VkQueue queue);
private:
    void createCommandPool();
    VkDevice device_;
    uint32_t queueFamilyIndex_;
    VkCommandPool commandPool_ = VK_NULL_HANDLE;
};

} // namespace particulas
#endif // PARTICULAS_CORE_COMMAND_POOL_HPP