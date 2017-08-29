#ifndef __LAVA_COMMANDBUFFER__
#define __LAVA_COMMANDBUFFER__

#include "includes.hpp"
#include "Pipeline.h"

#include "VulkanResource.h"

namespace lava
{
  class Device;
  class Buffer;
  class Image;
  class RenderPass;
  class Framebuffer;
  class CommandBuffer;
  class CommandPool : public VulkanResource, public std::enable_shared_from_this<CommandPool>
  {
  public:
    CommandPool( const DeviceRef& device,
      vk::CommandPoolCreateFlags flags = vk::CommandPoolCreateFlags( ),
      uint32_t familyIndex = 0 );
    virtual ~CommandPool( );

    inline operator vk::CommandPool( ) const
    {
      return _commandPool;
    }
    const DeviceRef& getDevice( )
    {
      return _device;
    }
    std::shared_ptr<CommandBuffer> allocateCommandBuffer(
      vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary );
  protected:
    vk::CommandPool _commandPool;
    std::vector<CommandBuffer*> _commandBuffers;
  };
  class CommandBuffer
  {
  public:
    CommandBuffer( const std::shared_ptr<CommandPool>& cmdPool,
      vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary );
    virtual ~CommandBuffer( void );

    inline operator vk::CommandBuffer( ) const
    {
      return _commandBuffer;
    }

    void clearAttachments( vk::ArrayProxy< const vk::ClearAttachment> attachments,
      vk::ArrayProxy<const vk::ClearRect> rects );
    void clearColorImage( const std::shared_ptr<Image>& image,
      vk::ImageLayout imageLayout, const vk::ClearColorValue& color,
      vk::ArrayProxy<const vk::ImageSubresourceRange> ranges = {
        { vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1 }
    } );
    void clearDepthStencilImage( const std::shared_ptr<Image>& image,
      vk::ImageLayout imageLayout, float depth, uint32_t stencil,
      vk::ArrayProxy<const vk::ImageSubresourceRange> ranges );

    void beginRenderPass( const std::shared_ptr<RenderPass>& renderPass,
      const std::shared_ptr<Framebuffer>& framebuffer, const vk::Rect2D& area,
      vk::ArrayProxy<const vk::ClearValue> clearValues, vk::SubpassContents contents );
    void endRenderPass( );

    void setStencilCompareMask( vk::StencilFaceFlags faceMask, uint32_t stencilCompareMask );
    void setStencilReference( vk::StencilFaceFlags faceMask, uint32_t stencilReference );
    void setStencilWriteMask( vk::StencilFaceFlags faceMask, uint32_t stencilWriteMask );

    void begin( vk::CommandBufferUsageFlags flags = vk::CommandBufferUsageFlags( ),
      const std::shared_ptr<RenderPass>& renderPass = std::shared_ptr<RenderPass>( ),
      uint32_t subpass = 0,
      const std::shared_ptr<Framebuffer>& framebuffer = std::shared_ptr<Framebuffer>( ),
      vk::Bool32 occlusionQueryEnable = false,
      vk::QueryControlFlags queryFlags = vk::QueryControlFlags( ),
      vk::QueryPipelineStatisticFlags pipelineStatistics = vk::QueryPipelineStatisticFlags( ) );
    void end( );

    inline bool isRecording( void ) const
    {
      return _isRecording;
    }
    template <typename T> void pushConstants( vk::PipelineLayout layout,
      vk::ShaderStageFlags stageFlags, uint32_t start, vk::ArrayProxy<const T> values );
    void bindPipeline( vk::PipelineBindPoint bindingPoint, const std::shared_ptr<Pipeline>& pipeline );

    void setScissor( uint32_t first, vk::ArrayProxy<const vk::Rect2D> scissors );
    void setViewport( uint32_t first, vk::ArrayProxy<const vk::Viewport> viewports );
    void setLineWidth( float lineWidth );
    void dispatch( uint32_t x, uint32_t y, uint32_t z );
    void draw( uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
      uint32_t firstInstance );
    void drawIndexed( uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex,
      int32_t vertexOffset, uint32_t firstInstance );

    void bindIndexBuffer( const std::shared_ptr<Buffer>& buffer, vk::DeviceSize offset, vk::IndexType indexType );
    void bindVertexBuffer( uint32_t startBinding, const std::shared_ptr<Buffer>& buffer,
      vk::DeviceSize offset );
    void bindVertexBuffers( uint32_t startBinding, vk::ArrayProxy<const std::shared_ptr<Buffer>> buffers,
      vk::ArrayProxy<const vk::DeviceSize> offsets );
    template <typename T> void updateBuffer( const std::shared_ptr<Buffer>& destBuffer,
      vk::DeviceSize destOffset, vk::ArrayProxy<const T> data );

  protected:
    std::shared_ptr<CommandPool> _commandPool;
    vk::CommandBuffer _commandBuffer;
    std::shared_ptr<RenderPass> _renderPass;
    std::shared_ptr<Framebuffer> _framebuffer;
    bool _isRecording;
    std::vector<::vk::Buffer> _bindVertexBuffers;
  };
  template<typename T>
  inline void CommandBuffer::pushConstants( vk::PipelineLayout layout,
    vk::ShaderStageFlags stageFlags, uint32_t start, vk::ArrayProxy<const T> values )
  {
    _commandBuffer.pushConstants<T>( layout, stageFlags, start, values );
  }
  template<typename T>
  inline void CommandBuffer::updateBuffer( const std::shared_ptr<Buffer>& destBuffer,
    vk::DeviceSize destOffset, vk::ArrayProxy<const T> data )
  {
    _commandBuffer.updateBuffer<T>( *destBuffer, destOffset, data );
  }
}

#endif /* __LAVA_COMMANDBUFFER__ */