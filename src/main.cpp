#include "window/window.hpp"
#include "core/instance.hpp"
#include "core/device.hpp"
#include "core/swapchain.hpp"
#include "core/command_pool.hpp"
#include "core/sync.hpp"
#include "core/pipeline.hpp"
#include "core/render_pass.hpp"
#include "particles/particle_system.hpp"
#include "rendering/particle_renderer.hpp"
#include "utils/vulkan_debug.hpp"

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

#include <stdexcept>
#include <iostream>
#include <vector>
#include <cstdlib>
#include <chrono>
#include <limits>
#include <array>
#include <memory>
#include <thread>
#include <string>
#include <fstream>     // <-- Para ofstream
#include <iomanip>     // <-- Para put_time, setprecision
#include <sstream>     // <-- Para stringstream
#include <filesystem>  // <-- Para path, exists, create_directory
#include <algorithm>   // <-- Para min, replace (opcional)

// <-- Headers específicos de plataforma -->
#ifdef _WIN32
    #define WIN32_LEAN_AND_MEAN
    #include <windows.h> // <-- Para DWORD, MAX_COMPUTERNAME_LENGTH, GetComputerNameA
    #include <Lmcons.h>  // <-- Para UNLEN, GetUserNameA
#else // Linux / macOS
    #include <unistd.h>
    #include <limits.h> // Para HOST_NAME_MAX (o usar POSIX_HOST_NAME_MAX)
    #include <pwd.h>    // Para getpwuid, getuid
    // LOGIN_NAME_MAX puede necesitar <stdio.h> o estar definido en otro lugar
    #ifndef LOGIN_NAME_MAX
    #define LOGIN_NAME_MAX 256 // Definición común si no existe
    #endif
    #ifndef HOST_NAME_MAX
    #define HOST_NAME_MAX 256 // Definición común si no existe
    #endif
#endif

// --- Constantes ---
const uint32_t WINDOW_WIDTH = 1920;
const uint32_t WINDOW_HEIGHT = 1080;
const int PARTICLE_COUNT = 10000;
const std::string APP_VERSION = "1.0-OOP_Metrics";
// --- Aplicación Principal ---
class ParticleSimulationApp {
public:
    void run() {
        try {
            initWindow();
            initVulkan();
            initSimulation();
            mainLoop();
        } catch (const std::exception& e) {
            std::cerr << "FATAL ERROR during initialization or main loop: " << e.what() << std::endl;
            // Intenta limpiar lo que se haya podido inicializar
            try {
                cleanup();
            } catch (const std::exception& cleanup_e) {
                 std::cerr << "FATAL ERROR during cleanup: " << cleanup_e.what() << std::endl;
            }
             throw; // Relanzar la excepción original para indicar fallo
        } catch (...) {
             std::cerr << "FATAL ERROR: Unknown exception caught!" << std::endl;
             cleanup(); // Intentar limpiar
             throw;
        }

        // Limpieza normal si todo fue bien
        cleanup();
    }

private:
    // --- Miembros Principales ---
    std::unique_ptr<particulas::Window> window_;
    std::unique_ptr<particulas::Instance> instance_;
    VkDebugUtilsMessengerEXT debugMessenger_ = VK_NULL_HANDLE;
    VkSurfaceKHR surface_ = VK_NULL_HANDLE;
    std::unique_ptr<particulas::Device> device_;
    std::unique_ptr<particulas::Swapchain> swapchain_;
    std::unique_ptr<particulas::RenderPass> renderPass_;
    std::unique_ptr<particulas::Pipeline> pipeline_;
    std::vector<VkFramebuffer> swapchainFramebuffers_;
    std::unique_ptr<particulas::CommandPool> commandPool_; // <-- Tipo Correcto
    std::vector<VkCommandBuffer> commandBuffers_;
    std::unique_ptr<particulas::Sync> sync_;
    

    // --- Recursos de Simulación y Renderizado ---
    std::unique_ptr<particulas::ParticleSystem> particleSystem_;
    std::unique_ptr<particulas::ParticleRenderer> particleRenderer_; // <-- Tipo Correcto

    // --- Recursos de Profundidad ---
    VkImage depthImage_ = VK_NULL_HANDLE;
    VkDeviceMemory depthImageMemory_ = VK_NULL_HANDLE;
    VkImageView depthImageView_ = VK_NULL_HANDLE;

