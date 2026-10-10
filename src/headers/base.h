#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include "include.h"

#include "vertex.h"
#include "buffer.h"
#include "camera.h"
#include "image.h"

constexpr int MaxFramesInFlight = 3;
constexpr u32 MaxSwapchainImages = 8;

struct Renderer;

//main.cpp
void run(Renderer *renderer, u32 windowWidth, u32 windowHeight);

//base.cpp
void createInstance(Renderer *renderer);
void createSurface(Renderer *renderer);
void selectGPU(Renderer *renderer);
void createLogicalDevice(Renderer *renderer);
void createSwapchain(Renderer *renderer);
void createSwapchainImageViews(Renderer *renderer);
void cleanupSwapchain(Renderer *renderer);
void recreateSwapchain(Renderer *renderer);
void createDepthImage(Renderer *renderer);
void cleanupDepthImage(Renderer *renderer);

//pipeline.cpp
void createGraphicsPipeline(Renderer *renderer);

//command.cpp
void createCommandPool(Renderer *renderer);
void allocateCommandBuffer(Renderer *renderer);
void freeCommandBuffers(Renderer *renderer);

//buffer.cpp
void createDescriptorPools(Renderer *renderer);
void createUniformBufferDescriptorSets(Renderer *renderer);

//image.cpp
Image createImage(Renderer *renderer);
void destroyImage(Renderer *renderer, Image& image, bool hasImageView);

//camera.cpp
void createCameraBuffers(Renderer *renderer);
void getKeyInputsForMovement(Renderer *renderer, double delta_t, Vec2 delta_mouse);
void updateCameraBuffer(Renderer *renderer, u32 index, double delta_t, Vec2 delta_mouse);
void freeCameraBuffers(Renderer *renderer);

//vertex.cpp
void createVertexBuffer(Renderer *renderer);
void createIndexBuffer(Renderer *renderer);
void freeMeshBuffers(Renderer *renderer);

//draw.cpp
void createDrawSyncPrimitives(Renderer *renderer);
void drawFrame(Renderer *renderer);
void destroyDrawSyncPrimitives(Renderer *renderer);

struct Renderer {
    // permanent holds data that lives for the whole program (mesh data).
    // scratch is for temporary query results; save pos, use, arenaPopTo back.
    Arena *permanent;
    Arena *scratch;

    GLFWwindow *window;
    u32 width;
    u32 height;
    bool framebufferResized;

    VkInstance instance;

    VkSurfaceKHR surface;

    VkPhysicalDevice GPU;
    VkDevice device;
    VkQueue graphicsQueue;
    u32 graphicsQueueIndex;

    VmaAllocator allocator;

    VkSwapchainKHR swapchain;
    u32 swapchainImageCount;
    VkImage swapchainImages[MaxSwapchainImages];
    VkSurfaceFormatKHR swapchainSurfaceFormat;
    VkExtent2D swapchainExtent;
    VkImageView swapchainImageViews[MaxSwapchainImages];

    Image depthBuffer;

    VkDescriptorPool uniformBufferDescriptorPool;
    VkDescriptorSetLayout cameraBufferDescriptorSetLayouts[MaxFramesInFlight];
    VkDescriptorSet cameraBufferDescriptorSets[MaxFramesInFlight];

    VkPipelineLayout graphicsPipelineLayout;
    VkPipeline graphicsPipeline;

    CameraState cameraState;
    Buffer cameraBuffers[MaxFramesInFlight];

    Vertex *mesh;
    u32 meshCount;
    u16 *indices;
    u32 indexCount;
    Buffer meshVertices;
    Buffer meshIndices;
    Mat4 model;

    VkCommandPool commandPool;
    VkCommandBuffer commandBuffers[MaxFramesInFlight];

    VkSemaphore renderReadySemaphores[MaxFramesInFlight];
    VkSemaphore renderCompleteSemaphores[MaxSwapchainImages];
    VkFence drawFences[MaxFramesInFlight];
    u32 frameIndex;

    double timer;
    Vec2 previousCursorPos;
};
