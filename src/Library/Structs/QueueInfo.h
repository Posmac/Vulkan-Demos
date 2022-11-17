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

    bool IsComplete()
    {
        return graphicsFamily.has_value() && computeFamily.has_value();
    }

    bool IsQueuesIdentical()
    {
        return graphicsFamily.value() == computeFamily.value();
    }
};

struct QueueInfo
{
    uint32_t index;
    std::vector<float> priorities;
};

#endif