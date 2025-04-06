#ifndef PARTICULAS_PARTICLES_PARTICLE_HPP
#define PARTICULAS_PARTICLES_PARTICLE_HPP

#include <glm/glm.hpp> // Asegúrate de que GLM esté accesible

namespace particulas {

struct Particle {
    glm::vec2 position;  // Posición de la partícula en el espacio 2D
    glm::vec2 velocity;  // Velocidad de la partícula (dirección y magnitud)
    glm::vec4 color;     // Color de la partícula (RGBA)
    float radius;        // Radio de la partícula (usado para colisiones)
};

} // namespace particulas

#endif // PARTICULAS_PARTICLES_PARTICLE_HPP