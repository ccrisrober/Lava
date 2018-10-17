/**
 * Copyright (c) 2017 - 2018, Pompeii
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

#ifndef __POMPEII_COMMANDBUFFER__
#define __POMPEII_COMMANDBUFFER__

#include <pompeii/Device.h>
#include <pompeii/VulkanResource.h>
#include <memory>

namespace pompeii
{
  class Buffer;
  class IndexBuffer;
  class Image;
  class QueryPool;
  class RenderPass;
  class Framebuffer;
}

namespace pompeii
{
  struct ImageMemoryBarrier
  {
    POMPEII_API
    ImageMemoryBarrier( void ) { };
    POMPEII_API
    ImageMemoryBarrier(
      vk::AccessFlags srcAccessMask, vk::AccessFlags dstAccessMask,
      vk::ImageLayout oldLayout, vk::ImageLayout newLayout,
      uint32_t srcQueueFamilyIndex, uint32_t dstQueueFamilyIndex,
      const std::shared_ptr<Image>& image,
      const vk::ImageSubresourceRange& subresourceRange );
    POMPEII_API
    ImageMemoryBarrier( const ImageMemoryBarrier& rhs );
    POMPEII_API
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
    POMPEII_API
    CommandPool( const std::shared_ptr<Device>& device, 
      vk::CommandPoolCreateFlags flags = { }, uint32_t familyIndex = 0 );
    POMPEII_API
    virtual ~CommandPool( void );
    POMPEII_API
    void reset( vk::CommandPoolResetFlags flags );
#if !defined(NDEBUG)
    POMPEII_API
    uint32_t getFamilyIndex( void ) const;
    POMPEII_API
    bool individuallyResetCommandBuffers( void ) const;
    POMPEII_API
    bool shortLivedCommandBuffers( void ) const;
    POMPEII_API
    bool supportsCompute( void ) const;
    POMPEII_API
    bool supportsGraphics( void ) const;
    POMPEII_API
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
    POMPEII_API
    std::shared_ptr<CommandBuffer> allocateCommandBuffer(
      vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary );
    POMPEII_API
    std::vector< std::shared_ptr<CommandBuffer > > allocateCommandBuffers(
      uint32_t count, vk::CommandBufferLevel level );
  protected:
    vk::CommandPool _commandPool;
#if !defined(NDEBUG)
    std::vector<std::shared_ptr<CommandBuffer>> _commandBuffers;
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
    friend class CommandPool;
  private:
	CommandBuffer(const vk::CommandBuffer& cmd, 
      const std::shared_ptr<CommandPool>& cmdPool,
	  vk::CommandBufferLevel level );
  public:
    POMPEII_API
    CommandBuffer( const std::shared_ptr<CommandPool>& cmdPool,
      vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary );
    POMPEII_API
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
    POMPEII_API
    std::shared_ptr< pompeii::CommandBuffer > getPrimaryCommandBuffer( void ) const
    {
      return _primaryCommandBuffer.lock( );
    }
    POMPEII_API
    bool isOneTimeSubmit( void ) const
    {
      return !!( _flags & vk::CommandBufferUsageFlagBits::eOneTimeSubmit );
    }
    POMPEII_API
    bool isRecording( void ) const
    {
      return _isRecording;
    }
    POMPEII_API
    bool isSimultaneousUsageAllowed( void ) const
    {
      return !!( _flags & vk::CommandBufferUsageFlagBits::eSimultaneousUse );
    }
#endif

    POMPEII_API
    void begin( vk::CommandBufferUsageFlags flags = { },
      const std::shared_ptr<RenderPass>& renderPass = { },
      uint32_t subpass = 0,
      const std::shared_ptr<Framebuffer>& framebuffer = { },
      vk::Bool32 occlusionQueryEnable = false,
      vk::QueryControlFlags queryFlags = { },
      vk::QueryPipelineStatisticFlags pipelineStatistics = { } );
    POMPEII_API
    void end( void );

    #pragma region EventCommands
    POMPEII_API
    void resetEvent( const std::shared_ptr<Event>& ev,
      vk::PipelineStageFlags stageMask );
    POMPEII_API
    void setEvent( const std::shared_ptr<Event>& ev,
      vk::PipelineStageFlags stageMask );
    POMPEII_API
    void waitEvents( vk::ArrayProxy<const std::shared_ptr<Event>> events,
      vk::PipelineStageFlags srcStageMask, vk::PipelineStageFlags dstStageMask,
      vk::ArrayProxy<const vk::MemoryBarrier> memoryBarriers,
      vk::ArrayProxy<const vk::BufferMemoryBarrier> bufferMemoryBarriers,
      vk::ArrayProxy<const vk::ImageMemoryBarrier> imageMemoryBarriers
    );
    #pragma endregion

    POMPEII_API
    void beginRenderPass( const vk::RenderPassBeginInfo& beginInfo, 
      vk::SubpassContents contents );
    POMPEII_API
    void beginRenderPass( const std::shared_ptr<RenderPass>& renderPass,
      const std::shared_ptr<Framebuffer>& framebuffer, const vk::Rect2D& area,
      vk::ArrayProxy<const vk::ClearValue> clearValues,
      vk::SubpassContents contents );

    POMPEII_API
    void nextSubpass( vk::SubpassContents contents );

    POMPEII_API
    void endRenderPass( void );

    POMPEII_API
    void executeCommands( const std::vector<
      std::shared_ptr<pompeii::CommandBuffer> >& secondaryCmds );

    #pragma region ClearCommands
    POMPEII_API
    void clearAttachments(
      vk::ArrayProxy< const vk::ClearAttachment> attachments,
      vk::ArrayProxy<const vk::ClearRect> rects );
    POMPEII_API
    void clearColorImage( const std::shared_ptr<Image>& image,
      vk::ImageLayout imageLayout, const vk::ClearColorValue& color,
      vk::ArrayProxy<const vk::ImageSubresourceRange> ranges = {
        { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
    } );
    POMPEII_API
    void clearDepthStencilImage( const std::shared_ptr<Image>& image,
      vk::ImageLayout imageLayout, float depth, uint32_t stencil,
      vk::ArrayProxy<const vk::ImageSubresourceRange> ranges );
    POMPEII_API
    void resolveImage( const std::shared_ptr<Image>& srcImage, 
      const std::shared_ptr<Image>& dstImage,
      const vk::ImageResolve& region );
    #pragma endregion

    #pragma region QueryCommands
    POMPEII_API
    void beginQuery( const std::shared_ptr<pompeii::QueryPool>& queryPool,
      uint32_t slot, vk::QueryControlFlags flags );
    POMPEII_API
    void beginQuery( const std::shared_ptr<pompeii::QueryPool>& queryPool,
      uint32_t slot, bool precise );
    POMPEII_API
    void copyQueryPoolResults( const std::shared_ptr<pompeii::QueryPool>& queryPool,
      uint32_t startQuery, uint32_t queryCount,
      const std::shared_ptr<pompeii::Buffer>& dstBuffer, vk::DeviceSize dstOffset,
      vk::DeviceSize dstStride, vk::QueryResultFlags flags );
    POMPEII_API
    void endQuery( const std::shared_ptr<pompeii::QueryPool>& queryPool,
      uint32_t slot );
    POMPEII_API
    void resetQueryPool( const std::shared_ptr<pompeii::QueryPool>& queryPool,
      uint32_t startQuery, uint32_t queryCount );
    POMPEII_API
    void writeTimestamp( vk::PipelineStageFlagBits pipelineStage,
      const std::shared_ptr<pompeii::QueryPool>& queryPool, uint32_t entry );  
    #pragma endregion

    #pragma region SetterCommands
    POMPEII_API
    void setViewportScissors( uint32_t width, uint32_t height );
    POMPEII_API
    void setViewportScissors( const vk::Extent2D& dimensions );
    POMPEII_API
    void setScissor( uint32_t first,
      vk::ArrayProxy<const vk::Rect2D> scissors );
    POMPEII_API
    void setScissor( const vk::Rect2D scissor );
    POMPEII_API
    void setViewport( uint32_t first,
      vk::ArrayProxy<const vk::Viewport> viewports );
    POMPEII_API
    void setViewport( const vk::Viewport viewport );
    POMPEII_API
    void setDepthBias( float depthBias, float depthBiasClamp,
      float slopeScaledDepthBias );
    POMPEII_API
    void setDepthBounds( float minDepthBounds, float maxDepthBounds );

    POMPEII_API
    void setLineWidth( float lineWidth );
    POMPEII_API
    void setBlendConstants( const float blendConst[ 4 ] );
    POMPEII_API
    void setStencilCompareMask( vk::StencilFaceFlags faceMask,
      uint32_t stencilCompareMask );
    POMPEII_API
    void setStencilReference( vk::StencilFaceFlags faceMask,
      uint32_t stencilReference );
    POMPEII_API
    void setStencilWriteMask( vk::StencilFaceFlags faceMask,
      uint32_t stecilWriteMask );
    #pragma endregion

    #pragma region DrawDispatchCommands
    POMPEII_API
    void dispatch( uint32_t x, uint32_t y, uint32_t z );
    POMPEII_API
    void draw( uint32_t vertexCount, uint32_t instanceCount,
      uint32_t firstVertex, uint32_t firstInstance );
    POMPEII_API
    void drawIndirect( const std::shared_ptr<Buffer>& buffer,
      vk::DeviceSize offset, uint32_t count, uint32_t stride );
    POMPEII_API
    void drawIndexed( uint32_t indexCount, uint32_t instanceCount,
      uint32_t firstIndex, int32_t vertexOffset, uint32_t firstInstance );
    POMPEII_API
    void drawIndexedIndirect( const std::shared_ptr<Buffer>& buffer,
      vk::DeviceSize offset, uint32_t count, uint32_t stride );

    POMPEII_API
    void draw( int vertexCount );
    POMPEII_API
    void drawIndexed( int indexCount );
    #pragma endregion

    /*POMPEII_API
    inline bool isRecording( void ) const
    {
      return _state == State::Recording;
    }*/

    POMPEII_API
    void fillBuffer( const std::shared_ptr<pompeii::Buffer>& dstBuffer,
      vk::DeviceSize dstOffset, vk::DeviceSize fillSize, uint32_t data );

    POMPEII_API
    void blitImage( const std::shared_ptr<Image>& srcImage,
      vk::ImageLayout srcImageLayout, const std::shared_ptr<Image>& dstImage,
      vk::ImageLayout dstImageLayout,
      vk::ArrayProxy<const vk::ImageBlit> regions, vk::Filter filter );

    POMPEII_API
    void reset( vk::CommandBufferResetFlagBits flags = { } );

    POMPEII_API
    void pushConstant( const std::shared_ptr<PipelineLayout>& layout, 
      vk::ShaderStageFlagBits stageFlags, void* data );

    template <typename T>
    void pushConstants( const std::shared_ptr<PipelineLayout>& layout,
      vk::ShaderStageFlags stageFlags, uint32_t start,
      vk::ArrayProxy<const T> values );

    #pragma region BindCommands
    POMPEII_API
    void bindVertexBuffer( uint32_t startBinding,
      const std::shared_ptr<Buffer>& buffer, vk::DeviceSize offset = 0 );
    POMPEII_API
    void bindIndexBuffer( const std::shared_ptr<Buffer>& buffer,
      vk::DeviceSize offset, vk::IndexType indexType );
    POMPEII_API
    void bindIndexBuffer( const std::shared_ptr<IndexBuffer>& buffer,
      vk::DeviceSize offset = 0 );
    POMPEII_API
    void bindVertexBuffers( uint32_t startBinding,
      vk::ArrayProxy<const std::shared_ptr<Buffer>> buffers,
      vk::ArrayProxy<const vk::DeviceSize> offsets = { } );
    POMPEII_API
    void bindDescriptorSets( vk::PipelineBindPoint pipelineBindPoint,
      const std::shared_ptr<PipelineLayout>& pipelineLayout, uint32_t firstSet,
      vk::ArrayProxy<const std::shared_ptr<DescriptorSet>> descriptorSets,
      vk::ArrayProxy<const uint32_t> dynamicOffsets = {});
    POMPEII_API
    void bindPipeline( vk::PipelineBindPoint bindingPoint,
      const std::shared_ptr<Pipeline>& pipeline );
    POMPEII_API
    void bindGraphicsPipeline( const std::shared_ptr<Pipeline>& pipeline );
    POMPEII_API
    void bindComputePipeline( const std::shared_ptr<Pipeline>& pipeline );
    #pragma endregion

    #pragma region CopyCommands
    POMPEII_API
    void copyBuffer( const std::shared_ptr<Buffer>& srcBuffer,
      const std::shared_ptr<Buffer>& dstBuffer,
      vk::ArrayProxy<const vk::BufferCopy> regions );
    POMPEII_API
    void copyBufferToImage( const std::shared_ptr<Buffer>& srcBuffer,
      const std::shared_ptr<Image>& dstImage, vk::ImageLayout dstImageLayout,
      vk::ArrayProxy<const vk::BufferImageCopy> regions );
    POMPEII_API
    void copyImage( const std::shared_ptr<Image>& srcImage,
      vk::ImageLayout srcImageLayout, const std::shared_ptr<Image>& dstImage,
      vk::ImageLayout dstImageLayout, vk::ArrayProxy<const vk::ImageCopy> regions );
    POMPEII_API
    void copyImageToBuffer( const std::shared_ptr<Image>& srcImage,
      vk::ImageLayout srcImageLayout, const std::shared_ptr<Buffer>& dstBuffer,
      vk::ArrayProxy<const vk::BufferImageCopy> regions );
    #pragma endregion

    template <typename T> void updateBuffer(
      const std::shared_ptr<Buffer>& destBuffer,
      vk::DeviceSize destOffset, vk::ArrayProxy<const T> data );

    POMPEII_API
    void pipelineBarrier(
      vk::PipelineStageFlags srcStageMask, vk::PipelineStageFlags destStageMask,
      vk::DependencyFlags dependencyFlags,
      vk::ArrayProxy<const vk::MemoryBarrier> barriers,
      vk::ArrayProxy<const vk::BufferMemoryBarrier> bufferMemoryBarriers,
      vk::ArrayProxy<const ImageMemoryBarrier> imageMemoryBarriers
    );

    POMPEII_API
    void pipelineBarrier(
      vk::PipelineStageFlags srcStageMask, vk::PipelineStageFlags destStageMask,
      const vk::ArrayProxy< std::shared_ptr< Buffer > >& buffers, 
      const vk::BufferMemoryBarrier& barrier, vk::DependencyFlags dependencyFlags );

    POMPEII_API
    void pipelineBarrier(
      vk::PipelineStageFlags srcStageMask, vk::PipelineStageFlags destStageMask,
      const vk::ArrayProxy< std::shared_ptr< Image > >& images, 
      const ImageMemoryBarrier& barrier, vk::DependencyFlags dependencyFlags );

    POMPEII_API
    inline std::shared_ptr<pompeii::RenderPass> getRenderPass( void ) const
    {
      return _renderPass;
    }
    POMPEII_API
    inline std::shared_ptr<pompeii::Framebuffer> getFramebuffer( void ) const
    {
      return _framebuffer;
    }
