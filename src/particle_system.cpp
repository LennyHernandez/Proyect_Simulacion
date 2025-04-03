#include "particle_system.h"
#include <iostream>
#include <cstdlib>
#include <ctime>

void ParticleSystem::init() {
    std::cout << "Inicializando partículas...\n";
    
    // Inicialización de partículas con valores aleatorios
    srand(static_cast<unsigned>(time(0)));
    
    for (int i = 0; i < numParticles; ++i) {
        Particle p;
       p.position = {float(rand() % 100 - 50), float(rand() % 100 - 50)}; // Coordenadas aleatorias
       p.velocity = {float(rand() % 10 - 5), float(rand() % 10 - 5)}; // Velocidad aleatoria
       p.color = {1.0f, 0.5f, 0.0f, 1.0f}; // Color de la partícula
        particles.push_back(p);
    }
}

void ParticleSystem::update() {
    for (auto& p : particles) {
        // Actualizar la posición de la partícula
        p.position.x += p.velocity.x;
        p.position.y += p.velocity.y;

        // Rebotar en los bordes
        if (p.position.x < -50.0f || p.position.x > 50.0f) {
            p.velocity.x = -p.velocity.x; // Rebotar en el eje X
        }
        if (p.position.y < -50.0f || p.position.y > 50.0f) {
            p.velocity.y = -p.velocity.y; // Rebotar en el eje Y
        }
    }
}

void ParticleSystem::cleanup() {
    std::cout << "Limpiando partículas...\n";
    particles.clear();
}
