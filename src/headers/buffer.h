#pragma once

#include "include.h"

struct Buffer {
    vk::raii::Buffer buffer = nullptr;
    vk::raii::DeviceMemory memory = nullptr;
    void *map = nullptr;

    ~Buffer() {
        if (map) memory.unmapMemory();
        map = nullptr;
    }
};

uint32_t findMemoryType(vk::PhysicalDevice gpu, uint32_t typeFilter, vk::MemoryPropertyFlags properties);
