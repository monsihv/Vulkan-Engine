#pragma once

#define GLFW_INCLUDE_VULKAN
#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS

#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>
#include <vulkan/vulkan.hpp>
#include <vulkan/vulkan_raii.hpp>

class Renderer;

void createInstance(Renderer *renderer);
void createSurface(Renderer *renderer);
void selectGPU(Renderer *renderer);
void createLogicalDevice(Renderer *renderer);

class Renderer {
    public:
        GLFWwindow *window = nullptr;
        uint32_t width {};
        uint32_t height {};

        vk::raii::Context context;
        vk::raii::Instance instance = nullptr;

        vk::raii::SurfaceKHR surface = nullptr;

        vk::raii::PhysicalDevice GPU = nullptr;
        vk::raii::Device device = nullptr;
        vk::raii::Queue graphicsQueue = nullptr;

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
            glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

            window = glfwCreateWindow(windowWidth, windowHeight, "ada wong", nullptr, nullptr);

            int fbWidth, fbHeight;
            glfwGetFramebufferSize(window, &fbWidth, &fbHeight);

            width = (uint32_t)fbWidth;
            height = (uint32_t)fbHeight;
        }

        void initVulkan() {
            createInstance(this);
            createSurface(this);
            selectGPU(this);
            createLogicalDevice(this);
        }

        void mainLoop() {
            while (!glfwWindowShouldClose(window)) {
                glfwPollEvents();
            }
        }

        void cleanup() {
            glfwDestroyWindow(window);

            glfwTerminate();
        }
};
