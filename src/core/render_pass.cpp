#include "render_pass.hpp"

#include <stdexcept>
#include <array> // Para std::array

namespace particulas {

RenderPass::RenderPass(VkDevice device, VkFormat swapChainImageFormat, VkFormat depthFormat)
    : device_(device), renderPass_(VK_NULL_HANDLE) {
    createRenderPass(swapChainImageFormat, depthFormat);
}

RenderPass::~RenderPass() {
    vkDestroyRenderPass(device_, renderPass_, nullptr);
}

void RenderPass::createRenderPass(VkFormat swapChainImageFormat, VkFormat depthFormat) {
    // Descripción de la Attachment de color (la imagen de la cadena de intercambio)
    VkAttachmentDescription colorAttachment{};
    colorAttachment.format = swapChainImageFormat;
    colorAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    colorAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR; // Limpiar el attachment al inicio
    colorAttachment.storeOp = VK_ATTACHMENT_STORE_OP_STORE; // Almacenar el resultado después del renderizado
    colorAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    colorAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    colorAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    colorAttachment.finalLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR; // La imagen se presentará después

    // Descripción de la Attachment de profundidad (para gestionar la profundidad)
    VkAttachmentDescription depthAttachment{};
    depthAttachment.format = depthFormat;
    depthAttachment.samples = VK_SAMPLE_COUNT_1_BIT;
    depthAttachment.loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR;
    depthAttachment.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE; // No necesitamos almacenar los datos de profundidad
    depthAttachment.stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
    depthAttachment.stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    depthAttachment.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
    depthAttachment.finalLayout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // Referencia a la Attachment de color para el subpass
    VkAttachmentReference colorAttachmentRef{};
    colorAttachmentRef.attachment = 0; // Índice del attachment en el array pAttachments
    colorAttachmentRef.layout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

    // Referencia a la Attachment de profundidad para el subpass
    VkAttachmentReference depthAttachmentRef{};
    depthAttachmentRef.attachment = 1; // Índice del attachment en el array pAttachments
    depthAttachmentRef.layout = VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

    // Descripción del Subpass
    VkSubpassDescription subpass{};
    subpass.pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS;
    subpass.colorAttachmentCount = 1;
    subpass.pColorAttachments = &colorAttachmentRef; // Solo una attachment de color
    subpass.pDepthStencilAttachment = &depthAttachmentRef; // Attachment de profundidad

    // Dependencia de subpass (para asegurar que la imagen esté lista antes de renderizar)
    VkSubpassDependency dependency{};
    dependency.srcSubpass = VK_SUBPASS_EXTERNAL; // Fuera del render pass
    dependency.dstSubpass = 0;                   // Nuestro único subpass
    dependency.srcStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.dstStageMask = VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
    dependency.srcAccessMask = 0;
    dependency.dstAccessMask = VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;


    // Array de Attachments
    std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};

    // Descripción del Render Pass
    VkRenderPassCreateInfo renderPassInfo{};
    renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
    renderPassInfo.attachmentCount = static_cast<uint32_t>(attachments.size());
    renderPassInfo.pAttachments = attachments.data();
    renderPassInfo.subpassCount = 1;
    renderPassInfo.pSubpasses = &subpass;
    renderPassInfo.dependencyCount = 1;
    renderPassInfo.pDependencies = &dependency; // Añadir la dependencia

    if (vkCreateRenderPass(device_, &renderPassInfo, nullptr, &renderPass_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create render pass!");
    }
}

} // namespace particulas