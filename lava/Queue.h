#ifndef __LAVA_QUEUE__
#define __LAVA_QUEUE__

#include "includes.hpp"
#include <vector>
#include <map>

#include "VulkanResource.h"

#include "noncopyable.hpp"

#include <lava/api.h>

namespace lava
{
  class Device;
  class Semaphore;
  class CommandBuffer;
  class Swapchain;
  class CommandBuffer;

  struct SubmitInfo
  {
    LAVA_API
    SubmitInfo( 
      vk::ArrayProxy<const std::shared_ptr<Semaphore>> const& waitSemaphores,
      vk::ArrayProxy<const vk::PipelineStageFlags> waitDstStageMasks,
      vk::ArrayProxy<const std::shared_ptr<CommandBuffer>> const& commandBuffers,
      vk::ArrayProxy<const std::shared_ptr<Semaphore>> signalSemaphores
    );
    LAVA_API
    SubmitInfo( SubmitInfo const& rhs );
    LAVA_API
    SubmitInfo & operator=( SubmitInfo const& rhs );

    std::vector< std::shared_ptr< Semaphore > > waitSemaphores;
    std::vector< vk::PipelineStageFlags > waitDstStageMasks;
    std::vector< std::shared_ptr< CommandBuffer > > commandBuffers;
    std::vector< std::shared_ptr< Semaphore > > signalSemaphores;
  };

  class Fence
    : public VulkanResource
    , private NonCopyable<Fence>
  {
  public:
    LAVA_API
    Fence( const DeviceRef& device, bool signaled );
    LAVA_API
    virtual ~Fence( void );
    LAVA_API
    bool isSignaled( void ) const;
    LAVA_API
    void reset( void );
    LAVA_API
    void wait( uint64_t timeout = UINT64_MAX ) const;
    LAVA_API
    inline operator vk::Fence( void ) const
    {
      return _fence;
    }
    LAVA_API
    static void waitForFences(
      vk::ArrayProxy<const std::shared_ptr< Fence > > fences,
      bool all, uint32_t timeout
    );
    LAVA_API
    static void resetFences(
      vk::ArrayProxy< const std::shared_ptr< Fence > > fences );

  private:
    vk::Fence _fence;
  };

  class Queue : public VulkanResource, private NonCopyable<Queue>
  {
  public:
    LAVA_API
    vk::Result submit( vk::ArrayProxy<const SubmitInfo> submitInfos,
      const std::shared_ptr<Fence>& fence = std::shared_ptr<Fence>( ) );

    LAVA_API
    void submit( const std::shared_ptr<CommandBuffer>& commandBuffer,
      const std::shared_ptr<Fence>& fence = std::shared_ptr<Fence>( ) );

    LAVA_API
    std::vector<vk::Result> present(
      vk::ArrayProxy<const std::shared_ptr<Semaphore>> waitSemaphores,
      vk::ArrayProxy<const std::shared_ptr<Swapchain>> swapchains,
      vk::ArrayProxy<const uint32_t> imageIndices );

    LAVA_API
    void waitIdle( void );

    inline operator vk::Queue( void ) const
    {
      return _queue;
    }

    LAVA_API
    inline uint32_t getQueueFamilyIndex( void ) const
    {
      return _queueFamilyIndex;
    }

    LAVA_API
    void submitAndWait( std::shared_ptr<CommandBuffer>& cmd );
  protected:
    friend class Device;
    Queue( const DeviceRef& device, vk::Queue queue, uint32_t queueIndex );

    std::map<std::shared_ptr<Fence>, std::vector<SubmitInfo>> _submitInfos;
    vk::Queue _queue;
    uint32_t _queueFamilyIndex;
  };
}

#endif /* __LAVA_QUEUE__ */