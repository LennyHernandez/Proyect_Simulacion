#include "command_buffer.hpp"

#include <stdexcept>
#include <iostream>
#include <cstdlib>  // Para rand()
#include <ctime>    // Para semilla aleatoria
#include <cmath>    // Para sqrt() y pow()
#include <iostream> // Para std::cout y std::endl
#include <cstring>  // Para std::memset

namespace particulas {

CommandBuffer::CommandBuffer(VkDevice device, VkCommandPool commandPool, uint32_t bufferCount)
    : device_(device), commandPool_(commandPool), bufferCount_(bufferCount) {
    commandBuffers_.resize(bufferCount_);
    allocateCommandBuffers();
}

CommandBuffer::~CommandBuffer() {
    vkFreeCommandBuffers(device_, commandPool_, static_cast<uint32_t>(commandBuffers_.size()), commandBuffers_.data());
}

void CommandBuffer::allocateCommandBuffers() {
    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.commandPool = commandPool_;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = bufferCount_;

    if (vkAllocateCommandBuffers(device_, &allocInfo, commandBuffers_.data()) != VK_SUCCESS) {
        throw std::runtime_error("Failed to allocate command buffers!");
    }
}

void CommandBuffer::beginRecording(VkCommandBufferUsageFlags flags) {
    for (auto& commandBuffer : commandBuffers_) {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        beginInfo.flags = flags;

        if (vkBeginCommandBuffer(commandBuffer, &beginInfo) != VK_SUCCESS) {
            throw std::runtime_error("Failed to begin recording command buffer!");
        }
    }
}

void CommandBuffer::endRecording() {
    for (auto& commandBuffer : commandBuffers_) {
        if (vkEndCommandBuffer(commandBuffer) != VK_SUCCESS) {
            throw std::runtime_error("Failed to end command buffer!");
        }
    }
}

void CommandBuffer::submit(VkQueue queue, VkFence fence) {
    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
    submitInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers_.size());
    submitInfo.pCommandBuffers = commandBuffers_.data();

    if (vkQueueSubmit(queue, 1, &submitInfo, fence) != VK_SUCCESS) {
        throw std::runtime_error("Failed to submit command buffers!");
    }

    vkQueueWaitIdle(queue); // Esperar a que la GPU termine de ejecutar (esto generalmente no se hace aquí en el bucle de renderizado)
}

} // namespace particulas