//free fall

struct Particle {
    float3 position;
    float3 velocity;
    float3 forces;
    float timeStep;
    float mass;
};

[shader("compute")]
[numthreads(64, 1, 1)]
void main(RWStructuredBuffer<Particle> particles, uint3 id : SV_DispatchThreadID) {
    if (id.x >= 1000) return;

    float g = 9.81f;
    particles[id.x].velocity.y += -g * particles[id.x].timeStep;
    particles[id.x].position += particles[id.x].velocity * particles[id.x].timeStep;
}
