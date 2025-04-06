#version 450

// Entrada desde el vertex shader (interpolada)
layout(location = 0) in vec4 fragColor; // Recibe el color de la partícula

// Salida hacia el framebuffer
layout(location = 0) out vec4 outColor;

void main() {
    // Asignar el color recibido del vertex shader
    outColor = fragColor;
}