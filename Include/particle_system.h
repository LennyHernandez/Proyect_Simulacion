#pragma once
#include <vector>

// Estructura para definir las propiedades de cada partícula
struct Particle {
    struct { float x, y; } position;   // Posición de la partícula
    struct { float x, y; } velocity;  // Velocidad de la partícula
    struct { float r, g, b, a; } color; // Color RGBA de la partícula
};

// Clase para manejar el sistema de partículas
class ParticleSystem {
public:
    // Inicializar las partículas
    void init();

    // Actualizar las partículas (movimiento, interacción, etc.)
    void update();

    // Limpiar y liberar recursos
    void cleanup();

private:
    static const int numParticles = 100; // Número de partículas en el sistema
    std::vector<Particle> particles;    // Contenedor para almacenar las partículas
};