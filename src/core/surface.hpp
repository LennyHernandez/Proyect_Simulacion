#pragma once

#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>

namespace particulas {

class Surface {
public:
    Surface(VkInstance instance, GLFWwindow* window);
    ~Surface();

    VkSurfaceKHR get() const { return surface_; }

private:
    VkInstance instance_; // Guardamos la instancia para la destrucción
    VkSurfaceKHR surface_;
};

} // namespace particulas