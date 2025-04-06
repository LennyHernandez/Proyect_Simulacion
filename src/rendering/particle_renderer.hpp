#ifndef PARTICULAS_RENDERING_PARTICLE_RENDERER_HPP
#define PARTICULAS_RENDERING_PARTICLE_RENDERER_HPP

#include "core/device.hpp"       // <-- ASEGÚRATE QUE ES .hpp
#include "core/command_pool.hpp" // <-- ASEGÚRATE QUE ES .hpp
#include "particles/particle.hpp"// <-- ASEGÚRATE QUE ES .hpp

#include <vulkan/vulkan.h>
#include <vector>
#include <array>

namespace particulas {

class ParticleRenderer {
public:
    ParticleRenderer(const Device& device, const CommandPool& commandPool);
    ~ParticleRenderer();

    void createBuffers(const std::vector<Particle>& particles);
    void updateBuffers(const std::vector<Particle>& particles);
    void recordCommandBuffer(VkCommandBuffer commandBuffer, VkPipeline pipeline, VkPipelineLayout pipelineLayout, VkExtent2D swapChainExtent, uint32_t particleCount);

    static VkVertexInputBindingDescription getBindingDescription();
    static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();

private:
    void createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory);
    void copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size);
    VkCommandBuffer beginSingleTimeCommands();
    void endSingleTimeCommands(VkCommandBuffer commandBuffer);

    const Device& deviceRef_; // Guardar referencia a Device
    VkDevice device_;
    VkPhysicalDevice physicalDevice_;
    VkCommandPool commandPool_;
    VkQueue graphicsQueue_;

    VkBuffer vertexBuffer_ = VK_NULL_HANDLE;
    VkDeviceMemory vertexBufferMemory_ = VK_NULL_HANDLE;
    VkDeviceSize currentBufferSize_ = 0;
};

} // namespace particulas

#endif // PARTICULAS_RENDERING_PARTICLE_RENDERER_HPP