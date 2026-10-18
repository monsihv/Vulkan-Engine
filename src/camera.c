#include "headers/camera.h"
#include "headers/base.h"
#include "headers/include.h"

void createCameraBuffers(Renderer *renderer) {
    VkBufferUsageFlags usageFlags = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
    auto allocationFlags = vmaHostAccessWriteNoRead | vmaKeepMapped;
    for (u32 i = 0; i < MaxFramesInFlight; ++i) {
        renderer->cameraBuffers[i] = createBuffer(renderer, NULL, sizeof(Camera), usageFlags, allocationFlags, NULL);
    }

    VkDescriptorBufferInfo descriptorBufferInfos[MaxFramesInFlight];
    VkWriteDescriptorSet descriptorWrites[MaxFramesInFlight];
    for (u32 i = 0; i < MaxFramesInFlight; ++i) {
        descriptorBufferInfos[i] = (VkDescriptorBufferInfo){
            .buffer = renderer->cameraBuffers[i].buffer,
            .offset = 0,
            .range = VK_WHOLE_SIZE
        };

        descriptorWrites[i] = (VkWriteDescriptorSet){
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = renderer->cameraBufferDescriptorSets[i],
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pBufferInfo = &descriptorBufferInfos[i]
        };
    }

    vkUpdateDescriptorSets(renderer->device, MaxFramesInFlight, descriptorWrites, 0, NULL);
}

void getKeyInputsForMovement(Renderer *renderer, double delta_t, Vec2 delta_mouse) {
    auto cameraPosition = &renderer->cameraState.position;

    auto mouseSensitivity = renderer->cameraState.mouseSensitivity;
    auto yaw = &renderer->cameraState.yaw;
    auto pitch = &renderer->cameraState.pitch;
    if (glfwGetKey(renderer->window, GLFW_KEY_R)) {
        *yaw = 0;
        *pitch = 0;
        *cameraPosition = point3(0, 0, 3);
    }

    auto delta_mouse_true = delta_mouse * mouseSensitivity;
    *yaw += delta_mouse_true.x;
    *pitch += delta_mouse_true.y;

    //printf("yaw: %f, pitch: %f\n", *yaw, *pitch);

    float l = cosf(*pitch);
    float y = sinf(*pitch);
    float x = l * sinf(*yaw);
    float z = -1.0f * l * cosf(*yaw);
    Vec4 forward = dir3(x, y, z);
    Vec4 center = *cameraPosition + forward;

    auto moveSpeed = renderer->cameraState.moveSpeed;
    auto delta_pos = (float)(delta_t * moveSpeed);

    Vec2 normalizedDirection = normalize2(forward.xz);
    auto x_change_along = normalizedDirection.x;
    auto z_change_along = normalizedDirection.y;

    Vec4 up_vector = dir3(0, 1, 0);
    Vec4 perpendicular_change = cross3(up_vector, dir3(normalizedDirection.x, 0, normalizedDirection.y));
    Vec4 perpendicular_change_norm = normalize3(perpendicular_change);
    auto x_change_perp = perpendicular_change_norm.x;
    auto z_change_perp = perpendicular_change_norm.z;

    Vec4 alongMove = dir3(x_change_along, 0, z_change_along);
    Vec4 perpMove = dir3(x_change_perp, 0, z_change_perp);
    if (glfwGetKey(renderer->window, GLFW_KEY_W))
        *cameraPosition += alongMove * delta_pos;
    if (glfwGetKey(renderer->window, GLFW_KEY_A))
        *cameraPosition += perpMove * delta_pos;
    if (glfwGetKey(renderer->window, GLFW_KEY_S))
        *cameraPosition += -alongMove * delta_pos;
    if (glfwGetKey(renderer->window, GLFW_KEY_D))
        *cameraPosition += -perpMove * delta_pos;

    //printf("pos %.2f %.2f %.2f  dt %.4f\n", cameraPosition->x, cameraPosition->y, cameraPosition->z, delta_t);

    Mat4 view = lookAt(
        *cameraPosition,
        center,
        dir3(0, 1, 0));

    auto aspect = (float)(renderer->swapchainExtent.width) / (renderer->swapchainExtent.height);
    Mat4 proj = perspective(
        radians(45.0f),
        aspect,
        0.1f,
        100.0f);

    Vec4 translation = dir3((float)sin(renderer->timer) * l, 0, (float)-cos(renderer->timer) * l);
    renderer->model = translate(translation);

    renderer->cameraState.camera.view = view;
    renderer->cameraState.camera.proj = proj;
}

void updateCameraBuffer(Renderer *renderer, u32 index, double delta_t, Vec2 delta_mouse) {
    getKeyInputsForMovement(renderer, delta_t, delta_mouse);
    auto cameraBuffer = &renderer->cameraBuffers[index];
    memcpy(cameraBuffer->map, &renderer->cameraState.camera, sizeof(Camera));
}

void freeCameraBuffers(Renderer *renderer) {
    for (u32 i = 0; i < MaxFramesInFlight; ++i) {
        destroyBuffer(renderer, &renderer->cameraBuffers[i]);
    }

    VK_CHECK(vkFreeDescriptorSets(renderer->device, renderer->uniformBufferDescriptorPool,
                                  MaxFramesInFlight, renderer->cameraBufferDescriptorSets));

    for (u32 i = 0; i < MaxFramesInFlight; ++i) {
        vkDestroyDescriptorSetLayout(renderer->device, renderer->cameraBufferDescriptorSetLayouts[i], NULL);
    }
}
