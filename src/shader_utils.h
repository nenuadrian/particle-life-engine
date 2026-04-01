#pragma once

#include <vulkan/vulkan.h>
#include <string>
#include <vector>

namespace ShaderUtils {
    std::vector<char> readFile(const std::string& filepath);
    VkShaderModule createShaderModule(VkDevice device, const std::vector<char>& code);
}
