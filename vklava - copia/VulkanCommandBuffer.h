#ifndef __VKLAVA_VULKANCOMMANDBUFFER__
#define __VKLAVA_VULKANCOMMANDBUFFER__

#include "VulkanDevice.h"
#include <unordered_map>
#include "VulkanResource.h"
#include <vector>
#include <iostream>
#include <assert.h>

#include <unordered_map>

#include "VulkanBuffer.h"

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

  class Fence : public VulkanResource
  {
  public:
    Fence( VulkanDevicePtr device, uint32_t num_fences )
      : VulkanResource( device )
      , _fences( num_fences )
    {
      VkFenceCreateInfo info = { };

      info.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
      info.flags = 0;
      info.pNext = nullptr;

      for ( auto &fence : _fences )
      {
        vkCreateFence( _device->getLogical( ), &info, nullptr, &fence );
      }
    }

    void Fence::wait( uint64_t timeout )
    {
      vkWaitForFences( _device->getLogical( ), _fences.size( ), &_fences[ 0 ], VK_TRUE, timeout );
    }

    void Fence::reset( std::size_t index )
    {
      vkResetFences( _device->getLogical( ), 1, &_fences[ index ] );
    }

    VkFence *Fence::fence( std::size_t index )
    {
      return &_fences[ index ];
    }

    Fence::~Fence( )
    {
      for ( auto &fence : _fences )
      {
        vkDestroyFence( _device->getLogical( ), fence, nullptr );
      }
    }

  protected:
    std::vector<VkFence> _fences;
  };
  class VulkanCmdBufferPool
  {
  public:
    VulkanCmdBufferPool( VulkanDevice& device )
      : _device( device )
    {
      for ( uint32_t i = 0; i < GPUT_COUNT; ++i )
      {
        uint32_t familyIdx = _device.getQueueFamily( ( GpuQueueType ) i );
        if ( familyIdx == ( uint32_t ) -1 )
        {
          continue;
        }

        VkCommandPoolCreateInfo poolCI;
        poolCI.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
        poolCI.pNext = nullptr;
        poolCI.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
        poolCI.queueFamilyIndex = familyIdx;

        PoolInfo& poolInfo = _pools[ familyIdx ];
        poolInfo.queueFamily = familyIdx;
        //memset( poolInfo.buffers, 0, sizeof( poolInfo.buffers ) );

        if ( vkCreateCommandPool( _device.getLogical( ), &poolCI, nullptr,
          &poolInfo.pool ) != VK_SUCCESS )
        {
          throw std::runtime_error( "failed to create command pool!" );
        }
      }
    }
    ~VulkanCmdBufferPool( void )
    {
      for ( auto& entry : _pools )
      {
        PoolInfo& poolInfo = entry.second;
        // TODO: Destroy buffers
        vkDestroyCommandPool( _device.getLogical( ), poolInfo.pool, nullptr );
      }
    }
    void getBuffer( uint32_t queueFamily, bool secondary )
    {
      auto iterFind = _pools.find( queueFamily );
      if ( iterFind == _pools.end( ) )
      {
        //return nullptr;
      }
    }
  private:
    /** Command buffer pool and related information. */
    struct PoolInfo
    {
      VkCommandPool pool = VK_NULL_HANDLE;
      //VulkanCmdBuffer* buffers[ BS_MAX_VULKAN_CB_PER_QUEUE_FAMILY ];
      uint32_t queueFamily = -1;
    };
  protected:
    std::unordered_map<uint32_t, PoolInfo> _pools;
    VulkanDevice& _device;
  };

  struct Rect2
  {
    float x;
    float y;
    float width;
    float height;
  };
  struct Rect2I
  {
    uint32_t x;
    uint32_t y;
    uint32_t width;
    uint32_t height;
  };

  class VulkanCmdBuffer
  {
    enum class State
    {
      // Buffer is ready to be re-used.
      Ready,
      // Buffer is currently recording commands, but isn't recording a render pass.
      Recording,
      // Buffer is currently recording render pass commands.
      RecordingRenderPass,
      // Buffer is done recording but hasn't been submitted.
      RecordingDone,
      // Buffer is done recording and is currently submitted on a queue.
      Submitted
    };
  public:
    VulkanCmdBuffer( VulkanDevicePtr device/*, uint32_t id*/, 
      VkCommandPool pool/*, uint32_t queueFamily*/, bool secondary );
    ~VulkanCmdBuffer( void );
    void begin( void );
    void end( void );

    // Returns the handle to the internal Vulkan command buffer 
    //  wrapped by this object.
    VkCommandBuffer getHandle( void ) const { return _cmdBuffer; }

    void beginRenderPass( VkRenderPass renderPass, std::vector< VkClearValue > clearValues,
      VkExtent2D swapChainExtent, VkFramebuffer swapChainFramebuffer );

    void endRenderPass( void );

    void bindDynamicStates( bool forceAll = false );
    


    void setViewport( const Rect2& area );
    void setScissorRect( const Rect2I& value );
    void setStencilRef( uint32_t value );

    //void setVertexBuffers( uint32_t index, std::vector<VulkanVertexBuffer*> buffers );

    void setIndexBuffer( const VulkanIndexBuffer* indexBuffer );
    void drawIndexed( uint32_t startIndex, uint32_t indexCount,
      uint32_t vertexOffset, uint32_t instanceCount );
  protected:
    State _state;
    VulkanDevicePtr _device;
    VkCommandPool _pool;
    VkCommandBuffer _cmdBuffer;



    bool isScissorsEnabled = false;
    Rect2 _viewport;
    Rect2I _scissor;
    uint32_t _stencilRef;
    uint32_t _renderTargetWidth, _renderTargetHeight;

    bool _viewportRequiresBind = true;
    bool _stencilRefRequiresBind = true;
    bool _scissorRequiresBind = true;
  };
}

#endif /* __VKLAVA_VULKANCOMMANDBUFFER__ */