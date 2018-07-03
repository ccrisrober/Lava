/**
 * Copyright (c) 2017 - 2018, Lava
 * All rights reserved.
 * 
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 * 
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 **/

#ifndef __LAVA_COMMANDBUFFER__
#define __LAVA_COMMANDBUFFER__

#include <lava/Device.h>
#include <lava/VulkanResource.h>
#include <memory>

namespace lava
{
  class Buffer;
  class IndexBuffer;
  class Image;
  class QueryPool;
  class RenderPass;
  class Framebuffer;
}

namespace lava
{
  struct ImageMemoryBarrier
  {
    LAVA_API
    ImageMemoryBarrier(
      vk::AccessFlags srcAccessMask, vk::AccessFlags dstAccessMask,
      vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
      uint32_t srcQueueFamilyIndex, uint32_t dstQueueFamilyIndex,
      const std::shared_ptr<Image>& image,
      const vk::ImageSubresourceRange& subresourceRange );
    LAVA_API
    ImageMemoryBarrier( const ImageMemoryBarrier& rhs );
    LAVA_API
    ImageMemoryBarrier & operator=( const ImageMemoryBarrier& rhs );

    vk::AccessFlags           srcAccessMask;
    vk::AccessFlags           dstAccessMask;
    vk::ImageLayout           oldLayout;
    vk::ImageLayout           newLayout;
    uint32_t                  srcQueueFamilyIndex;
    uint32_t                  dstQueueFamilyIndex;
    std::shared_ptr<Image>    image;
    vk::ImageSubresourceRange subresourceRange;
  };
  class CommandBuffer;
  
  class CommandPool :
    public VulkanResource,
    public std::enable_shared_from_this<CommandPool>
  {
  public:
    LAVA_API
    CommandPool( const std::shared_ptr<Device>& device, 
      vk::CommandPoolCreateFlags flags = { }, uint32_t familyIndex = 0 );
    LAVA_API
    virtual ~CommandPool( void );
    LAVA_API
    void reset( vk::CommandPoolResetFlags flags );
#if !defined(NDEBUG)
    LAVA_API
    uint32_t getFamilyIndex( void ) const;
    LAVA_API
    bool individuallyResetCommandBuffers( void ) const;
    LAVA_API
    bool shortLivedCommandBuffers( void ) const;
    LAVA_API
    bool supportsCompute( void ) const;
    LAVA_API
    bool supportsGraphics( void ) const;
    LAVA_API
    bool supportsTransfer( void ) const;
#endif
    inline operator vk::CommandPool( void ) const
    {
      return _commandPool;
    }
    const std::shared_ptr<Device>& getDevice( void )
    {
      return _device;
    }
    LAVA_API
    std::shared_ptr<CommandBuffer> allocateCommandBuffer(
      vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary );
  protected:
    vk::CommandPool _commandPool;
#if !defined(NDEBUG)
    std::vector<lava::CommandBuffer*> _commandBuffers;
    vk::CommandPoolCreateFlags _createFlags;
    uint32_t _familyIndex;
#endif
  };

  class CommandBuffer:
    public std::enable_shared_from_this<CommandBuffer>
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
    LAVA_API
    CommandBuffer( const std::shared_ptr<CommandPool>& cmdPool,
      vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary );
    LAVA_API
    virtual ~CommandBuffer( void );

    CommandBuffer( const CommandBuffer& ) = delete;
    CommandBuffer( CommandBuffer&& ) = delete;

    CommandBuffer& operator=( const CommandBuffer& ) = delete;
    CommandBuffer& operator=( CommandBuffer&& ) = delete;

    inline operator vk::CommandBuffer( void ) const
    {
      return _commandBuffer;
    }

#if !defined(NDEBUG)
    LAVA_API
    std::shared_ptr< lava::CommandBuffer > getPrimaryCommandBuffer( void ) const
    {
      return _primaryCommandBuffer.lock( );
    }
    LAVA_API
    bool isOneTimeSubmit( void ) const
    {
      return !!( _flags & vk::CommandBufferUsageFlagBits::eOneTimeSubmit );
    }
    LAVA_API
    bool isRecording( void ) const
    {
      return _isRecording;
    }
    LAVA_API
    bool isSimultaneousUsageAllowed( void ) const
    {
      return !!( _flags & vk::CommandBufferUsageFlagBits::eSimultaneousUse );
    }
#endif

