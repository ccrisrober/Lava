#include "VulkanCommandBuffer.h"
#include <assert.h>

namespace lava
{
  VulkanSemaphore::VulkanSemaphore( VulkanDevicePtr device )
    : VulkanResource( device )
  {
    VkSemaphoreCreateInfo semaphoreCI;
    semaphoreCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
    semaphoreCI.pNext = nullptr;
    semaphoreCI.flags = 0;

    VkResult result = vkCreateSemaphore( _device->getLogical( ),
      &semaphoreCI, nullptr, &_semaphore );
    assert( result == VK_SUCCESS );
  }

  VulkanSemaphore::~VulkanSemaphore( void )
  {
    vkDestroySemaphore( _device->getLogical( ), _semaphore, nullptr );
  }

  VulkanCmdBuffer::VulkanCmdBuffer( VulkanDevicePtr device/*, uint32_t id*/, 
    VkCommandPool pool/*, uint32_t queueFamily*/, bool secondary )
    : _device( device )
    , _state( State::Ready )
  {
    VkCommandBufferAllocateInfo allocInfo;
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.pNext = nullptr;
    allocInfo.commandPool = pool;
    allocInfo.level = secondary ?
      VK_COMMAND_BUFFER_LEVEL_SECONDARY :
      VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandBufferCount = 1;

    VkResult result = vkAllocateCommandBuffers( _device->getLogical( ), 
      &allocInfo, &_cmdBuffer );
    assert( result == VK_SUCCESS );
  }
  VulkanCmdBuffer::~VulkanCmdBuffer( void )
  {
    if ( _state == State::Submitted )
    {
      // Wait for finish
    }
    else if ( _state != State::Ready )
    {

    }
    VkDevice device = _device->getLogical( );
    vkFreeCommandBuffers( device, _pool, 1, &_cmdBuffer );
  }

  void VulkanCmdBuffer::begin( void )
  {
    assert( _state == State::Ready );

    VkCommandBufferBeginInfo beginInfo;
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
    beginInfo.pNext = nullptr;
    beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
    beginInfo.pInheritanceInfo = nullptr;

    VkResult result = vkBeginCommandBuffer( _cmdBuffer, &beginInfo );
    assert( result == VK_SUCCESS );

    _state = State::Recording;
  }

  void VulkanCmdBuffer::end( void )
  {
    assert( _state == State::Recording );

    VkResult result = vkEndCommandBuffer( _cmdBuffer );
    assert( result == VK_SUCCESS );

    _state = State::RecordingDone;
  }

  void VulkanCmdBuffer::beginRenderPass( VkRenderPass renderPass, std::vector< VkClearValue > clearValues,
    VkExtent2D swapChainExtent, VkFramebuffer swapChainFramebuffer )
  {
    assert( _state == State::Recording );

    VkRenderPassBeginInfo renderPassBeginInfo;
    renderPassBeginInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    renderPassBeginInfo.pNext = nullptr;
    renderPassBeginInfo.renderPass = renderPass;
    renderPassBeginInfo.framebuffer = swapChainFramebuffer;
    renderPassBeginInfo.renderArea.offset = { 0, 0 };
    renderPassBeginInfo.renderArea.extent = swapChainExtent;

    renderPassBeginInfo.clearValueCount = static_cast< uint32_t >( clearValues.size( ) );
    renderPassBeginInfo.pClearValues = clearValues.data( );

    vkCmdBeginRenderPass(_cmdBuffer, &renderPassBeginInfo, VK_SUBPASS_CONTENTS_INLINE);

    _state = State::RecordingRenderPass;
  }

  void VulkanCmdBuffer::endRenderPass( void )
  {
    assert( _state == State::RecordingRenderPass );

    vkCmdEndRenderPass(_cmdBuffer);

    _state = State::Recording;
  }

