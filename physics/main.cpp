#include <stdio.h>
#include <math.h>
#include <glm/glm.hpp>

struct StateVector {
    glm::vec3 position;
    glm::vec3 velocity;
};

struct Particle {
    float mass;
    StateVector stateVec;
    glm::vec3 force;
};

struct ParticleSystem {
    Particle *particles;
    int count;
    float timeStep;
};

int main(void) {
    constexpr float pi = 3.14159265f;
    constexpr float w = 2.0f * pi;
    constexpr float gamma = .25f;
    constexpr glm::mat2 A(glm::vec2(0.0f, -(w * w)), glm::vec2(1.0f, -gamma / w));

    constexpr glm::vec2 initialConditions(2, 0);
    constexpr uint32_t T = 10;
    constexpr float dt = 0.0001f;

    glm::vec2 stateVector {initialConditions};
    double timePassed;
    constexpr auto cycleCount = static_cast<uint32_t>(T / dt);
    for (uint32_t cycle {}; cycle < cycleCount; ++cycle) {
        timePassed = cycle * dt;

        stateVector += dt * (A * stateVector);

        if (cycle % 10 == 0) {
            printf("Time passed: %f, position: %f, velocity: %f\n",
                    timePassed, stateVector.x, stateVector.y);
        }
    }

    return 0;
}
