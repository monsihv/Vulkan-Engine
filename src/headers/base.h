#pragma once

#define GLFW_INCLUDE_VULKAN

#include <GLFW/glfw3.h>

#include "include.h"

#include <vector>

#include "vertex.h"
#include "buffer.h"

constexpr int MaxFramesInFlight = 3;

class Renderer;

//base.cpp
void createInstance(Renderer *renderer);
void createSurface(Renderer *renderer);
void selectGPU(Renderer *renderer);
void createLogicalDevice(Renderer *renderer);
void createSwapchain(Renderer *renderer);
void createSwapchainImageViews(Renderer *renderer);
void cleanupSwapchain(Renderer *renderer);
void recreateSwapchain(Renderer *renderer);

//pipeline.cpp
void createGraphicsPipeline(Renderer *renderer);

//command.cpp
void createCommandPool(Renderer *renderer);
void allocateCommandBuffer(Renderer *renderer);
void freeCommandBuffers(Renderer *renderer);

//buffer.cpp
void createDescriptorPools(Renderer *renderer);
void createUniformBufferDescriptorSets(Renderer *renderer);

//camera.cpp
void createCameraBuffers(Renderer *renderer);
void freeCameraBuffers(Renderer *renderer);

//vertex.cpp
void createVertexBuffer(Renderer *renderer);
void createIndexBuffer(Renderer *renderer);
void freeMeshBuffers(Renderer *renderer);

//draw.cpp
void createDrawSyncPrimitives(Renderer *renderer);
void drawFrame(Renderer *renderer);
void destroyDrawSyncPrimitives(Renderer *renderer);

class Renderer {
    public:
        GLFWwindow *window = nullptr;
        uint32_t width;
        uint32_t height;
        bool framebufferResized = false;

        vk::Instance instance;

        vk::SurfaceKHR surface;

        vk::PhysicalDevice GPU;
        vk::Device device;
        vk::Queue graphicsQueue;
        uint32_t graphicsQueueIndex;

        VmaAllocator allocator;

        vk::SwapchainKHR swapchain;
        std::vector<vk::Image> swapchainImages;
        vk::SurfaceFormatKHR swapchainSurfaceFormat;
        vk::Extent2D swapchainExtent;

        std::vector<vk::ImageView> swapchainImageViews;

        vk::DescriptorPool uniformBufferDescriptorPool = nullptr;
        std::vector<vk::DescriptorSetLayout> cameraBufferDescriptorSetLayouts;
        std::vector<vk::DescriptorSet> cameraBufferDescriptorSets;

        vk::PipelineLayout graphicsPipelineLayout;
        vk::Pipeline graphicsPipeline;

        std::vector<Buffer> cameraBuffers;

        std::vector<Vertex> mesh;
        std::vector<uint16_t> indices;
        Buffer meshVertices;
        Buffer meshIndices;

        vk::CommandPool commandPool;
        std::vector<vk::CommandBuffer> commandBuffers;

        std::vector<vk::Semaphore> renderReadySemaphores;
        std::vector<vk::Semaphore> renderCompleteSemaphores;
        std::vector<vk::Fence> drawFences;
        uint32_t frameIndex {};

    public:
        void run(uint32_t windowWidth, uint32_t windowHeight) {
            initWindow(windowWidth, windowHeight);
            initVulkan();
            mainLoop();
            cleanup();
        }

    private:
        void initWindow(uint32_t windowWidth, uint32_t windowHeight) {
            glfwInit();
            glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
            glfwWindowHint(GLFW_RESIZABLE, GLFW_TRUE);

            window = glfwCreateWindow(windowWidth, windowHeight, "ada wong", nullptr, nullptr);

            int fbWidth, fbHeight;
            glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

            width = (uint32_t)fbWidth;
            height = (uint32_t)fbHeight;

            glfwSetWindowUserPointer(window, this);
            glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
        }

        static void framebufferResizeCallback(GLFWwindow *window, int width, int height) {
            auto app = reinterpret_cast<Renderer*>(glfwGetWindowUserPointer(window));
            app->framebufferResized = true;
        }

        void initVulkan() {
            createInstance(this);
            createSurface(this);
            selectGPU(this);
            createLogicalDevice(this);
            createSwapchain(this);
            createSwapchainImageViews(this);
            createGraphicsPipeline(this);
            createCommandPool(this);
            createDescriptorPools(this);
            createUniformBufferDescriptorSets(this);
            createCameraBuffers(this);
            createVertexBuffer(this);
            createIndexBuffer(this);
            allocateCommandBuffer(this);
            createDrawSyncPrimitives(this);
        }

        void mainLoop() {
            while (!glfwWindowShouldClose(window)) {
                glfwPollEvents();
                drawFrame(this);
            }

            device.waitIdle();
        }

        void cleanup() {
            destroyDrawSyncPrimitives(this);
            freeCommandBuffers(this);
            device.destroy(commandPool);
            freeMeshBuffers(this);
            device.destroy(graphicsPipeline);
            device.destroy(graphicsPipelineLayout);
            freeCameraBuffers(this);
            device.destroy(uniformBufferDescriptorPool);
            cleanupSwapchain(this);
            vmaDestroyAllocator(allocator);
            device.destroy();
            instance.destroy(surface);
            instance.destroy();
            glfwDestroyWindow(window);
            glfwTerminate();
        }
};
