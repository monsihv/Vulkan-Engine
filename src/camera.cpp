#include "headers/camera.h"

void createCameraBuffers(Renderer *renderer) {
    renderer->cameraBuffers.reserve(MaxFramesInFlight);
    auto usageFlags = vk::BufferUsageFlagBits::eUniformBuffer;
    auto memoryFlags = vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent;
    for (uint32_t i {}; i < MaxFramesInFlight; ++i) {
        Buffer buffer = createBuffer(renderer, NULL, sizeof(Camera), usageFlags, memoryFlags);
        renderer->cameraBuffers.push_back(std::move(buffer));
    }

    vk::DescriptorBufferInfo descriptorBufferInfo {
        .offset = 0,
        .range = vk::WholeSize
    };
    vk::WriteDescriptorSet writeDescriptorSet {
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = 1,
        .descriptorType = vk::DescriptorType::eUniformBuffer,
    };

    auto size = renderer->cameraBuffers.size();
    std::vector<vk::DescriptorBufferInfo> descriptorBufferInfos;
    descriptorBufferInfos.reserve(size);
    std::vector<vk::WriteDescriptorSet> descriptorWrites;
    descriptorWrites.reserve(size);
    for (uint32_t i{}; i < size; ++i) {
        writeDescriptorSet.dstSet = *renderer->cameraBufferDescriptorSets[i];
        descriptorBufferInfo.buffer = *renderer->cameraBuffers[i].buffer;

        descriptorBufferInfos.push_back(descriptorBufferInfo);
        writeDescriptorSet.pBufferInfo = &descriptorBufferInfos[i];

        descriptorWrites.push_back(writeDescriptorSet);
    }

    renderer->device.updateDescriptorSets(descriptorWrites, NULL);
}
