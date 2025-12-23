#ifndef INSTANCE_H
#define INSTANCE_H

#include "VmaUsage.h"

namespace CookEngine {

    class Instance
    {
        public:
            Instance();
            ~Instance();

            operator VkInstance();
        private:
        VkInstance m_instance;
    };
}

#endif