#include "headers/base.h"
#include "headers/vertex.h"

#include <cmath>

std::vector<Vertex> vertices;
std::vector<uint16_t> indices;

void makeSphere(std::vector<Vertex>& vertices, std::vector<uint16_t>& indices,
                float radius, uint32_t stacks, uint32_t slices) {
    vertices.clear();
    indices.clear();

    constexpr float PI = 3.14159265358979323846f;

    for (uint32_t i = 0; i <= stacks; ++i) {
        float phi = PI * (float)i / (float)stacks;          // 0 at north pole, PI at south
        float y   = std::cos(phi);
        float r   = std::sin(phi);

        for (uint32_t j = 0; j <= slices; ++j) {
            float theta = 2.0f * PI * (float)j / (float)slices;
            float x = r * std::sin(theta);
            float z = r * std::cos(theta);

            Vertex v;
            v.pos   = glm::vec3(x, y, z) * radius;
            v.color = glm::vec3(x, y, z) * 0.5f + 0.5f;     // normal-as-color, handy for debug
            vertices.push_back(v);
        }
    }

    for (uint32_t i = 0; i < stacks; ++i) {
        for (uint32_t j = 0; j < slices; ++j) {
            uint16_t a = (uint16_t)( i      * (slices + 1) + j);
            uint16_t b = (uint16_t)((i + 1) * (slices + 1) + j);

            indices.push_back(a);
            indices.push_back(b);
            indices.push_back(a + 1);

            indices.push_back(a + 1);
            indices.push_back(b);
            indices.push_back(b + 1);
        }
    }
}

int main() {
    makeSphere(vertices, indices, 0.125f, 16, 24);
    Renderer renderer;
    renderer.mesh = std::move(vertices);
    renderer.indices = std::move(indices);
    renderer.run(800, 600);

    return 0;
}