    LAVA_API
    void begin( vk::CommandBufferUsageFlags flags = { },
      const std::shared_ptr<RenderPass>& renderPass = { },
      uint32_t subpass = 0,
      const std::shared_ptr<Framebuffer>& framebuffer = { },
      vk::Bool32 occlusionQueryEnable = false,
      vk::QueryControlFlags queryFlags = { },
      vk::QueryPipelineStatisticFlags pipelineStatistics = { } );
    LAVA_API
    void end( void );

    #pragma region EventCommands
    LAVA_API
    void resetEvent( const std::shared_ptr<Event>& ev,
      vk::PipelineStageFlags stageMask );
    LAVA_API
    void setEvent( const std::shared_ptr<Event>& ev,
      vk::PipelineStageFlags stageMask );
    LAVA_API
    void waitEvents( vk::ArrayProxy<const std::shared_ptr<Event>> events,
      vk::PipelineStageFlags srcStageMask, vk::PipelineStageFlags dstStageMask,
      vk::ArrayProxy<const vk::MemoryBarrier> memoryBarriers,
      vk::ArrayProxy<const vk::BufferMemoryBarrier> bufferMemoryBarriers,
      vk::ArrayProxy<const vk::ImageMemoryBarrier> imageMemoryBarriers
    );
    #pragma endregion

    LAVA_API
    void beginRenderPass( const vk::RenderPassBeginInfo& beginInfo, 
      vk::SubpassContents contents );
    LAVA_API
    void beginRenderPass( const std::shared_ptr<RenderPass>& renderPass,
      const std::shared_ptr<Framebuffer>& framebuffer, const vk::Rect2D& area,
      vk::ArrayProxy<const vk::ClearValue> clearValues,
      vk::SubpassContents contents );

    LAVA_API
    void nextSubpass( vk::SubpassContents contents );

    LAVA_API
    void endRenderPass( void );

    LAVA_API
    void executeCommands( const std::vector<
      std::shared_ptr<lava::CommandBuffer> >& secondaryCmds );

