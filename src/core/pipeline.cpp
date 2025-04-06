#include "pipeline.hpp"
#include "rendering/particle_renderer.hpp" // <-- ASEGÚRATE QUE ESTÁ INCLUIDO
#include "utils/vulkan_debug.hpp"

#include <stdexcept>
#include <iostream>
#include <fstream>
#include <vector>
#include <array>

namespace particulas {

Pipeline::Pipeline(VkDevice device, VkRenderPass renderPass, VkDescriptorSetLayout descriptorSetLayout)
    : device_(device), renderPass_(renderPass), descriptorSetLayout_(descriptorSetLayout),
      graphicsPipeline_(VK_NULL_HANDLE), pipelineLayout_(VK_NULL_HANDLE) {
    try {
        createPipelineLayout();
        createGraphicsPipeline(); // Llama a la función corregida
    } catch (const std::exception& e) {
        if (pipelineLayout_ != VK_NULL_HANDLE) { vkDestroyPipelineLayout(device_, pipelineLayout_, nullptr); }
        throw std::runtime_error(std::string("Pipeline Initialization failed: ") + e.what());
    }
}

Pipeline::~Pipeline() {
    if (graphicsPipeline_ != VK_NULL_HANDLE) { vkDestroyPipeline(device_, graphicsPipeline_, nullptr); }
    if (pipelineLayout_ != VK_NULL_HANDLE) { vkDestroyPipelineLayout(device_, pipelineLayout_, nullptr); }
}

void Pipeline::createPipelineLayout() {
    VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
    pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
    if (descriptorSetLayout_ != VK_NULL_HANDLE) {
        pipelineLayoutInfo.setLayoutCount = 1;
        pipelineLayoutInfo.pSetLayouts = &descriptorSetLayout_;
    } else { pipelineLayoutInfo.setLayoutCount = 0; }
    VkResult result = vkCreatePipelineLayout(device_, &pipelineLayoutInfo, nullptr, &pipelineLayout_);
    particulas::debug::checkVkResult(result, "Create pipeline layout");
}

void Pipeline::createGraphicsPipeline() {
    VkShaderModule vertShaderModule = VK_NULL_HANDLE, fragShaderModule = VK_NULL_HANDLE;
    try {
        vertShaderModule = createShaderModule("shaders/particle.vert.spv");
        fragShaderModule = createShaderModule("shaders/particle.frag.spv");
    } catch (...) { // Limpiar si falla la carga
        if (vertShaderModule != VK_NULL_HANDLE) vkDestroyShaderModule(device_, vertShaderModule, nullptr);
        if (fragShaderModule != VK_NULL_HANDLE) vkDestroyShaderModule(device_, fragShaderModule, nullptr);
        throw;
    }

    VkPipelineShaderStageCreateInfo vertShaderStageInfo{}; /*...*/ vertShaderStageInfo.sType=VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO; vertShaderStageInfo.stage=VK_SHADER_STAGE_VERTEX_BIT; vertShaderStageInfo.module=vertShaderModule; vertShaderStageInfo.pName="main";
    VkPipelineShaderStageCreateInfo fragShaderStageInfo{}; /*...*/ fragShaderStageInfo.sType=VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO; fragShaderStageInfo.stage=VK_SHADER_STAGE_FRAGMENT_BIT; fragShaderStageInfo.module=fragShaderModule; fragShaderStageInfo.pName="main";
    VkPipelineShaderStageCreateInfo shaderStages[] = {vertShaderStageInfo, fragShaderStageInfo};

    // *** Usar descripciones de ParticleRenderer ***
    auto bindingDescription = particulas::ParticleRenderer::getBindingDescription();
    auto attributeDescriptions = particulas::ParticleRenderer::getAttributeDescriptions();
    VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
    vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
    vertexInputInfo.vertexBindingDescriptionCount = 1;
    vertexInputInfo.pVertexBindingDescriptions = &bindingDescription;
    vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescriptions.size());
    vertexInputInfo.pVertexAttributeDescriptions = attributeDescriptions.data();

