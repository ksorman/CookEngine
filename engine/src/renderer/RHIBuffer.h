#ifndef RHI_BUFFER_H
#define RHI_BUFFER_H

#include "VmaUsage.h"

namespace CookEngine {

    struct RHIBuffer{
        VkBuffer buffer;
        VmaAllocation bufferAllocation;
    };

}

#endif 