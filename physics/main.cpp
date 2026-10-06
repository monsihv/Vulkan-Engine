#include <stdio.h>
#include <math.h>
#include <glm/glm.hpp>

constexpr float pi = 3.14159265f;

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

enum Integrator {
    forwardEuler,
    implicitEuler,
    RK4,
};

int main(void) {
    constexpr Integrator integrator = RK4;
    constexpr glm::mat2 identity(1.0f);

    constexpr float m = 2.0f;
    constexpr float w = 2.0f * pi;
    constexpr float gamma = 2.0f;

    constexpr glm::mat2 A(glm::vec2(0.0f, -(w * w)), glm::vec2(1.0f, -gamma / m));

    constexpr glm::vec2 initialConditions(2, 1);
    constexpr uint32_t T = 10;
    constexpr float dt = 0.0011f;

    glm::vec2 stateVector {initialConditions};
    double timePassed;
    constexpr auto cycleCount = static_cast<uint32_t>(T / dt);
    for (uint32_t cycle {}; cycle < cycleCount; ++cycle) {
        timePassed = cycle * dt;

        if constexpr (integrator == forwardEuler) stateVector += dt * (A * stateVector);

        if constexpr (integrator == implicitEuler)
            stateVector = glm::inverse((identity - dt * A)) * stateVector;

        if constexpr (integrator == RK4) {
            float weights[4] = {1.0f, 2.0f, 2.0f, 1.0f};
            float stepSize[4] = {0.0f, 0.5f, 0.5f, 1.0f};

            glm::vec2 derivative = (A * stateVector);
            glm::vec2 weightedDerivative = weights[0] * derivative;
            for (uint32_t i {1}; i < 4; ++i) {
                derivative = A * (stateVector + dt * stepSize[i] * derivative);
                weightedDerivative += weights[i] * derivative;
            }

            stateVector += (dt / 6) * weightedDerivative;
        }

        if (cycle % 10 == 0) {
            printf("Time passed: %f, position: %f, velocity: %f\n",
                    timePassed, stateVector.x, stateVector.y);
        }
    }

    return 0;
}
