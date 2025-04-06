#include "particle_renderer.hpp"    // <-- Incluir la propia declaración PRIMERO
#include "particles/particle.hpp" // <-- Incluir Particle (necesario para sizeof, offsetof)
#include "utils/vulkan_debug.hpp" // Para checkVkResult

#include <stdexcept>
#include <cstring> // Para memcpy
#include <iostream>
#include <vector>
#include <array>

namespace particulas {

// --- Constructor ---
ParticleRenderer::ParticleRenderer(const Device& device, const CommandPool& commandPool)
    : deviceRef_(device),
      device_(device.getLogicalDevice()),
      physicalDevice_(device.getPhysicalDevice()),
      commandPool_(commandPool.get()),
      graphicsQueue_(device.getGraphicsQueue())
{}

// --- Destructor ---
ParticleRenderer::~ParticleRenderer() {
    if (vertexBuffer_ != VK_NULL_HANDLE) vkDestroyBuffer(device_, vertexBuffer_, nullptr);
    if (vertexBufferMemory_ != VK_NULL_HANDLE) vkFreeMemory(device_, vertexBufferMemory_, nullptr);
}

// --- createBuffers ---
void ParticleRenderer::createBuffers(const std::vector<Particle>& particles) {
    if (particles.empty()) {
         if (vertexBuffer_ != VK_NULL_HANDLE) vkDestroyBuffer(device_, vertexBuffer_, nullptr);
         if (vertexBufferMemory_ != VK_NULL_HANDLE) vkFreeMemory(device_, vertexBufferMemory_, nullptr);
         vertexBuffer_ = VK_NULL_HANDLE; vertexBufferMemory_ = VK_NULL_HANDLE; currentBufferSize_ = 0;
         std::cout << "Warning: ParticleRenderer::createBuffers called with empty particle vector.\n"; return;
    }
    VkDeviceSize bufferSize = sizeof(Particle) * particles.size();
    if (vertexBuffer_ != VK_NULL_HANDLE) vkDestroyBuffer(device_, vertexBuffer_, nullptr);
    if (vertexBufferMemory_ != VK_NULL_HANDLE) vkFreeMemory(device_, vertexBufferMemory_, nullptr);
    vertexBuffer_ = VK_NULL_HANDLE; vertexBufferMemory_ = VK_NULL_HANDLE;

    VkBuffer stagingBuffer = VK_NULL_HANDLE; VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;
    VkBuffer tempVertexBuffer = VK_NULL_HANDLE; VkDeviceMemory tempVertexBufferMemory = VK_NULL_HANDLE; // Usar temporales
    try {
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
        void* data;
        particulas::debug::checkVkResult(vkMapMemory(device_, stagingBufferMemory, 0, bufferSize, 0, &data), "Map staging buffer memory");
        memcpy(data, particles.data(), (size_t)bufferSize);
        vkUnmapMemory(device_, stagingBufferMemory);

        createBuffer(bufferSize, VK_BUFFER_USAGE_VERTEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT, tempVertexBuffer, tempVertexBufferMemory);
        copyBuffer(stagingBuffer, tempVertexBuffer, bufferSize);

        // Asignar a miembros solo si todo fue exitoso
        vertexBuffer_ = tempVertexBuffer;
        vertexBufferMemory_ = tempVertexBufferMemory;
        currentBufferSize_ = bufferSize;
        tempVertexBuffer = VK_NULL_HANDLE; // Evitar doble liberación en catch/finally
        tempVertexBufferMemory = VK_NULL_HANDLE;

    } catch (...) {
         if (tempVertexBuffer != VK_NULL_HANDLE) vkDestroyBuffer(device_, tempVertexBuffer, nullptr); // Limpiar temporal si falla
         if (tempVertexBufferMemory != VK_NULL_HANDLE) vkFreeMemory(device_, tempVertexBufferMemory, nullptr);
         if (stagingBuffer != VK_NULL_HANDLE) vkDestroyBuffer(device_, stagingBuffer, nullptr);
         if (stagingBufferMemory != VK_NULL_HANDLE) vkFreeMemory(device_, stagingBufferMemory, nullptr);
         throw;
    }
    // Limpieza final del staging buffer
    if (stagingBuffer != VK_NULL_HANDLE) vkDestroyBuffer(device_, stagingBuffer, nullptr);
    if (stagingBufferMemory != VK_NULL_HANDLE) vkFreeMemory(device_, stagingBufferMemory, nullptr);

    std::cout << "Particle vertex buffer created. Size: " << currentBufferSize_ << " bytes.\n";
}

// --- updateBuffers ---
void ParticleRenderer::updateBuffers(const std::vector<Particle>& particles) {
    if (vertexBuffer_ == VK_NULL_HANDLE) return; // No se puede actualizar si no existe
    if (particles.empty()) return; // No hacer nada si no hay datos

    VkDeviceSize bufferSize = sizeof(Particle) * particles.size();
    if (bufferSize != currentBufferSize_) { createBuffers(particles); return; } // Recrear si cambia tamaño

    VkBuffer stagingBuffer = VK_NULL_HANDLE; VkDeviceMemory stagingBufferMemory = VK_NULL_HANDLE;
     try {
        createBuffer(bufferSize, VK_BUFFER_USAGE_TRANSFER_SRC_BIT, VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, stagingBuffer, stagingBufferMemory);
        void* data;
        particulas::debug::checkVkResult(vkMapMemory(device_, stagingBufferMemory, 0, bufferSize, 0, &data), "Map staging buffer memory (update)");
        memcpy(data, particles.data(), (size_t)bufferSize);
        vkUnmapMemory(device_, stagingBufferMemory);
        copyBuffer(stagingBuffer, vertexBuffer_, bufferSize); // Copiar a buffer existente
     } catch (...) {
        if (stagingBuffer != VK_NULL_HANDLE) vkDestroyBuffer(device_, stagingBuffer, nullptr);
        if (stagingBufferMemory != VK_NULL_HANDLE) vkFreeMemory(device_, stagingBufferMemory, nullptr); throw;
     }
     if (stagingBuffer != VK_NULL_HANDLE) vkDestroyBuffer(device_, stagingBuffer, nullptr);
     if (stagingBufferMemory != VK_NULL_HANDLE) vkFreeMemory(device_, stagingBufferMemory, nullptr);
}

// --- recordCommandBuffer ---
void ParticleRenderer::recordCommandBuffer(VkCommandBuffer commandBuffer, VkPipeline pipeline, VkPipelineLayout pipelineLayout, VkExtent2D swapChainExtent, uint32_t particleCount) {
    if (vertexBuffer_ == VK_NULL_HANDLE || particleCount == 0) return;
    vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
    VkViewport viewport{}; viewport.width = (float)swapChainExtent.width; viewport.height = (float)swapChainExtent.height; viewport.maxDepth = 1.0f;
    vkCmdSetViewport(commandBuffer, 0, 1, &viewport);
    VkRect2D scissor{}; scissor.extent = swapChainExtent;
    vkCmdSetScissor(commandBuffer, 0, 1, &scissor);
    VkBuffer vertexBuffers[] = {vertexBuffer_}; VkDeviceSize offsets[] = {0};
    vkCmdBindVertexBuffers(commandBuffer, 0, 1, vertexBuffers, offsets);
    vkCmdDraw(commandBuffer, particleCount, 1, 0, 0);
}

// --- createBuffer ---
void ParticleRenderer::createBuffer(VkDeviceSize size, VkBufferUsageFlags usage, VkMemoryPropertyFlags properties, VkBuffer& buffer, VkDeviceMemory& bufferMemory) {
    buffer = VK_NULL_HANDLE; bufferMemory = VK_NULL_HANDLE; // Resetear handles de salida
    VkBufferCreateInfo bufferInfo{}; bufferInfo.sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO; bufferInfo.size = size; bufferInfo.usage = usage; bufferInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
    particulas::debug::checkVkResult(vkCreateBuffer(device_, &bufferInfo, nullptr, &buffer), "Buffer creation");
    VkMemoryRequirements memRequirements; vkGetBufferMemoryRequirements(device_, buffer, &memRequirements);
    VkMemoryAllocateInfo allocInfo{}; allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO; allocInfo.allocationSize = memRequirements.size;
    allocInfo.memoryTypeIndex = deviceRef_.findMemoryType(memRequirements.memoryTypeBits, properties); // <-- Llamar a Device
    particulas::debug::checkVkResult(vkAllocateMemory(device_, &allocInfo, nullptr, &bufferMemory), "Buffer memory allocation");
    particulas::debug::checkVkResult(vkBindBufferMemory(device_, buffer, bufferMemory, 0), "Bind buffer memory");
}

// --- copyBuffer (CORREGIDO) ---
void ParticleRenderer::copyBuffer(VkBuffer srcBuffer, VkBuffer dstBuffer, VkDeviceSize size) {
    if (!srcBuffer || !dstBuffer || size == 0) throw std::runtime_error("Invalid arguments for copyBuffer");
    VkCommandBuffer commandBuffer = beginSingleTimeCommands(); // Obtener CB
    VkBufferCopy copyRegion{}; // Definir región
    copyRegion.size = size;
    vkCmdCopyBuffer(commandBuffer, srcBuffer, dstBuffer, 1, &copyRegion); // <-- Pasar ©Region
    endSingleTimeCommands(commandBuffer); // Enviar y limpiar CB
}

// --- beginSingleTimeCommands ---
VkCommandBuffer ParticleRenderer::beginSingleTimeCommands() {
    if (commandPool_ == VK_NULL_HANDLE) throw std::runtime_error("Command pool is null in beginSingleTimeCommands");
    VkCommandBufferAllocateInfo allocInfo{}; allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO; allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY; allocInfo.commandPool = commandPool_; allocInfo.commandBufferCount = 1;
    VkCommandBuffer commandBuffer;
    particulas::debug::checkVkResult(vkAllocateCommandBuffers(device_, &allocInfo, &commandBuffer), "ST CB Alloc");
    VkCommandBufferBeginInfo beginInfo{}; beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO; beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    particulas::debug::checkVkResult(vkBeginCommandBuffer(commandBuffer, &beginInfo), "ST CB Begin");
    return commandBuffer;
}

// --- endSingleTimeCommands ---
void ParticleRenderer::endSingleTimeCommands(VkCommandBuffer commandBuffer) {
    if (commandBuffer == VK_NULL_HANDLE) return;
    particulas::debug::checkVkResult(vkEndCommandBuffer(commandBuffer), "ST CB End");
    VkSubmitInfo submitInfo{}; submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO; submitInfo.commandBufferCount = 1; submitInfo.pCommandBuffers = &commandBuffer;
    particulas::debug::checkVkResult(vkQueueSubmit(graphicsQueue_, 1, &submitInfo, VK_NULL_HANDLE), "ST CB Submit");
    particulas::debug::checkVkResult(vkQueueWaitIdle(graphicsQueue_), "ST CB Wait Idle");
    vkFreeCommandBuffers(device_, commandPool_, 1, &commandBuffer);
}

// --- Funciones estáticas (CON CUERPO) ---
VkVertexInputBindingDescription ParticleRenderer::getBindingDescription() {
    VkVertexInputBindingDescription bindingDescription{};
    bindingDescription.binding = 0;
    bindingDescription.stride = sizeof(Particle); // <-- Usar Particle::
    bindingDescription.inputRate = VK_VERTEX_INPUT_RATE_VERTEX;
    return bindingDescription; // <-- Return
}

std::array<VkVertexInputAttributeDescription, 3> ParticleRenderer::getAttributeDescriptions() {
    std::array<VkVertexInputAttributeDescription, 3> attributeDescriptions{};
    // Location 0: Position
    attributeDescriptions[0].location = 0; attributeDescriptions[0].binding = 0;
    attributeDescriptions[0].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[0].offset = offsetof(Particle, position); // <-- Usar Particle::
    // Location 1: Color
    attributeDescriptions[1].location = 1; attributeDescriptions[1].binding = 0;
    attributeDescriptions[1].format = VK_FORMAT_R32G32B32A32_SFLOAT;
    attributeDescriptions[1].offset = offsetof(Particle, color); // <-- Usar Particle::
    // Location 2: Velocity
    attributeDescriptions[2].location = 2; attributeDescriptions[2].binding = 0;
    attributeDescriptions[2].format = VK_FORMAT_R32G32_SFLOAT;
    attributeDescriptions[2].offset = offsetof(Particle, velocity); // <-- Usar Particle::
    return attributeDescriptions; // <-- Return
}

} // namespace particulas