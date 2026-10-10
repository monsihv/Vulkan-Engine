#include "headers/base.h"
#include "headers/vertex.h"

#include <math.h>

void makeSphere(Arena *arena, Vertex **outVertices, u32 *outVertexCount,
                u16 **outIndices, u32 *outIndexCount,
                float radius, u32 stacks, u32 slices) {
    u32 vertexCount = (stacks + 1) * (slices + 1);
    u32 indexCount = stacks * slices * 6;

    Vertex *vertices = pushArray(arena, Vertex, vertexCount);
    u16 *indices = pushArray(arena, u16, indexCount);

    constexpr float PI = 3.14159265358979323846f;

    u32 v = 0;
    for (u32 i = 0; i <= stacks; ++i) {
        float phi = PI * (float)i / (float)stacks;          // 0 at north pole, PI at south
        float y   = cosf(phi);
        float r   = sinf(phi);

        for (u32 j = 0; j <= slices; ++j) {
            float theta = 2.0f * PI * (float)j / (float)slices;
            float x = r * sinf(theta);
            float z = r * cosf(theta);

            vertices[v].pos   = glm::vec3(x, y, z) * radius;
            vertices[v].color = glm::vec3(x, y, z) * 0.5f + 0.5f;     // normal-as-color, handy for debug
            v++;
        }
    }

    u32 n = 0;
    for (u32 i = 0; i < stacks; ++i) {
        for (u32 j = 0; j < slices; ++j) {
            u16 a = (u16)( i      * (slices + 1) + j);
            u16 b = (u16)((i + 1) * (slices + 1) + j);

            indices[n++] = a;
            indices[n++] = b;
            indices[n++] = a + 1;

            indices[n++] = a + 1;
            indices[n++] = b;
            indices[n++] = b + 1;
        }
    }

    *outVertices = vertices;
    *outVertexCount = vertexCount;
    *outIndices = indices;
    *outIndexCount = indexCount;
}

static void framebufferResizeCallback(GLFWwindow *window, int width, int height) {
    Renderer *renderer = (Renderer *)glfwGetWindowUserPointer(window);
    renderer->framebufferResized = true;
}

static void initWindow(Renderer *renderer, u32 windowWidth, u32 windowHeight) {
    glfwInit();
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

    renderer->window = glfwCreateWindow(windowWidth, windowHeight, "ada wong", NULL, NULL);

    int fbWidth, fbHeight;
    glfwGetFramebufferSize(renderer->window, &fbWidth, &fbHeight);

    renderer->width = (u32)fbWidth;
    renderer->height = (u32)fbHeight;

    glfwSetWindowUserPointer(renderer->window, renderer);
    glfwSetFramebufferSizeCallback(renderer->window, framebufferResizeCallback);

    glfwSetInputMode(renderer->window, GLFW_CURSOR, GLFW_CURSOR_NORMAL);
}

static void initVulkan(Renderer *renderer) {
    createInstance(renderer);
    createSurface(renderer);
    selectGPU(renderer);
    createLogicalDevice(renderer);
    createSwapchain(renderer);
    createSwapchainImageViews(renderer);
    createDepthImage(renderer);
    createCommandPool(renderer);
    allocateCommandBuffer(renderer);
    createDescriptorPools(renderer);
    createUniformBufferDescriptorSets(renderer);
    createCameraBuffers(renderer);
    createVertexBuffer(renderer);
    createIndexBuffer(renderer);
    createGraphicsPipeline(renderer);
    createDrawSyncPrimitives(renderer);
}

static void mainLoop(Renderer *renderer) {
    while (!glfwWindowShouldClose(renderer->window)) {
        glfwPollEvents();
        drawFrame(renderer);
    }

    vkDeviceWaitIdle(renderer->device);
}

static void cleanup(Renderer *renderer) {
    VkDevice device = renderer->device;

    destroyDrawSyncPrimitives(renderer);
    freeCommandBuffers(renderer);
    vkDestroyCommandPool(device, renderer->commandPool, NULL);
    freeMeshBuffers(renderer);
    vkDestroyPipeline(device, renderer->graphicsPipeline, NULL);
    vkDestroyPipelineLayout(device, renderer->graphicsPipelineLayout, NULL);
    freeCameraBuffers(renderer);
    vkDestroyDescriptorPool(device, renderer->uniformBufferDescriptorPool, NULL);
    destroyImage(renderer, renderer->depthBuffer, true);
    cleanupSwapchain(renderer);
    vmaDestroyAllocator(renderer->allocator);
    vkDestroyDevice(device, NULL);
    vkDestroySurfaceKHR(renderer->instance, renderer->surface, NULL);
    vkDestroyInstance(renderer->instance, NULL);
    glfwDestroyWindow(renderer->window);
    glfwTerminate();
}

void run(Renderer *renderer, u32 windowWidth, u32 windowHeight) {
    initWindow(renderer, windowWidth, windowHeight);
    initVulkan(renderer);
    mainLoop(renderer);
    cleanup(renderer);
}

int main() {
    static Renderer renderer = {};

    renderer.permanent = arenaAlloc(MEGABYTE(64));
    renderer.scratch = arenaAlloc(MEGABYTE(16));

    renderer.cameraState.position = glm::vec3(0.0f, 0.0f, 3.0f);
    renderer.cameraState.moveSpeed = 1.0f;
    renderer.cameraState.mouseSensitivity = 0.0035f;

    makeSphere(renderer.permanent, &renderer.mesh, &renderer.meshCount,
               &renderer.indices, &renderer.indexCount, 0.125f, 16, 24);

    run(&renderer, 800, 600);

    arenaFree(renderer.scratch);
    arenaFree(renderer.permanent);

    return 0;
}
