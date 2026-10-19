#include <stdio.h>
#include <stdint.h>

typedef float Mat2 __attribute__((matrix_type(2, 2)));
typedef float Col2 __attribute__((matrix_type(2, 1)));
typedef float Vec4 __attribute__((ext_vector_type(4)));

static inline Mat2 mat2(float c0x, float c0y, float c1x, float c1y) {
    float m[4] = { c0x, c0y, c1x, c1y };                   // column-major
    return __builtin_matrix_column_major_load(m, 2, 2, 2);
}

static inline Col2 col2(float x, float y) {
    float v[2] = { x, y };
    return __builtin_matrix_column_major_load(v, 2, 1, 2);
}

static inline Mat2 inverse_m2(Mat2 m) {
    float a = m[0][0], c = m[0][1];
    float b = m[1][0], d = m[1][1];
    float invDet = 1.0f / (a * d - c * b);
    return mat2(d, -b, -c, a) * invDet;
}

constexpr float pi = 3.14159265f;

typedef struct {
    Vec4 position;   // w = 1
    Vec4 velocity;   // w = 0
} StateVector;

typedef struct {
    float mass;
    StateVector stateVec;
    Vec4 force;      // w = 0
} Particle;

typedef struct {
    Particle *particles;
    int count;
    float timeStep;
} ParticleSystem;

typedef enum {
    forwardEuler,
    implicitEuler,
    RK4,
} Integrator;

int main(void) {
    constexpr Integrator integrator = RK4;
    const Mat2 identity = mat2(1.0f, 0.0f, 0.0f, 1.0f);

    constexpr float m = 2.0f;
    constexpr float w = 2.0f * pi;
    constexpr float gamma = 2.0f;

    Mat2 A = mat2(0.0f, -(w * w), 1.0f, -gamma / m);

    constexpr uint32_t T = 10;
    constexpr float dt = 0.0011f;

    Col2 stateVector = col2(2, 1);
    double timePassed;
    constexpr uint32_t cycleCount = (uint32_t)(T / dt);
    for (uint32_t cycle = 0; cycle < cycleCount; ++cycle) {
        timePassed = cycle * dt;

        switch (integrator) {
        case forwardEuler:
            stateVector += dt * (A * stateVector);
            break;

        case implicitEuler:
            stateVector = inverse_m2(identity - dt * A) * stateVector;
            break;

        case RK4: {
            float weights[4] = {1.0f, 2.0f, 2.0f, 1.0f};
            float stepSize[4] = {0.0f, 0.5f, 0.5f, 1.0f};

            Col2 derivative = A * stateVector;
            Col2 weightedDerivative = weights[0] * derivative;
            for (uint32_t i = 1; i < 4; ++i) {
                derivative = A * (stateVector + dt * stepSize[i] * derivative);
                weightedDerivative += weights[i] * derivative;
            }

            stateVector += (dt / 6) * weightedDerivative;
        } break;
        }

        if (cycle % 10 == 0) {
            printf("Time passed: %f, position: %f, velocity: %f\n",
                    timePassed, stateVector[0][0], stateVector[1][0]);
        }
    }

    return 0;
}
