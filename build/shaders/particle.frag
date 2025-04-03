#version 450

layout(location = 0) in vec3 fragColor;  // Atributo de entrada desde el shader de vértices

layout(location = 0) out vec4 outColor;  // Atributo de salida para el color final del fragmento

void main() {
    outColor = vec4(fragColor, 1.0);  //
}