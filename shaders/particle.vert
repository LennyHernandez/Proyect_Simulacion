#version 450

layout(location = 0) in vec2 inPosition;  // Atributo de entrada para las posiciones de los vértices
layout(location = 1) in vec3 inColor;     // Atributo de entrada para los colores de los vértices

layout(location = 0) out vec3 fragColor;  // Atributo de salida para el color del fragmento

void main() {
    gl_Position = vec4(inPosition, 0.0, 1.0); // Pasar las coordenadas de los vértices
    fragColor = inColor;  // Pasar el color al fragment shader
}
