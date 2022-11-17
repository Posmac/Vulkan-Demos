#include "Tools.h"

namespace vk
{
    bool GetBinaryFileContents(std::string const& filename,
        std::vector<unsigned char>& contents)
    {
        contents.clear();

        std::ifstream file(filename, std::ios::binary);
        if (file.fail()) {
            std::cout << "Could not open '" << filename << "' file." << std::endl;
            return false;
        }

        std::streampos begin;
        std::streampos end;
        begin = file.tellg();
        file.seekg(0, std::ios::end);
        end = file.tellg();

        if ((end - begin) == 0) {
            std::cout << "The '" << filename << "' file is empty." << std::endl;
            return false;
        }
        contents.resize(static_cast<size_t>(end - begin));
        file.seekg(0, std::ios::beg);
        file.read(reinterpret_cast<char*>(contents.data()), end - begin);
        file.close();

        return true;
    }
}
