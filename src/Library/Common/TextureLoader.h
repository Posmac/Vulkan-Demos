#pragma once
 
#include <iostream>
#include <memory>
#include <vector>

#include "../Core/Core.h"
#include "../../../external/stb_image.h"

namespace vk
{
    bool LoadTextureDataFromFile(char const* filename,
        int                          num_requested_components,
        std::vector<unsigned char>& image_data,
        int* image_width,
        int* image_height,
        int* image_num_components,
        int* image_data_size);
}