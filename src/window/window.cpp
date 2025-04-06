#include "window.hpp"

#include <iostream> // Para depuración si es necesario

namespace particulas {

Window::Window(int width, int height, const std::string& title)
    : width_(width), height_(height), title_(title) {
    initWindow();
}

Window::~Window() {
    if (window_) {
        glfwDestroyWindow(window_);
    }
    glfwTerminate(); // Terminar GLFW cuando la última ventana se destruye
                     // Nota: Para múltiples ventanas, esto requeriría un manejo más cuidadoso.
}

void Window::initWindow() {
    // Inicializar GLFW (solo la primera vez que se crea una ventana)
    if (!glfwInit()) {
        throw std::runtime_error("Failed to initialize GLFW!");
    }

    // Configurar GLFW para NO usar OpenGL y no ser redimensionable (por ahora)
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE); // Cambiar a TRUE si quieres manejar redimensionamiento

    // Crear la ventana
    window_ = glfwCreateWindow(width_, height_, title_.c_str(), nullptr, nullptr);
    if (!window_) {
        glfwTerminate(); // Limpiar si la creación de la ventana falla
        throw std::runtime_error("Failed to create GLFW window!");
    }

     // Puedes añadir callbacks aquí si los necesitas (p.ej., teclado, ratón)
     // glfwSetKeyCallback(window_, key_callback);
}

bool Window::shouldClose() const {
    return glfwWindowShouldClose(window_);
}

void Window::pollEvents() const {
    glfwPollEvents();
}

VkExtent2D Window::getFramebufferExtent() const {
    int width, height;
    glfwGetFramebufferSize(window_, &width, &height);
    return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
}

VkResult Window::createSurface(VkInstance instance, VkSurfaceKHR* surface) {
    // GLFW proporciona esta función para crear la superficie de Vulkan específica de la plataforma
    if (glfwCreateWindowSurface(instance, window_, nullptr, surface) != VK_SUCCESS) {
        return VK_ERROR_INITIALIZATION_FAILED; // O algún otro código de error
    }
    return VK_SUCCESS;
}


} // namespace particulas