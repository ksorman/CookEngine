#ifndef COOK_ENGINE_H
#define COOK_ENGINE_H

#include <memory>
#include "renderer/Renderer.h"
namespace CookEngine {
    class EngineApi{
        public:
            void Init();
            void Deinit();
        private:
            std::unique_ptr<Renderer> m_renderer;
    };
}
#endif //COOK_ENGINE_H
