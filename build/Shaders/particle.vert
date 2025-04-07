#version 450

// Entradas desde el buffer de vértices (coinciden con ParticleRenderer::getAttributeDescriptions)
layout(location = 0) in vec2 inPosition; // Posición en coordenadas de simulación/mundo
layout(location = 1) in vec4 inColor;    // Color de la partícula

// Salidas hacia el fragment shader
layout(location = 0) out vec4 fragColor; // Color interpolado para el fragmento

// Parámetros (Idealmente desde UBO o Push Constant, pero hardcodeados temporalmente)
const float SIM_WIDTH = 1920.0;
const float SIM_HEIGHT = 1080.0;
const float POINT_SIZE = 4.0; // Tamaño del punto en píxeles

void main() {
    // 1. Transformar coordenadas de simulación a NDC [-1, +1]
    //    NDC_X = (WorldX / Width) * 2.0 - 1.0
    //    NDC_Y = (WorldY / Height) * 2.0 - 1.0
    //    (Vulkan invierte Y implícitamente en el viewport por defecto, así que esta simple transformación funciona)
    vec2 ndcPos;
    ndcPos.x = (inPosition.x / SIM_WIDTH) * 2.0 - 1.0;
    ndcPos.y = (inPosition.y / SIM_HEIGHT) * 2.0 - 1.0; // Y se mapea de [0, H] a [-1, 1]

    // 2. Asignar la posición final en coordenadas de clip
    //    Z = 0.0 (en el plano cercano), W = 1.0 (sin perspectiva)
    gl_Position = vec4(ndcPos, 0.0, 1.0);

    // 3. Establecer el tamaño del punto a renderizar
    gl_PointSize = POINT_SIZE; // Necesita habilitar la característica 'shaderPointSize' en el Device

    // 4. Pasar el color de la partícula al fragment shader
    fragColor = inColor;
}