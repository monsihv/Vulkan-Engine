#include "headers/base.h"
#include "headers/vertex.h"

const std::vector<Vertex> vertices {
    {{0.0f, -0.5f}, {1.0f, 0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}},
    {{-0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}},
};

int main() {
    Renderer renderer;
    renderer.mesh = std::move(vertices);
    renderer.run(800, 600);

    return 0;
}
