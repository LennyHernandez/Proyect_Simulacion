#include "command_pool.hpp"
#include "utils/vulkan_debug.hpp"

#include <stdexcept>
#include <iostream>

namespace particulas {

CommandPool::CommandPool(VkDevice device, uint32_t queueFamilyIndex)
    : device_(device), queueFamilyIndex_(queueFamilyIndex) {
    createCommandPool();
}

CommandPool::~CommandPool() {
    if (commandPool_ != VK_NULL_HANDLE) {
        vkDestroyCommandPool(device_, commandPool_, nullptr);
    }
}

void CommandPool::createCommandPool() {
    VkCommandPoolCreateInfo poolInfo{};
    poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
    poolInfo.queueFamilyIndex = queueFamilyIndex_;
    poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
    VkResult result = vkCreateCommandPool(device_, &poolInfo, nullptr, &commandPool_);
    particulas::debug::checkVkResult(result, "Command pool creation");
}

VkCommandBuffer CommandPool::beginSingleTimeCommands() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool_;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;
    VkCommandBuffer commandBuffer;
    particulas::debug::checkVkResult(vkAllocateCommandBuffers(device_, &allocInfo, &commandBuffer), "Single time command buffer allocation");

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    particulas::debug::checkVkResult(vkBeginCommandBuffer(commandBuffer, &beginInfo), "Begin single time command buffer");
    return commandBuffer;
}

void CommandPool::endSingleTimeCommands(VkCommandBuffer commandBuffer, VkQueue queue) {
    particulas::debug::checkVkResult(vkEndCommandBuffer(commandBuffer), "End single time command buffer");
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &commandBuffer;
    VkResult submitResult = vkQueueSubmit(queue, 1, &submitInfo, VK_NULL_HANDLE);
    particulas::debug::checkVkResult(submitResult, "Single time command buffer submit");
    VkResult waitResult = vkQueueWaitIdle(queue);
    particulas::debug::checkVkResult(waitResult, "Queue wait idle after single time command");
    vkFreeCommandBuffers(device_, commandPool_, 1, &commandBuffer);
}

} // namespace particulas