    // --- Estado ---
    bool framebufferResized_ = false;
    // uint32_t currentFrame_ = 0; // No necesario si usamos getter de Sync

    // --- Miembros NUEVOS para Métricas ---
    std::vector<float> frameTimesMs_;
    std::chrono::time_point<std::chrono::system_clock> runStartTime_;
    std::string gpuName_ = "Unknown";
    bool metricsSaved_ = false;

    // --- Inicialización ---
    void initWindow() {
        std::cout << "Initializing Window..." << std::endl;
        window_ = std::make_unique<particulas::Window>(WINDOW_WIDTH, WINDOW_HEIGHT, "Simulación de Partículas Vulkan");
        // TODO: Añadir callback GLFW para detectar redimensionamiento
        std::cout << "Window Initialized." << std::endl;
    }

    void initVulkan() {
        std::cout << "Initializing Vulkan..." << std::endl;
        createInstance();
        setupDebugMessenger();
        createSurface();
        createDevice();
        createSwapchain();
        createRenderPass();
        createGraphicsPipeline();
        createDepthResources();
        createFramebuffers();
        createCommandPool();
        createCommandBuffers();
        createSyncObjects();
        std::cout << "Vulkan Initialized." << std::endl;
    }

    void initSimulation() {
        std::cout << "Initializing Simulation..." << std::endl;
        if (!swapchain_) throw std::runtime_error("Swapchain not initialized before simulation init.");
        VkExtent2D extent = swapchain_->getExtent();
        particleSystem_ = std::make_unique<particulas::ParticleSystem>(
            PARTICLE_COUNT, static_cast<float>(extent.width), static_cast<float>(extent.height) );

        if (!device_ || !commandPool_) throw std::runtime_error("Device or CommandPool not initialized before renderer init.");
        // Usar el tipo correcto aquí también
        particleRenderer_ = std::make_unique<particulas::ParticleRenderer>(*device_, *commandPool_); // <-- Tipo Correcto
        particleRenderer_->createBuffers(particleSystem_->getParticles()); // <-- Usar ->
        std::cout << "Simulation Initialized." << std::endl;
    }

    // --- Bucle Principal ---
    void mainLoop() {
        std::cout << "Starting Main Loop..." << std::endl;
        runStartTime_ = std::chrono::system_clock::now(); // <-- Guardar hora inicio para archivo/metadata
        auto lastTime = std::chrono::high_resolution_clock::now();
        frameTimesMs_.reserve(3600); // <-- Reservar espacio

        while (window_ && !window_->shouldClose()) {
            window_->pollEvents();
            auto currentTime = std::chrono::high_resolution_clock::now();
            // Usar high_resolution_clock para deltaTime también es más preciso
            float deltaTime = std::chrono::duration<float>(currentTime - lastTime).count();
            lastTime = currentTime;
            deltaTime = std::min(deltaTime, 0.1f); // Clamp

            // --- NUEVO: Registrar Métrica ---
            if (deltaTime > 1e-6f) { // Evitar valores inválidos
                frameTimesMs_.push_back(deltaTime * 1000.0f); // Guardar en ms
            }
            // --------------------------------

            if (particleSystem_ && deltaTime > 0.0f) {
                 particleSystem_->update(deltaTime);
            }
            drawFrame();
        }
        std::cout << "Exiting Main Loop." << std::endl;
        if(device_) { vkDeviceWaitIdle(device_->getLogicalDevice()); std::cout << "GPU Idle." << std::endl; }
    }
    // --- Limpieza ---
    void cleanup() {
         std::cout << "Starting Cleanup..." << std::endl;
         if(device_) { vkDeviceWaitIdle(device_->getLogicalDevice()); }

        cleanupSwapchainRelated();

        // Usar el tipo correcto particleRenderer_
        if (particleRenderer_) { std::cout << "Cleaning up Particle Renderer..." << std::endl; particleRenderer_.reset(); } // <-- Usar .reset()
        if (particleSystem_) { std::cout << "Cleaning up Particle System..." << std::endl; particleSystem_.reset(); }
        if (sync_) { std::cout << "Cleaning up Sync Objects..." << std::endl; sync_.reset(); }

        if (commandPool_ && device_ && !commandBuffers_.empty()) {
            std::cout << "Freeing Command Buffers..." << std::endl;
            vkFreeCommandBuffers(device_->getLogicalDevice(), commandPool_->get(), static_cast<uint32_t>(commandBuffers_.size()), commandBuffers_.data());
            commandBuffers_.clear();
        }
        if (commandPool_) { std::cout << "Cleaning up Command Pool..." << std::endl; commandPool_.reset(); } // <-- Usar .reset()

        if(device_) { std::cout << "Cleaning up Logical Device..." << std::endl; device_.reset(); }

        if (instance_) {
            if (debugMessenger_ != VK_NULL_HANDLE) {
                 std::cout << "Cleaning up Debug Messenger..." << std::endl;
                 particulas::debug::destroyDebugMessenger(instance_->get(), debugMessenger_, nullptr);
                 debugMessenger_ = VK_NULL_HANDLE;
            }
            if (surface_ != VK_NULL_HANDLE) {
                 std::cout << "Cleaning up Surface..." << std::endl;
                vkDestroySurfaceKHR(instance_->get(), surface_, nullptr);
                 surface_ = VK_NULL_HANDLE;
            }
        }

        if (instance_) { std::cout << "Cleaning up Vulkan Instance..." << std::endl; instance_.reset(); }
        if (window_) { std::cout << "Cleaning up Window..." << std::endl; window_.reset(); }
        else { glfwTerminate(); }
        std::cout << "Cleanup Finished." << std::endl;

            // --- Guardar métricas si no se han guardado ---
            if (!metricsSaved_ && !frameTimesMs_.empty()) {
                saveMetricsToFile();
            }
    }


