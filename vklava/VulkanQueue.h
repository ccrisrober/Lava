#ifndef __VKLAVA_VULKANQUEUE__
#define __VKLAVA_VULKANQUEUE__

#include <vulkan/vulkan.h>

namespace lava
{
  class VulkanDevice;

  // Types of GPU queues.
  enum GpuQueueType: short
  {
    /**
    * Queue used for rendering. Allows the use of draw commands,
    * but also all commands supported by compute or upload buffers.
    */
    GPUT_GRAPHICS,
    /** Discrete queue used for compute operations. Allows the use of dispatch and upload commands. */
    GPUT_COMPUTE,
    /** Queue used for memory transfer operations only. No rendering or compute dispatch allowed. */
    GPUT_TRANSFER,
    GPUT_COUNT
  };

  //#include "VulkanCommandBuffer.h"

  class VulkanQueue
  {
  public:
    VulkanQueue( VulkanDevice& device, VkQueue queue, GpuQueueType type,
      uint32_t index );
    ~VulkanQueue( void );

    VkQueue getQueue( void ) const
    {
      return _queue;
    }

    void waitIdle( void ) const;

  protected:
    VulkanDevice& _device;
    VkQueue _queue;
    GpuQueueType _type;
    uint32_t _index;
  };
}

#endif /* __VKLAVA_VULKANQUEUE__ */