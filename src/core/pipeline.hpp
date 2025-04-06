#ifndef PARTICULAS_CORE_PIPELINE_HPP // Guarda de inclusión
#define PARTICULAS_CORE_PIPELINE_HPP

#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <array> // Necesario para las funciones estáticas si las declaras aquí, aunque están en ParticleRenderer

namespace particulas {

class Pipeline {
public:
    // Constructor: necesita dispositivo, render pass, y opcionalmente layout de descriptores
    Pipeline(VkDevice device, VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout = VK_NULL_HANDLE);
    ~Pipeline();

    // --- Getters ---
    VkPipeline getGraphicsPipeline() const { return graphicsPipeline_; }
    VkPipelineLayout getPipelineLayout() const { return pipelineLayout_; }

    // --- Funciones estáticas para obtener descripciones de vértices ---
    // Movidas a ParticleRenderer, pero podrían estar aquí si fueran genéricas.
    // static VkVertexInputBindingDescription getBindingDescription();
    // static std::array<VkVertexInputAttributeDescription, 3> getAttributeDescriptions();

private:
    // --- Métodos Privados ---
    void createPipelineLayout();
    void createGraphicsPipeline();
    VkShaderModule createShaderModule(const std::string& filepath);
    std::vector<char> readFile(const std::string& filepath);

    // --- Miembros ---
    VkDevice device_;                   // Handle del dispositivo lógico
    VkRenderPass renderPass_;           // Handle del render pass compatible
    VkDescriptorSetLayout descriptorSetLayout_; // Handle del layout (puede ser VK_NULL_HANDLE)

    VkPipeline graphicsPipeline_ = VK_NULL_HANDLE; // Handle del pipeline gráfico creado
    VkPipelineLayout pipelineLayout_ = VK_NULL_HANDLE; // Handle del layout del pipeline creado
};

} // namespace particulas

#endif // PARTICULAS_CORE_PIPELINE_HPP