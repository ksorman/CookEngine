#ifndef MODEL_LOADER_H
#define MODEL_LOADER_H

#include "utils/Model.h"
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>

namespace CookEngine {

class ModelLoader
{
  public:
    std::shared_ptr<Model> LoadModel(std::string_view modelName);
    std::shared_ptr<Model> GetModel(std::string_view modelName);

  private:
    std::unordered_map<std::string, std::weak_ptr<Model>> m_modelsCache;
};
}// namespace CookEngine

#endif