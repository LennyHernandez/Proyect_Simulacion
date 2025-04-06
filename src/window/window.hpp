#ifndef PARTICULAS_WINDOW_WINDOW_HPP
#define PARTICULAS_WINDOW_WINDOW_HPP

#define GLFW_INCLUDE_VULKAN // Debe estar ANTES de incluir glfw3.h
#include <GLFW/glfw3.h>
#include <string> // Usar std::string para el título
#include <stdexcept> // Para excepciones

namespace particulas {

class Window {
public:
    // Constructor: Inicializa GLFW y crea la ventana. Lanza excepción en caso de error.
    Window(int width, int height, const std::string& title);

    // Destructor: Destruye la ventana y termina GLFW.
    ~Window();

    // --- Métodos principales ---

    // Devuelve el puntero a la ventana GLFW nativa.
    GLFWwindow* getGLFWWindow() const { return window_; }

    // Comprueba si el usuario ha solicitado cerrar la ventana.
    bool shouldClose() const;

    // Procesa los eventos pendientes de GLFW (teclado, ratón, etc.).
    void pollEvents() const;

    // --- Getters ---

    int getWidth() const { return width_; }
    int getHeight() const { return height_; }

    // Obtiene el tamaño del framebuffer (puede ser diferente al tamaño de la ventana en pantallas HiDPI).
    VkExtent2D getFramebufferExtent() const;

    // Crea la superficie de Vulkan para esta ventana.
    // Necesita la instancia de Vulkan para crear la superficie.
    VkResult createSurface(VkInstance instance, VkSurfaceKHR* surface);

    // --- Eliminar copia y asignación ---
    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

private:
    // --- Miembros ---
    GLFWwindow* window_ = nullptr; // Puntero a la ventana GLFW
    int width_;
    int height_;
    std::string title_;

    // --- Inicialización ---
    void initWindow(); // Función privada llamada por el constructor
};

} // namespace particulas

#endif // PARTICULAS_WINDOW_WINDOW_HPP