    #pragma region ClearCommands
    LAVA_API
    void clearAttachments(
      vk::ArrayProxy< const vk::ClearAttachment> attachments,
      vk::ArrayProxy<const vk::ClearRect> rects );
    LAVA_API
    void clearColorImage( const std::shared_ptr<Image>& image,
      vk::ImageLayout imageLayout, const vk::ClearColorValue& color,
      vk::ArrayProxy<const vk::ImageSubresourceRange> ranges = {
        { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
    } );
    LAVA_API
    void clearDepthStencilImage( const std::shared_ptr<Image>& image,
      vk::ImageLayout imageLayout, float depth, uint32_t stencil,
      vk::ArrayProxy<const vk::ImageSubresourceRange> ranges );
    #pragma endregion

    #pragma region QueryCommands
    LAVA_API
    void beginQuery( const std::shared_ptr<lava::QueryPool>& queryPool,
      uint32_t slot, vk::QueryControlFlags flags );
    LAVA_API
    void copyQueryPoolResults( const std::shared_ptr<lava::QueryPool>& queryPool,
      uint32_t startQuery, uint32_t queryCount,
      const std::shared_ptr<lava::Buffer>& dstBuffer, vk::DeviceSize dstOffset,
      vk::DeviceSize dstStride, vk::QueryResultFlags flags );
    LAVA_API
    void endQuery( const std::shared_ptr<lava::QueryPool>& queryPool,
      uint32_t slot );
    LAVA_API
    void resetQueryPool( const std::shared_ptr<lava::QueryPool>& queryPool,
      uint32_t startQuery, uint32_t queryCount );
    LAVA_API
    void writeTimestamp( vk::PipelineStageFlagBits pipelineStage,
      const std::shared_ptr<lava::QueryPool>& queryPool, uint32_t entry );  
    #pragma endregion

    #pragma region SetterCommands
    LAVA_API
    void setViewportScissors( uint32_t width, uint32_t height );
    LAVA_API
    void setViewportScissors( const vk::Extent2D& dimensions );
    LAVA_API
    void setScissor( uint32_t first,
      vk::ArrayProxy<const vk::Rect2D> scissors );
    LAVA_API
    void setViewport( uint32_t first,
      vk::ArrayProxy<const vk::Viewport> viewports );
    LAVA_API
    void setDepthBias( float depthBias, float depthBiasClamp,
      float slopeScaledDepthBias );
    LAVA_API
    void setDepthBounds( float minDepthBounds, float maxDepthBounds );

    LAVA_API
    void setLineWidth( float lineWidth );
    LAVA_API
    void setBlendConstants( const float blendConst[ 4 ] );
    LAVA_API
    void setStencilCompareMask( vk::StencilFaceFlags faceMask,
      uint32_t stencilCompareMask );
    LAVA_API
    void setStencilReference( vk::StencilFaceFlags faceMask,
      uint32_t stencilReference );
    LAVA_API
    void setStencilWriteMask( vk::StencilFaceFlags faceMask,
      uint32_t stecilWriteMask );
    #pragma endregion

    #pragma region DrawDispatchCommands
    LAVA_API
    void dispatch( uint32_t x, uint32_t y, uint32_t z );
    LAVA_API
    void draw( uint32_t vertexCount, uint32_t instanceCount,
      uint32_t firstVertex, uint32_t firstInstance );
    LAVA_API
    void drawIndirect( const std::shared_ptr<Buffer>& buffer,
      vk::DeviceSize offset, uint32_t count, uint32_t stride );
    LAVA_API
    void drawIndexed( uint32_t indexCount, uint32_t instanceCount,
      uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance );
    LAVA_API
    void drawIndexedIndirect( const std::shared_ptr<Buffer>& buffer,
      vk::DeviceSize offset, uint32_t count, uint32_t stride );
    #pragma endregion

    /*LAVA_API
    inline bool isRecording( void ) const
    {
      return _state == State::Recording;
    }*/

    LAVA_API
    void fillBuffer( const std::shared_ptr<lava::Buffer>& dstBuffer,
      vk::DeviceSize dstOffset, vk::DeviceSize fillSize, uint32_t data );

    LAVA_API
    void blitImage( const std::shared_ptr<Image>& srcImage,
      vk::ImageLayout srcImageLayout, const std::shared_ptr<Image>& dstImage,
      vk::ImageLayout dstImageLayout,
      vk::ArrayProxy<const vk::ImageBlit> regions, vk::Filter filter );

    LAVA_API
    void reset( vk::CommandBufferResetFlagBits flags = { } );

    template <typename T> void pushConstants( vk::PipelineLayout layout,
      vk::ShaderStageFlags stageFlags, uint32_t start,
      vk::ArrayProxy<const T> values );

    #pragma region BindCommands
    LAVA_API
    void bindVertexBuffer( uint32_t startBinding,
      const std::shared_ptr<Buffer>& buffer, vk::DeviceSize offset );
    LAVA_API
    void bindIndexBuffer( const std::shared_ptr<Buffer>& buffer,
      vk::DeviceSize offset, vk::IndexType indexType );
    LAVA_API
    void bindIndexBuffer( const std::shared_ptr<IndexBuffer>& buffer,
      vk::DeviceSize offset = { } );
    LAVA_API
    void bindVertexBuffers( uint32_t startBinding,
      vk::ArrayProxy<const std::shared_ptr<Buffer>> buffers,
      vk::ArrayProxy<const vk::DeviceSize> offsets );
    LAVA_API
    void bindDescriptorSets( vk::PipelineBindPoint pipelineBindPoint,
      const std::shared_ptr<PipelineLayout>& pipelineLayout, uint32_t firstSet,
      vk::ArrayProxy<const std::shared_ptr<DescriptorSet>> descriptorSets,
      vk::ArrayProxy<const uint32_t> dynamicOffsets );
    LAVA_API
    void bindPipeline( vk::PipelineBindPoint bindingPoint,
      const std::shared_ptr<Pipeline>& pipeline );
    LAVA_API
    void bindGraphicsPipeline( const std::shared_ptr<Pipeline>& pipeline );
    LAVA_API
    void bindComputePipeline( const std::shared_ptr<Pipeline>& pipeline );
    #pragma endregion

    #pragma region CopyCommands
    LAVA_API
    void copyBuffer( const std::shared_ptr<Buffer>& srcBuffer,
      const std::shared_ptr<Buffer>& dstBuffer,
      vk::ArrayProxy<const vk::BufferCopy> regions );
    LAVA_API
    void copyBufferToImage( const std::shared_ptr<Buffer>& srcBuffer,
      const std::shared_ptr<Image>& dstImage, vk::ImageLayout dstImageLayout,
      vk::ArrayProxy<const vk::BufferImageCopy> regions );
    LAVA_API
    void copyImage( const std::shared_ptr<Image>& srcImage,
      vk::ImageLayout srcImageLayout, const std::shared_ptr<Image>& dstImage,
      vk::ImageLayout dstImageLayout, vk::ArrayProxy<const vk::ImageCopy> regions );
    LAVA_API
    void copyImageToBuffer( const std::shared_ptr<Image>& srcImage,
      vk::ImageLayout srcImageLayout, const std::shared_ptr<Buffer>& dstBuffer,
      vk::ArrayProxy<const vk::BufferImageCopy> regions );
    #pragma endregion

    template <typename T> void updateBuffer(
      const std::shared_ptr<Buffer>& destBuffer,
      vk::DeviceSize destOffset, vk::ArrayProxy<const T> data );

    LAVA_API
    void pipelineBarrier(
      vk::PipelineStageFlags srcStageMask, vk::PipelineStageFlags destStageMask,
      vk::DependencyFlags dependencyFlags,
      vk::ArrayProxy<const vk::MemoryBarrier> barriers,
      vk::ArrayProxy<const vk::BufferMemoryBarrier> bufferMemoryBarriers,
      vk::ArrayProxy<const ImageMemoryBarrier> imageMemoryBarriers
    );

    LAVA_API
    inline std::shared_ptr<lava::RenderPass> getRenderPass( void ) const
    {
      return _renderPass;
    }
    LAVA_API
    inline std::shared_ptr<lava::Framebuffer> getFramebuffer( void ) const
    {
      return _framebuffer;
    }
#if !defined(NDEBUG)
  private:
    void setPrimaryCommandBuffer(
      const std::shared_ptr<lava::CommandBuffer>& primaryCommandBuffer )
    {
      _primaryCommandBuffer = primaryCommandBuffer;
    }
#endif
  protected:
    friend class CommandPool;
    LAVA_API
    void onReset( void );
  private:
    std::shared_ptr< lava::CommandPool > _commandPool;
    //State _state;

    vk::CommandBuffer _commandBuffer;
    std::shared_ptr<RenderPass> _renderPass;
    std::shared_ptr<Framebuffer> _framebuffer;
    std::vector< std::shared_ptr< lava::CommandBuffer > > _secondaryCommandBuffers;

    std::vector<::vk::DescriptorSet> _bindDescriptorSets;
    std::vector<::vk::Buffer> _bindVertexBuffers;

#if !defined(NDEBUG)
    struct QueryInfo
    {
      bool active;
      bool contained;
      vk::QueryControlFlags flags;
    };
    vk::CommandBufferUsageFlags _flags;
    bool _inRenderPass;
    bool _isRecording;
    bool _isResetFromCommandPool;
    vk::CommandBufferLevel _level;
    vk::Bool32 _occlusionQueryEnable;
    std::weak_ptr<lava::CommandBuffer> _primaryCommandBuffer;
    QueryInfo _queryInfo[ VK_QUERY_TYPE_RANGE_SIZE ];
    vk::PipelineStageFlags _stageFlags;
#endif

  public:
    LAVA_API
    void pushDescriptorSetKHR( vk::PipelineBindPoint bindpoint, 
      std::shared_ptr<PipelineLayout> pipLayout, uint32_t set, 
      vk::ArrayProxy<WriteDescriptorSet> descriptorWrites );
  protected:
    PFN_vkCmdPushDescriptorSetKHR vkCmdPushDescriptorSetKHR = nullptr;
  };
  
  template<typename T>
  inline void CommandBuffer::pushConstants( vk::PipelineLayout layout, 
    vk::ShaderStageFlags stageFlags, uint32_t start, 
    vk::ArrayProxy<const T> values )
  {
    _commandBuffer.pushConstants<T>( layout, stageFlags, start, values );
  }
  template<typename T>
  inline void CommandBuffer::updateBuffer(
    const std::shared_ptr<Buffer>& destBuffer,
    vk::DeviceSize destOffset, vk::ArrayProxy<const T> data )
  {
    _commandBuffer.updateBuffer<T>( *destBuffer, destOffset, data );
  }
}

#endif /* __LAVA_COMMANDBUFFER__ */