    // --- Funciones Auxiliares de Inicialización ---

    void createInstance() {
        std::vector<const char*> validationLayers;
        // Poner #ifndef en su propia línea
        #ifndef NDEBUG
            validationLayers.push_back("VK_LAYER_KHRONOS_validation");
        // Poner #endif en su propia línea
        #endif
        std::vector<const char*> requiredExtensions; // <-- ¡Este faltaba!
        instance_ = std::make_unique<particulas::Instance>(validationLayers, requiredExtensions);
    }

    void setupDebugMessenger() {
        #ifndef NDEBUG
        if (!instance_) return;
        VkResult result = particulas::debug::setupDebugMessenger(instance_->get(), &debugMessenger_);
        if (result != VK_SUCCESS) { std::cerr << "Warning: Failed to set up debug messenger! Code: " << result << std::endl; debugMessenger_ = VK_NULL_HANDLE; }
        else { std::cout << "Debug messenger set up successfully." << std::endl; }
        #endif
    }

    void createSurface() {
        if (!instance_ || !window_) throw std::runtime_error("Instance or Window not initialized before creating surface.");
        VkResult result = window_->createSurface(instance_->get(), &surface_);
        particulas::debug::checkVkResult(result, "Window surface creation");
    }

    void createDevice() {
        if (!instance_ || surface_ == VK_NULL_HANDLE) throw std::runtime_error("Instance or Surface not initialized before creating device.");
        device_ = std::make_unique<particulas::Device>(instance_->get(), surface_);
    }

    void createSwapchain() {
        if (!device_ || surface_ == VK_NULL_HANDLE || !window_) throw std::runtime_error("Cannot create swapchain: dependencies missing.");
        swapchain_ = std::make_unique<particulas::Swapchain>(*device_, surface_, *window_);
    }

