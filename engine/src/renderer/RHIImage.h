#ifndef RHI_IMAGE_H
#define RHI_IMAGE_H

#include "VmaUsage.h"

namespace CookEngine {

    struct RHIImage{
        VkImage image;
        VmaAllocation imageAllocation;
    };

}

#endif 