#pragma once

#include <string_view>

vk::raii::ShaderModule readSpv(Renderer *renderer, const char *fileName);
