#ifndef PIPELINE_H
#define PIPELINE_H

#include "VmaUsage.h"

namespace CookEngine {
class Pipeline
{
  public:
    ~Pipeline();
    bool CreatePipeline(VkDevice& device, const VkGraphicsPipelineCreateInfo* createInfo);
    void DestroyPipeline();
    VkPipeline& GetPipeline();

  private:
    VkPipeline m_pipeline;
    VkDevice* m_device;

    bool m_isInitialized = false;
};
}// namespace CookEngine

#endif
