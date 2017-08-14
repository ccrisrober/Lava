#pragma once

#include "VulkanDevice.h"

class VulkanResource
{
public:
  VulkanResource( VulkanDevicePtr device );
  ~VulkanResource( void );
protected:
  VulkanDevicePtr _device;
};

