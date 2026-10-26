#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <memory.h>
#include <string.h>
#include <arm_neon.h>
#include <math.h>

typedef float vec4 __attribute__((vector_size(16)));

typedef struct {
    vec4 *positions;
    vec4 *velocities;
    vec4 *forces;
    float mass;

    int particleCount;
    float timeStep;

} N_Body_Simulation;

vec4 computeSingleParticleForce(vec4 r, float rMagnitude, float m1, float m2) {
    auto rCubed = rMagnitude * rMagnitude * rMagnitude;

    return r * (m1 * m2 / rCubed);
}

vec4 computeForce(N_Body_Simulation *sim, int particle1) {
    vec4 cumulativeForce = {0, 0, 0, 0};

    float particleMass = sim->mass;

    int particle2 = 0;
    int count = sim->particleCount;

    float32x4_t particle1Pos = vld1q_f32((float*)&sim->positions[particle1]);
    float32x4_t particle2Pos;
    float32x4_t r, rSquared;
    float rMagnitude;
    for (particle2; particle2 < count; ++particle2) {
        if (particle2 == particle1) continue;
        particle2Pos = vld1q_f32((float*)&sim->positions[particle2]);
        r = vsubq_f32(particle1Pos, particle2Pos);
        rSquared = vmulq_f32(r, r);
        rMagnitude = sqrtf(vaddvq_f32(rSquared));

        cumulativeForce += computeSingleParticleForce((vec4)r, rMagnitude, particleMass, particleMass);
    }

    return cumulativeForce;
}

void solveForForces(N_Body_Simulation *sim) {
    int particle = 0;
    int count = sim->particleCount;
    for (particle; particle < count; ++particle) {
        sim->forces[particle] = computeForce(sim, particle);
    }
}

void updateParticleDynamics (N_Body_Simulation *sim) {
    int particle = 0;
    int count = sim->particleCount;
    float mass = sim->mass;
    float timeStep = sim->timeStep;

    float32x4_t particlePos, particleVelocity, particleForce;
    for (particle; particle < count; ++particle) {
        particleForce = vld1q_f32((float*)&sim->forces[particle]);
        particleVelocity = vld1q_f32((float*)&sim->velocities[particle]);

        particleVelocity = (vec4)particleForce / mass * timeStep + (vec4)particleVelocity;
        vst1q_f32((float*)&sim->velocities[particle], particleVelocity);
        particleVelocity = vld1q_f32((float*)&sim->velocities[particle]);

        particlePos = vld1q_f32((float*)&sim->positions[particle]);
        particlePos = (vec4)particleVelocity * timeStep + (vec4)particlePos;
        vst1q_f32((float*)&sim->positions[particle], particlePos);
    }
}
