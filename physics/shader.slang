//free fall

struct Particle {
    float3 position;
    float3 velocity;
    float3 forces;
    float timeStep;
    float mass;
};

struct PushConstants {
    Particle *particles;
};

[[vk::push_constant]]
ConstantBuffer<PushConstants> pc;

[shader("compute")]
[numthreads(64, 1, 1)]
void main (uint3 threadId : SV_DispatchThreadID) {
    uint id = threadId.x;
    Particle *particle = &pc.particles[id];

    float dt = particle->timeStep;
    float3 acceleration = particle->forces / particle->mass;
    particle->velocity += acceleration * dt;
    particle->position += particle->velocity * dt;
}
