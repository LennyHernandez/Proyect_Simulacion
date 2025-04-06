#include "sync.hpp"
#include "utils/vulkan_debug.hpp" // Para checkVkResult

#include <stdexcept>
#include <iostream>
#include <limits>

namespace particulas {

Sync::Sync(VkDevice device)
    : device_(device), currentFrame_(0) {

    imageAvailableSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);
    renderFinishedSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);
    inFlightFences_.resize(MAX_FRAMES_IN_FLIGHT);

    VkSemaphoreCreateInfo semaphoreInfo{};
    semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

    VkFenceCreateInfo fenceInfo{};
    fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
    fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
         VkResult result1 = vkCreateSemaphore(device_, &semaphoreInfo, nullptr, &imageAvailableSemaphores_[i]);
         VkResult result2 = vkCreateSemaphore(device_, &semaphoreInfo, nullptr, &renderFinishedSemaphores_[i]);
         VkResult result3 = vkCreateFence(device_, &fenceInfo, nullptr, &inFlightFences_[i]);
         if (result1 != VK_SUCCESS || result2 != VK_SUCCESS || result3 != VK_SUCCESS) {
            // Limpiar parcialmente si falla
             for(size_t j = 0; j <= i; ++j) {
                if (imageAvailableSemaphores_[j] != VK_NULL_HANDLE) vkDestroySemaphore(device_, imageAvailableSemaphores_[j], nullptr);
                if (renderFinishedSemaphores_[j] != VK_NULL_HANDLE) vkDestroySemaphore(device_, renderFinishedSemaphores_[j], nullptr);
                if (inFlightFences_[j] != VK_NULL_HANDLE) vkDestroyFence(device_, inFlightFences_[j], nullptr);
             }
            throw std::runtime_error("Failed to create synchronization objects for frame " + std::to_string(i));
        }
    }
}

Sync::~Sync() {
    // No es estrictamente necesario llamar a vkDeviceWaitIdle aquí si se hace en cleanup() principal
    // vkDeviceWaitIdle(device_);

    for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
        // Comprobar handles antes de destruir
        if (renderFinishedSemaphores_[i] != VK_NULL_HANDLE) {
            vkDestroySemaphore(device_, renderFinishedSemaphores_[i], nullptr);
        }
        if (imageAvailableSemaphores_[i] != VK_NULL_HANDLE) {
            vkDestroySemaphore(device_, imageAvailableSemaphores_[i], nullptr);
        }
        if (inFlightFences_[i] != VK_NULL_HANDLE) {
            vkDestroyFence(device_, inFlightFences_[i], nullptr);
        }
    }
}

VkSemaphore Sync::getImageAvailableSemaphore() const {
    return imageAvailableSemaphores_[currentFrame_];
}

VkSemaphore Sync::getRenderFinishedSemaphore() const {
    return renderFinishedSemaphores_[currentFrame_];
}

VkFence Sync::getInFlightFence() const {
    return inFlightFences_[currentFrame_];
}

void Sync::waitForFence() const {
    VkResult result = vkWaitForFences(device_, 1, &inFlightFences_[currentFrame_], VK_TRUE, std::numeric_limits<uint64_t>::max());
    particulas::debug::checkVkResult(result, "Wait for fence");
}

void Sync::resetFence() const {
    VkResult result = vkResetFences(device_, 1, &inFlightFences_[currentFrame_]);
     particulas::debug::checkVkResult(result, "Reset fence");
}

void Sync::nextFrame() {
    currentFrame_ = (currentFrame_ + 1) % MAX_FRAMES_IN_FLIGHT;
}

} // namespace particulas