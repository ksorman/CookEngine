# Unexpected Behavior of vkGetPhysicalDeviceSurfaceSupportKHR with RenderDoc

## Issue Description
When using **RenderDoc**, the Vulkan function `vkGetPhysicalDeviceSurfaceSupportKHR` may incorrectly report that a queue family supports presentation (i.e., displaying images on a surface). However, in reality, the queue family does not support this functionality. This behavior is specific to RenderDoc and does not occur when the application is run normally without RenderDoc.