#include "gui.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include "vulkan_setup.h"
#include <stdexcept> // Para manejar excepciones con std::runtime_error

void GUI::init(GLFWwindow* window, VulkanSetup* vulkan) {
    this->vulkan = vulkan;  // Asignar el puntero vulkan

    // Crear contexto de ImGui
    ImGui::CreateContext();
    ImGui_ImplGlfw_InitForVulkan(window, true);

    // Configurar Descriptor Pool para ImGui
    VkDescriptorPoolSize pool_sizes[] = {
        { VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 10 }
    };
    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 10;
    pool_info.poolSizeCount = 1;
    pool_info.pPoolSizes = pool_sizes;

    // Crear el Descriptor Pool
    if (vkCreateDescriptorPool(vulkan->getDevice(), &pool_info, nullptr, &descriptorPool) != VK_SUCCESS) {
        throw std::runtime_error("Error al crear el Descriptor Pool para ImGui");
    }

    // Configurar ImGui para Vulkan
    ImGui_ImplVulkan_InitInfo init_info = {};
    init_info.Instance = vulkan->getInstance();
    init_info.PhysicalDevice = vulkan->getPhysicalDevice();
    init_info.Device = vulkan->getDevice();
    init_info.QueueFamily = vulkan->getQueueFamilyIndex();
    init_info.Queue = vulkan->getGraphicsQueue();
    init_info.PipelineCache = VK_NULL_HANDLE;
    init_info.DescriptorPool = descriptorPool;
    init_info.Subpass = 0;
    init_info.MinImageCount = 2;
    init_info.ImageCount = 2;
    init_info.MSAASamples = VK_SAMPLE_COUNT_1_BIT;

    ImGui_ImplVulkan_Init(&init_info); // Inicializar ImGui con Vulkan
}

void GUI::render() {
    // Crear frames para ImGui
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();

    // Ejemplo: Crear ventana "Configuración"
    ImGui::Begin("Configuración");
    ImGui::Text("FPS: 60");
    ImGui::End();

    // Renderizar la interfaz
    ImGui::Render();
}

void GUI::cleanup() {
    // Limpiar el Descriptor Pool
    if (vulkan) {
        vkDestroyDescriptorPool(vulkan->getDevice(), descriptorPool, nullptr);
    }

    // Finalizar ImGui
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
}
