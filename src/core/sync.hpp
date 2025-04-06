#ifndef PARTICULAS_CORE_SYNC_HPP
#define PARTICULAS_CORE_SYNC_HPP

#include <vulkan/vulkan.h>
#include <vector>

namespace particulas {

// Definir MAX_FRAMES_IN_FLIGHT en el namespace
const int MAX_FRAMES_IN_FLIGHT = 2; // Hacerla accesible como particulas::MAX_FRAMES_IN_FLIGHT

class Sync {
public:
    Sync(VkDevice device);
    ~Sync();

    VkSemaphore getImageAvailableSemaphore() const;
    VkSemaphore getRenderFinishedSemaphore() const;
    VkFence getInFlightFence() const;

    void waitForFence() const;
    void resetFence() const;

    // Obtener el índice del frame actual (0..MAX_FRAMES_IN_FLIGHT-1)
    uint32_t getCurrentFrameIndex() const { return currentFrame_; } // <-- Getter añadido

    void nextFrame(); // Hacerla pública

private:
    VkDevice device_;
    std::vector<VkSemaphore> imageAvailableSemaphores_;
    std::vector<VkSemaphore> renderFinishedSemaphores_;
    std::vector<VkFence> inFlightFences_;
    uint32_t currentFrame_ = 0;
};

} // namespace particulas

#endif // PARTICULAS_CORE_SYNC_HPP