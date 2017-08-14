#pragma once

#include <vulkan/vulkan.h>
#include <memory>

#include "VulkanSwapChain.h"

class VulkanRenderAPI;

class RenderWindow
{
public:
  RenderWindow( VulkanRenderAPI& renderAPI );
  ~RenderWindow( void );

//protected:
  VkColorSpaceKHR _colorSpace;
  VkFormat _colorFormat;
  //VkFormat _depthFormat;
  std::shared_ptr<VulkanSwapChain> _swapChain;
  VulkanRenderAPI& _renderAPI;
  VkSurfaceKHR _surface;
};

