#pragma once

#define GLFW_INCLUDE_VULKAN
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS

#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

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

//vertex.cpp
void createVertexBuffer(Renderer *renderer);

//draw.cpp
void createSyncPrimitives(Renderer *renderer);
void drawFrame(Renderer *renderer);

class Renderer {
    public:
        GLFWwindow *window = nullptr;
        uint32_t width;
        uint32_t height;
        bool framebufferResized = false;

        vk::raii::Context context;
        vk::raii::Instance instance = nullptr;

        vk::raii::SurfaceKHR surface = nullptr;

        vk::raii::PhysicalDevice GPU = nullptr;
        vk::raii::Device device = nullptr;
        vk::raii::Queue graphicsQueue = nullptr;
        uint32_t graphicsQueueIndex;

        vk::raii::SwapchainKHR swapchain = nullptr;
        std::vector<vk::Image> swapchainImages;
        vk::SurfaceFormatKHR swapchainSurfaceFormat;
        vk::Extent2D swapchainExtent;

        std::vector<vk::raii::ImageView> swapchainImageViews;

        vk::raii::PipelineLayout graphicsPipelineLayout = nullptr;
        vk::raii::Pipeline graphicsPipeline = nullptr;

        std::vector<Vertex> mesh;
        Buffer triangleData;

        vk::raii::CommandPool commandPool = nullptr;
        std::vector<vk::raii::CommandBuffer> commandBuffers;

        std::vector<vk::raii::Semaphore> renderReadySemaphores;
        std::vector<vk::raii::Semaphore> renderCompleteSemaphores;
        std::vector<vk::raii::Fence> drawFences;
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
            createVertexBuffer(this);
            createCommandPool(this);
            allocateCommandBuffer(this);
            createSyncPrimitives(this);
        }

        void mainLoop() {
            while (!glfwWindowShouldClose(window)) {
                glfwPollEvents();
                drawFrame(this);
            }

            device.waitIdle();
        }

        void cleanup() {
            cleanupSwapchain(this);

            glfwDestroyWindow(window);

            glfwTerminate();
        }
};