    // findSupportedFormat y findDepthFormat
    VkFormat findSupportedFormat(const std::vector<VkFormat>& candidates, VkImageTiling tiling, VkFormatFeatureFlags features) {
        if (!device_) throw std::runtime_error("Device not initialized before finding supported format.");
        for (VkFormat format : candidates) {
            VkFormatProperties props; vkGetPhysicalDeviceFormatProperties(device_->getPhysicalDevice(), format, &props);
            if (tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & features) == features) { return format; }
            else if (tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & features) == features) { return format; }
        }
        throw std::runtime_error("failed to find supported format!");
    }
    VkFormat findDepthFormat() {
        return findSupportedFormat( {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
            VK_IMAGE_TILING_OPTIMAL, VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT );
    }

    void createRenderPass() {
        if (!device_ || !swapchain_) throw std::runtime_error("Cannot create render pass: dependencies missing.");
        VkFormat depthFormat = findDepthFormat();
        renderPass_ = std::make_unique<particulas::RenderPass>(device_->getLogicalDevice(), swapchain_->getImageFormat(), depthFormat);
    }

    void createGraphicsPipeline() {
         if (!device_ || !renderPass_) throw std::runtime_error("Cannot create pipeline: dependencies missing.");
        pipeline_ = std::make_unique<particulas::Pipeline>(device_->getLogicalDevice(), renderPass_->get());
    }

    // createImage, createImageView
     void createImage(uint32_t width, uint32_t height, VkFormat format, VkImageTiling tiling, VkImageUsageFlags usage, VkMemoryPropertyFlags properties, VkImage& image, VkDeviceMemory& imageMemory) {
        if (!device_) throw std::runtime_error("Device not initialized before creating image.");
        VkImageCreateInfo imageInfo{}; imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO; imageInfo.imageType = VK_IMAGE_TYPE_2D; imageInfo.extent = {width, height, 1};
        imageInfo.mipLevels = 1; imageInfo.arrayLayers = 1; imageInfo.format = format; imageInfo.tiling = tiling; imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
        imageInfo.usage = usage; imageInfo.samples = VK_SAMPLE_COUNT_1_BIT; imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;
        particulas::debug::checkVkResult(vkCreateImage(device_->getLogicalDevice(), &imageInfo, nullptr, &image), "Image creation");
        VkMemoryRequirements memRequirements; vkGetImageMemoryRequirements(device_->getLogicalDevice(), image, &memRequirements);
        VkMemoryAllocateInfo allocInfo{}; allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO; allocInfo.allocationSize = memRequirements.size;
        allocInfo.memoryTypeIndex = device_->findMemoryType(memRequirements.memoryTypeBits, properties); // <-- Usar Device
        particulas::debug::checkVkResult(vkAllocateMemory(device_->getLogicalDevice(), &allocInfo, nullptr, &imageMemory), "Image memory allocation");
        particulas::debug::checkVkResult(vkBindImageMemory(device_->getLogicalDevice(), image, imageMemory, 0), "Bind image memory");
    }
     VkImageView createImageView(VkImage image, VkFormat format, VkImageAspectFlags aspectFlags) {
        if (!device_) throw std::runtime_error("Device not initialized before creating image view.");
        VkImageViewCreateInfo viewInfo{}; viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO; viewInfo.image = image; viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D; viewInfo.format = format;
        viewInfo.subresourceRange.aspectMask = aspectFlags; viewInfo.subresourceRange.baseMipLevel = 0; viewInfo.subresourceRange.levelCount = 1; viewInfo.subresourceRange.baseArrayLayer = 0; viewInfo.subresourceRange.layerCount = 1;
        VkImageView imageView = VK_NULL_HANDLE;
        particulas::debug::checkVkResult(vkCreateImageView(device_->getLogicalDevice(), &viewInfo, nullptr, &imageView), "Image view creation");
        return imageView;
     }

    void createDepthResources() {
        if (!device_ || !swapchain_) throw std::runtime_error("Cannot create depth resources: dependencies missing.");
        VkFormat depthFormat = findDepthFormat();
        VkExtent2D swapChainExtent = swapchain_->getExtent();
        createImage(swapChainExtent.width, swapChainExtent.height, depthFormat, VK_IMAGE_TILING_OPTIMAL,
                    VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
                    depthImage_, depthImageMemory_);
        depthImageView_ = createImageView(depthImage_, depthFormat, VK_IMAGE_ASPECT_DEPTH_BIT);
    }

    void createFramebuffers() {
         if (!device_ || !renderPass_ || !swapchain_ || depthImageView_ == VK_NULL_HANDLE) throw std::runtime_error("Cannot create framebuffers: dependencies missing.");
        swapchainFramebuffers_.resize(swapchain_->getImageViews().size());
        VkExtent2D swapChainExtent = swapchain_->getExtent();
        for (size_t i = 0; i < swapchain_->getImageViews().size(); i++) {
            std::array<VkImageView, 2> attachments = { swapchain_->getImageViews()[i], depthImageView_ };
            VkFramebufferCreateInfo framebufferInfo{}; framebufferInfo.sType = VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO; framebufferInfo.renderPass = renderPass_->get();
            framebufferInfo.attachmentCount = static_cast<uint32_t>(attachments.size()); framebufferInfo.pAttachments = attachments.data();
            framebufferInfo.width = swapChainExtent.width; framebufferInfo.height = swapChainExtent.height; framebufferInfo.layers = 1;
            particulas::debug::checkVkResult( vkCreateFramebuffer(device_->getLogicalDevice(), &framebufferInfo, nullptr, &swapchainFramebuffers_[i]),
                "Framebuffer creation for swapchain image " + std::to_string(i) );
        }
    }

     void createCommandPool() {
         if (!device_) throw std::runtime_error("Device not initialized before creating command pool.");
        commandPool_ = std::make_unique<particulas::CommandPool>(device_->getLogicalDevice(), device_->getGraphicsQueueFamilyIndex());
    }

    void createCommandBuffers() {
         if (!device_ || !commandPool_) throw std::runtime_error("Cannot create command buffers: dependencies missing.");
        commandBuffers_.resize(particulas::MAX_FRAMES_IN_FLIGHT); // Usar constante del namespace
        VkCommandBufferAllocateInfo allocInfo{}; allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.commandPool = commandPool_->get(); allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandBufferCount = (uint32_t) commandBuffers_.size();
        VkResult allocResult = vkAllocateCommandBuffers(device_->getLogicalDevice(), &allocInfo, commandBuffers_.data());
        particulas::debug::checkVkResult(allocResult, "Command buffer allocation");
    }

    void createSyncObjects() {
        if (!device_) throw std::runtime_error("Device not initialized before creating sync objects.");
         sync_ = std::make_unique<particulas::Sync>(device_->getLogicalDevice());
    }

    // --- Funciones de Renderizado ---
    void recordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex) {
        if (!renderPass_ || imageIndex >= swapchainFramebuffers_.size() || !pipeline_ || !particleRenderer_ || !particleSystem_ || !swapchain_) {
             throw std::runtime_error("Cannot record command buffer: dependencies missing or imageIndex out of bounds.");
        }
        VkCommandBufferBeginInfo beginInfo{}; beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
        particulas::debug::checkVkResult(vkBeginCommandBuffer(commandBuffer, &beginInfo), "Begin command buffer");

        VkRenderPassBeginInfo renderPassInfo{}; renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO; renderPassInfo.renderPass = renderPass_->get();
        renderPassInfo.framebuffer = swapchainFramebuffers_[imageIndex]; renderPassInfo.renderArea.offset = {0, 0}; renderPassInfo.renderArea.extent = swapchain_->getExtent();
        std::array<VkClearValue, 2> clearValues{}; clearValues[0].color = {{0.1f, 0.1f, 0.1f, 1.0f}}; clearValues[1].depthStencil = {1.0f, 0};
        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size()); renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
        // Usar el tipo correcto particleRenderer_ y ->
        particleRenderer_->recordCommandBuffer( commandBuffer, pipeline_->getGraphicsPipeline(), pipeline_->getPipelineLayout(),
            swapchain_->getExtent(), static_cast<uint32_t>(particleSystem_->getParticles().size()) ); // <-- Usar ->
        vkCmdEndRenderPass(commandBuffer);
        particulas::debug::checkVkResult(vkEndCommandBuffer(commandBuffer), "End command buffer");
    }

     void drawFrame() {
         if (!sync_ || !device_ || !swapchain_ || commandBuffers_.empty() || !particleRenderer_ || !particleSystem_) {
             std::cerr << "Warning: Skipping drawFrame, dependencies not ready." << std::endl;
             std::this_thread::sleep_for(std::chrono::milliseconds(10)); return;
         }
         sync_->waitForFence();

         uint32_t imageIndex;
         VkResult acquireResult = vkAcquireNextImageKHR(device_->getLogicalDevice(), swapchain_->get(),
                                                std::numeric_limits<uint64_t>::max(),
                                                sync_->getImageAvailableSemaphore(), VK_NULL_HANDLE, &imageIndex);
         if (acquireResult == VK_ERROR_OUT_OF_DATE_KHR || acquireResult == VK_SUBOPTIMAL_KHR || framebufferResized_) {
             framebufferResized_ = false; recreateSwapchain(); return;
         } else { particulas::debug::checkVkResult(acquireResult, "Acquire next image"); }

         // Usar el tipo correcto particleRenderer_ y ->
         particleRenderer_->updateBuffers(particleSystem_->getParticles()); // <-- Usar ->
         sync_->resetFence();

         uint32_t syncFrameIndex = sync_->getCurrentFrameIndex();
         VkCommandBuffer currentCommandBuffer = commandBuffers_[syncFrameIndex];
         vkResetCommandBuffer(currentCommandBuffer, 0);
         recordCommandBuffer(currentCommandBuffer, imageIndex);

         VkSubmitInfo submitInfo{}; submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
         VkSemaphore waitSemaphores[] = {sync_->getImageAvailableSemaphore()}; VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
         submitInfo.waitSemaphoreCount = 1; submitInfo.pWaitSemaphores = waitSemaphores; submitInfo.pWaitDstStageMask = waitStages;
         submitInfo.commandBufferCount = 1;
         submitInfo.pCommandBuffers = &currentCommandBuffer; // <-- CORREGIDO
         VkSemaphore signalSemaphores[] = {sync_->getRenderFinishedSemaphore()};
         submitInfo.signalSemaphoreCount = 1; submitInfo.pSignalSemaphores = signalSemaphores;

         particulas::debug::checkVkResult( vkQueueSubmit(device_->getGraphicsQueue(), 1, &submitInfo, sync_->getInFlightFence()), "Queue submit");

         VkPresentInfoKHR presentInfo{}; presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR; presentInfo.waitSemaphoreCount = 1; presentInfo.pWaitSemaphores = signalSemaphores;
         VkSwapchainKHR swapChains[] = {swapchain_->get()};
         presentInfo.swapchainCount = 1; presentInfo.pSwapchains = swapChains; presentInfo.pImageIndices = &imageIndex;
         VkResult presentResult = vkQueuePresentKHR(device_->getPresentQueue(), &presentInfo);
         if (presentResult == VK_ERROR_OUT_OF_DATE_KHR || presentResult == VK_SUBOPTIMAL_KHR || framebufferResized_) {
            framebufferResized_ = false; recreateSwapchain();
         } else if (presentResult != VK_SUCCESS) { particulas::debug::checkVkResult(presentResult, "Queue present"); }

         sync_->nextFrame();
    }

    // --- Recreación del Swapchain ---
    void cleanupSwapchainRelated() { /* ... (código como antes) ... */ }
    void recreateSwapchain() { /* ... (código como antes) ... */ }

    // --- Implementación de Funciones Auxiliares para Métricas --- NUEVO ---

    std::string getTimestamp(const std::chrono::time_point<std::chrono::system_clock>& timePoint) {
        auto timet = std::chrono::system_clock::to_time_t(timePoint);
        #ifdef _MSC_VER // Manejo seguro para MSVC
            std::tm tm_buf; localtime_s(&tm_buf, &timet);
        #else // Otros compiladores
            std::tm tm_buf = *std::localtime(&timet);
        #endif
        std::stringstream ss; ss << std::put_time(&tm_buf, "%Y%m%d_%H%M%S"); return ss.str();
    }

    std::string getUsername() {
        #ifdef _WIN32
            char username[UNLEN + 1]; DWORD username_len = UNLEN + 1;
            return (GetUserNameA(username, &username_len)) ? std::string(username) : "unknown_user";
        #else
            char username[LOGIN_NAME_MAX]; if (getlogin_r(username, sizeof(username)) == 0) return std::string(username);
            char* user_env = getenv("USER"); if (user_env != nullptr) return std::string(user_env);
            struct passwd *pw = getpwuid(getuid()); if (pw) return std::string(pw->pw_name);
            return "unknown_user";
        #endif
    }

    std::string getHostname() {
        #ifdef _WIN32
            char hostname[MAX_COMPUTERNAME_LENGTH + 1]; DWORD hostname_len = MAX_COMPUTERNAME_LENGTH + 1;
            return (GetComputerNameA(hostname, &hostname_len)) ? std::string(hostname) : "unknown_pc";
        #else
            char hostname[HOST_NAME_MAX]; return (gethostname(hostname, sizeof(hostname)) == 0) ? std::string(hostname) : "unknown_pc";
        #endif
    }

    std::string generateFilename() {
        std::string timestamp = getTimestamp(runStartTime_);
        std::string username = getUsername();
        std::string hostname = getHostname();
        // Podrías reemplazar espacios u otros caracteres aquí si quieres
        // std::replace(username.begin(), username.end(), ' ', '_');
        return "metrics_" + timestamp + "_" + username + "@" + hostname + ".csv";
    }

    void saveMetricsToFile() {
        std::cout << "[Metrics] Entering saveMetricsToFile(). "
                  << "metricsSaved_ = " << std::boolalpha << metricsSaved_
                  << ", frameTimesMs_.empty() = " << std::boolalpha << frameTimesMs_.empty()
                  << ", frameTimesMs_.size() = " << frameTimesMs_.size() << std::endl;
    
        if (metricsSaved_ || frameTimesMs_.empty()) {
            std::cout << "[Metrics] Skipping: " << (metricsSaved_ ? "Metrics already saved." : "No frame times recorded.") << std::endl;
            return; // Salir si ya se guardó o no hay datos
        }
    
        std::string filename = generateFilename();
        std::string metricsDir = "metrics_output";
        
        try {
            std::filesystem::path dirPath = metricsDir;
            std::cout << "[Metrics] Checking directory existence: " << dirPath.string() << std::endl;
    
            if (!std::filesystem::exists(dirPath)) {
                std::cout << "[Metrics] Directory does not exist. Attempting to create..." << std::endl;
                if (std::filesystem::create_directory(dirPath)) {
                    std::cout << "[Metrics] Created directory: " << dirPath.string() << std::endl;
                } else {
                    std::cerr << "[Metrics] Error: Could not create directory. Saving in the current directory." << std::endl;
                    metricsDir = "."; // Fallback to current directory
                    dirPath = metricsDir;
                }
            } else {
                std::cout << "[Metrics] Directory exists." << std::endl;
            }
    
            std::filesystem::path fullPath = dirPath / filename;
            std::cout << "[Metrics] Full file path: " << fullPath.string() << std::endl;
    
            std::ofstream outFile(fullPath);
            if (!outFile.is_open()) {
                std::cerr << "[Metrics] Error opening file for writing: " << fullPath.string() << std::endl;
                return;
            } else {
                std::cout << "[Metrics] File opened successfully." << std::endl;
            }
    
            outFile << "# METRICS DATA\n" 
                    << "# Run Start Timestamp: " << getTimestamp(runStartTime_) << "\n" 
                    << "# Program Version: " << ::APP_VERSION << "\n"
                    << "# User: " << getUsername() << "\n" 
                    << "# Hostname: " << getHostname() << "\n" 
                    << "# GPU: " << gpuName_ << "\n"
                    << "# Requested Particle Count: " << PARTICLE_COUNT << "\n" 
                    << "# Actual Particle Count: " << (particleSystem_ ? std::to_string(particleSystem_->getParticleCount()) : "N/A") << "\n"
                    << "# Frame Count Recorded: " << frameTimesMs_.size() << "\n\n" 
                    << "FrameTime_ms\n";
    
            outFile << std::fixed << std::setprecision(4);
            for (float frameTime : frameTimesMs_) {
                outFile << frameTime << "\n";
            }
    
            outFile.close();
            metricsSaved_ = true;
            std::cout << "[Metrics] Metrics saved successfully (" << frameTimesMs_.size() << " frames)." << std::endl;
    
        } catch (const std::filesystem::filesystem_error& fs_err) {
            std::cerr << "[Metrics] Filesystem error: " << fs_err.what() << " Path1: " << fs_err.path1().string() << " Path2: " << fs_err.path2().string() << std::endl;
        } catch (const std::exception& e) {
            std::cerr << "[Metrics] Exception: " << e.what() << std::endl;
        } catch (...) {
            std::cerr << "[Metrics] Unknown error." << std::endl;
        }
    }

}; // Fin de la clase ParticleSimulationApp

// --- Punto de Entrada ---
int main() {
    ParticleSimulationApp app;
    try { app.run(); }
    catch (const std::exception& e) { std::cerr << "FATAL ERROR (std::exception): " << e.what() << std::endl; return EXIT_FAILURE; }
    catch (...) { std::cerr << "FATAL ERROR: Unknown exception caught!" << std::endl; return EXIT_FAILURE; }
    std::cout << "Application finished successfully." << std::endl;
    return EXIT_SUCCESS;
}