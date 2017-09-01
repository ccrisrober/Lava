#ifndef __LAVA_COMMANDBUFFER__
#define __LAVA_COMMANDBUFFER__

#include "includes.hpp"
#include "Pipeline.h"

#include "VulkanResource.h"

#include <lava/api.h>

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
    LAVA_API
    CommandPool( const DeviceRef& device,
      vk::CommandPoolCreateFlags flags = vk::CommandPoolCreateFlags( ),
      uint32_t familyIndex = 0 );
    LAVA_API
    virtual ~CommandPool( );

    inline operator vk::CommandPool( ) const
    {
      return _commandPool;
    }
    const DeviceRef& getDevice( )
    {
      return _device;
    }
    LAVA_API
    std::shared_ptr<CommandBuffer> allocateCommandBuffer(
      vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary );
  protected:
    vk::CommandPool _commandPool;
    std::vector<CommandBuffer*> _commandBuffers;
  };
  class CommandBuffer
  {
  public:
    LAVA_API
    CommandBuffer( const std::shared_ptr<CommandPool>& cmdPool,
      vk::CommandBufferLevel level = vk::CommandBufferLevel::ePrimary );
    LAVA_API
    virtual ~CommandBuffer( void );

    inline operator vk::CommandBuffer( ) const
    {
      return _commandBuffer;
    }

    LAVA_API
    void clearAttachments( vk::ArrayProxy< const vk::ClearAttachment> attachments,
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

    LAVA_API
    void beginRenderPass( const std::shared_ptr<RenderPass>& renderPass,
      const std::shared_ptr<Framebuffer>& framebuffer, const vk::Rect2D& area,
      vk::ArrayProxy<const vk::ClearValue> clearValues, vk::SubpassContents contents );

    LAVA_API
    void bindDescriptorSets( vk::PipelineBindPoint pipelineBindPoint, 
      const std::shared_ptr<PipelineLayout>& pipelineLayout, 
      uint32_t firstSet, 
      vk::ArrayProxy<const std::shared_ptr<DescriptorSet>> descriptorSets, vk::ArrayProxy<const uint32_t> dynamicOffsets );


    LAVA_API
    void endRenderPass( );

    LAVA_API
    void setStencilCompareMask( vk::StencilFaceFlags faceMask, uint32_t stencilCompareMask );
    LAVA_API
    void setStencilReference( vk::StencilFaceFlags faceMask, uint32_t stencilReference );
    LAVA_API
    void setStencilWriteMask( vk::StencilFaceFlags faceMask, uint32_t stencilWriteMask );
    LAVA_API
    void setBlendConstants( const float blendConst[ 4 ] );

    LAVA_API
    void begin( vk::CommandBufferUsageFlags flags = vk::CommandBufferUsageFlags( ),
      const std::shared_ptr<RenderPass>& renderPass = std::shared_ptr<RenderPass>( ),
      uint32_t subpass = 0,
      const std::shared_ptr<Framebuffer>& framebuffer = std::shared_ptr<Framebuffer>( ),
      vk::Bool32 occlusionQueryEnable = false,
      vk::QueryControlFlags queryFlags = vk::QueryControlFlags( ),
      vk::QueryPipelineStatisticFlags pipelineStatistics = vk::QueryPipelineStatisticFlags( ) );
    LAVA_API
    void end( );

    inline bool isRecording( void ) const
    {
      return _isRecording;
    }

    LAVA_API
    std::shared_ptr<RenderPass> getRenderPass( void ) const
    {
      return _renderPass;
    }
    LAVA_API
    std::shared_ptr<Framebuffer> getFramebuffer( void ) const
    {
      return _framebuffer;
    }

    template <typename T> void pushConstants( vk::PipelineLayout layout,
      vk::ShaderStageFlags stageFlags, uint32_t start, vk::ArrayProxy<const T> values );
    LAVA_API
    void bindPipeline( vk::PipelineBindPoint bindingPoint, 
      const std::shared_ptr<Pipeline>& pipeline );
    
    /*void bindDescriptorSets( vk::PipelineBindPoint pipelineBindPoint, 
      std::shared_ptr<PipelineLayout> const& pipelineLayout, uint32_t firstSet, 
      vk::ArrayProxy<const std::shared_ptr<DescriptorSet>> descriptorSets, 
      vk::ArrayProxy<const uint32_t> dynamicOffsets );*/

    LAVA_API
    void setScissor( uint32_t first, vk::ArrayProxy<const vk::Rect2D> scissors );
    LAVA_API
    void setViewport( uint32_t first, vk::ArrayProxy<const vk::Viewport> viewports );
    LAVA_API
    void setLineWidth( float lineWidth );
    LAVA_API
    void dispatch( uint32_t x, uint32_t y, uint32_t z );
    LAVA_API
    void draw( uint32_t vertexCount, uint32_t instanceCount, uint32_t firstVertex,
      uint32_t firstInstance );
    LAVA_API
    void drawIndexed( uint32_t indexCount, uint32_t instanceCount, uint32_t firstIndex,
      int32_t vertexOffset, uint32_t firstInstance );

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

    LAVA_API
    void bindIndexBuffer( const std::shared_ptr<Buffer>& buffer, 
      vk::DeviceSize offset, vk::IndexType indexType );
    LAVA_API
    void bindVertexBuffer( uint32_t startBinding, const std::shared_ptr<Buffer>& buffer,
      vk::DeviceSize offset );
    LAVA_API
    void bindVertexBuffers( uint32_t startBinding, 
      vk::ArrayProxy<const std::shared_ptr<Buffer>> buffers,
      vk::ArrayProxy<const vk::DeviceSize> offsets );
    template <typename T> void updateBuffer( const std::shared_ptr<Buffer>& destBuffer,
      vk::DeviceSize destOffset, vk::ArrayProxy<const T> data );

  protected:
    std::shared_ptr<CommandPool> _commandPool;
    vk::CommandBuffer _commandBuffer;
    std::shared_ptr<RenderPass> _renderPass;
    std::shared_ptr<Framebuffer> _framebuffer;
    bool _isRecording;
    std::vector<::vk::DescriptorSet> _bindDescriptorSets;
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