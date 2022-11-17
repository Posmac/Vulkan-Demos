#pragma once

#include <fstream>
#include <iostream>
#include <cmath>
#include <vector>

namespace vk
{
    bool GetBinaryFileContents(std::string const& filename,
        std::vector<unsigned char>& contents);
}