    VkPipelineInputAssemblyStateCreateInfo inputAssembly{}; /*...*/ inputAssembly.sType=VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO; inputAssembly.topology=VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
    VkPipelineViewportStateCreateInfo viewportState{}; /*...*/ viewportState.sType=VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO; viewportState.viewportCount=1; viewportState.scissorCount=1;
    VkPipelineRasterizationStateCreateInfo rasterizer{}; /*...*/ rasterizer.sType=VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO; rasterizer.polygonMode=VK_POLYGON_MODE_FILL; rasterizer.lineWidth=1.0f; rasterizer.cullMode=VK_CULL_MODE_NONE; rasterizer.frontFace=VK_FRONT_FACE_CLOCKWISE;
    VkPipelineMultisampleStateCreateInfo multisampling{}; /*...*/ multisampling.sType=VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO; multisampling.rasterizationSamples=VK_SAMPLE_COUNT_1_BIT;
    VkPipelineDepthStencilStateCreateInfo depthStencil{}; /*...*/ depthStencil.sType=VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO; depthStencil.depthTestEnable=VK_TRUE; depthStencil.depthWriteEnable=VK_TRUE; depthStencil.depthCompareOp=VK_COMPARE_OP_LESS;
    VkPipelineColorBlendAttachmentState colorBlendAttachment{}; /*...*/ colorBlendAttachment.colorWriteMask=VK_COLOR_COMPONENT_R_BIT|VK_COLOR_COMPONENT_G_BIT|VK_COLOR_COMPONENT_B_BIT|VK_COLOR_COMPONENT_A_BIT; colorBlendAttachment.blendEnable=VK_FALSE;
    VkPipelineColorBlendStateCreateInfo colorBlending{}; /*...*/ colorBlending.sType=VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO; colorBlending.attachmentCount=1; colorBlending.pAttachments=&colorBlendAttachment;
    std::vector<VkDynamicState> dynamicStates = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
    VkPipelineDynamicStateCreateInfo dynamicStateInfo{}; /*...*/ dynamicStateInfo.sType=VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO; dynamicStateInfo.dynamicStateCount=static_cast<uint32_t>(dynamicStates.size()); dynamicStateInfo.pDynamicStates=dynamicStates.data();

    VkGraphicsPipelineCreateInfo pipelineInfo{};
    pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
    pipelineInfo.stageCount = 2; pipelineInfo.pStages = shaderStages;
    pipelineInfo.pVertexInputState = &vertexInputInfo; // <-- Asignado correctamente
    pipelineInfo.pInputAssemblyState = &inputAssembly;
    pipelineInfo.pViewportState = &viewportState;
    pipelineInfo.pRasterizationState = &rasterizer;
    pipelineInfo.pMultisampleState = &multisampling;
    pipelineInfo.pDepthStencilState = &depthStencil;
    pipelineInfo.pColorBlendState = &colorBlending;
    pipelineInfo.pDynamicState = &dynamicStateInfo;
    pipelineInfo.layout = pipelineLayout_;
    pipelineInfo.renderPass = renderPass_;
    pipelineInfo.subpass = 0;

    VkResult result = vkCreateGraphicsPipelines(device_, VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline_);
    // Limpiar módulos incluso si falla
    vkDestroyShaderModule(device_, fragShaderModule, nullptr);
    vkDestroyShaderModule(device_, vertShaderModule, nullptr);
    // Comprobar resultado y lanzar si falla
    particulas::debug::checkVkResult(result, "Create graphics pipeline");
    std::cout << "Graphics pipeline created successfully.\n";
}

VkShaderModule Pipeline::createShaderModule(const std::string& filepath) { /* ... (código como antes) ... */
    std::vector<char> shaderCode = readFile(filepath);
    VkShaderModuleCreateInfo createInfo{}; createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO; createInfo.codeSize = shaderCode.size(); createInfo.pCode = reinterpret_cast<const uint32_t*>(shaderCode.data());
    VkShaderModule shaderModule = VK_NULL_HANDLE;
    VkResult result = vkCreateShaderModule(device_, &createInfo, nullptr, &shaderModule);
    particulas::debug::checkVkResult(result, "Create shader module from " + filepath);
    return shaderModule;
}
std::vector<char> Pipeline::readFile(const std::string& filepath) { /* ... (código como antes) ... */
    std::ifstream file(filepath, std::ios::ate | std::ios::binary);
    if (!file.is_open()) throw std::runtime_error("Failed to open shader file: " + filepath);
    size_t fileSize = (size_t) file.tellg(); if (fileSize == 0) { file.close(); throw std::runtime_error("Shader file is empty: " + filepath); }
    std::vector<char> buffer(fileSize); file.seekg(0); file.read(buffer.data(), fileSize); file.close(); return buffer;
}

} // namespace particulas