#if !defined(NDEBUG)
  private:
    void setPrimaryCommandBuffer(
      const std::shared_ptr<pompeii::CommandBuffer>& primaryCommandBuffer )
    {
      _primaryCommandBuffer = primaryCommandBuffer;
    }
#endif
  protected:
    friend class CommandPool;
    POMPEII_API
    void onReset( void );
  private:
    std::shared_ptr< pompeii::CommandPool > _commandPool;
    //State _state;

    vk::CommandBuffer _commandBuffer;
    std::shared_ptr<RenderPass> _renderPass;
    std::shared_ptr<Framebuffer> _framebuffer;
    std::vector< std::shared_ptr< pompeii::CommandBuffer > > _secondaryCommandBuffers;

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
    std::weak_ptr<pompeii::CommandBuffer> _primaryCommandBuffer;
    QueryInfo _queryInfo[ VK_QUERY_TYPE_RANGE_SIZE ];
    vk::PipelineStageFlags _stageFlags;
#endif

  public:
    /*POMPEII_API
    void pushDescriptorSetKHR( vk::PipelineBindPoint bindpoint, 
      std::shared_ptr<PipelineLayout> pipLayout, uint32_t set, 
      vk::ArrayProxy<WriteDescriptorSet> descriptorWrites );
  //protected:
  //  PFN_vkCmdPushDescriptorSetKHR vkCmdPushDescriptorSetKHR = nullptr;*/
  };
  
  template<typename T>
  inline void CommandBuffer::pushConstants( 
    const std::shared_ptr<PipelineLayout>& layout,
    vk::ShaderStageFlags stageFlags, uint32_t start, 
    vk::ArrayProxy<const T> values )
  {
    _commandBuffer.pushConstants<T>( *layout, stageFlags, start, values );
  }
  template<typename T>
  inline void CommandBuffer::updateBuffer(
    const std::shared_ptr<Buffer>& destBuffer,
    vk::DeviceSize destOffset, vk::ArrayProxy<const T> data )
  {
    _commandBuffer.updateBuffer<T>( *destBuffer, destOffset, data );
  }
}

#endif /* __POMPEII_COMMANDBUFFER__ */