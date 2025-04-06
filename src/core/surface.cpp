#include "surface.hpp"

#include <stdexcept>
#include <iostream> // Incluido por si acaso, aunque no se usa directamente

namespace particulas {

Surface::Surface(VkInstance instance, GLFWwindow* window)
    : instance_(instance), surface_(VK_NULL_HANDLE) {
    if (glfwCreateWindowSurface(instance_, window, nullptr, &surface_) != VK_SUCCESS) {
        throw std::runtime_error("Failed to create window surface!");
    }
}

Surface::~Surface() {
    vkDestroySurfaceKHR(instance_, surface_, nullptr);
}

} // namespace particulas