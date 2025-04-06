#include "particle_system.hpp"

#include <cstdlib>  // Para std::rand(), std::srand()
#include <ctime>    // Para std::time()
#include <cmath>    // Para std::sqrt(), std::pow() (aunque no se usan aquí directamente)
#include <iostream> // Para depuración si es necesario (std::cout, std::endl)
#include <stdexcept> // Para excepciones si fueran necesarias

namespace particulas {

ParticleSystem::ParticleSystem(int particleCount, float width, float height)
    : width_(width), height_(height) {
    if (particleCount <= 0) {
        throw std::invalid_argument("Particle count must be positive.");
    }
    if (width <= 0.0f || height <= 0.0f) {
       throw std::invalid_argument("Width and height must be positive.");
    }

    // Inicializar la semilla del generador de números aleatorios una sola vez
    std::srand(static_cast<unsigned>(std::time(nullptr)));

    particles_.resize(particleCount);
    initializeParticles();
}

void ParticleSystem::initializeParticles() {
    for (auto& particle : particles_) {
        // Posición aleatoria dentro del cuadro (evitando los bordes exactos inicialmente)
        particle.position = {
            (static_cast<float>(std::rand()) / RAND_MAX * (width_ - 2.0f)) + 1.0f, // Evita 0 y width
            (static_cast<float>(std::rand()) / RAND_MAX * (height_ - 2.0f)) + 1.0f // Evita 0 y height
        };

        // Velocidad aleatoria en el rango [-1, 1] en ambas direcciones, con una magnitud base
        float speed_factor = 50.0f; // Ajusta esta velocidad base
        particle.velocity = glm::normalize(glm::vec2(
            (static_cast<float>(std::rand()) / RAND_MAX * 2.0f - 1.0f),
            (static_cast<float>(std::rand()) / RAND_MAX * 2.0f - 1.0f)
        )) * speed_factor;

         // Asegurarse de que la velocidad no sea cero
        if (glm::length(particle.velocity) < 0.01f) {
             particle.velocity = glm::vec2(speed_factor, 0.0f);
        }


        // Color aleatorio (RGBA), asegurando que no sea completamente negro
        particle.color = {
            (static_cast<float>(std::rand()) / RAND_MAX * 0.8f) + 0.2f, // Rango [0.2, 1.0]
            (static_cast<float>(std::rand()) / RAND_MAX * 0.8f) + 0.2f, // Rango [0.2, 1.0]
            (static_cast<float>(std::rand()) / RAND_MAX * 0.8f) + 0.2f, // Rango [0.2, 1.0]
            1.0f // Alfa opaco
        };

        // Radio fijo para las partículas
        particle.radius = 2.0f; // Ajusta el tamaño visual de las partículas
    }
}

void ParticleSystem::update(float deltaTime) {
    // Asegurar que deltaTime no sea negativo o excesivamente grande
    if (deltaTime <= 0.0f) return;
    // float max_dt = 0.1f; // Límite superior opcional para deltaTime
    // deltaTime = std::min(deltaTime, max_dt);

    for (auto& particle : particles_) {
        // 1. Actualizar posición según la velocidad
        particle.position += particle.velocity * deltaTime;

        // 2. Manejar colisiones con los bordes
        // Colisión con borde izquierdo
        if (particle.position.x - particle.radius < 0.0f) {
            particle.position.x = particle.radius; // Corregir posición para evitar que se quede pegada
            particle.velocity.x = std::abs(particle.velocity.x); // Asegurar velocidad positiva en X
        }
        // Colisión con borde derecho
        else if (particle.position.x + particle.radius > width_) {
            particle.position.x = width_ - particle.radius; // Corregir posición
            particle.velocity.x = -std::abs(particle.velocity.x); // Asegurar velocidad negativa en X
        }

        // Colisión con borde superior (y=0)
        if (particle.position.y - particle.radius < 0.0f) {
            particle.position.y = particle.radius; // Corregir posición
            particle.velocity.y = std::abs(particle.velocity.y); // Asegurar velocidad positiva en Y
        }
        // Colisión con borde inferior
        else if (particle.position.y + particle.radius > height_) {
            particle.position.y = height_ - particle.radius; // Corregir posición
            particle.velocity.y = -std::abs(particle.velocity.y); // Asegurar velocidad negativa en Y
        }
    }
}

const std::vector<Particle>& ParticleSystem::getParticles() const {
    return particles_;
}

} // namespace particulas