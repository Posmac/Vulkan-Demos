#ifndef QUEUEINFO
#define QUEUEINFO

#include <vector>
#include <optional>

struct QueuesInfo
{
    std::optional<uint32_t> graphicsFamily;
    std::optional<uint32_t> computeFamily;
    VkQueue graphicsQueue;
    VkQueue computeQueue;

    bool isComplete()
    {
        return graphicsFamily.has_value() && computeFamily.has_value();
    }
};

#endif