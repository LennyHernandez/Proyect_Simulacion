#ifndef PARTICULAS_PARTICLES_PARTICLE_SYSTEM_HPP
#define PARTICULAS_PARTICLES_PARTICLE_SYSTEM_HPP

#include "particle.hpp" // Incluye la definición de Particle
#include <vector>

namespace particulas {

class ParticleSystem {
public:
    // Constructor: inicializa el sistema con un número de partículas y las dimensiones del área
    ParticleSystem(int particleCount, float width, float height);

    // Actualiza el estado de todas las partículas (posición, colisiones con bordes)
    void update(float deltaTime);

    // Devuelve una referencia constante al vector de partículas (para renderización)
    const std::vector<Particle>& getParticles() const;

    
    // --- NUEVO GETTER ---
    // Devuelve el número actual de partículas
    size_t getParticleCount() const { return particles_.size(); }

private:
    // Inicializa las partículas con posiciones, velocidades y colores aleatorios
    void initializeParticles();

    std::vector<Particle> particles_; // Almacenamiento de las partículas
    float width_;                     // Ancho del área de simulación
    float height_;                    // Alto del área de simulación
};

} // namespace particulas

#endif // PARTICULAS_PARTICLES_PARTICLE_SYSTEM_HPP