  void VulkanCmdBuffer::bindDynamicStates( bool forceAll )
  {
    if ( _viewportRequiresBind || forceAll )
    {
      VkViewport viewport;
      viewport.x = _viewport.x;
      viewport.y = _viewport.y;
      viewport.width = _viewport.width;
      viewport.height = _viewport.height;
      viewport.minDepth = 0.0f;
      viewport.maxDepth = 1.0f;

      vkCmdSetViewport( _cmdBuffer, 0, 1, &viewport );
      _viewportRequiresBind = false;
    }
    
    if( _stencilRefRequiresBind || forceAll )
    {
      vkCmdSetStencilReference( _cmdBuffer, 
        VK_STENCIL_FRONT_AND_BACK, _stencilRef );
      _stencilRefRequiresBind = false;
    }

    if( _scissorRequiresBind || forceAll )
    {
      VkRect2D scissorRect;
      if ( isScissorsEnabled ) 
      {
        scissorRect.offset.x = _scissor.x;
        scissorRect.offset.y = _scissor.y;
        scissorRect.extent.width = _scissor.width;
        scissorRect.extent.height = _scissor.height;
      }
      else
      {
        scissorRect.offset.x = 0;
        scissorRect.offset.y = 0;
        scissorRect.extent.width = _renderTargetWidth;
        scissorRect.extent.height = _renderTargetHeight;
      }
      vkCmdSetScissor( _cmdBuffer, 0, 1, &scissorRect ); 

      _scissorRequiresBind = false;
    }
  }
  /*void VulkanCmdBuffer::setVertexBuffers( uint32_t index, 
    std::vector<VulkanVertexBuffer*> buffers );
  {
    if ( buffers.empty( ) )
      return;

    uint32_t numBuffers = buffers.size( );

    for ( uint32_t i = 0; i < numBuffers; ++i )
    {
      VulkanVertexBuffer* vertexBuffer = buffers[ i ];

      if ( vertexBuffer != nullptr )
      {
        VulkanBuffer* resource = vertexBuffer->getResource( _device->getIndex( ) );
        if ( resource != nullptr )
        {
          mVertexBuffersTemp[ i ] = resource->getHandle( );
        }
        else
          mVertexBuffersTemp[ i ] = VK_NULL_HANDLE;
      }
      else
        mVertexBuffersTemp[ i ] = VK_NULL_HANDLE;
    }

    vkCmdBindVertexBuffers( _cmdBuffer, index, 
      numBuffers, mVertexBuffersTemp, mVertexBufferOffsetsTemp );
  }*/
  void VulkanCmdBuffer::setIndexBuffer( const VulkanIndexBuffer* indexBuffer )
  {
    VkBuffer buff = VK_NULL_HANDLE;
    VkIndexType indexType = VK_INDEX_TYPE_UINT32;

    if ( indexBuffer != nullptr )
    {
      VulkanBuffer* resource = indexBuffer->getResource( _device->getIndex( ) );
      if ( resource != nullptr )
      {
        buff = resource->getHandle( );
        indexType = indexBuffer->getType( );
      }
    }
    vkCmdBindIndexBuffer( _cmdBuffer, buff, 0, indexType );
  }
  void VulkanCmdBuffer::drawIndexed( uint32_t startIndex, 
    uint32_t indexCount, uint32_t vertexOffset, uint32_t instanceCount )
  {



    if ( instanceCount <= 0 )
    {
      instanceCount = 1;
    }

    vkCmdDrawIndexed( _cmdBuffer, indexCount, instanceCount, 
      startIndex, vertexOffset, 0 );
  }
  void VulkanCmdBuffer::setViewport( const Rect2& area )
  {
    //if ( _viewport == area )
    //  return;

    _viewport = area;
    _viewportRequiresBind = true;
  }

  void VulkanCmdBuffer::setScissorRect( const Rect2I& value )
  {
    //if ( _scissor == value )
    //  return;

    _scissor = value;
    _scissorRequiresBind = true;
  }

  void VulkanCmdBuffer::setStencilRef( uint32_t value )
  {
    //if ( _stencilRef == value )
    //  return;

    _stencilRef = value;
    _stencilRefRequiresBind = true;
  }
}