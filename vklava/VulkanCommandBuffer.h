#ifndef __VKLAVA_VULKANCOMMANDBUFFER__
#define __VKLAVA_VULKANCOMMANDBUFFER__

#include "VulkanDevice.h"
#include <unordered_map>
#include "VulkanResource.h"
#include <vector>
#include <iostream>
#include <assert.h>

namespace lava
{
  class VulkanSemaphore : public VulkanResource
  {
  public:
    VulkanSemaphore( VulkanDevicePtr device );
    ~VulkanSemaphore( void );

    // Returns the internal handle to the Vulkan object
    VkSemaphore getHandle( ) const
    {
      return _semaphore;
    }

  private:
    VkSemaphore _semaphore;
  };
  class VulkanCmdBuffer;
  class VulkanCmdBufferPool
  {
  public:
    VulkanCmdBufferPool( VulkanDevicePtr device );
    ~VulkanCmdBufferPool( );


    /**
    * Attempts to find a free command buffer, or creates a new one if not found.
    * Caller must guarantee the provided queue family is valid.
    */
    //VulkanCmdBuffer* getBuffer( uint32_t queueFamily, bool secondary );

  protected:
    //VulkanDevicePtr _device;

    // Command buffer pool and related information.
    struct PoolInfo
    {
      VkCommandPool pool = VK_NULL_HANDLE;
      std::vector<VulkanCmdBuffer*> buffers;  // [ BS_MAX_VULKAN_CB_PER_QUEUE_FAMILY ];
      uint32_t queueFamily = -1;
    };

    // Creates a new command buffer.
    VulkanCmdBuffer* createBuffer( uint32_t queueFamily, bool secondary );

    VulkanDevicePtr _device;
    std::unordered_map<uint32_t, PoolInfo> _pools;
    uint32_t _nextId;
  };

  class VulkanCmdBuffer
  {
    /** Possible states a command buffer can be in. */
    enum class State
    {
      /** Buffer is ready to be re-used. */
      Ready,
      /** Buffer is currently recording commands, but isn't recording a render pass. */
      Recording,
      /** Buffer is currently recording render pass commands. */
      RecordingRenderPass,
      /** Buffer is done recording but hasn't been submitted. */
      RecordingDone,
      /** Buffer is done recording and is currently submitted on a queue. */
      Submitted
    };
  public:
    VulkanCmdBuffer( VulkanDevicePtr device, uint32_t id, VkCommandPool pool,
      uint32_t queueFamily )
    {
      _queueFamily = queueFamily;
      _device = device;
      _state = State::Ready;

      VkCommandBufferAllocateInfo cmdBufferAllocInfo;
      cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
      cmdBufferAllocInfo.pNext = nullptr;
      cmdBufferAllocInfo.commandPool = pool;
      cmdBufferAllocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
      cmdBufferAllocInfo.commandBufferCount = 1;

      VkResult result = vkAllocateCommandBuffers( _device->getLogical( ), 
        &cmdBufferAllocInfo, &_cmdBuffer );
      assert( result == VK_SUCCESS );

      VkFenceCreateInfo fenceCI;
      fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
      fenceCI.pNext = nullptr;
      fenceCI.flags = 0;

      result = vkCreateFence( _device->getLogical( ), &fenceCI, nullptr, &_fence );
      assert( result == VK_SUCCESS );
    }

    void reset( void )
    {
      bool wasSubmitted = _state == State::Submitted;

      _state = State::Ready;
      vkResetCommandBuffer( _cmdBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT );

      // ...
    }
    
    ~VulkanCmdBuffer( void )
    {
      VkDevice device = _device->getLogical( );

      if ( _state == State::Submitted )
      {
        // Wait 1s
        uint64_t waitTime = 1000 * 1000 * 1000;
        VkResult result = vkWaitForFences( device, 1, &_fence, true, waitTime );
        assert( result == VK_SUCCESS || result == VK_TIMEOUT );

        if ( result == VK_TIMEOUT )
          std::cerr << ( "Freeing a command buffer before done executing because fence wait expired!" ) << std::endl;

        // Resources have been marked as used, make sure to notify them we're done with them
        reset( );
      }
      else if ( _state != State::Ready )
      {
        /*// Notify any resources that they are no longer bound
        for ( auto& entry : mResources )
        {
          ResourceUseHandle& useHandle = entry.second;
          assert( !useHandle.used );

          entry.first->notifyUnbound( );
        }

        for ( auto& entry : mImages )
        {
          uint32_t imageInfoIdx = entry.second;
          ImageInfo& imageInfo = mImageInfos[ imageInfoIdx ];

          ResourceUseHandle& useHandle = imageInfo.useHandle;
          assert( !useHandle.used );

          entry.first->notifyUnbound( );
        }

        for ( auto& entry : mBuffers )
        {
          ResourceUseHandle& useHandle = entry.second.useHandle;
          assert( !useHandle.used );

          entry.first->notifyUnbound( );
        }*/
      }

      /*if ( mIntraQueueSemaphore != nullptr )
        mIntraQueueSemaphore->destroy( );

      for ( uint32_t i = 0; i < BS_MAX_VULKAN_CB_DEPENDENCIES; ++i )
      {
        if ( mInterQueueSemaphores[ i ] != nullptr )
          mInterQueueSemaphores[ i ]->destroy( );
      }*/

      vkDestroyFence( device, _fence, nullptr );
      vkFreeCommandBuffers( device, _pool, 1, &_cmdBuffer );

      //bs_free( mDescriptorSetsTemp );
    }

    uint32_t getQueueFamily( void ) const
    {
      return _queueFamily;
    }
    uint32_t getDeviceIdx( void ) const
    {
      return _device->getIndex( );
    }
    void begin( void )
    {
      assert( _state == State::Ready );
      VkCommandBufferBeginInfo beginInfo;
      beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
      beginInfo.pNext = nullptr;
      beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
      beginInfo.pInheritanceInfo = nullptr;

      VkResult result = vkBeginCommandBuffer( _cmdBuffer, &beginInfo );
      assert( result == VK_SUCCESS );
    }
    void end( void )
    {
      assert( _state == State::Recording );

      // If a clear is queued, execute the render pass with no additional instructions
      //if ( mClearMask )
      //  executeClearPass( );

      VkResult result = vkEndCommandBuffer( _cmdBuffer );
      assert( result == VK_SUCCESS );

      _state = State::RecordingDone;
    }
    void beginRenderPass( void );
    void endRenderPass( void );
  protected:
    VulkanDevicePtr _device;
    VkCommandPool _pool;
    State _state;
    VkCommandBuffer _cmdBuffer;
    uint32_t _queueFamily;
    VkFence _fence;
  };
}

#endif /* __VKLAVA_VULKANCOMMANDBUFFER__ */