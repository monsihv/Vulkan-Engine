#include "headers/camera.h"
#include "headers/base.h"
#include "headers/include.h"

void createCameraBuffers(Renderer *renderer) {
    renderer->cameraBuffers.reserve(MaxFramesInFlight);
    auto usageFlags = vk::BufferUsageFlagBits::eUniformBuffer;
    auto allocationFlags = vmaHostAccessWriteNoRead | vmaKeepMapped;
    for (uint32_t i {}; i < MaxFramesInFlight; ++i) {
        Buffer buffer = createBuffer(renderer, NULL, sizeof(Camera), usageFlags, allocationFlags, NULL);
        renderer->cameraBuffers.push_back(buffer);
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
        writeDescriptorSet.dstSet = renderer->cameraBufferDescriptorSets[i];
        descriptorBufferInfo.buffer = renderer->cameraBuffers[i].buffer;

        descriptorBufferInfos.push_back(descriptorBufferInfo);
        writeDescriptorSet.pBufferInfo = &descriptorBufferInfos[i];

        descriptorWrites.push_back(writeDescriptorSet);
    }

    renderer->device.updateDescriptorSets(descriptorWrites, NULL);
}

void getKeyInputsForMovement(Renderer *renderer, double delta_t, glm::vec2 delta_mouse) {
    auto& cameraPosition = renderer->cameraState.position;

    auto mouseSensitivity = renderer->cameraState.mouseSensitivity;
    auto& yaw = renderer->cameraState.yaw;
    auto& pitch = renderer->cameraState.pitch;
    if (glfwGetKey(renderer->window, GLFW_KEY_R)) {
        yaw = 0;
        pitch = 0;
        cameraPosition = glm::vec3(0, 0, 3);
    }

    auto delta_mouse_true = delta_mouse * mouseSensitivity;
    yaw += delta_mouse_true.x;
    pitch += delta_mouse_true.y;

    printf("yaw: %f, pitch: %f\n", yaw, pitch);

    float l = glm::cos(pitch);
    float y = glm::sin(pitch);
    float x = l * glm::sin(yaw);
    float z = -1.0f * l * glm::cos(yaw);
    auto forward = glm::vec3(x, y, z);
    auto center = cameraPosition + forward;

    auto moveSpeed = renderer->cameraState.moveSpeed;
    auto delta_pos = (float)(delta_t * moveSpeed);

    auto normalizedDirection = glm::normalize(glm::vec2(forward.x, forward.z));
    auto x_change_along = normalizedDirection.x;
    auto z_change_along = normalizedDirection.y;

    auto up_vector = glm::vec3(0, 1, 0);
    auto perpendicular_change = glm::cross(up_vector, glm::vec3(normalizedDirection.x, 0, normalizedDirection.y));
    auto perpendicular_change_norm = glm::normalize(perpendicular_change);
    auto x_change_perp = perpendicular_change_norm.x;
    auto z_change_perp = perpendicular_change_norm.z;

    auto alongMove = glm::vec3(x_change_along, 0, z_change_along);
    auto perpMove = glm::vec3(x_change_perp, 0, z_change_perp);
    if (glfwGetKey(renderer->window, GLFW_KEY_W))
        cameraPosition += alongMove * delta_pos;
    if (glfwGetKey(renderer->window, GLFW_KEY_A))
        cameraPosition += perpMove * delta_pos;
    if (glfwGetKey(renderer->window, GLFW_KEY_S))
        cameraPosition += -alongMove * delta_pos;
    if (glfwGetKey(renderer->window, GLFW_KEY_D))
        cameraPosition += -perpMove * delta_pos;

    printf("pos %.2f %.2f %.2f  dt %.4f\n", cameraPosition.x, cameraPosition.y, cameraPosition.z, delta_t);

    const auto view = glm::lookAt(
        cameraPosition,
        center,
        glm::vec3(0, 1, 0));

    auto aspect = (float)(renderer->swapchainExtent.width) / (renderer->swapchainExtent.height);
    const auto proj = glm::perspective(
        glm::radians(45.0f),
        aspect,
        0.1f,
        100.0f);

    renderer->cameraState.camera.view = view;
    renderer->cameraState.camera.proj = proj;
}

void updateCameraBuffer(Renderer *renderer, uint32_t index, double delta_t, glm::vec2 delta_mouse) {
    getKeyInputsForMovement(renderer, delta_t, delta_mouse);
    auto& cameraBuffer = renderer->cameraBuffers[index];
    memcpy(cameraBuffer.map, &renderer->cameraState.camera, sizeof(Camera));
}

void freeCameraBuffers(Renderer *renderer) {
    for (auto& buffer : renderer->cameraBuffers) {
        destroyBuffer(renderer, buffer);
    }

    for (auto& descriptorSet : renderer->cameraBufferDescriptorSets) {
        renderer->device.free(renderer->uniformBufferDescriptorPool, descriptorSet);
    }

    for (auto& descriptorSetLayout : renderer->cameraBufferDescriptorSetLayouts) {
        renderer->device.destroy(descriptorSetLayout);
    }
}
