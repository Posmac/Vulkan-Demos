#ifndef QUEUEINFO
#define QUEUEINFO

#include <vector>
#include <optional>

struct QueueInfo
{
    std::optional<uint32_t> graphicsFamily;

    bool isComplete()
    {
        return graphicsFamily.has_value();
    }
};

#endif