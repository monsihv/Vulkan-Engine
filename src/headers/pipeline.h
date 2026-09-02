#pragma once

#include <string_view>

vk::raii::ShaderModule readSpv(const char *fileName, Renderer *renderer);
